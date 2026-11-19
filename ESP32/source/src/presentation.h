#pragma once
#include "display_model.h"
namespace smash_display {
struct Presentation { Scene scene = LOST; uint32_t age = 0; bool syncing = false; };
inline Presentation presentation(const State& s, const Model& model, bool live, bool locked, uint32_t now) {
  Presentation result;
  if (!live) { result.scene = s.session ? LOST : REFERENCE; result.syncing = !s.session; return result; }
  if (s.flags & (FAULT | HOT | TMC_FAULT | THERMAL)) result.scene = model.view(now);
  else if (s.phase == RUN) result.scene = model.view(now);
  else if ((s.phase == SCHEDULED || s.phase == WELCOME) && locked && int32_t(now - s.epoch) >= 0) {
    result.scene = BOOT; result.age = now - s.epoch; return result;
  } else { result.scene = REFERENCE; result.syncing = true; }
  if (result.scene == JACKPOT1 || result.scene == JACKPOT2 || result.scene == CHAMPION1 || result.scene == CHAMPION2)
    result.age = now - s.phaseAt;
  else if (result.scene == GO || result.scene == BONUS || result.scene == POINT1 || result.scene == POINT2 || result.scene == BOUNCE)
    result.age = now - model.started;
  else result.age = now - (s.epoch + WELCOME_MS);
  return result;
}
inline void demoPlan(uint32_t elapsed,State&sample,Scene&scene,uint32_t&age){
 uint8_t n=(elapsed/3200)%10;age=elapsed%3200;
 sample.score1=sample.score2=10;sample.sets1=sample.sets2=1;sample.server=1;sample.rally=8;sample.flags=WAIT_SERVE;
 switch(n){
 case 0:scene=BOOT;break;
 case 1:scene=SCORE;break;
 case 2:sample.flags=0;scene=age<960?GO:SCORE;break;
 case 3:sample.flags=0;sample.event=3;age%=1000;scene=age<200?BOUNCE:SCORE;break;
 case 4:sample.flags=0;scene=age<700?BONUS:SCORE;break;
 case 5:sample.score1=12;sample.sets2=0;sample.winner=1;scene=JACKPOT1;break;
 case 6:sample.score2=12;sample.winner=2;scene=JACKPOT2;break;
 case 7:sample.score1=12;sample.sets1=2;sample.winner=1;scene=CHAMPION1;break;
 case 8:sample.score2=12;sample.sets2=2;sample.winner=2;scene=CHAMPION2;break;
 default:sample.flags=THERMAL;scene=HEAT;break;
 }
}
}
