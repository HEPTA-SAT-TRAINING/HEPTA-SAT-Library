#ifdef HEPTA_ZIGBEE_TEST
#include "../../drv/zigbee.h"
#include <cassert>
#include <deque>
#include <string>
#include <vector>
#include <map>
#include <cstdio>
#include <initializer_list>

static uint32_t ticks;
uint32_t millis() { return ticks; }
uint32_t micros() { return ticks * 1000; }
void delay(uint32_t ms) { ticks += ms; }
using Bytes = std::vector<uint8_t>;
const uint64_t ground = 0x0013a20012345678ULL;
const uint64_t other = ground + 1;
static void append64(Bytes& b, uint64_t n) {
  for (int i=7;i>=0;--i) b.push_back(uint8_t(n>>(8*i)));
}
class Port : public Stream {
public:
  std::deque<uint8_t> rx;
  Bytes partial;
  std::vector<Bytes> tx;
  std::map<std::string,uint64_t> regs{{"HV",0x2e4e},{"VR",0x4061},{"AP",1},{"AO",0},{"NP",84},
    {"DH",ground>>32},{"DL",uint32_t(ground)},{"ID",32},{"SC",0x2000},{"CE",0}};
  bool reply=true;
  std::string reject;
  int available() override { return int(rx.size()); }
  int read() override { if(rx.empty()) return -1; auto b=rx.front(); rx.pop_front(); return b; }
  Bytes wire(const Bytes& b) {
    Bytes f{0x7e,uint8_t(b.size()>>8),uint8_t(b.size())};
    uint8_t cs=255;
    for(auto c:b) { cs-=c; f.push_back(c); }
    f.push_back(cs); return f;
  }
  void emit(const Bytes& b) { auto f=wire(b); rx.insert(rx.end(),f.begin(),f.end()); }
  void packet(uint64_t sender,Bytes payload) {
    Bytes b{0x90}; append64(b,sender); b.insert(b.end(),{0xff,0xfe,1});
    b.insert(b.end(),payload.begin(),payload.end()); emit(b);
  }
  void status(uint8_t id,uint8_t code) { emit({0x8b,id,0xff,0xfe,0,code,0}); }
  size_t write(const uint8_t* data,size_t n) override {
    partial.insert(partial.end(),data,data+n);
    if(partial.size()<4 || partial.size()<size_t(partial[1]*256+partial[2]+4)) return n;
    Bytes b(partial.begin()+3,partial.end()-1);
    assert(wire(b)==partial); partial.clear(); tx.push_back(b);
    if(b[0]==8 || b[0]==9) {
      std::string k(b.begin()+2,b.begin()+4);
      Bytes res{0x88,b[1],b[2],b[3],0};
      if(k==reject && b[0]==9) { res[4]=3; reject.clear(); }
      else if(b.size()>4) {
        uint64_t v=0; for(size_t i=4;i<b.size();++i) v=(v<<8)|b[i]; regs[k]=v;
      } else if(k!="AC") append64(res,regs.at(k));
      emit(res);
    } else if(reply) {
      status(b[1],0);
      if(b[0]==0x11) packet(other,Bytes(b.begin()+20,b.end()));
    }
    return n;
  }
};
int main() {
  Port p;
  HeptaZigbee radio(p,XbeeReceive::Packets);
  assert(radio.initialize() && radio.paired()==ground);
  const Bytes special{0,0x7e,0x7d,0x11,0x13,255};
  p.reply=false;
  auto id=radio.send_to(other,special.data(),special.size()); assert(id);
  auto request=p.tx.back();
  assert(Bytes(request.begin()+14,request.end())==special);
  Bytes address; append64(address,other);
  assert(Bytes(request.begin()+2,request.begin()+10)==address);
  assert(!radio.send_to(other,special.data(),1));
  p.packet(other,special);
  p.status(id+1,0);
  assert(radio.result().state==XbeeTxResult::Pending);
  XbeePacket packet;
  assert(radio.receive_packet(packet) && packet.source==other);
  assert(Bytes(packet.data,packet.data+packet.length)==special);
  p.status(id,0x24); assert(radio.result().delivery==0x24);
  assert(!radio.send_to(0xffff,special.data(),1));
  assert(!radio.send_to(other,special.data(),85));
  id=radio.send_to(other,special.data(),1); ticks+=10001;
  assert(radio.result().state==XbeeTxResult::TimedOut);
  auto next=radio.send_to(other,special.data(),1); assert(next && next!=id);
  p.status(id,0); assert(radio.result().state==XbeeTxResult::Pending);
  p.status(next,0); assert(radio.result().state==XbeeTxResult::Delivered);
  p.packet(other,{9}); p.rx.back()^=1;
  assert(!radio.receive_packet(packet));
  p.packet(other,special);
  auto tail=p.rx;
  p.rx.clear();
  for (size_t i=0;i<5;++i) { p.rx.push_back(tail.front()); tail.pop_front(); }
  assert(!radio.receive_packet(packet));
  p.rx=tail;
  assert(radio.receive_packet(packet) && packet.length==special.size());
  p.rx={0x7e,0,28,0x90};
  assert(!radio.receive_packet(packet));
  ticks+=1500;
  p.emit({0x91,42});
  p.packet(other,{8});
  assert(radio.receive_packet(packet) && packet.length==1 && packet.data[0]==8);
  for(int i=0;i<5;++i) p.packet(other,{uint8_t(i)});
  for(int i=0;i<4;++i) assert(radio.receive_packet(packet) && packet.data[0]==i);
  assert(!radio.receive_packet(packet) && radio.dropped()==1);
  p.packet(other,special);
  uint64_t vr=0;
  assert(radio.query("VR",vr) && vr==0x4061 && radio.receive_packet(packet));
  assert(!radio.query("WR",vr));
  p.reject="SC";
  assert(!radio.configure_network(48,1,0) && p.regs["ID"]==32 && p.regs["SC"]==0x2000);
  assert(radio.configure_network(48,1,0) && p.regs["ID"]==48);
  p.reply=true;
  assert(radio.ping(other));
  HeptaZigbee stream(p,XbeeReceive::Stream);
  assert(stream.initialize());
  p.packet(other,{1}); p.packet(ground,special);
  for(auto c:special) assert(stream.read_byte()==c);
  assert(stream.read_byte()==-1);
  Bytes big(180,0x7e); size_t start=p.tx.size();
  assert(stream.send(big.data(),big.size()) && p.tx.size()==start+3);
  for(size_t i=start;i<p.tx.size();++i) {
    auto& b=p.tx[i]; assert(b[0]==0x10 && b[9]==uint8_t(ground) && b.size()-14<=84);
  }
  assert(stream.paired_result().state==XbeeTxResult::Delivered);
  puts("PASS Zigbee frames, RX isolation, delivery, network rollback, loopback and stream");
}
#endif
