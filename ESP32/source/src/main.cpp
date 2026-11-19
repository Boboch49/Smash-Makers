#include <Arduino.h>
#include "radio_off.h"
#include "display_transport.h"
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "config.h"
#include "display_model.h"
#include "animations_index.h"
#include "presentation.h"
#include "animation_data.h"
#include "animations_flipper_index.h"
#include "pinball_data.h"
#include "rle_decoder.h"
#include "strange_assets.h"
#include "hockey_assets.h"
#include "audio_output.h"
#include "audio_sync.h"
static AudioOutput audio;
static AudioSync audioSync;
#ifndef SMASH_HOCKEY_TEST
#define SMASH_HOCKEY_TEST 0
#endif
#ifndef SMASH_STRANGE_TEST
#define SMASH_STRANGE_TEST 0
#endif
#ifndef SMASH_STRANGE_CHECK
#define SMASH_STRANGE_CHECK 0
#endif
#ifndef SMASH_DEMO
#define SMASH_DEMO 0
#endif
#ifndef SMASH_TEST
#define SMASH_TEST 0
#endif
using namespace smash_assets;
using namespace smash_display;
MatrixPanel_I2S_DMA* panel=nullptr;
HardwareSerial skr(2);Model model;DisplayTransport transport;
static bool audioReady=false;static bool radiosOff=false;static bool panelReady=false;static bool presentationStarted=false;static uint8_t presentedPhase=SYNC;static uint32_t presentedSession=0;
static const uint16_t WHITE=0xFFFF,RED=0xE061,YELLOW=0xFDA1,BLUE=0x05BF,AMBER=0xFBE0;
static bool frame(const Animation&a,uint32_t elapsed){
 const Frame&f=a.frames[(elapsed/a.duration_ms)%a.count];
 if(f.offset>sizeof(animationData)||f.byte_length>sizeof(animationData)-f.offset)return false;
 return drawFrame(animationData+f.offset,f.byte_length,[](uint16_t x,uint16_t y,uint16_t c){panel->drawPixel(x,y,c);});
}
static bool pinFrame(const smash_pinball::Animation&a,uint32_t elapsed,int player=0,bool once=false){
 uint16_t n=once?min(uint32_t(a.count-1),elapsed*uint32_t(a.count)/960):(elapsed/a.frame_ms)%a.count;
 // The source gallery alternates players; a live celebration stays with its winner.
 if(player)n=uint16_t((player==2?32:0)+(elapsed/a.frame_ms)%32);
 const smash_pinball::Frame&f=a.frames[n];
 if(f.offset>sizeof(pinballData)||f.bytes>sizeof(pinballData)-f.offset)return false;
 return drawFrame(pinballData+f.offset,f.bytes,[](uint16_t x,uint16_t y,uint16_t c){panel->drawPixel(x,y,c);});
}
static void label(const char*s,int y,uint16_t color,uint8_t scale=1,int left=0,int right=128){
 panel->setTextWrap(false);panel->setTextSize(scale);panel->setTextColor(color);
 int x=left+(right-left-int(strlen(s))*6*scale)/2;if(x<left)x=left;
 panel->setCursor(x,y);panel->print(s);
}
static void message(const char*a,const char*b,uint16_t c){panel->fillScreen(0);label(a,3,c);label(b,19,WHITE);}
static void footer(const State&s,int left=29,int right=99){
 char text[24];snprintf(text,sizeof(text),"%u:%u M%u:%u",s.score1,s.score2,s.sets1,s.sets2);
 if(int(strlen(text))*6>right-left)snprintf(text,sizeof(text),"%u:%u",s.score1,s.score2);
 panel->fillRect(left,25,right-left,7,0);label(text,25,WHITE,1,left,right);
}
static void scores(const State&s,uint32_t now,bool bumper=false){
 frame(score,now);panel->fillRect(27,0,75,32,0);
 if(bumper){panel->drawCircle(33,17,5,RED);panel->drawCircle(64,17,5,AMBER);panel->drawCircle(95,17,5,BLUE);}
 char a[6],b[6];snprintf(a,sizeof(a),"%u",s.score1);snprintf(b,sizeof(b),"%u",s.score2);
 uint8_t scale=s.score1<100&&s.score2<100?2:1;
 panel->setTextSize(scale);panel->setTextColor(WHITE);panel->setCursor(59-strlen(a)*6*scale,9);panel->print(a);
 panel->setTextSize(1);panel->setCursor(61,10);panel->print(':');
 panel->setTextSize(scale);panel->setCursor(70,9);panel->print(b);
 panel->setTextSize(1);panel->setTextColor(RED);panel->setCursor(31,0);panel->print(s.mode==3?"IA":"J1");
 panel->setTextColor(YELLOW);panel->setCursor(83,0);panel->print(s.mode==1?"IA":"J2");
 char status[16];snprintf(status,sizeof(status),"M%u:%u S%u",s.sets1,s.sets2,s.server);label(status,25,WHITE,1,29,99);
 // Top middle shows placement / next server or a set/match point warning.
 if((s.flags&(MATCH_POINT|SET_POINT))&&!(s.flags&REPOSITION))label(s.flags&MATCH_POINT?"MATCH":"SET",0,AMBER,1,44,82);
 else if(s.flags&WAIT_SERVE)label(s.flags&REPOSITION?"PLACE":"PRET",0,BLUE,1,44,82);
}
static void celebration(const State&s,uint32_t elapsed,bool champion){
 int player=s.winner==2?2:1,left=player==1?28:0,right=player==1?128:101;
 pinFrame(champion?smash_pinball::champion:smash_pinball::jackpot,elapsed,player);
 // Keep the approved ghost, border and particles; fit live text above the footer.
 panel->fillRect(left+2,1,right-left-4,30,0);
 label(player==1?(champion?"J1 MATCH":"J1 MANCHE"):(champion?"J2 MATCH":"J2 MANCHE"),1,player==1?RED:YELLOW,1,left,right);
 label(champion?"CHAMPION":"JACKPOT",10,champion?WHITE:YELLOW,2,left,right);
 footer(s,left+2,right-2);
}
static void bonus(const State&s,uint32_t elapsed){
 pinFrame(smash_pinball::bonus,elapsed);
 // No multiplier: the bonus rewards the rally visually, never adds extra points.
 panel->fillRect(29,9,70,23,0);char n[8];snprintf(n,sizeof(n),"%u",s.rally);label(n,9,YELLOW,2,29,99);footer(s);
}
static void selection(const State&s,uint32_t elapsed){
  frame(score,elapsed);panel->fillRect(27,0,75,32,0);
  label(s.selectedMode==1?"SOLO J1":s.selectedMode==3?"SOLO J2":"2 JOUEURS",1,s.selectedMode==3?YELLOW:s.selectedMode==1?RED:WHITE,1,29,99);
  label("MODE = CHOIX",12,BLUE);
  label(s.selectedMode==1?"J1 DEMARRER":s.selectedMode==3?"J2 DEMARRER":"J1/J2 START",23,WHITE);
}
static void gallery(uint32_t elapsed){
 const uint32_t sceneMs=3200;uint8_t scene=(elapsed/sceneMs)%10;uint32_t t=elapsed%sceneMs;
 State sample;sample.score1=10;sample.score2=10;sample.sets1=1;sample.sets2=1;sample.rally=8;sample.flags=WAIT_SERVE;sample.mode=2;
 switch(scene){
 case 0:frame(accueil,t);break;
 case 1:scores(sample,t);break;
 case 2:pinFrame(smash_pinball::skill_shot,t);footer(sample);break;
 case 3:scores(sample,t,true);break;
 case 4:bonus(sample,t);break;
 case 5:sample.score1=12;sample.sets1=1;sample.sets2=0;sample.winner=1;celebration(sample,t,false);break;
 case 6:sample.score2=12;sample.winner=2;celebration(sample,t,false);break;
 case 7:sample.score1=12;sample.sets1=2;sample.winner=1;celebration(sample,t,true);break;
 case 8:sample.score2=12;sample.sets2=2;sample.winner=2;celebration(sample,t,true);break;
 default:frame(securite,t);break;
 }
}
#include "strange_display.h"
#include "strange_intro_display.h"
#include "hockey_display.h"
#if SMASH_HOCKEY_TEST
#include "hockey_standalone.h"
#endif
#include "multi_game_display.h"
#include "strange_runtime.h"
#include "hockey_runtime.h"
#if SMASH_TEST
#include "standalone_test.h"
#endif
static void sdProgress(const char*status,unsigned percent){
 if(!panelReady)return;
 char line[20];snprintf(line,sizeof(line),"%s %u%%",status,percent);
 message("SMASH MAKERS",line,YELLOW);panel->flipDMABuffer();
}
void setup(){
 Serial.begin(115200);radiosOff=disableRadios();
 if(!radiosOff)Serial.println("ERROR: radio stop failed");
 if(!SMASH_HOCKEY_TEST&&!SMASH_TEST&&!SMASH_STRANGE_TEST&&!SMASH_STRANGE_CHECK){skr.setRxBufferSize(512);skr.begin(115200,SERIAL_8N1,SKR_RX_PIN,SKR_TX_PIN);transport.begin();}
 Serial.println("ETAPE 1: MATRICE");
 HUB75_I2S_CFG cfg(64,32,2);cfg.gpio={R1_PIN,G1_PIN,B1_PIN,R2_PIN,G2_PIN,B2_PIN,A_PIN,B_PIN,C_PIN,D_PIN,E_PIN,LAT_PIN,OE_PIN,CLK_PIN};
 cfg.double_buff=true;cfg.clkphase=CLOCK_PHASE;cfg.driver=PANEL_DRIVER;cfg.i2sspeed=HUB75_I2S_CFG::HZ_10M;cfg.setPixelColorDepthBits(6);
 panel=new MatrixPanel_I2S_DMA(cfg);panelReady=panel->begin();
 if(panelReady){panel->setBrightness8(DISPLAY_BRIGHTNESS);panel->clearScreen();panel->flipDMABuffer();}
 if(panelReady){message("SMASH MAKERS","DEMARRAGE",WHITE);panel->flipDMABuffer();delay(700);}
 if(!radiosOff){if(panelReady){message("ERREUR","RADIOS",RED);panel->flipDMABuffer();}return;}
 Serial.println("ETAPE 2: CARTE SD");
 if(SMASH_HOCKEY_TEST)hockeyAssets.begin(sdProgress);
 else if(!SMASH_STRANGE_CHECK){
  strangeAssets.begin(sdProgress);
  if(!SMASH_STRANGE_TEST&&!SMASH_TEST)hockeyAssets.begin(sdProgress);
 }
 if(panelReady){message("SMASH MAKERS","INIT AUDIO",WHITE);panel->flipDMABuffer();}
 Serial.println("ETAPE 3: AUDIO");
 audioReady=audio.begin(&strangeAssets.view(),&hockeyAssets.view());
 if(audioReady){audio.beep(660,200,0);delay(250);}
 if(SMASH_STRANGE_TEST)audio.setGame(makers::STRANGE);
 if(!panelReady||!audioReady)Serial.println("ERROR: matrix/audio initialization; game remains locked");
 Serial.printf("MAKERS V15 JEU SKR / INTRO COMPLETE / SD / AUDIO REDUIT; MATRICE=%d AUDIO=%d SD=%d; WIFI=OFF BT=OFF; core=%d; protocol=9\n",panelReady,audioReady,strangeAssets.view().ready,xPortGetCoreID());
#if SMASH_TEST
 testBegin(millis());
#endif
#if SMASH_HOCKEY_TEST
 hockeyTestBegin(millis());
#endif
}
void loop(){
 if(!radiosOff){delay(10);return;}
#if SMASH_HOCKEY_TEST
 hockeyTestLoop(millis());return;
#endif
 if(SMASH_STRANGE_CHECK){
  static uint32_t last=0;uint32_t t=millis();
  if(t-last>=1000){last=t;if(audioReady)audio.beep(440+220*((t/1000)%3),250,0);}
  if(panelReady){panel->fillScreen(0);panel->fillRect(0,0,42,7,RED);panel->fillRect(43,0,42,7,0x07E0);panel->fillRect(86,0,42,7,BLUE);label("TEST SANS SD",10,WHITE);label(audioReady&&audio.healthy()?"AUDIO OK":"ERREUR AUDIO",23,YELLOW);panel->flipDMABuffer();}
  delay(40);return;
 }
 if(SMASH_STRANGE_TEST){strangeStandalone(millis());return;}
#if SMASH_TEST
 testLoop(millis());return;
#endif
 gameUsbVolume();
 using namespace smash_link;
 const uint32_t local=millis();transport.poll(skr,model,local);
 const uint32_t now=transport.clock.now(local);
 const State &s=transport.state;
 bool needMedia=s.menu==makers::CHOOSE_PLAYERS?s.selectedGame==makers::STRANGE:s.game==makers::STRANGE&&(s.menu==makers::INTRO||s.menu==makers::PLAYING||s.menu==makers::RESULTS);
 bool needHockey=s.menu==makers::CHOOSE_PLAYERS?s.selectedGame==makers::HOCKEY:s.game==makers::HOCKEY&&(s.menu==makers::INTRO||s.menu==makers::PLAYING||s.menu==makers::RESULTS);
 bool mediaReady=(!needMedia||strangeAssets.view().ready)&&(!needHockey||(hockeyAssets.view().ready&&!hockeyPresentationFailed));
 uint8_t resources=(panelReady?MATRIX_READY:0)|(audioReady&&audio.healthy()&&mediaReady?AUDIO_READY:0);
 bool live=transport.online(local);
 bool both=resources==9;
 bool welcomed=live&&transport.clock.locked&&s.phase==WELCOME&&int32_t(now-(s.epoch+WELCOME_MS))>=0;
 bool running=live&&s.phase==RUN;
 // Mute custom media even when a readiness failure returns before drawing.
 audioSync.control(s,live&&both,transport.clock.locked,now,audio);
 strangeAudioControl(s,live&&both,transport.clock.locked,now);
 hockeyAudioControl(s,live&&both,transport.clock.locked,now);
 transport.reply(skr,local,resources,both&&presentationStarted&&presentedPhase==s.phase&&presentedSession==s.session&&(welcomed||running));
 if(!both){
  if(panelReady){if(needMedia&&!strangeAssets.view().ready)message("SD STRANGE", "ABSENTE/INVALIDE",RED);else if(needHockey&&(!hockeyAssets.view().ready||hockeyPresentationFailed))message("SD HOCKEY",hockeyPresentationFailed?"ANIMATION INVALIDE":hockeyAssets.error(),RED);else message("ERREUR SORTIE",audio.healthy()?"MATRICE":"AUDIO",RED);panel->flipDMABuffer();}
  delay(10);return;
 }
 static uint32_t rendered=0;
 static uint8_t appliedGame=255;
 uint8_t wantedGame=(s.menu==makers::CHOOSE_GAME||s.menu==makers::CHOOSE_PLAYERS)?s.selectedGame:s.game;
 if(live&&wantedGame!=appliedGame&&audio.setGame(wantedGame))appliedGame=wantedGame;
 if(uint32_t(local-rendered)<40){delay(1);return;}
 rendered=local;
 auto plan=presentation(s,model,live,transport.clock.locked,now);
 Scene scene=plan.scene;bool syncing=plan.syncing;uint32_t elapsed=plan.age;
 if(SMASH_DEMO&&!transport.sawAny){scene=DEMO;elapsed=local;syncing=false;}
 if(live)panel->setBrightness8(s.matrixBrightness);
 State demo;const State *visualState=&s;
 if(scene==DEMO){demoPlan(elapsed,demo,scene,elapsed);visualState=&demo;}
 const State &v=*visualState;
 if(!multiGameDisplay(s,scene,now))switch(scene){
 case BOOT:frame(accueil,elapsed);break;
 case READY:selection(s,elapsed);break;
 case SCORE:scores(v,now);break;
 case BOUNCE:scores(v,now,true);break;
 case POINT1:frame(point_j1,960+elapsed);footer(v);break;
 case POINT2:frame(point_j2,960+elapsed);footer(v);break;
 case JACKPOT1:case JACKPOT2:celebration(v,elapsed,false);break;
 case CHAMPION1:case CHAMPION2:celebration(v,elapsed,true);break;
 case BONUS:bonus(v,elapsed);break;
 case GO:pinFrame(smash_pinball::skill_shot,elapsed,0,true);footer(v);break;
 case HEAT:frame(securite,elapsed);break;
 case SECURITY:message("SECU ARRET",s.flags&HOT?"CHAUFFE":s.flags&TMC_FAULT?"TMC UART":s.text[0]?s.text:"DEFAUT SKR",RED);break;
 case PAUSE:message("PAUSE","SMASH MAKERS",YELLOW);break;
 case REFERENCE:if(syncing)message("SYNCHRO","SKR + ESP32",YELLOW);else message("REFERENCE",s.text[0]?s.text:"ZERO / ARM",YELLOW);break;
 case SERVICE_SCREEN:message(s.app==6?"TEST LED":"SERVICE",s.text[0]?s.text:"SKR",BLUE);break;
 case DEMO:gallery(elapsed);break;
 default:message("LIEN SKR",transport.seen?"PERDU":"EN ATTENTE",RED);break;
 }
 panel->flipDMABuffer();
 presentationStarted=true;presentedPhase=s.phase;presentedSession=s.session;
 if((SMASH_DEMO&&!transport.sawAny)||(live&&s.app==3&&s.phase==RUN))audioSync.demo(scene,audio);
 else if(wantedGame==makers::HOCKEY&&hockeyAssets.view().ready){} // Dedicated PCM event stream; no duplicate generic sounds.
 else if(!(s.game==makers::STRANGE&&strangeAssets.view().ready))audioSync.present(s,scene,elapsed,live,transport.clock.locked,now,audio);
 else strangeShortEffects(s,live,transport.clock.locked,now);
}
