#include "zigbee.h"
#include <string.h>
#include <initializer_list>

namespace {
constexpr uint32_t delivery_timeout = 10000;
uint64_t big_endian(const uint8_t* bytes, size_t n) {
  uint64_t value = 0;
  while (n--) value = (value << 8) | *bytes++;
  return value;
}
void encode(uint64_t value, uint8_t* bytes, size_t n) {
  while (n) { bytes[--n] = uint8_t(value); value >>= 8; }
}
bool unicast(uint64_t a) { return a > 0xffff && a != UINT64_MAX; }
}

bool HeptaZigbee::fail(const char* reason) { error_ = reason; return false; }

uint8_t HeptaZigbee::allocate_id() {
  for (unsigned i = 0; i < 255; ++i) {
    if (++next_id_ == 0) ++next_id_;
    if (tx_.state == XbeeTxResult::Pending && next_id_ == tx_.frame_id) continue;
    if (paired_tx_.state == XbeeTxResult::Pending && next_id_ == paired_tx_.frame_id) continue;
    if (!(retired_[next_id_ / 8] & (1u << (next_id_ % 8)))) return next_id_;
  }
  fail("frame IDs exhausted; reinitialize radio");
  return 0;
}

bool HeptaZigbee::write_frame(const uint8_t* body, size_t n) {
  uint8_t header[] = {0x7e, uint8_t(n >> 8), uint8_t(n)};
  uint8_t checksum = 0xff;
  for (size_t i = 0; i < n; ++i) checksum -= body[i];
  if (uart_.write(header, sizeof(header)) != sizeof(header)) return false;
  // Arduino-Pico SoftwareSerial has a 32-byte RX FIFO. Service it while a
  // longer outgoing frame is draining, before the peer can answer this frame.
  for (size_t offset = 0; offset < n; ) {
    size_t size = n - offset;
    if (size > 16) size = 16;
    if (uart_.write(body + offset, size) != size) return false;
    offset += size;
    poll();
  }
  return uart_.write(checksum) == 1;
}

void HeptaZigbee::poll() {
  if (phase_ && millis() - last_byte_ > 1000) phase_ = 0;
  while (uart_.available()) {
    int c = uart_.read();
    if (c < 0) break;
    parse(uint8_t(c));
    last_byte_ = millis();
  }
  if (tx_.state == XbeeTxResult::Pending && millis() - tx_time_ >= delivery_timeout) {
    retire(tx_.frame_id);
    tx_.state = XbeeTxResult::TimedOut;
  }
  if (paired_tx_.state == XbeeTxResult::Pending && millis() - paired_tx_time_ >= delivery_timeout) {
    retire(paired_tx_.frame_id);
    paired_tx_.state = XbeeTxResult::TimedOut;
  }
}

void HeptaZigbee::parse(uint8_t c) {
  switch (phase_) {
    case 0: if (c == 0x7e) phase_ = 1; break;
    case 1: length_ = uint16_t(c) << 8; phase_ = 2; break;
    case 2:
      length_ |= c;
      used_ = 0; sum_ = 0;
      phase_ = length_ && length_ <= sizeof(frame_) ? 3 : 0;
      break;
    case 3:
      frame_[used_++] = c;
      sum_ += c;
      if (used_ == length_) phase_ = 4;
      break;
    case 4:
      if (uint8_t(sum_ + c) == 0xff) dispatch();
      phase_ = 0;
      break;
  }
}

void HeptaZigbee::dispatch() {
  if (frame_[0] == 0x8b && length_ == 7) {
    for (XbeeTxResult* result : {&tx_, &paired_tx_}) {
      if (result->state == XbeeTxResult::Pending && frame_[1] == result->frame_id) {
        result->delivery = frame_[5];
        result->state = result->delivery == 0 ? XbeeTxResult::Delivered : XbeeTxResult::Failed;
        break;
      }
    }
  } else if (frame_[0] == 0x88 && length_ >= 5 && length_ <= 13 && waiting_id_ &&
             frame_[1] == waiting_id_ && memcmp(frame_ + 2, waiting_command_, 2) == 0) {
    at_value_ = big_endian(frame_ + 5, length_ - 5);
    at_status_ = frame_[4];
    waiting_id_ = 0;
  } else if (frame_[0] == 0x90 && length_ > 12) {
    uint64_t source = big_endian(frame_ + 1, 8);
    if (ping_peer_ && source == ping_peer_ && length_ == 12 + sizeof(probe_) &&
        memcmp(frame_ + 12,probe_,sizeof(probe_)) == 0) {
      echoed_ = true;
      return;
    }
    if (mode_ == XbeeReceive::Stream && source != paired_) return;
    if (count_ == 4) { ++dropped_; return; }
    XbeePacket& pkt = queue_[(front_ + count_) % 4];
    pkt.source = source;
    pkt.options = frame_[11];
    pkt.length = length_ - 12;
    memcpy(pkt.data, frame_ + 12, pkt.length);
    ++count_;
  }
}

bool HeptaZigbee::at(uint8_t type, const char* command, uint64_t value, size_t size) {
  poll();
  uint8_t id = allocate_id();
  if (!id) return false;
  uint8_t body[12] = {type, id, uint8_t(command[0]), uint8_t(command[1])};
  encode(value, body + 4, size);
  memcpy(waiting_command_, command, 2);
  waiting_id_ = id;
  at_status_ = 0xff;
  if (write_frame(body, 4 + size)) {
    uint32_t start = millis();
    while (waiting_id_ && millis() - start < 1200) { poll(); delay(1); }
  }
  if (waiting_id_) retire(id);
  waiting_id_ = 0;
  if (at_status_ != 0) return fail("local AT failed or timed out");
  return true;
}

bool HeptaZigbee::query(const char* name, uint64_t& value) {
  // Only numeric, read-only queries. A bare AC, WR or FR is an action.
  static const char* const names[] = {"AP","AO","HV","VR","SH","SL","DH","DL",
    "NP","ID","OP","OI","MY","SC","CH","CE","AI","EE","ZS","NJ","SM","DB"};
  for (auto key : names) {
    if (name && strcmp(name,key)==0) {
      if (!at(0x08,key,0,0)) return false;
      value = at_value_;
      error_ = "";
      return true;
    }
  }
  return fail("unsupported or failed AT query");
}

bool HeptaZigbee::initialize() {
  uint64_t hw, vr, ap, ao, high, low, np;
  if (!query("HV",hw) || (hw >> 8) != 0x2e ||
      !query("VR",vr) || (vr >> 12) != 4 || !query("AP",ap) || ap != 1 ||
      !query("AO",ao) || ao != 0) return fail("expected S2C Zigbee AP=1 AO=0");
  if (!query("DH",high) || !query("DL",low) || !query("NP",np)) return false;
  paired_ = (high << 32) | low;
  if (!unicast(paired_) || np == 0 || np > XbeePacket::capacity)
    return fail("invalid factory peer or payload limit");
  max_payload_ = size_t(np);
  ready_ = true;
  return true;
}

bool HeptaZigbee::apply_network(uint64_t pan, uint16_t mask, uint8_t coordinator) {
  return at(0x09,"ID",pan,8) && at(0x09,"SC",mask,2) &&
         at(0x09,"CE",coordinator,1) && at(0x08,"AC",0,0);
}

bool HeptaZigbee::configure_network(uint64_t pan, uint16_t mask, uint8_t coordinator) {
  uint64_t id, sc, ce;
  if (!ready_ || !mask || coordinator > 1) return fail("invalid network settings");
  while (result().state == XbeeTxResult::Pending ||
         paired_result().state == XbeeTxResult::Pending) delay(1);
  if (!query("ID",id) || !query("SC",sc) || !query("CE",ce)) return false;
  if (apply_network(pan,mask,coordinator)) return true;
  if (!apply_network(id,sc,ce)) return fail("network rollback unconfirmed");
  return fail("network change failed; previous settings applied");
}

bool HeptaZigbee::transmit(uint64_t peer, const uint8_t* data, size_t size,
                           XbeeTxResult& tx, uint32_t& started) {
  poll();
  if (!ready_ || !unicast(peer) || !data || !size || size > max_payload_) {
    fail("invalid unicast address or payload"); return 0;
  }
  if (tx.state == XbeeTxResult::Pending) return fail("transmit busy");
  uint8_t body[14 + XbeePacket::capacity] = {0x10, allocate_id()};
  if (!body[1]) return 0;
  encode(peer,body + 2,8);
  body[10] = 0xff; body[11] = 0xfe;
  memcpy(body + 14,data,size);
  if (!write_frame(body,14+size)) { fail("UART write failed"); return 0; }
  tx.frame_id = body[1];
  tx.delivery = 0xff;
  tx.state = XbeeTxResult::Pending;
  started = millis();
  error_ = "";
  return true;
}

uint8_t HeptaZigbee::send_to(uint64_t peer, const uint8_t* data, size_t size) {
  return transmit(peer, data, size, tx_, tx_time_) ? tx_.frame_id : 0;
}

XbeeTxResult HeptaZigbee::paired_result() { poll(); return paired_tx_; }
XbeeTxResult HeptaZigbee::result() { poll(); return tx_; }

bool HeptaZigbee::send(const uint8_t* data, size_t size) {
  if (!ready_ || !data || !size) return fail("invalid payload or API not ready");
  while (size) {
    const size_t n = size < max_payload_ ? size : max_payload_;
    while (paired_result().state == XbeeTxResult::Pending) delay(1);
    if (!transmit(paired_, data, n, paired_tx_, paired_tx_time_)) return false;
    size -= n; data += n;
  }
  return true;
}

void HeptaZigbee::consume() { front_ = (front_ + 1) % 4; --count_; cursor_ = 0; }

bool HeptaZigbee::receive_packet(XbeePacket& out) {
  poll();
  if (mode_ != XbeeReceive::Packets || !count_) return false;
  out = queue_[front_]; consume();
  return true;
}

int HeptaZigbee::peek_byte() {
  poll();
  return mode_ == XbeeReceive::Stream && count_ ? queue_[front_].data[cursor_] : -1;
}
int HeptaZigbee::read_byte() {
  int c = peek_byte();
  if (c >= 0 && ++cursor_ == queue_[front_].length) consume();
  return c;
}

bool HeptaZigbee::ping(uint64_t peer) {
  if (!ready_ || !unicast(peer) || result().state == XbeeTxResult::Pending)
    return fail("invalid peer or transmission pending");
  uint8_t body[20 + sizeof(probe_)] = {0x11, allocate_id()};
  if (!body[1]) return false;
  encode(peer, body + 2, 8);
  const uint8_t endpoint[] = {0xff,0xfe,0xe8,0xe8,0x00,0x12,0xc1,0x05,0,0};
  memcpy(body + 10, endpoint, sizeof(endpoint));
  encode(micros(), probe_ + 6, 4);
  memcpy(body + 20, probe_, sizeof(probe_));
  if (!write_frame(body, sizeof(body))) return fail("UART write failed");
  tx_.frame_id = body[1]; tx_.delivery = 0xff; tx_.state = XbeeTxResult::Pending;
  tx_time_ = millis();
  ping_peer_ = peer;
  echoed_ = false;
  do {
    poll();
    if (tx_.state == XbeeTxResult::Failed) break;
    if (echoed_ && tx_.state == XbeeTxResult::Delivered) break;
    delay(1);
  } while (millis() - tx_time_ < delivery_timeout);
  ping_peer_ = 0;
  poll();
  if (!echoed_ || tx_.state != XbeeTxResult::Delivered) return fail("loopback failed");
  error_ = "";
  return true;
}
