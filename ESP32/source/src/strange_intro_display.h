#pragma once
#include "strange_intro_player.h"
static strange_intro::Player strangeIntroPlayer;
static bool strangeIntroDraw(uint32_t elapsed){
 const auto&m=strangeAssets.view();if(!m.ready)return false;
 if(!strangeIntroPlayer.boundTo(m.data[strange::INTRO_VIDEO])&&
    !strangeIntroPlayer.begin(m.data[strange::INTRO_VIDEO],m.bytes[strange::INTRO_VIDEO]))return false;
 if(!strangeIntroPlayer.frameFor(elapsed))return false;
 const uint8_t*p=strangeIntroPlayer.pixels();
 for(unsigned y=0;y<32;++y)for(unsigned x=0;x<128;++x)panel->drawPixel(x,y,strangePixel(p,y*128+x));
 return true;
}
static void gameUsbVolume(){
 static char line[20];static unsigned used=0;
 for(unsigned n=0;n<32&&Serial.available();++n){
  char c=Serial.read();
  if(c=='\r'||c=='\n'){
   line[used]=0;used=0;
   if(!strncmp(line,"VOL ",4)){
    bool valid=line[4]!=0;unsigned v=0;
    for(unsigned k=4;line[k];++k){if(line[k]<'0'||line[k]>'9'){valid=false;break;}v=v*10+unsigned(line[k]-'0');if(v>100){valid=false;break;}}
    if(valid){audio.setMasterVolume(v);Serial.printf("VOLUME=%u\n",audio.volume());}else Serial.println("VOL 0..100");
   }else if(line[0])Serial.println("JEU SKR: commandes de jeu sur la SKR; VOL 0..100 ici.");
  }else if(used<sizeof(line)-1&&c>=' '&&c<='~')line[used++]=c>='a'&&c<='z'?c-32:c;
 }
}
