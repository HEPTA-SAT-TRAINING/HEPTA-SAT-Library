#pragma once
#include <Arduino.h>

enum XbeeMode { XBEE_MODE_API };
enum class XbeeReceive { Packets, Stream };
struct XbeePacket {
  static constexpr size_t capacity = 255;
  uint64_t source;
  uint8_t options;
  size_t length;
  uint8_t data[capacity];
};

struct XbeeTxResult {
  enum State { Rejected, Pending, Delivered, Failed, TimedOut };
  State state = Rejected;
  uint8_t frame_id = 0;
  uint8_t delivery = 0xff;
};

// One outstanding unicast. All UART reads pass through poll(), including
// while waiting for a local AT response, so unrelated packets stay queued.
class HeptaZigbee {
public:
  HeptaZigbee(Stream& uart, XbeeReceive mode) : uart_(uart), mode_(mode) {}
  bool initialize();
  bool send(const uint8_t* data, size_t size);
  uint8_t send_to(uint64_t peer, const uint8_t* data, size_t size);
  XbeeTxResult result();
  XbeeTxResult paired_result();
  bool ping(uint64_t peer);
  bool receive_packet(XbeePacket& out);
  int peek_byte();
  int read_byte();
  bool query(const char* name, uint64_t& value);
  bool configure_network(uint64_t pan, uint16_t mask, uint8_t coordinator);
  uint32_t dropped() const { return dropped_; }
  const char* error() const { return error_; }
  uint64_t paired() const { return paired_; }

private:
  Stream& uart_;
  XbeeReceive mode_;
  bool ready_ = false;
  uint64_t paired_ = 0;
  size_t max_payload_ = 0;
  const char* error_ = "";
  XbeeTxResult tx_;
  XbeeTxResult paired_tx_;
  uint32_t paired_tx_time_ = 0;
  uint8_t next_id_ = 0;
  uint8_t retired_[32] = {};
  uint64_t ping_peer_ = 0;
  uint8_t probe_[10] = {'H','E','P','T','A','?'};
  bool echoed_ = false;
  void retire(uint8_t id) { retired_[id / 8] |= 1u << (id % 8); }
  uint32_t tx_time_ = 0;
  uint8_t waiting_id_ = 0;
  char waiting_command_[2] = {};
  uint8_t at_status_ = 0xff;
  uint64_t at_value_ = 0;
  uint8_t frame_[12 + XbeePacket::capacity];
  size_t used_ = 0;
  uint16_t length_ = 0;
  uint8_t phase_ = 0, sum_ = 0;
  uint32_t last_byte_ = 0;
  XbeePacket queue_[4] = {};
  size_t front_ = 0, count_ = 0, cursor_ = 0;
  uint32_t dropped_ = 0;
  bool transmit(uint64_t peer, const uint8_t* data, size_t size, XbeeTxResult& tx, uint32_t& started);
  uint8_t allocate_id();
  bool fail(const char* reason);
  void consume();
  bool write_frame(const uint8_t* body, size_t length);
  bool ping_once(uint64_t peer);
  void poll();
  void parse(uint8_t byte);
  void dispatch();
  bool at(uint8_t type, const char* name, uint64_t value, size_t size);
  bool apply_network(uint64_t pan, uint16_t mask, uint8_t coordinator);
};
