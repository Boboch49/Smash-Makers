#pragma once
#include "display_protocol.h"
#include "hockey_media_catalog.h"
#include "audio_types.h"
namespace hockey {
struct Plan{Slot id;uint32_t age;Plan(Slot i=JEU,uint32_t a=0):id(i),age(a){}};
inline uint32_t elapsed(uint32_t now,uint32_t start){int32_t d=int32_t(now-start);return d<0?0:uint32_t(d);}
inline unsigned human(const smash_link::State&s){uint8_t m=s.menu==makers::CHOOSE_PLAYERS?s.selectedMode:s.mode;return m==1?1:m==3?2:0;}
inline Plan presentation(const smash_link::State&s,uint32_t now){
 using namespace smash_link;uint32_t menuAge=elapsed(now,s.menuAt);
 if(s.flags&PAUSED)return {PAUSE,0};
 if(s.menu==makers::CHOOSE_PLAYERS)return {s.selectedMode==1?SOLO_J1:s.selectedMode==3?SOLO_J2:DUEL,menuAge};
 if(s.menu==makers::INTRO)return {TITLE,menuAge};
 if(s.menu==makers::RESULTS){
  if(menuAge<400)return {FIN_CHRONO,3000+menuAge};
  if(menuAge>=7400)return {RETOUR_MENU,menuAge-7400};
  if(!s.winner)return {EGALITE,menuAge-400};
  if(human(s)&&human(s)!=s.winner)return {human(s)==1?DEFAITE_SOLO_J1:DEFAITE_SOLO_J2,menuAge-400};
  return {s.winner==1?VICTOIRE_J1:VICTOIRE_J2,menuAge-400};
 }
 uint32_t eventAge=elapsed(now,s.eventAt);
 if((s.event==uint8_t(Sfx::SCORE_LEFT)||s.event==uint8_t(Sfx::SCORE_RIGHT))&&eventAge<960)
  return {s.event==uint8_t(Sfx::SCORE_LEFT)?BUT_J1:BUT_J2,eventAge};
 if(s.flags&WAIT_SERVE){unsigned server=human(s)?human(s):s.server;return {server==1?ATTENTE_J1:ATTENTE_J2,menuAge};}
 uint32_t cueAge=elapsed(now,s.hockeyAt);Slot cue=JEU;
 switch(s.hockeyCue){
  case hockey_event::SHOT_J1:cue=TIR_J1;break;case hockey_event::SHOT_J2:cue=TIR_J2;break;
  case hockey_event::SAVE_J1:cue=PARADE_J1;break;case hockey_event::SAVE_J2:cue=PARADE_J2;break;
  case hockey_event::POST:case hockey_event::BOARD:cue=POTEAU;break;
 }
 if(cue!=JEU&&cueAge<specs[cue].duration)return {cue,cueAge};
 if(s.event==uint8_t(Sfx::LAUNCH)&&eventAge<1200)return {ENGAGEMENT,eventAge};
 if(s.remainingSeconds<=3)return {FIN_CHRONO,(3-s.remainingSeconds)*1000+elapsed(now,s.time)};
 if(s.remainingSeconds<=10)return {DIX_SECONDES,(10-s.remainingSeconds)*1000+elapsed(now,s.time)};
 return {JEU,menuAge};
}
}
