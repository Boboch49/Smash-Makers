#pragma once
#include "display_protocol.h"
namespace smash_sync {
using namespace smash_link;
struct Peer {
 uint32_t session=0,lastAck=0,epoch=0;int32_t offset=0;
 uint16_t ackSeq=0;uint8_t phase=SYNC,samples=0;bool seen=false,ready=false,welcomed=false;
 uint32_t bestRtt=1000,bestSince=0;uint16_t sentSeq[8]={};uint32_t sentTime[8]={};bool occupied[8]={};
 void sent(uint16_t seq,uint32_t now){unsigned n=seq&7;sentSeq[n]=seq;sentTime[n]=now;occupied[n]=true;}
 bool usable(uint32_t now)const{return ready&&phase==RUN&&uint32_t(now-lastAck)<LINK_TIMEOUT_MS;}
 void tick(uint32_t now){
  if(seen&&uint32_t(now-lastAck)>=LINK_TIMEOUT_MS){ready=false;phase=SYNC;samples=0;bestRtt=1000;}
  if(phase==SCHEDULED&&int32_t(now-epoch)>=0)phase=WELCOME;
 }
 bool receive(const Ack&a,uint32_t now){
  if(!a.session)return false;
  if(a.session!=session){if(a.kind!=HELLO)return false;session=a.session;phase=SYNC;samples=0;ready=false;seen=false;bestRtt=1000;for(auto &v:occupied)v=false;}
  if(seen&&uint32_t(now-lastAck)<LINK_TIMEOUT_MS){uint16_t delta=a.seq-ackSeq;if(!delta||delta>=32768)return false;}
  seen=true;lastAck=now;ackSeq=a.seq;
  if((a.resources&9)!=9){ready=false;phase=SYNC;samples=0;return true;}
  if(phase==RUN&&ready&&!(a.resources&PRESENTING)){ready=false;phase=SYNC;samples=0;bestRtt=1000;}
  unsigned n=a.echoSeq&7;
  if(occupied[n]&&sentSeq[n]==a.echoSeq&&sentTime[n]==a.sent){
   int32_t span=int32_t(a.replied-a.received),rtt=int32_t(now-a.sent)-span;
   if(span>=0&&span<100&&rtt>=0&&rtt<100){
    if(uint32_t(now-bestSince)>3000){bestRtt=1000;bestSince=now;}
    if(uint32_t(rtt)<=bestRtt){offset=int32_t((int64_t(int32_t(a.received-a.sent))+int32_t(a.replied-now))/2);bestRtt=rtt;bestSince=now;}
    if(samples<3)++samples;
   }
   occupied[n]=false;
  }
  if(phase==SYNC&&samples>=3){if(!welcomed){epoch=now+250;phase=SCHEDULED;}else phase=RUN;}
  if(a.kind==ACK_READY&&(a.resources&13)==13&&samples>=3){
   if(phase==RUN||(phase==WELCOME&&int32_t(now-(epoch+WELCOME_MS))>=0)){phase=RUN;ready=true;welcomed=true;}
  }
  return true;
 }
};
struct Clock {
 int32_t offset=0;uint32_t last=0;bool locked=false;
 void reset(){*this=Clock();}
 void correct(int32_t target,bool hard=false){if(!locked||hard){offset=target;locked=true;last=0;}else{int32_t d=int32_t(uint32_t(target)-uint32_t(offset));if(d>0)offset=int32_t(uint32_t(offset)+1);else if(d<0)offset=int32_t(uint32_t(offset)-1);}}
 uint32_t now(uint32_t local){uint32_t candidate=local-uint32_t(offset);if(!last||int32_t(candidate-last)>=0)last=candidate;return last;}
};
}
