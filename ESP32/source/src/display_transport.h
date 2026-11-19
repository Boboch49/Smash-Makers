#pragma once
#include "peer_sync.h"
#include "display_model.h"
#include <esp_system.h>
class DisplayTransport {
public:
 smash_sync::Clock clock;smash_link::State state;
 uint32_t nonce=0,lastRx=0,receivedLocal=0;uint16_t ackSeq=0;bool seen=false,confirmed=false,sawAny=false;
private:
 smash_link::Parser parser;uint32_t lastByte=0,lastReply=0;bool zeroSeen=false;uint8_t previousPhase=smash_link::SYNC;
public:
 void begin(){nonce=esp_random();if(!nonce)nonce=1;}
 bool online(uint32_t now)const{return seen&&uint32_t(now-lastRx)<smash_link::LINK_TIMEOUT_MS;}
 bool poll(HardwareSerial&port,smash_display::Model&model,uint32_t local){
  using namespace smash_link;bool updated=false;
  if(uint32_t(local-lastByte)>50)parser.reset();
  for(unsigned n=0;n<160&&port.available();++n){
   State s;lastByte=local;if(!parser.push(uint8_t(port.read()),s))continue;sawAny=true;
   if(s.session==0){
    if(!zeroSeen){begin();model=smash_display::Model();clock.reset();seen=false;confirmed=false;previousPhase=SYNC;}zeroSeen=true;
    // Send a fresh HELLO; a zero-session snapshot is never treated as live gameplay.
    state=s;receivedLocal=local;updated=true;continue;
   }
   if(s.session!=nonce)continue;
   zeroSeen=false;
   if(seen&&online(local)){uint16_t delta=s.seq-state.seq;if(!delta||delta>=32768)continue;}
   if(s.phase==SYNC&&previousPhase!=SYNC){model=smash_display::Model();confirmed=false;clock.reset();}
   if(s.flags&CLOCK_VALID)clock.correct(s.offset,s.phase==SYNC);
   state=s;seen=true;lastRx=receivedLocal=local;previousPhase=s.phase;updated=true;
   if(s.phase==RUN||s.flags&(FAULT|HOT|TMC_FAULT|THERMAL))model.receive(s,clock.now(local));
  }
  return updated;
 }
 void reply(HardwareSerial&port,uint32_t local,uint8_t resources,bool presented){
  using namespace smash_link;if(uint32_t(local-lastReply)<100||port.availableForWrite()<int(ACK_SIZE))return;
  Ack a;a.session=nonce;a.seq=++ackSeq;a.resources=resources;a.echoSeq=state.seq;a.sent=state.time;a.received=receivedLocal;a.replied=local;
  a.kind=online(local)&&state.session==nonce?TIME_ACK:HELLO;
  if(presented){a.kind=ACK_READY;a.resources|=PRESENTING;}
  uint8_t data[ACK_SIZE];encodeAck(a,data);port.write(data,sizeof(data));lastReply=local;
 }
};
