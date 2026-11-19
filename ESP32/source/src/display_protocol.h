#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "game_catalog.h"
#include "hockey_events.h"
namespace smash_link {
constexpr size_t SIZE=108;
constexpr uint32_t WELCOME_MS=4800,LINK_TIMEOUT_MS=600;
enum Phase:uint8_t {SYNC=0,SCHEDULED=1,WELCOME=2,RUN=3};
enum Flags:uint16_t {FAULT=1,PAUSED=2,REFERENCED=4,WAIT_SERVE=8,MATCH_OVER=16,THERMAL=32,HOT=64,TMC_FAULT=128,REPOSITION=256,SERVICE=512,NEEDS_ARM=1024,SET_OVER=2048,MATCH_POINT=4096,SET_POINT=8192,LINK_WAIT=16384,CLOCK_VALID=32768};
struct State {uint16_t seq=0,flags=0,score1=0,score2=0,eventId=0,rally=0;uint8_t app=0,mode=0,event=255,winner=0,sets1=0,sets2=0,server=1,setIndex=1,firstServer=1,setsToWin=2,target=11;uint32_t session=0,time=0,eventAt=0,epoch=0,phaseAt=0;int32_t offset=0;uint16_t ackSeq=0,stripLimitMa=2000;uint8_t phase=SYNC,stripBrightness=100,selectedMode=1,matrixBrightness=10;uint16_t toneId=0,toneHz=0,toneMs=0,stopId=0;uint32_t toneAt=0;char text[16]={};uint8_t game=0,selectedGame=0,menu=makers::PLAYING,gameEvent=0;uint16_t remainingSeconds=0,worldAge=0;uint32_t menuAt=0,hockeyAt=0;uint8_t hockeyCue=0;};
inline uint16_t crc(const uint8_t*p,size_t n){uint16_t c=0xFFFF;while(n--){c^=uint16_t(*p++)<<8;for(int i=0;i<8;i++)c=c&0x8000?(c<<1)^0x1021:c<<1;}return c;}
inline void put(uint8_t*p,uint16_t v){p[0]=v;p[1]=v>>8;}
inline uint16_t get(const uint8_t*p){return uint16_t(p[0])|(uint16_t(p[1])<<8);}
inline void put32(uint8_t*p,uint32_t v){put(p,uint16_t(v));put(p+2,uint16_t(v>>16));}
inline uint32_t get32(const uint8_t*p){return uint32_t(get(p))|(uint32_t(get(p+2))<<16);}
inline void encode(const State&s,uint8_t*p){memset(p,0,SIZE);p[0]=0xA5;p[1]=0x5A;p[2]=9;p[3]=102;put(p+4,s.seq);put(p+6,s.flags);p[8]=s.app;p[9]=s.mode;put(p+10,s.score1);put(p+12,s.score2);put(p+14,s.eventId);p[16]=s.event;p[17]=s.winner;p[18]=s.sets1;p[19]=s.sets2;p[20]=s.server;p[21]=s.setIndex;p[22]=s.firstServer;p[23]=s.setsToWin;p[24]=s.target;p[25]=s.phase;put(p+26,s.rally);memcpy(p+28,s.text,15);put32(p+44,s.session);put32(p+48,s.time);put32(p+52,s.eventAt);put32(p+56,s.epoch);put32(p+60,uint32_t(s.offset));put(p+64,s.ackSeq);put32(p+66,s.phaseAt);p[70]=s.stripBrightness;put(p+71,s.stripLimitMa);p[43]=s.selectedMode;put(p+73,s.toneId);put32(p+75,s.toneAt);put(p+79,s.toneHz);put(p+81,s.toneMs);put(p+83,s.stopId);p[85]=s.matrixBrightness;p[86]=s.game;p[87]=s.selectedGame;p[88]=s.menu;p[89]=s.gameEvent;put(p+90,s.remainingSeconds);put32(p+92,s.menuAt);put(p+96,s.worldAge);p[98]=s.hockeyCue;put32(p+100,s.hockeyAt);put(p+106,crc(p,106));}
inline bool decode(const uint8_t*p,State&s){
 if(p[0]!=0xA5||p[1]!=0x5A||p[2]!=9||p[3]!=102||get(p+106)!=crc(p,106)||p[98]>=hockey_event::COUNT||p[99]||p[104]||p[105]||p[86]>=makers::GAME_COUNT||p[87]>=makers::GAME_COUNT||p[88]>makers::RESULTS||p[89]>2||get(p+90)>900||get(p+96)>22280||p[8]>8||p[9]>3||(p[16]>19&&p[16]!=255)||p[17]>2||p[18]>2||p[19]>2||p[18]+p[19]>3||p[20]<1||p[20]>2||p[21]<1||p[21]>3||p[22]<1||p[22]>2||p[23]<1||p[23]>2||p[24]<2||p[24]>99||p[25]>RUN||p[70]>150||get(p+71)>12000||p[43]<1||p[43]>3||get(p+79)>8000||get(p+81)>5000||p[85]>100)return false;
 State t;t.hockeyCue=p[98];t.hockeyAt=get32(p+100);t.game=p[86];t.selectedGame=p[87];t.menu=p[88];t.gameEvent=p[89];t.remainingSeconds=get(p+90);t.menuAt=get32(p+92);t.worldAge=get(p+96);t.selectedMode=p[43];t.toneId=get(p+73);t.toneAt=get32(p+75);t.toneHz=get(p+79);t.toneMs=get(p+81);t.stopId=get(p+83);t.matrixBrightness=p[85];t.seq=get(p+4);t.flags=get(p+6);t.app=p[8];t.mode=p[9];t.score1=get(p+10);t.score2=get(p+12);t.eventId=get(p+14);t.event=p[16];t.winner=p[17];t.sets1=p[18];t.sets2=p[19];t.server=p[20];t.setIndex=p[21];t.firstServer=p[22];t.setsToWin=p[23];t.target=p[24];t.rally=get(p+26);t.phase=p[25];t.session=get32(p+44);t.time=get32(p+48);t.eventAt=get32(p+52);t.epoch=get32(p+56);t.offset=int32_t(get32(p+60));t.ackSeq=get(p+64);t.phaseAt=get32(p+66);t.stripBrightness=p[70];t.stripLimitMa=get(p+71);memcpy(t.text,p+28,15);for(unsigned i=0;i<15;i++)if(t.text[i]&&(t.text[i]<32||t.text[i]>126))return false;s=t;return true;
}
class Parser {uint8_t data[SIZE]={};size_t used=0;public:void reset(){used=0;}bool push(uint8_t b,State&s){if(!used&&b!=0xA5)return false;data[used++]=b;if(used<SIZE)return false;if(decode(data,s)){used=0;return true;}memmove(data,data+1,SIZE-1);used=SIZE-1;return false;}};

constexpr size_t ACK_SIZE=30;
enum AckKind:uint8_t {HELLO=1,TIME_ACK=2,ACK_READY=3};
enum Resources:uint8_t {MATRIX_READY=1,STRIP_READY=2,PRESENTING=4,AUDIO_READY=8};
struct Ack {uint8_t kind=HELLO,resources=0;uint16_t seq=0,echoSeq=0;uint32_t session=0,sent=0,received=0,replied=0;};
inline void encodeAck(const Ack&a,uint8_t*p){memset(p,0,ACK_SIZE);p[0]=0x5A;p[1]=0xA5;p[2]=9;p[3]=24;p[4]=a.kind;p[5]=a.resources;put(p+6,a.seq);put32(p+8,a.session);put(p+12,a.echoSeq);put32(p+16,a.sent);put32(p+20,a.received);put32(p+24,a.replied);put(p+28,crc(p,28));}
inline bool decodeAck(const uint8_t*p,Ack&a){if(p[0]!=0x5A||p[1]!=0xA5||p[2]!=9||p[3]!=24||p[4]<HELLO||p[4]>ACK_READY||(p[5]&~15)||get32(p+8)==0||get(p+28)!=crc(p,28))return false;Ack t;t.kind=p[4];t.resources=p[5];t.seq=get(p+6);t.session=get32(p+8);t.echoSeq=get(p+12);t.sent=get32(p+16);t.received=get32(p+20);t.replied=get32(p+24);a=t;return true;}
class AckParser {uint8_t data[ACK_SIZE]={};size_t used=0;public:void reset(){used=0;}bool push(uint8_t b,Ack&a){if(!used&&b!=0x5A)return false;data[used++]=b;if(used<ACK_SIZE)return false;if(decodeAck(data,a)){used=0;return true;}memmove(data,data+1,ACK_SIZE-1);used=ACK_SIZE-1;return false;}};
}
