#pragma once
static void hockeyAudioControl(const State&s,bool live,bool locked,uint32_t now){
 HockeyAudioState n;uint8_t wanted=(s.menu==makers::CHOOSE_GAME||s.menu==makers::CHOOSE_PLAYERS)?s.selectedGame:s.game;
 n.enabled=wanted==makers::HOCKEY&&hockeyAssets.view().ready;
 n.blocked=!live||!locked||s.phase!=RUN||s.app!=0||hockeyPresentationFailed||
  (s.flags&(smash_link::FAULT|smash_link::PAUSED|smash_link::HOT|smash_link::TMC_FAULT|smash_link::LINK_WAIT|smash_link::NEEDS_ARM))||
  s.menu==makers::CHOOSE_GAME||s.menu==makers::CHOOSE_PLAYERS;
 n.intro=s.game==makers::HOCKEY&&s.menu==makers::INTRO;n.introAge=min(uint32_t(10519),hockey::elapsed(now,s.menuAt));
 n.result=s.game==makers::HOCKEY&&s.menu==makers::RESULTS;n.resultAge=hockey::elapsed(now,s.menuAt);n.winner=s.winner;
 n.session=s.session;n.event=s.event;n.eventId=s.eventId;n.eventAge=hockey::elapsed(now,s.eventAt);
 n.cue=s.hockeyCue;n.cueAt=s.hockeyAt;n.cueAge=hockey::elapsed(now,s.hockeyAt);audio.hockeyControl(n);
}
