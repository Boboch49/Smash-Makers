#pragma once
#include "display_protocol.h"
namespace smash_display {
using namespace smash_link;
enum Scene {LOST,BOOT,READY,SCORE,POINT1,POINT2,CHAMPION1,CHAMPION2,BOUNCE,GO,SECURITY,PAUSE,REFERENCE,SERVICE_SCREEN,DEMO,HEAT,JACKPOT1,JACKPOT2,BONUS};
class Model {
public:
 State state;bool seen=false;uint32_t lastRx=0,connectedAt=0,started=0;Scene effect=SCORE;uint16_t duration=0;
 bool online(uint32_t now)const{return seen&&uint32_t(now-lastRx)<1500;}
 bool receive(const State&s,uint32_t now){
  bool fresh=!online(now);
  if(!fresh){uint16_t delta=s.seq-state.seq;if(!delta||delta>=32768)return false;}
  const uint32_t stamp=s.eventAt&&int32_t(now-s.eventAt)>=0?s.eventAt:now;
  State old=state;state=s;seen=true;lastRx=now;
  if(fresh){connectedAt=now;effect=SCORE;duration=0;return true;}
  if(s.flags&(FAULT|PAUSED|HOT|TMC_FAULT|SET_OVER|MATCH_OVER)){duration=0;return true;}
  if(s.mode==0||s.app!=0||!(s.flags&REFERENCED)){duration=0;return true;}
  if(s.setIndex!=old.setIndex||s.score1<old.score1||s.score2<old.score2){duration=0;effect=SCORE;return true;}
  if((old.flags&WAIT_SERVE)&&!(s.flags&WAIT_SERVE)){start(GO,stamp,960);return true;}
  if(s.score1>old.score1){start(POINT1,stamp,1600);return true;}
  if(s.score2>old.score2){start(POINT2,stamp,1600);return true;}
  if(s.rally>old.rally&&s.rally/8>old.rally/8){start(BONUS,stamp,700);return true;}
  if(s.eventId!=old.eventId){
   if(s.event==8){start(GO,stamp,960);return true;}
   if((s.event==1||s.event==2||s.event==3||s.event==16||s.event==17||s.event==18)&&!(s.flags&WAIT_SERVE)&&!(duration&&uint32_t(now-started)<duration))start(BOUNCE,stamp,200);
  }
  return true;
 }
 Scene view(uint32_t now)const{
  if(!online(now))return seen&&(state.flags&(FAULT|HOT|TMC_FAULT))?SECURITY:LOST;
  if(state.flags&(FAULT|HOT|TMC_FAULT))return SECURITY;
  if(state.flags&THERMAL)return HEAT;
  if(state.flags&NEEDS_ARM)return REFERENCE;
  if(state.flags&PAUSED)return PAUSE;
  if(state.flags&SERVICE)return SERVICE_SCREEN;
  if(state.app==3)return DEMO;
  if(state.app!=0)return SERVICE_SCREEN;
  if(!(state.flags&REFERENCED))return REFERENCE;
  if(state.flags&MATCH_OVER)return state.winner==2?CHAMPION2:CHAMPION1;
  if(state.flags&SET_OVER)return state.winner==2?JACKPOT2:JACKPOT1;
  if(state.mode==0)return READY; // Welcome is owned by the startup barrier, once only.
  if(duration&&uint32_t(now-started)<duration)return effect;
  return SCORE;
 }
private:void start(Scene s,uint32_t now,uint16_t ms){effect=s;started=now;duration=ms;}
};
}
