#pragma once
#include "test_sequence.h"
// Included after the production rendering helpers: same assets, fonts and codec.
static smash_test::Timeline testTimeline;
static uint32_t testRendered=0;
static uint16_t testAudioSlot=0xFFFF;
static char testCommand[24];static unsigned testCommandLength=0;
static void testList(){
 Serial.println("TEST AUTONOME WAVESHARE - NEXT PREV REPLAY PAUSE PLAY LIST");
 for(unsigned n=0;n<smash_test::COUNT;++n)
  Serial.printf("%02u %s (%u ms)\n",n+1,smash_test::steps[n].name,smash_test::steps[n].ms);
}
static void testBegin(uint32_t now){testTimeline.begin(now);testList();}
static void testControls(){
 for(unsigned n=0;n<32&&Serial.available();++n){
  char c=Serial.read();
  if(c=='\n'||c=='\r'){
   if(!testCommandLength)continue;
   testCommand[testCommandLength]=0;testCommandLength=0;
   if(!strcmp(testCommand,"NEXT"))testTimeline.next();
   else if(!strcmp(testCommand,"PREV"))testTimeline.previous();
   else if(!strcmp(testCommand,"REPLAY"))testTimeline.replay();
   else if(!strcmp(testCommand,"PAUSE")){testTimeline.pause(true);audio.stopEffects();}
   else if(!strcmp(testCommand,"PLAY"))testTimeline.pause(false);
   else if(!strcmp(testCommand,"LIST"))testList();
   else Serial.println("Commandes: NEXT PREV REPLAY PAUSE PLAY LIST");
  }else if(testCommandLength<sizeof(testCommand)-1){
   testCommand[testCommandLength++]=(c>='a'&&c<='z')?c-'a'+'A':c;
  }
 }
}
static void testLoop(uint32_t now){
 using namespace smash_test;
 testTimeline.tick(now);testControls();
 if(!panelReady){delay(10);return;}
 if(!audioReady||!audio.healthy()){
  message("ERREUR AUDIO","TEST ARRETE",RED);panel->flipDMABuffer();delay(10);return;
 }
 if(uint32_t(now-testRendered)<40){delay(1);return;}
 testRendered=now;
 const Step& step=testTimeline.step();uint32_t age=testTimeline.age();
 bool changed=testTimeline.takeChanged();
 if(changed){
  audio.stopEffects();testAudioSlot=0xFFFF;
  Serial.printf("SCENE %02u/%u : %s - %u ms\n",testTimeline.number()+1,COUNT,step.name,step.ms);
 }
 State sample;sample.mode=2;sample.score1=10;sample.score2=10;sample.sets1=sample.sets2=1;
 sample.server=1;sample.rally=16;sample.flags=WAIT_SERVE;
 switch(step.visual){
 case WELCOME_SCREEN:frame(accueil,age);break;
 case WAITING:frame(attente,age);break;
 case SELECT_J1:case SELECT_J2:case SELECT_TWO:
  sample.selectedMode=step.visual==SELECT_J1?1:step.visual==SELECT_J2?3:2;selection(sample,age);break;
 case LIVE_SCORE:sample.score1=11;scores(sample,age);break;
 case COUNTDOWN:frame(engagement,age);break;
 case SERVICE_CUBE:pinFrame(smash_pinball::skill_shot,age);footer(sample);break;
 case REBOUND_ASSET:frame(rebond,age);break;
 case BUMPER_LIVE:sample.flags=0;scores(sample,age,true);break;
 case BUMPER_ASSET:pinFrame(smash_pinball::bumper,age);break;
 case POINT_J1:frame(point_j1,age);footer(sample);break;
 case POINT_J2:frame(point_j2,age);footer(sample);break;
 case BONUS_LIVE:bonus(sample,age);break;
 case JACKPOT_J1:case JACKPOT_J2:case CHAMPION_J1:case CHAMPION_J2:
  sample.winner=(step.visual==JACKPOT_J2||step.visual==CHAMPION_J2)?2:1;
  if(sample.winner==1)sample.score1=12;else sample.score2=12;
  celebration(sample,age,step.visual==CHAMPION_J1||step.visual==CHAMPION_J2);break;
 case VICTORY_J1:frame(victoire,age);break;
 case VICTORY_J2:frame(victoire_j2,age);break;
 case DEFEAT:frame(defaite,age);break;
 case HEAT_SCREEN:frame(securite,age);break;
 case PAUSE_SCREEN:message("PAUSE","SMASH MAKERS",YELLOW);break;
 case FAULT_SCREEN:message("SECU ARRET","DEFAUT SKR",RED);break;
 case REFERENCE_SCREEN:message("REFERENCE","ZERO / ARM",YELLOW);break;
 case MIX:sample.flags=0;scores(sample,age,true);label("MIXAGE",0,BLUE,1,44,82);break;
 case AUDIO_ONLY:
  panel->fillScreen(0);char title[24];snprintf(title,sizeof(title),"SON %02u/20",unsigned(step.sound)+1);
  label(title,1,YELLOW);label(step.name,12,WHITE);label("+ AMBIANCE",24,BLUE);break;
 }
 panel->flipDMABuffer();
 if(!testTimeline.isPaused()){
  // Two repetitions, aligned with each graphic loop; no accelerated animation.
  uint16_t period=step.ms/2;
  if(step.visual==SERVICE_CUBE||step.visual==BUMPER_ASSET||step.visual==BONUS_LIVE||
     step.visual==JACKPOT_J1||step.visual==JACKPOT_J2||step.visual==CHAMPION_J1||step.visual==CHAMPION_J2)period=5120;
  if(step.visual==REBOUND_ASSET)period=1920;
  if(step.visual==COUNTDOWN)period=960;
  if(step.visual==AUDIO_ONLY)period=step.ms;
  if(step.visual==MIX)period=1000;
  uint16_t slot=age/period;
  if(slot!=testAudioSlot){
   testAudioSlot=slot;
   if(step.visual==MIX){
    audio.play(slot%2?Sfx::PADDLE_RIGHT_HARD:Sfx::PADDLE_LEFT_HARD);
    audio.play(Sfx::RALLY_COMBO);
   }else if(step.visual==COUNTDOWN)audio.play(slot%4==3?Sfx::LAUNCH:Sfx::COUNTDOWN);
   else if(step.sound!=Sfx::AMBIENT)audio.play(step.sound);
  }
 }
}
