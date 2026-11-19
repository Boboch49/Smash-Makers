#pragma once
#include "hockey_presentation.h"
#include "hockey_font.h"
#include "strange_intro_player.h"
static strange_intro::Player hockeyPlayer;
static bool hockeyPresentationFailed=false;
static unsigned hockeyAdvance(char c){return c=='M'||c=='N'||c=='B'?6:5;}
static unsigned hockeyWidth(const char*s){unsigned w=0;for(;*s;++s)w+=hockeyAdvance(*s);return w;}
static void hockeyText(const char*s,int y,uint16_t c,int left=32,int right=96){
 int x=left+(right-left-int(hockeyWidth(s)))/2;if(x<left)x=left;
 for(;*s&&x<right;x+=hockeyAdvance(*s),++s){unsigned code=uint8_t(*s);if(code<32||code>95)continue;
  for(unsigned yy=0;yy<8&&y+int(yy)<32;++yy)for(unsigned xx=0;xx<5&&x+int(xx)<right;++xx)if(hockeyFont[code-32][yy]&(1<<xx))panel->drawPixel(x+xx,y+yy,c);
 }
}
static void hockeyScore(const char*s,int y,uint8_t scale){
 int x=32+(64-int(strlen(s))*6*scale)/2;
 static const uint8_t two[7]={14,17,16,8,4,2,31};
 for(;*s;++s,x+=6*scale){
  if(*s!='2')panel->drawChar(x,y,*s,WHITE,WHITE,scale);
  else for(unsigned yy=0;yy<7;++yy)for(unsigned xx=0;xx<5;++xx)if(two[yy]&(1<<xx))
   panel->fillRect(x+xx*scale,y+yy*scale,scale,scale,WHITE);
 }
}
// Dedicated 5x7 rectangular timer digits. Do not change the approved general font or scores.
static const uint8_t hockeyTimerDigits[10][7]={
 {31,17,17,17,17,17,31},{4,6,4,4,4,4,14},
 {31,16,16,31,1,1,31},{31,16,16,31,16,16,31},
 {17,17,17,31,16,16,16},{31,1,1,31,16,16,31},
 {31,1,1,31,17,17,31},{31,16,16,16,16,16,16},
 {31,17,17,31,17,17,31},{31,17,17,31,16,16,31}
};
static bool hockeyIsTimer(const char*s){
 unsigned colons=0,digits=0;for(;*s;++s){if(*s==':')++colons;else if(*s>='0'&&*s<='9')++digits;else return false;}
 return colons==1&&digits>=3&&digits<=4;
}
static void hockeyTimer(const char*s,uint16_t c){
 unsigned width=0;for(const char*p=s;*p;++p)width+=*p==':'?3:6;
 int x=32+(64-int(width-1))/2;
 for(;*s;++s){
  if(*s==':'){panel->drawPixel(x,3,c);panel->drawPixel(x,5,c);x+=3;continue;}
  if(*s<'0'||*s>'9')continue;
  for(unsigned y=0;y<7;++y)for(unsigned xx=0;xx<5;++xx)if(hockeyTimerDigits[*s-'0'][y]&(1<<xx))panel->drawPixel(x+xx,y+1,c);
  x+=6;
 }
}
static bool hockeyRender(const State&s,hockey::Plan plan,uint32_t now){
 const auto&m=hockeyAssets.view();if(!m.ready)return false;
 const auto&spec=hockey::specs[plan.id];uint32_t age=spec.loop?plan.age%spec.duration:min(plan.age,spec.duration-1);
 if(!hockeyPlayer.boundTo(m.data[plan.id])&&!hockeyPlayer.begin(m.data[plan.id],m.bytes[plan.id])){hockeyPresentationFailed=true;return false;}
 if(!hockeyPlayer.frameFor(age)){hockeyPresentationFailed=true;return false;}
 const uint8_t*p=hockeyPlayer.pixels();
 for(unsigned y=0;y<32;++y)for(unsigned x=0;x<128;++x)panel->drawPixel(x,y,strangePixel(p,y*128+x));
 if(plan.id==hockey::TITLE)return true;
 // Preserve the entire original goal animation; score returns after 960 ms.
 if(plan.id==hockey::BUT_J1||plan.id==hockey::BUT_J2){
  hockeyText(plan.id==hockey::BUT_J1?"BUT J1 !":"BUT J2 !",0,WHITE);return true;
 }
 uint16_t orange=0xFCA2,cyan=0x06FE,tone=WHITE;
 char top[20],bottom[20];snprintf(top,sizeof(top),"%u:%02u",s.remainingSeconds/60,s.remainingSeconds%60);strcpy(bottom,"EN JEU");
 switch(plan.id){
 case hockey::SOLO_J1:case hockey::SOLO_J2:case hockey::DUEL:strcpy(top,"CHOIX MODE");strcpy(bottom,plan.id==hockey::SOLO_J1?"J1 START":plan.id==hockey::SOLO_J2?"J2 START":"J1/J2 GO");break;
 case hockey::ATTENTE_J1:strcpy(top,"PRET J1");strcpy(bottom,s.flags&smash_link::REPOSITION?"PLACEMENT":"J1 SERVIR");tone=orange;break;
 case hockey::ATTENTE_J2:strcpy(top,"PRET J2");strcpy(bottom,s.flags&smash_link::REPOSITION?"PLACEMENT":"J2 SERVIR");tone=cyan;break;
 case hockey::ENGAGEMENT:strcpy(top,"ENGAGEMENT");bottom[0]=0;break;
 case hockey::TIR_J1:strcpy(top,"TIR J1");bottom[0]=0;tone=orange;break;
 case hockey::TIR_J2:strcpy(top,"TIR J2");bottom[0]=0;tone=cyan;break;
 case hockey::PARADE_J1:strcpy(top,"ARRET J1");bottom[0]=0; // Keep the complete puck visible at y=26..29.
 tone=orange;break;
 case hockey::PARADE_J2:strcpy(top,"ARRET J2");bottom[0]=0; // Keep the complete puck visible at y=26..29.
 tone=cyan;break;
 case hockey::POTEAU:strcpy(top,s.hockeyCue==hockey_event::BOARD?"REBOND":"POTEAU !");bottom[0]=0;break;
 case hockey::BUT_J1:strcpy(top,"BUT J1 !");strcpy(bottom,"+1 POINT");tone=orange;break;
 case hockey::BUT_J2:strcpy(top,"BUT J2 !");strcpy(bottom,"+1 POINT");tone=cyan;break;
 case hockey::DIX_SECONDES:strcpy(bottom,"FIN PROCHE");tone=orange;break;
 case hockey::FIN_CHRONO:strcpy(bottom,s.remainingSeconds?"DERNIERES S":"FIN DU TEMPS");tone=orange;break;
 case hockey::VICTOIRE_J1:strcpy(top,"J1 GAGNE");strcpy(bottom,"VICTOIRE");tone=orange;break;
 case hockey::VICTOIRE_J2:strcpy(top,"J2 GAGNE");strcpy(bottom,"VICTOIRE");tone=cyan;break;
 case hockey::DEFAITE_SOLO_J1:case hockey::DEFAITE_SOLO_J2:strcpy(top,"IA GAGNE");strcpy(bottom,"DEFAITE");tone=plan.id==hockey::DEFAITE_SOLO_J1?cyan:orange;break;
 case hockey::EGALITE:strcpy(top,"0:00");strcpy(bottom,"EGALITE");break;
 case hockey::PAUSE:strcpy(top,"PAUSE");strcpy(bottom,"ATTENTE");tone=YELLOW;break;
 case hockey::RETOUR_MENU:strcpy(top,"PARTIE FINIE");strcpy(bottom,"MODE=JEU");break;
 default:break;
 }
 // The complete background was redrawn above. Text is transparent; never erase the rink.
 if(hockeyIsTimer(top))hockeyTimer(top,tone);else hockeyText(top,0,tone);
 if(plan.id==hockey::SOLO_J1||plan.id==hockey::SOLO_J2||plan.id==hockey::DUEL)label(plan.id==hockey::DUEL?"DUEL":"SOLO",10,WHITE,2,32,96);
 else{char score[16];snprintf(score,sizeof(score),"%u:%u",s.score1,s.score2);hockeyScore(score,strlen(score)>5?12:9,strlen(score)>5?1:2);}
 hockeyText(bottom,27,tone);
 unsigned human=hockey::human(s);hockeyText(human==2?"IA":"J1",2,orange,0,29);hockeyText(human==1?"IA":"J2",2,cyan,99,128);
 (void)now;return true;
}
static bool hockeyDisplay(const State&s,uint32_t now){return hockeyRender(s,hockey::presentation(s,now),now);}
