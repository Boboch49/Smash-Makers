#pragma once
#include <stdint.h>
#include "audio_types.h"

namespace smash_test {
enum Visual : uint8_t {
 WELCOME_SCREEN, WAITING, SELECT_J1, SELECT_J2, SELECT_TWO, LIVE_SCORE, COUNTDOWN,
 SERVICE_CUBE, REBOUND_ASSET, BUMPER_LIVE, BUMPER_ASSET, POINT_J1, POINT_J2,
 BONUS_LIVE, JACKPOT_J1, JACKPOT_J2, CHAMPION_J1, CHAMPION_J2,
 VICTORY_J1, VICTORY_J2, DEFEAT, HEAT_SCREEN, PAUSE_SCREEN, FAULT_SCREEN,
 REFERENCE_SCREEN, AUDIO_ONLY, MIX
};
struct Step { const char* name; Visual visual; uint16_t ms; Sfx sound; };
static const Step steps[] = {
 {"ACCUEIL",WELCOME_SCREEN,9600,Sfx::BOOT},
 {"ATTENTE",WAITING,9600,Sfx::HOVER},
 {"SOLO J1",SELECT_J1,6000,Sfx::HOVER},
 {"SOLO J2",SELECT_J2,6000,Sfx::HOVER},
 {"DEUX JOUEURS",SELECT_TWO,6000,Sfx::HOVER},
 {"SCORE",LIVE_SCORE,8000,Sfx::MATCH_POINT},
 {"DECOMPTE",COUNTDOWN,7680,Sfx::COUNTDOWN},
 {"SERVICE CUBE",SERVICE_CUBE,10240,Sfx::LAUNCH},
 {"REBOND",REBOUND_ASSET,6000,Sfx::WALL},
 {"BUMPER JEU",BUMPER_LIVE,6000,Sfx::PADDLE_LEFT},
 {"BUMPER GALERIE",BUMPER_ASSET,10240,Sfx::WALL_ALT},
 {"POINT J1",POINT_J1,6400,Sfx::SCORE_LEFT},
 {"POINT J2",POINT_J2,6400,Sfx::SCORE_RIGHT},
 {"BONUS RENVOIS",BONUS_LIVE,10240,Sfx::RALLY_COMBO},
 {"JACKPOT J1",JACKPOT_J1,10240,Sfx::POWERUP},
 {"JACKPOT J2",JACKPOT_J2,10240,Sfx::POWERUP},
 {"CHAMPION J1",CHAMPION_J1,10240,Sfx::VICTORY},
 {"CHAMPION J2",CHAMPION_J2,10240,Sfx::VICTORY},
 {"VICTOIRE J1",VICTORY_J1,9600,Sfx::VICTORY},
 {"VICTOIRE J2",VICTORY_J2,9600,Sfx::VICTORY},
 {"DEFAITE",DEFEAT,8000,Sfx::DEFEAT},
 {"CHAUFFE",HEAT_SCREEN,6400,Sfx::AMBIENT},
 {"PAUSE",PAUSE_SCREEN,6000,Sfx::AMBIENT},
 {"SECURITE",FAULT_SCREEN,6000,Sfx::AMBIENT},
 {"REFERENCE",REFERENCE_SCREEN,6000,Sfx::AMBIENT},
 {"MIXAGE",MIX,10000,Sfx::LAUNCH},
 {"DEMARRAGE",AUDIO_ONLY,5000,Sfx::BOOT},
 {"RAQUETTE J1",AUDIO_ONLY,5000,Sfx::PADDLE_LEFT},
 {"RAQUETTE J2",AUDIO_ONLY,5000,Sfx::PADDLE_RIGHT},
 {"MUR",AUDIO_ONLY,5000,Sfx::WALL},
 {"VITESSE",AUDIO_ONLY,5000,Sfx::SPEED},
 {"POINT J1",AUDIO_ONLY,5000,Sfx::SCORE_LEFT},
 {"POINT J2",AUDIO_ONLY,5000,Sfx::SCORE_RIGHT},
 {"DECOMPTE",AUDIO_ONLY,5000,Sfx::COUNTDOWN},
 {"LANCEMENT",AUDIO_ONLY,5000,Sfx::LAUNCH},
 {"BALLE MATCH",AUDIO_ONLY,5000,Sfx::MATCH_POINT},
 {"VICTOIRE",AUDIO_ONLY,5000,Sfx::VICTORY},
 {"DEFAITE",AUDIO_ONLY,5000,Sfx::DEFEAT},
 {"SELECTION",AUDIO_ONLY,5000,Sfx::HOVER},
 {"POWER UP",AUDIO_ONLY,5000,Sfx::POWERUP},
 {"MANCHE",AUDIO_ONLY,5000,Sfx::ROUND},
 {"AMBIANCE",AUDIO_ONLY,8000,Sfx::AMBIENT},
 {"J1 FORT",AUDIO_ONLY,5000,Sfx::PADDLE_LEFT_HARD},
 {"J2 FORT",AUDIO_ONLY,5000,Sfx::PADDLE_RIGHT_HARD},
 {"MUR VARIANTE",AUDIO_ONLY,5000,Sfx::WALL_ALT},
 {"COMBO",AUDIO_ONLY,5000,Sfx::RALLY_COMBO}
};
constexpr unsigned COUNT=sizeof(steps)/sizeof(steps[0]);
class Timeline {
 uint32_t last=0,elapsed=0;
 bool paused=false,changed=true;
 unsigned index=0;
public:
 void begin(uint32_t now){last=now;elapsed=0;paused=false;index=0;changed=true;}
 void tick(uint32_t now){
  uint32_t delta=now-last;last=now;
  if(paused)return;
  // Preserve a complete first frame/audio event after a long USB or setup stall.
  if(delta>250)delta=250;
  elapsed+=delta;
  if(elapsed>=steps[index].ms){index=(index+1)%COUNT;elapsed=0;changed=true;}
 }
 void next(){index=(index+1)%COUNT;elapsed=0;changed=true;}
 void previous(){index=(index+COUNT-1)%COUNT;elapsed=0;changed=true;}
 void replay(){elapsed=0;changed=true;}
 void pause(bool value){paused=value;}
 bool takeChanged(){bool value=changed;changed=false;return value;}
 bool isPaused()const{return paused;}
 unsigned number()const{return index;}
 uint32_t age()const{return elapsed;}
 const Step& step()const{return steps[index];}
};
}
