#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "strange_fast.h"

namespace strange_intro {
inline uint32_t age(uint32_t now,uint32_t started){int32_t delta=int32_t(now-started);return delta<0?0:uint32_t(delta)>53479?53479:uint32_t(delta);}
// Keep only the current 128x32 frame; the full intro stays compressed in PSRAM.
class Player {
 const uint8_t* data=nullptr;size_t size=0,offset=16;
 uint32_t count=0,total=0,checksum=0xFFFFFFFFU,expected=0;
 bool valid=false;uint8_t frame[8192]={};
 void rewind(){offset=16;count=0;checksum=0xFFFFFFFFU;memset(frame,0,sizeof(frame));}
public:
 bool boundTo(const uint8_t*p)const{return data==p;}
 bool begin(const uint8_t*p,size_t n){
  data=p;size=n;valid=false;
  if(!p||n<16||memcmp(p,"SMD1",4)||(n-16)%4||strange_fast::read32(p+12)!=4096)return false;
  uint32_t bytes=strange_fast::read32(p+4);
  if(!bytes||bytes%8192||bytes/8192>2000)return false;
  total=bytes/8192;expected=strange_fast::read32(p+8);rewind();return valid=true;
 }
 bool frameFor(uint32_t milliseconds){
  if(!valid)return false;
  uint32_t wanted=milliseconds/40;if(wanted>=total)wanted=total-1;
  if(count&&wanted<count-1)rewind();
  while(count<=wanted){
   size_t pixel=0;
   while(pixel<4096){
    if(offset+4>size)return valid=false;
    uint16_t run=uint16_t(data[offset])|(uint16_t(data[offset+1])<<8);
    uint16_t diff=uint16_t(data[offset+2])|(uint16_t(data[offset+3])<<8);offset+=4;
    if(!run||run>4096-pixel)return valid=false;
    while(run--){size_t p=pixel++*2;frame[p]^=uint8_t(diff);frame[p+1]^=uint8_t(diff>>8);}
   }
   checksum=strange_fast::update(checksum,frame,sizeof(frame));++count;
   if(count==total&&(offset!=size||~checksum!=expected))return valid=false;
  }
  return true;
 }
 const uint8_t* pixels()const{return frame;}
};
}
