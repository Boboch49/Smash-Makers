#pragma once
#include "hockey_media_catalog.h"
#include "audio_types.h"
struct HockeyAudioState{
 bool enabled=false,blocked=true,intro=false,result=false;uint8_t winner=0,event=255,cue=0;
 uint16_t eventId=0;uint32_t session=0,introAge=0,resultAge=0,eventAge=99999,cueAt=0,cueAge=99999,sent=0;
};
class HockeyAudio {
 const hockey::View*media=nullptr;HockeyAudioState state;
 struct Voice{hockey::Slot slot=hockey::BOUNCE_AUDIO;uint32_t sample=0,fraction=0,step=65536;unsigned gain=0;bool on=false;};
 Voice foreground,bounce;uint32_t movie=0;bool seen=false,resultPlayed=false;
 int16_t at(hockey::Slot slot,uint32_t pos)const{
  if(!media||!media->ready||pos>=media->bytes[slot]/2)return 0;const uint8_t*p=media->data[slot]+pos*2;return int16_t(uint16_t(p[0])|(uint16_t(p[1])<<8));
 }
 void play(Voice&v,hockey::Slot slot,uint32_t age,unsigned gain){v.slot=slot;v.sample=age*16;v.gain=gain;v.on=true;v.fraction=0;v.step=65536;}
 int32_t next(Voice&v){if(!v.on)return 0;int32_t n=int32_t(at(v.slot,v.sample))*int(v.gain)/100;v.fraction+=v.step;v.sample+=v.fraction>>16;v.fraction&=65535;if(v.sample>=media->bytes[v.slot]/2)v.on=false;return n;}
public:
 void begin(const hockey::View*m){media=m;}
 void control(const HockeyAudioState&n,uint32_t received){
  bool fresh=!seen||n.session!=state.session;
  if(fresh||n.enabled!=state.enabled){foreground.on=bounce.on=false;movie=0;resultPlayed=false;}
  if(n.blocked||!n.enabled){foreground.on=bounce.on=false;movie=0;}
  else if(n.intro){
   uint32_t target=(n.introAge+received-n.sent)*16,d=movie>target?movie-target:target-movie;
   if(!state.intro||fresh||d>640)movie=target;foreground.on=bounce.on=false;
  }else{
   if(!n.result)resultPlayed=false;
   if(state.result&&!n.result)foreground.on=false;
   if(n.result&&!resultPlayed&&n.resultAge>=400){resultPlayed=true;if(n.winner)play(foreground,hockey::WIN_AUDIO,n.resultAge-400,48);}
   if(!n.result&&(fresh||n.eventId!=state.eventId)&&n.eventAge<=150){
    if(n.event==uint8_t(Sfx::SCORE_LEFT)||n.event==uint8_t(Sfx::SCORE_RIGHT))play(foreground,hockey::GOAL_AUDIO,n.eventAge,52);
    else if(n.event==uint8_t(Sfx::LAUNCH))play(foreground,hockey::SHOT_AUDIO,n.eventAge,32);
   }
   if(!n.result&&n.cue&&n.cueAge<=150&&(fresh||n.cueAt!=state.cueAt))play(bounce,hockey::BOUNCE_AUDIO,n.cueAge,n.cue==5?36:25);
  }
  state=n;seen=true;
 }
 bool ownsMusic()const{return media&&media->ready&&state.enabled;}
 int16_t sample(){
  if(!ownsMusic()||state.blocked)return 0;
  if(state.intro)return int16_t(int32_t(at(hockey::INTRO_AUDIO,movie++))*55/100);
  return int16_t(next(foreground)+next(bounce));
 }
};
