#pragma once
#include "display_model.h"
#include "audio_types.h"

// Audio commands are derived from accepted snapshots and the common game clock.
class AudioSync {
  uint32_t session = 0, welcomeEpoch = 0;
  uint16_t eventId = 0, toneId = 0, stopId = 0;
  bool eventSeen = false, toneSeen = false, welcomePlayed = false, blocked = false;
  smash_display::Scene demoScene = smash_display::LOST;
public:
  template<class Output>
  void control(const smash_link::State& state, bool live, bool locked, uint32_t now, Output& audio) {
    if (live && state.session != session) {
      session = state.session;
      eventSeen = toneSeen = welcomePlayed = false;
      stopId = state.stopId;
      audio.stopEffects();
    }
    const bool silentEffects = !live || !locked ||
      (state.flags & (smash_link::FAULT | smash_link::HOT | smash_link::TMC_FAULT | smash_link::PAUSED));
    if ((silentEffects && !blocked) || (live && stopId != state.stopId)) audio.stopEffects();
    blocked = silentEffects;
    if (!live || !locked) return;
    stopId = state.stopId;
    if (!toneSeen || toneId != state.toneId) {
      toneSeen = true;
      toneId = state.toneId;
      const int32_t age = int32_t(now - state.toneAt);
      // Diagnostic tones may announce a safety stop, so they are allowed while blocked.
      if (age >= 0 && age <= 150 && age < state.toneMs)
        audio.beep(state.toneHz, state.toneMs, uint16_t(age));
    }
  }
  template<class Output>
  void present(const smash_link::State& state, smash_display::Scene scene, uint32_t elapsed,
               bool live, bool locked, uint32_t now, Output& audio) {
    if (!live || !locked || blocked) return;
    if (scene == smash_display::BOOT && (!welcomePlayed || welcomeEpoch != state.epoch)) {
      welcomePlayed = true;
      welcomeEpoch = state.epoch;
      if (elapsed <= 150) audio.play(Sfx::BOOT, elapsed);
    }
    if (state.phase != smash_link::RUN || (eventSeen && eventId == state.eventId)) return;
    eventSeen = true;
    eventId = state.eventId;
    const int32_t age = int32_t(now - state.eventAt);
    if (age < 0 || age > 150) return;
    Sfx sound = static_cast<Sfx>(state.event);
    if ((scene == smash_display::JACKPOT1 || scene == smash_display::JACKPOT2) && elapsed <= 150)
      sound = Sfx::POWERUP;
    audio.play(sound, uint16_t(age));
  }
  template<class Output>
  void demo(smash_display::Scene scene, Output& audio) {
    using namespace smash_display;
    if (scene == demoScene) return;
    demoScene = scene;
    switch (scene) {
      case BOOT: audio.play(Sfx::BOOT); break;
      case GO: audio.play(Sfx::LAUNCH); break;
      case BOUNCE: audio.play(Sfx::WALL); break;
      case BONUS: audio.play(Sfx::RALLY_COMBO); break;
      case JACKPOT1: case JACKPOT2: audio.play(Sfx::POWERUP); break;
      case CHAMPION1: case CHAMPION2: audio.play(Sfx::VICTORY); break;
      default: break;
    }
  }
};
