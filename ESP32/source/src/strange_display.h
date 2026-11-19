#pragma once
#include "strange_media_catalog.h"

static uint16_t strangePixel(const uint8_t*p,size_t pixel){return uint16_t(p[pixel*2])|(uint16_t(p[pixel*2+1])<<8);}
static bool strangeAgainstAI(const State&s){return (s.mode==1&&s.winner==2)||(s.mode==3&&s.winner==1);}
static uint16_t strangeAge(const State&s,uint32_t now){
 uint32_t extra=uint32_t(now-s.time);if(extra>80)extra=0;
 return uint16_t(min(uint32_t(22280),uint32_t(s.worldAge)+extra));
}
static void strangeSprite(strange::Slot slot,int x,uint32_t now){
 const auto&m=strangeAssets.view();const uint8_t*p=m.data[slot]+((now/140)%8)*2048;
 for(unsigned y=0;y<32;++y)for(unsigned xx=0;xx<32;++xx){uint16_t c=strangePixel(p,y*32+xx);if(c)panel->drawPixel(x+xx,y,c);}
}
static bool strangeDisplay(const State&s,Scene scene,uint32_t now){
 const auto&m=strangeAssets.view();if(!m.ready)return false;
 if(s.gameEvent==2&&s.menu==makers::PLAYING){
  size_t f=min(size_t(strange::CLOCK_FRAMES-1),size_t(strangeAge(s,now)/40));
  const uint8_t*p=m.data[strange::CLOCK]+f*8192;
  for(unsigned y=0;y<32;++y)for(unsigned x=0;x<128;++x)panel->drawPixel(x,y,strangePixel(p,y*128+x));
  return true;
 }
 panel->fillScreen(0);bool inv=s.gameEvent==1;uint16_t c=inv?0xD8AD:0x05BF;
 strangeSprite(inv?strange::VECNA:strange::CHILD1,0,now);
 strangeSprite(inv?strange::DEMO:strange::CHILD2,96,now);
 label(s.menu==makers::RESULTS?(strangeAgainstAI(s)?"IA GAGNE":"VICTOIRE"):inv?"INVERSE":"J1   J2",0,c,1,32,96);
 char score[12];snprintf(score,sizeof(score),"%u:%u",s.score1,s.score2);
 label(score,10,WHITE,2,32,96);
 if(s.menu==makers::RESULTS)label(uint32_t(now-s.menuAt)>=6000?"MODE=JEU":strangeAgainstAI(s)?"DEFAITE":s.winner==1?"J1 GAGNE":"J2 GAGNE",25,c,1,32,96);
 else if(scene==POINT1||scene==POINT2)label(scene==POINT1?"POINT J1":"POINT J2",25,YELLOW,1,32,96);
 else if(inv){char left[8];snprintf(left,sizeof(left),"%u s",(8000-min(8000,int(strangeAge(s,now)))+999)/1000);label(left,25,c,1,32,96);}
 else label(s.flags&WAIT_SERVE?"SERVIR":"NORMAL",25,c,1,32,96);
 return true;
}
