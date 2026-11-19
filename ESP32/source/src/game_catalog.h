#pragma once
#include <stdint.h>
#include <math.h>

// Game identity is separate from the existing Solo J1 / Solo J2 / two-player mode.
namespace makers {
enum Game:uint8_t {SMASH=0,HOCKEY=1,STRANGE=2,GAME_COUNT=3};
enum Menu:uint8_t {CHOOSE_GAME=0,CHOOSE_PLAYERS=1,INTRO=2,PLAYING=3,RESULTS=4};
struct Profile {const char*name;uint32_t introMs,matchMs;uint8_t goalPercent;uint32_t riftPeriodMs,riftDurationMs,riftWarningMs;};
inline const Profile&profile(Game game){
 static const Profile profiles[]={
  {"SMASH MAKERS",2500,0,100,0,0,0},
  {"SMASH HOCKEY",10520,180000,40,0,0,0},
  {"STRANGE MAKERS",53480,0,100,60000,8000,22280}
 };
 return profiles[game<GAME_COUNT?game:SMASH];
}
enum Action:uint8_t {NONE,SHOW_MENU,SHOW_PLAYERS,START_MATCH};
struct Selector {
 Game selected=SMASH,active=SMASH;Menu stage=CHOOSE_GAME;uint32_t since=0;
 // Player mode values match game_modes.h: 1=solo J1, 2=duel, 3=solo J2.
 uint8_t players=1;
 Action input(bool mode,bool j1,bool j2,uint32_t now){
  if(stage==CHOOSE_GAME){
   if(mode){selected=Game((selected+1)%GAME_COUNT);since=now;return SHOW_MENU;}
   if(j1||j2){stage=CHOOSE_PLAYERS;since=now;return SHOW_PLAYERS;}
  }else if(stage==CHOOSE_PLAYERS){
   if(mode){players=players==1?3:players==3?2:1;since=now;return SHOW_PLAYERS;}
   if((players==1&&j1)||(players==3&&j2)||(players==2&&(j1||j2))){active=selected;stage=INTRO;since=now;return START_MATCH;}
  }else if(stage==RESULTS){
   if(mode){stage=CHOOSE_GAME;since=now;return SHOW_MENU;}
   if((players==1&&j1)||(players==3&&j2)||(players==2&&(j1||j2))){stage=INTRO;since=now;return START_MATCH;}
  }
  return NONE;
 }
 bool tick(uint32_t now){if(stage==INTRO&&uint32_t(now-since)>=profile(active).introMs){stage=PLAYING;since=now;return true;}return false;}
 void result(uint32_t now){stage=RESULTS;since=now;}
};

struct Session {
 Game game=SMASH;uint32_t activeMs=0,last=0,worldStart=0;uint8_t server=1;bool finished=false;
 void begin(Game selected,uint32_t now){game=selected;activeMs=worldStart=0;last=now;server=1;finished=false;}
 void tick(uint32_t now,bool advancing){
  uint32_t delta=now-last;last=now;
  // A late scheduler tick must not burn minutes or jump the rift phase.
  if(advancing&&!finished)activeMs+=delta>100?100:delta;
 }
 uint16_t secondsLeft()const{
  if(game!=HOCKEY)return 0;
  uint32_t duration=profile(game).matchMs;
  return activeMs>=duration?0:uint16_t((duration-activeMs+999)/1000);
 }
 bool expired()const{return game==HOCKEY&&activeMs>=profile(game).matchMs;}
 bool goal(float y,float height)const{
  if(game!=HOCKEY)return true;
  float half=height*profile(game).goalPercent*.005f;
  return y>=height*.5f-half&&y<=height*.5f+half;
 }
 // 0=normal, 1=active rift, 2=visual/audio announcement before the rift.
 uint8_t event()const{
  if(game!=STRANGE)return 0;
  const auto&p=profile(game);uint32_t phase=(activeMs-worldStart)%p.riftPeriodMs;
  if(phase>=p.riftPeriodMs-p.riftDurationMs)return 1;
  if(phase>=p.riftPeriodMs-p.riftDurationMs-p.riftWarningMs)return 2;
  return 0;
 }
 void normalWorld(){worldStart=activeMs;}
 uint16_t eventAge()const{
  if(game!=STRANGE)return 0;const auto&p=profile(game);uint32_t phase=(activeMs-worldStart)%p.riftPeriodMs;
  return event()==1?uint16_t(phase-(p.riftPeriodMs-p.riftDurationMs)):event()==2?uint16_t(phase-(p.riftPeriodMs-p.riftDurationMs-p.riftWarningMs)):0;
 }
 float riftEnvelope()const{
  if(event()!=1)return 0;
  const auto&p=profile(game);uint32_t elapsed=(activeMs-worldStart)%p.riftPeriodMs-(p.riftPeriodMs-p.riftDurationMs);
  float v=fminf(elapsed/800.0f,(p.riftDurationMs-elapsed)/800.0f);
  return fmaxf(0,fminf(1,v));
 }
};
}
