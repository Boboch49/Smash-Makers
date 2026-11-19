#pragma once
// Standalone gallery: no SKR UART, no motor commands and no physical buttons.
static const hockey::Slot hockeyTestScenes[]={hockey::TITLE,hockey::SOLO_J1,hockey::SOLO_J2,hockey::DUEL,hockey::ATTENTE_J1,hockey::ATTENTE_J2,hockey::ENGAGEMENT,hockey::JEU,hockey::TIR_J1,hockey::TIR_J2,hockey::PARADE_J1,hockey::PARADE_J2,hockey::POTEAU,hockey::BUT_J1,hockey::BUT_J2,hockey::DIX_SECONDES,hockey::FIN_CHRONO,hockey::VICTOIRE_J1,hockey::VICTOIRE_J2,hockey::DEFAITE_SOLO_J1,hockey::DEFAITE_SOLO_J2,hockey::EGALITE,hockey::PAUSE,hockey::RETOUR_MENU};
static const uint32_t hockeyTestDurations[]={10520,2000,2000,2000,2000,2000,2000,4000,2000,2000,2000,2000,2000,3000,3000,3000,4000,7000,7000,3000,3000,3000,3000,3000};
static unsigned hockeyTestIndex=0;
static uint32_t hockeyTestStart=0,hockeyTestSession=1,hockeyTestPauseAt=0;
static bool hockeyTestPaused=false;
static void hockeyTestSelect(unsigned i,uint32_t now){
 hockeyTestIndex=i%24;hockeyTestStart=now;hockeyTestPaused=false;++hockeyTestSession;
 Serial.printf("HOCKEY SCENE %u/24 %s DUREE=%lu ms\n",hockeyTestIndex+1,hockey::specs[hockeyTestScenes[hockeyTestIndex]].path,(unsigned long)hockeyTestDurations[hockeyTestIndex]);
}
static void hockeyTestList(){
 Serial.println("NEXT | PREV | REPLAY | TITRE | JEU | BUT1 | BUT2 | VICTOIRE1 | VICTOIRE2 | EGALITE | REBOND | PAUSE | VOL 0..100 | LIST");
 for(unsigned i=0;i<24;++i)Serial.printf("%u: %s\n",i+1,hockey::specs[hockeyTestScenes[i]].path);
}
static void hockeyTestBegin(uint32_t now){
 Serial.println("TEST HOCKEY ESP32 SEUL - SD /HOCKEY - SKR NON UTILISEE - WIFI/BT OFF");
 hockeyTestList();hockeyTestSelect(0,now);
}
static void hockeyTestUsb(uint32_t now){
 static char line[48];static size_t at=0;
 for(unsigned n=0;n<96&&Serial.available();++n){char c=Serial.read();
  if(c=='\r'||c=='\n'){
   if(!at)continue;line[at]=0;at=0;
   for(char*p=line;*p;++p)if(*p>='a'&&*p<='z')*p-=32;
   if(!strcmp(line,"NEXT"))hockeyTestSelect(hockeyTestIndex+1,now);
   else if(!strcmp(line,"PREV"))hockeyTestSelect((hockeyTestIndex+23)%24,now);
   else if(!strcmp(line,"REPLAY"))hockeyTestSelect(hockeyTestIndex,now);
   else if(!strcmp(line,"LIST"))hockeyTestList();
   else if(!strcmp(line,"PAUSE")){
    if(hockeyTestPaused){hockeyTestStart+=now-hockeyTestPauseAt;hockeyTestPaused=false;}
    else{hockeyTestPaused=true;hockeyTestPauseAt=now;}
   }else if(!strncmp(line,"VOL ",4)){
    char*end=nullptr;long volume=strtol(line+4,&end,10);
    if(end!=line+4&&!*end&&volume>=0&&volume<=100){audio.setMasterVolume(volume);Serial.printf("VOL=%ld\n",volume);}
    else Serial.println("VOL INVALIDE: 0..100");
   }else{
    const char*names[]={"TITRE","JEU","BUT1","BUT2","VICTOIRE1","VICTOIRE2","EGALITE","REBOND"};
    const unsigned targets[]={0,7,13,14,17,18,21,12};bool found=false;
    for(unsigned i=0;i<8;++i)if(!strcmp(line,names[i])){hockeyTestSelect(targets[i],now);found=true;break;}
    if(!found)Serial.println("COMMANDE INCONNUE - LIST");
   }
  }else if(c>=32&&c<127){if(at<sizeof(line)-1)line[at++]=c;else at=0;}
 }
}
static void hockeyTestLoop(uint32_t now){
 hockeyTestUsb(now);
 if(!panelReady||!audioReady||!audio.healthy()||!hockeyAssets.view().ready||hockeyPresentationFailed){
  HockeyAudioState mute;audio.hockeyControl(mute);
  if(panelReady){message(!hockeyAssets.view().ready?"SD HOCKEY":"ERREUR TEST",!hockeyAssets.view().ready?hockeyAssets.error():hockeyPresentationFailed?"ANIMATION":"AUDIO",RED);panel->flipDMABuffer();}
  delay(40);return;
 }
 uint32_t age=(hockeyTestPaused?hockeyTestPauseAt:now)-hockeyTestStart;
 if(!hockeyTestPaused&&age>=hockeyTestDurations[hockeyTestIndex]){hockeyTestSelect(hockeyTestIndex+1,now);age=0;}
 hockey::Slot slot=hockeyTestScenes[hockeyTestIndex];
 State s;s.phase=smash_link::RUN;s.game=s.selectedGame=makers::HOCKEY;s.menu=makers::PLAYING;
 s.mode=s.selectedMode=2;s.score1=2;s.score2=2;s.remainingSeconds=137;
 HockeyAudioState a;a.enabled=true;a.blocked=hockeyTestPaused;a.session=hockeyTestSession;
 a.eventId=1;a.eventAge=age;a.cueAge=age;a.cueAt=hockeyTestStart;a.introAge=age;
 hockey::Plan plan(slot,age);
 switch(slot){
  case hockey::TITLE:s.menu=makers::INTRO;a.intro=true;break;
  case hockey::SOLO_J1:s.menu=makers::CHOOSE_PLAYERS;s.mode=s.selectedMode=1;break;
  case hockey::SOLO_J2:s.menu=makers::CHOOSE_PLAYERS;s.mode=s.selectedMode=3;break;
  case hockey::DUEL:s.menu=makers::CHOOSE_PLAYERS;break;
  case hockey::ATTENTE_J1:case hockey::ATTENTE_J2:s.flags=smash_link::WAIT_SERVE;s.remainingSeconds=180;s.score1=s.score2=0;break;
  case hockey::ENGAGEMENT:s.remainingSeconds=180;s.score1=s.score2=0;a.event=uint8_t(Sfx::LAUNCH);break;
  case hockey::TIR_J1:a.cue=hockey_event::SHOT_J1;break;
  case hockey::TIR_J2:a.cue=hockey_event::SHOT_J2;break;
  case hockey::PARADE_J1:a.cue=hockey_event::SAVE_J1;break;
  case hockey::PARADE_J2:a.cue=hockey_event::SAVE_J2;break;
  case hockey::POTEAU:a.cue=hockey_event::POST;s.hockeyCue=a.cue;break;
  case hockey::BUT_J1:case hockey::BUT_J2:
   s.score1=3;s.score2=slot==hockey::BUT_J1?2:3;
   a.event=uint8_t(slot==hockey::BUT_J1?Sfx::SCORE_LEFT:Sfx::SCORE_RIGHT);
   if(age>=960)plan=hockey::Plan(hockey::JEU,age);break;
  case hockey::DIX_SECONDES:s.remainingSeconds=10-age/1000;break;
  case hockey::FIN_CHRONO:s.remainingSeconds=age>=3000?0:3-age/1000;break;
  case hockey::VICTOIRE_J1:case hockey::VICTOIRE_J2:
   s.remainingSeconds=0;s.score1=slot==hockey::VICTOIRE_J1?5:4;s.score2=slot==hockey::VICTOIRE_J2?5:4;
   a.result=true;a.resultAge=age+400;a.winner=slot==hockey::VICTOIRE_J1?1:2;break;
  case hockey::DEFAITE_SOLO_J1:case hockey::DEFAITE_SOLO_J2:
   s.mode=s.selectedMode=slot==hockey::DEFAITE_SOLO_J1?1:3;s.remainingSeconds=0;
   s.score1=slot==hockey::DEFAITE_SOLO_J1?3:5;s.score2=slot==hockey::DEFAITE_SOLO_J1?5:3;
   a.result=true;a.resultAge=age+400;a.winner=slot==hockey::DEFAITE_SOLO_J1?2:1;break;
  case hockey::EGALITE:s.remainingSeconds=0;s.score1=s.score2=4;break;
  case hockey::PAUSE:a.blocked=true;break;
  case hockey::RETOUR_MENU:s.remainingSeconds=0;break;
  default:break;
 }
 audio.hockeyControl(a);
 static uint32_t rendered=0;if(now-rendered<40){delay(1);return;}rendered=now;
 if(!hockeyRender(s,plan,now))message("ERREUR HOCKEY","ANIMATION",RED);
 panel->flipDMABuffer();
}
