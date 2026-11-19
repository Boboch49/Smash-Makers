#pragma once
#include <SD_MMC.h>
#include <esp_heap_caps.h>
#include "hockey_media_catalog.h"
#include "strange_fast.h"
class HockeyAssets {
 hockey::View view_;const char*error_="NON CHARGE";
 void clear(){for(auto&p:view_.data){if(p)heap_caps_free(const_cast<uint8_t*>(p));p=nullptr;}view_.ready=false;}
public:
 const hockey::View&view()const{return view_;}
 const char*error()const{return error_;}
 bool begin(void(*progress)(const char*,unsigned)=nullptr){
  auto report=[&](const char*s,unsigned n){Serial.printf("HOCKEY %s %u%%\n",s,n);if(progress)progress(s,n);};
  if(!psramFound()){error_="PSRAM ABSENTE";report(error_,0);return false;}
  if(!SD_MMC.setPins(1,44,17)||!SD_MMC.begin("/sdcard",true,false,20000)){
   SD_MMC.end();if(!SD_MMC.setPins(1,44,17)||!SD_MMC.begin("/sdcard",true,false,10000)){error_="SD ABSENTE";SD_MMC.end();report(error_,0);return false;}
  }
  size_t total=0,readTotal=0;for(const auto&s:hockey::specs)total+=s.bytes;
  size_t chunk=32768;uint8_t*buf=static_cast<uint8_t*>(heap_caps_malloc(chunk,MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT));
  if(!buf){chunk=8192;buf=static_cast<uint8_t*>(heap_caps_malloc(chunk,MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT));}
  if(!buf){error_="TAMPON MEMOIRE";SD_MMC.end();report(error_,0);return false;}
  bool ok=true;unsigned previous=101;
  for(unsigned i=0;i<hockey::SLOT_COUNT&&ok;++i){
   const auto&s=hockey::specs[i];File f=SD_MMC.open(s.path,FILE_READ);
   if(!f||f.size()!=s.bytes){error_=f?"TAILLE FICHIER":"FICHIER ABSENT";Serial.println(s.path);ok=false;break;}
   uint8_t*p=static_cast<uint8_t*>(heap_caps_malloc(s.bytes,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));
   if(!p){error_="PSRAM PLEINE";ok=false;break;}view_.data[i]=p;view_.bytes[i]=s.bytes;
   size_t at=0;uint32_t crc=0xFFFFFFFF;
   while(at<s.bytes){size_t n=f.read(buf,min(chunk,s.bytes-at));if(!n){error_="LECTURE SD";ok=false;break;}
    crc=strange_fast::update(crc,buf,n);memcpy(p+at,buf,n);at+=n;readTotal+=n;
    unsigned percent=readTotal*100/total;if(previous==101||percent>=previous+5||percent==100){previous=percent;if(progress)progress("HOCKEY SD",percent);}delay(1);
   }
   f.close();if(ok&&~crc!=s.crc){error_="CRC FICHIER";Serial.println(s.path);ok=false;}
  }
  heap_caps_free(buf);SD_MMC.end();if(!ok){clear();report(error_,0);return false;}
  error_="OK";view_.ready=true;report("PRET",100);return true;
 }
};
static HockeyAssets hockeyAssets;
