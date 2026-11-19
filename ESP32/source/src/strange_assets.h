#pragma once
#include <Arduino.h>
#include <SD_MMC.h>
#include <esp_heap_caps.h>
#include "strange_media_catalog.h"
#include "strange_fast.h"

// Load after the matrix boot screen and before audio. Playback never touches SD.
class StrangeAssets {
  strange::View view_;
  const char* error_="NON CHARGEE";
  void release(){for(auto&p:view_.data){if(p)heap_caps_free(const_cast<uint8_t*>(p));p=nullptr;}view_.ready=false;}
public:
  const strange::View& view()const{return view_;}
  const char* error()const{return error_;}
  using Progress=void(*)(const char*,unsigned);
  bool begin(Progress progress=nullptr){
    auto report=[&](const char*s,unsigned percent){Serial.printf("SD: %s %u%%\n",s,percent);if(progress)progress(s,percent);};
    report("MEMOIRE",0);
    if(!psramFound()){error_="PSRAM ABSENTE";report(error_,0);return false;}
    if(!SD_MMC.setPins(1,44,17)){error_="BROCHES SD";report(error_,0);return false;}
    report("MONTAGE SD",0);
    if(!SD_MMC.begin("/sdcard",true,false,20000)){
      SD_MMC.end();Serial.println("SD: repli 10 MHz");
      if(!SD_MMC.begin("/sdcard",true,false,10000)){error_="SD ABSENTE";report(error_,0);SD_MMC.end();return false;}
    }
    error_="LECTURE SD";
    const uint32_t started=millis();size_t blockSize=32768;
    uint8_t*block=static_cast<uint8_t*>(heap_caps_malloc(blockSize,MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT));
    if(!block){blockSize=8192;block=static_cast<uint8_t*>(heap_caps_malloc(blockSize,MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT));}
    if(!block){error_="MEMOIRE TAMPON";report(error_,0);SD_MMC.end();return false;}
    bool ok=true;size_t total=0;
    unsigned lastPercent=0;
    for(unsigned i=0;i<strange::SLOT_COUNT&&ok;++i){
      const auto&spec=strange::specs[i];File f;bool compressed=false;
      if(i==strange::CLOCK){f=SD_MMC.open("/STRANGE/ANIMATIONS/horloge.smd",FILE_READ);compressed=bool(f);}
      if(!f)f=SD_MMC.open(spec.path,FILE_READ);
      if(!f||(!compressed&&f.size()!=spec.bytes)){error_=!f?"FICHIER ABSENT":"TAILLE FICHIER";Serial.printf("STRANGE: %s %s attendu=%u obtenu=%u\n",error_,spec.path,unsigned(spec.bytes),f?unsigned(f.size()):0);ok=false;break;}
      if(compressed){
        uint8_t h[16];
        if(f.size()<16||f.size()>spec.bytes*2+16||(f.size()-16)%4||f.read(h,16)!=16||memcmp(h,"SMD1",4)||strange_fast::read32(h+4)!=spec.bytes||strange_fast::read32(h+8)!=spec.crc||strange_fast::read32(h+12)!=4096){error_="FORMAT HORLOGE";ok=false;break;}
      }
      uint8_t*p=static_cast<uint8_t*>(heap_caps_malloc(spec.bytes,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));
      if(!p){error_="MEMOIRE PLEINE";ok=false;break;}
      view_.data[i]=p;view_.bytes[i]=spec.bytes;
      size_t read=0;uint32_t checksum=0xFFFFFFFFU;
      strange_fast::Delta decoder(p,spec.bytes);
      while((compressed&&f.available())||(!compressed&&read<spec.bytes)){
        size_t n=f.read(block,min(blockSize,compressed?size_t(f.available()):spec.bytes-read));
        if(!n){error_="LECTURE COUPEE";ok=false;break;}
        if(compressed){if(!decoder.push(block,n)){error_="DONNEES HORLOGE";ok=false;break;}read=decoder.produced();}
        else{checksum=strange_fast::update(checksum,block,n);memcpy(p+read,block,n);read+=n;}
        unsigned percent=unsigned((total+read)*100/strange::PRELOAD_BYTES);
        if(percent>=lastPercent+5||percent==100){lastPercent=percent;if(progress)progress("CHARGEMENT",percent);}
        // Return to FreeRTOS between bounded CRC chunks, including the clock.
        delay(1);
      }
      f.close();
      if(ok&&(compressed?!decoder.complete(spec.crc):~checksum!=spec.crc)){error_="CRC FICHIER";Serial.printf("STRANGE: CRC incorrect %s\n",spec.path);ok=false;}
      total+=read;
    }
    heap_caps_free(block);
    SD_MMC.end();
    if(!ok){release();report(error_,0);return false;}
    error_="OK";view_.ready=true;report("MEDIAS OK",100);Serial.printf("STRANGE: medias V7 precharges %u octets en %u ms; SD fermee\n",unsigned(total),unsigned(millis()-started));return true;
  }
};
static StrangeAssets strangeAssets;
