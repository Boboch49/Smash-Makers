#pragma once
#include "game_catalog.h"

// Small original synthesized beds for the two V1 themes. The supplied full
// reference recording is intentionally a separate future SD asset.
class ThemeSynth {
 uint32_t tick=0,phase=0,sub=0,noise=0x81934A71;
 static int32_t triangle(uint32_t p){int32_t v=(p>>16)&65535;return v<32768?v*2-32768:98303-v*2;}
public:
 void reset(){tick=phase=sub=0;}
 int16_t sample(makers::Game game){
  ++tick;noise^=noise<<13;noise^=noise>>17;noise^=noise<<5;
  uint32_t slot=(tick/4000)%8,position=tick%4000;
  const uint16_t hockey[8]={220,330,440,330,262,392,524,392};
  const uint16_t strange[8]={110,165,131,196,98,147,123,185};
  uint16_t hz=game==makers::HOCKEY?hockey[slot]:strange[slot];
  phase+=uint32_t(uint64_t(hz)*4294967296ULL/16000);
  sub+=uint32_t(uint64_t(game==makers::HOCKEY?55:49)*4294967296ULL/16000);
  int32_t envelope=4000-position;
  int32_t note=triangle(phase)*envelope/4000;
  if(game==makers::HOCKEY)return int16_t(note/5+triangle(sub)/9+int16_t(noise>>16)/30);
  uint32_t slow=(tick/32)%3200;int32_t breath=slow<1600?slow:3200-slow;
  return int16_t(note/7+triangle(sub)*(600+breath)/9000);
 }
};
