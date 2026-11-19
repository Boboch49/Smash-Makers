#pragma once
#include "strange_intro_player.h"
// Intro player is shared with the live game renderer.
static void strangeAudioControl(const State&s,bool live,bool locked,uint32_t now){
 StrangeAudioState state;
 uint8_t wanted=(s.menu==makers::CHOOSE_GAME||s.menu==makers::CHOOSE_PLAYERS)?s.selectedGame:s.game;
 state.enabled=strangeAssets.view().ready&&wanted==makers::STRANGE;
 state.blocked=!live||!locked||s.phase!=RUN||s.app!=0||
  (s.flags&(smash_link::FAULT|smash_link::PAUSED|smash_link::HOT|smash_link::TMC_FAULT|smash_link::LINK_WAIT));
 state.intro=s.game==makers::STRANGE&&s.menu==makers::INTRO;
 state.clock=s.gameEvent==2&&s.menu==makers::PLAYING;
 state.inverted=s.gameEvent==1&&s.menu==makers::PLAYING;
 state.clockAge=state.intro?uint16_t(strange_intro::age(now,s.menuAt)):strangeAge(s,now);state.session=s.session;
 state.event=s.event;state.eventId=s.eventId;
 uint32_t age=uint32_t(now-s.eventAt);state.eventAge=uint16_t(min(age,uint32_t(65535)));
 state.defeat=s.menu==makers::RESULTS&&strangeAgainstAI(s);
 audio.mediaControl(state);
}
static void strangeShortEffects(const State&s,bool live,bool locked,uint32_t now){
 static uint32_t session=0;static uint16_t previous=0;static bool seen=false;
 if(s.session!=session){session=s.session;seen=false;}
 if(!live||!locked||s.phase!=RUN||(s.flags&(smash_link::FAULT|smash_link::PAUSED|smash_link::HOT|smash_link::TMC_FAULT))||s.gameEvent==2||s.menu==makers::INTRO)return;
 if(seen&&previous==s.eventId)return;seen=true;previous=s.eventId;
 uint32_t age=uint32_t(now-s.eventAt);
 if(age>150||s.event==uint8_t(Sfx::SCORE_LEFT)||s.event==uint8_t(Sfx::SCORE_RIGHT)||s.event==uint8_t(Sfx::VICTORY)||s.event==uint8_t(Sfx::MATCH_POINT)||s.event==uint8_t(Sfx::POWERUP))return;
 if(s.event<uint8_t(Sfx::COUNT))audio.play(Sfx(s.event),uint16_t(age));
}
static void strangeStandalone(uint32_t local){
 static uint32_t start=local,last=0;
 static uint32_t commandSession=1;static char command[20];static uint8_t used=0;static bool announced=false;
 if(!announced){announced=true;Serial.println("TEST STRANGE SD: REPLAY INTRO NORMAL POINT HORLOGE INVERSE DEFAITE VOL 0..100 LIST");}
 for(unsigned n=0;n<40&&Serial.available();++n){
  char c=Serial.read();
  if(c=='\r'||c=='\n'){
   command[used]=0;used=0;uint32_t offset=0;bool change=true;
   if(!strncmp(command,"VOL ",4)){
    bool digits=command[4]!=0;unsigned v=0;for(unsigned k=4;command[k];++k){if(command[k]<'0'||command[k]>'9'){digits=false;break;}v=v*10+unsigned(command[k]-'0');if(v>100){digits=false;break;}}
    if(digits){audio.setMasterVolume(v);Serial.printf("VOLUME=%u\n",audio.volume());}else Serial.println("VOL 0..100");continue;
   }
   if(!strcmp(command,"REPLAY")||!strcmp(command,"INTRO"))offset=0;
   else if(!strcmp(command,"NORMAL"))offset=strange::INTRO_MS;
   else if(!strcmp(command,"POINT"))offset=strange::INTRO_MS+8000;
   else if(!strcmp(command,"HORLOGE"))offset=strange::INTRO_MS+12000;
   else if(!strcmp(command,"INVERSE"))offset=strange::INTRO_MS+34280;
   else if(!strcmp(command,"DEFAITE"))offset=strange::INTRO_MS+47280;
   else change=false;
   if(change){start=local-offset;commandSession+=17;Serial.printf("TEST: %s\n",command);}
   else if(command[0])Serial.println("REPLAY INTRO NORMAL POINT HORLOGE INVERSE DEFAITE VOL 0..100 LIST");
  }else if(used<sizeof(command)-1&&c>=' '&&c<='~')command[used++]=c>='a'&&c<='z'?c-32:c;
 }
 if(!panelReady||!audioReady||!audio.healthy()||!strangeAssets.view().ready){
  if(panelReady){message("TEST STRANGE",strangeAssets.view().ready?"ERREUR AUDIO":strangeAssets.error(),RED);panel->flipDMABuffer();}delay(10);return;
 }
 if(uint32_t(local-last)<40){delay(1);return;}last=local;
 uint32_t elapsed=local-start;
 if(elapsed<strange::INTRO_MS){
  const auto&m=strangeAssets.view();
  if(!strangeIntroPlayer.boundTo(m.data[strange::INTRO_VIDEO]))strangeIntroPlayer.begin(m.data[strange::INTRO_VIDEO],m.bytes[strange::INTRO_VIDEO]);
  StrangeAudioState intro;intro.enabled=true;intro.blocked=false;intro.intro=true;intro.clockAge=uint16_t(elapsed);intro.session=commandSession;audio.mediaControl(intro);
  if(!strangeIntroPlayer.frameFor(elapsed)){message("ERREUR INTRO","FICHIER",RED);panel->flipDMABuffer();return;}
  const uint8_t*p=strangeIntroPlayer.pixels();
  for(unsigned y=0;y<32;++y)for(unsigned x=0;x<128;++x)panel->drawPixel(x,y,strangePixel(p,y*128+x));
  panel->flipDMABuffer();return;
 }
 constexpr uint32_t PERIOD=54280;uint32_t t=(elapsed-strange::INTRO_MS)%PERIOD;
 State s;s.session=commandSession+(elapsed-strange::INTRO_MS)/PERIOD;s.phase=RUN;s.time=local;s.game=s.selectedGame=makers::STRANGE;
 s.menu=makers::PLAYING;s.mode=1;s.selectedMode=1;s.flags=smash_link::REFERENCED;s.score1=t>=8000?1:0;
 Scene scene=SCORE;
 if(t>=8000&&t<12000){s.eventId=1;s.event=uint8_t(Sfx::SCORE_LEFT);s.eventAt=local-(t-8000);scene=POINT1;}
 if(t>=12000&&t<34280){s.gameEvent=2;s.worldAge=uint16_t(t-12000);}
 if(t>=34280&&t<42280){s.gameEvent=1;s.worldAge=uint16_t(t-34280);}
 if(t>=47280){s.menu=makers::RESULTS;s.menuAt=local-(t-47280);s.winner=2;s.flags|=smash_link::MATCH_OVER;s.eventId=2;}
 strangeAudioControl(s,true,true,local);strangeDisplay(s,scene,local);panel->flipDMABuffer();
}
