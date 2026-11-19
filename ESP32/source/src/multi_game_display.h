#pragma once
#include "game_catalog.h"

static uint16_t gameColour(uint8_t game){return game==makers::HOCKEY?0x4E9F:game==makers::STRANGE?0xF800:AMBER;}
static void gameMenuScreen(const State&s,uint32_t now){
 panel->fillScreen(0);uint16_t c=gameColour(s.selectedGame);
 panel->drawRect(0,0,128,32,c);
 if(s.menu==makers::CHOOSE_GAME){
  char title[18];snprintf(title,sizeof(title),"JEU %u/3",s.selectedGame+1);label(title,2,WHITE);
  label(makers::profile(makers::Game(s.selectedGame)).name,12,c);
  label("MODE >  J1/J2 OK",23,WHITE);
 }else{
  label(makers::profile(makers::Game(s.selectedGame)).name,2,c);
  label(s.selectedMode==1?"SOLO J1":s.selectedMode==3?"SOLO J2":"DEUX JOUEURS",12,WHITE);
  label(s.selectedMode==1?"MODE > J1 START":s.selectedMode==3?"MODE > J2 START":"MODE > J1/J2 GO",23,c);
 }
 (void)now;
}
static void gameIntro(const State&s,uint32_t age){
 if(s.game==makers::SMASH){frame(accueil,age);return;}
 panel->fillScreen(0);uint16_t c=gameColour(s.game);
 if(s.game==makers::HOCKEY){
  panel->drawRoundRect(1,1,126,30,6,c);panel->drawFastVLine(64,1,30,c);
  panel->drawCircle(64,16,10,c);
  int puck=10+int(age*108/makers::profile(makers::HOCKEY).introMs);
  panel->fillCircle(puck,16,3,WHITE);
 }else{
  // Lightweight procedural introduction. The 53.48 s movie is an SD asset,
  // kept outside this firmware until the streaming reader is integrated.
  int pulse=4+int((age/80)%10);
  panel->drawCircle(64,16,pulse,0x780F);panel->drawCircle(64,16,pulse+9,0x5007);
  panel->drawFastHLine(9,1,110,c);panel->drawFastHLine(9,30,110,c);
 }
 panel->fillRect(18,4,92,9,0);panel->fillRect(22,19,84,9,0);
 label(s.game==makers::HOCKEY?"HOCKEY":"STRANGE",5,c);label("MAKERS",20,WHITE);
}
static void themedScore(const State&s,Scene scene,uint32_t now){
 panel->fillScreen(0);uint16_t c=gameColour(s.game);
 if(s.game==makers::HOCKEY){
  panel->drawRoundRect(0,0,128,32,7,c);panel->drawCircle(64,16,9,0x116A);
  panel->drawFastVLine(64,8,16,0x116A);
  char time[8];snprintf(time,sizeof(time),"%u:%02u",s.remainingSeconds/60,s.remainingSeconds%60);label(time,0,WHITE);
 }else{
  for(unsigned n=0;n<3;++n){int radius=5+int((now/90+n*5)%14);panel->drawCircle(13,16,radius,0x680C);panel->drawCircle(114,16,radius,0x680C);}
  label(s.gameEvent==2?"FAILLE ARRIVE":s.gameEvent==1?"FAILLE ACTIVE":"STRANGE MAKERS",0,s.gameEvent?0xF81F:c);
 }
 char score[14];snprintf(score,sizeof(score),"%u:%u",s.score1,s.score2);
 panel->fillRect(27,9,74,16,0);label(score,10,WHITE,2);
 if(scene==POINT1||scene==POINT2){
  label(s.game==makers::HOCKEY?(scene==POINT1?"BUT J1":"BUT J2"):(scene==POINT1?"POINT J1":"POINT J2"),25,c);
 }else label(s.flags&WAIT_SERVE?(s.flags&REPOSITION?"PLACEMENT":"APPUYER POUR SERVIR"):(s.game==makers::HOCKEY?"HOCKEY MAKERS":"STRANGE MAKERS"),25,c);
}
static bool multiGameDisplay(const State&s,Scene scene,uint32_t now){
 if(s.app!=0||s.phase!=RUN||scene==SECURITY||scene==HEAT||scene==REFERENCE||scene==LOST)return false;
 const bool hockeySelected=s.menu==makers::CHOOSE_PLAYERS?s.selectedGame==makers::HOCKEY:s.game==makers::HOCKEY;
 if(scene==PAUSE){if(hockeySelected)return hockeyDisplay(s,now);return false;}
 if(s.menu!=makers::CHOOSE_GAME&&hockeySelected)return hockeyDisplay(s,now);
 if(s.menu==makers::CHOOSE_GAME||s.menu==makers::CHOOSE_PLAYERS){gameMenuScreen(s,now);return true;}
 if(s.menu==makers::INTRO){
  if(s.game==makers::STRANGE){if(!strangeIntroDraw(strange_intro::age(now,s.menuAt)))message("ERREUR INTRO","VERIFIER SD",RED);}
  else gameIntro(s,uint32_t(now-s.menuAt));return true;
 }
 if(s.game==makers::STRANGE&&strangeDisplay(s,scene,now))return true;
 if(s.menu==makers::RESULTS&&uint32_t(now-s.menuAt)>=5000){
  panel->fillScreen(0);label(s.winner==1?"J1 GAGNE":s.winner==2?"J2 GAGNE":"EGALITE",1,gameColour(s.game));
  char score[14];snprintf(score,sizeof(score),"%u:%u",s.score1,s.score2);label(score,10,WHITE,2);
  label("MODE = AUTRE JEU",25,gameColour(s.game));return true;
 }
 if(s.game==makers::SMASH)return false;
 if(s.menu==makers::RESULTS){
  panel->fillScreen(0);panel->drawRect(0,0,128,32,gameColour(s.game));
  label(s.winner==1?"VICTOIRE J1":s.winner==2?"VICTOIRE J2":"EGALITE",4,WHITE);
  label(makers::profile(makers::Game(s.game)).name,18,gameColour(s.game));return true;
 }
 themedScore(s,scene,now);return true;
}
