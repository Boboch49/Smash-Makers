#pragma once
#include <stdint.h>
#include "display_model.h"
#ifndef SMASH_STRIP_LEDS
#define SMASH_STRIP_LEDS 60
#endif
namespace smash_lights {
static constexpr unsigned COUNT=SMASH_STRIP_LEDS;
static_assert(COUNT>0&&COUNT<=300,"Supported strip length: 1..300");
struct RGB {uint8_t r,g,b;};
inline int high(int a,int b){return a>b?a:b;}
inline int pulse(int i,int pos,int width){int d=i-pos;if(d<0)d=-d;int v=255-d*255/width;return v>0?v:0;}
inline void render(const smash_link::State&s,smash_display::Scene scene,uint32_t age,uint32_t now,RGB*out){
 using namespace smash_display;using namespace smash_link;
 for(unsigned i=0;i<COUNT;++i){
  int r=i<COUNT/2?14:16,g=i<COUNT/2?1:10,b=1;
  switch(scene){
  case LOST:r=18;g=7;b=0;break;
  case SECURITY:case HEAT:{int p=int((age+800)%3200);int v=40+(p<1600?p:3200-p)*80/1600;r=v;g=0;b=0;break;}
  case PAUSE:r=2;g=6;b=30;break;
  case REFERENCE:case SERVICE_SCREEN:r=18;g=8;b=0;if(s.app==6){unsigned c=(now/1000)%3;r=c==0?100:0;g=c==1?100:0;b=c==2?100:0;}break;
  case BOOT:{int v=pulse(i,int((age/18)%(COUNT+12))-6,8);r=v;g=v*180/255;b=0;break;}
  case READY:r=i<COUNT/2?20:24;g=i<COUNT/2?1:18;b=2;break;
  case GO:{int pos=int(age*(COUNT+12)/960)-6;int v=pulse(i,pos,8);r=0;g=high(5,v*205/255);b=high(6,v*245/255);break;}
  case BOUNCE:{int origin=(s.event==1||s.event==16)?0:(s.event==2||s.event==17)?COUNT-1:COUNT/2;int dist=int(i)-origin;if(dist<0)dist=-dist;int v=pulse(dist,int(age*COUNT/400),5);r=v;g=(s.event==1||s.event==16)?v/10:v*160/255;b=(s.event==3||s.event==18)?v/8:0;break;}
  case POINT1:case POINT2:{int origin=scene==POINT1?0:COUNT-1;int dist=int(i)-origin;if(dist<0)dist=-dist;int v=high(8,pulse(dist,int(age*COUNT/700),8));r=v;g=scene==POINT2?v*205/255:v/20;b=0;break;}
  case BONUS:r=i<=age*COUNT/700?150:8;g=r*180/255;b=0;break;
  case JACKPOT1:case JACKPOT2:case CHAMPION1:case CHAMPION2:{int dist=int(2*i)-(int(COUNT)-1);if(dist<0)dist=-dist;int v=high(12,pulse(dist,int((age%1800)*(COUNT+10)/1800),9));r=v;g=v*205/255;b=0;break;}
  default:if((s.flags&WAIT_SERVE)&&((s.server==1&&i<COUNT/2)||(s.server==2&&i>=COUNT/2))){r+=10;g+=s.server==2?8:0;}break;
  }
  // Safety/status colours keep priority over all game themes.
  bool themed=s.app==0&&s.phase==RUN&&scene!=SECURITY&&scene!=HEAT&&scene!=PAUSE&&scene!=REFERENCE&&scene!=LOST&&scene!=SERVICE_SCREEN;
  uint8_t game=(s.menu==makers::CHOOSE_GAME||s.menu==makers::CHOOSE_PLAYERS)?s.selectedGame:s.game;
  if(themed&&game==makers::HOCKEY){
    int glow=12+pulse(i,int((now/40)%(COUNT+12))-6,9)/3;
    r=glow/3;g=glow*3/4;b=high(20,glow);
    if(scene==POINT1||scene==POINT2){r=120;g=20;b=10;}
    if(scene==CHAMPION1||scene==CHAMPION2){r=90;g=130;b=160;}
  }else if(themed&&game==makers::STRANGE){
    int p=int((now+s.menuAt)%3600),wave=14+(p<1800?p:3600-p)*30/1800;
    r=wave;g=0;b=s.gameEvent?wave/2:wave/4;
    if(s.gameEvent==2){int v=pulse(i,int((now/25)%(COUNT+12))-6,6);r=high(r,v/2);b=high(b,v/2);}
    if(scene==POINT1||scene==POINT2||s.gameEvent==1){r=high(r,pulse(i,int((now/20)%(COUNT+12))-6,8));b=r/2;}
  }
  out[i]={uint8_t(r),uint8_t(g),uint8_t(b)};
 }
}
}
