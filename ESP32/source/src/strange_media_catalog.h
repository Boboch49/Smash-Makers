#pragma once
#include <stdint.h>
#include <stddef.h>
namespace strange {
enum Slot:uint8_t {CLOCK,NORMAL,INVERSE,DEFEAT,POINT,CLOCK_AUDIO,CHILD1,CHILD2,VECNA,DEMO,INTRO_VIDEO,INTRO_AUDIO,SLOT_COUNT};
struct Spec {const char* path;size_t bytes;uint32_t crc;};
static const Spec specs[]={
 {"/STRANGE/ANIMATIONS/horloge.rgb565",4562944U,0x0C8755D1U},
 {"/STRANGE/AUDIO/ambiance_normale.pcm",904464U,0xA9BCF9BBU},
 {"/STRANGE/AUDIO/ambiance_inversee.pcm",984712U,0x5992A779U},
 {"/STRANGE/AUDIO/defaite_ia.pcm",192030U,0xC2E354A5U},
 {"/STRANGE/AUDIO/point_marque.pcm",186920U,0x9927D394U},
 {"/STRANGE/AUDIO/horloge.pcm",712574U,0x2ED96026U},
 {"/STRANGE/ANIMATIONS/J1_ENFANT.rgb565",16384U,0xAF31649CU},
 {"/STRANGE/ANIMATIONS/J2_FILLE.rgb565",16384U,0xAADD06A1U},
 {"/STRANGE/ANIMATIONS/J1_VECNA.rgb565",16384U,0x3F903FCEU},
 {"/STRANGE/ANIMATIONS/J2_DEMOGORGON.rgb565",16384U,0x03532CA8U},
 {"/STRANGE/INTRO/intro_complete.smd",3244112U,0x2C8159ECU},
 {"/STRANGE/INTRO/intro_complete.pcm",1711218U,0x262E741AU},
};
constexpr uint32_t CLOCK_MS=22280,INVERSE_MS=8000;
constexpr uint32_t INTRO_MS=53480;
constexpr size_t PRELOAD_BYTES=12564510U;
constexpr size_t CLOCK_FRAMES=557,FRAME_PIXELS=4096,SPRITE_PIXELS=1024;
struct View {const uint8_t* data[SLOT_COUNT]={};size_t bytes[SLOT_COUNT]={};bool ready=false;};
inline uint32_t crc32(const uint8_t*p,size_t n){uint32_t c=0xFFFFFFFF;while(n--){c^=*p++;for(int k=0;k<8;++k)c=(c>>1)^((c&1)?0xEDB88320U:0);}return ~c;}
}
