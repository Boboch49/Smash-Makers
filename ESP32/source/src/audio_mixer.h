#pragma once
#include "audio_bank.h"
#include "ima_decoder.h"
#include "theme_audio.h"

// Mono 16 kHz mixer. The driver repeats samples three times for 48 kHz.
class AudioMixer {
  struct Voice {
    const AudioClip* clip = nullptr;
    ImaState decoder;
    uint32_t position = 0;
    uint8_t priority = 0;
    int16_t sample() {
      if (!clip) return 0;
      const uint8_t packed = audioBank[clip->offset + position / 2];
      const int16_t result = decoder.decode(position % 2 ? packed >> 4 : packed & 15);
      if (++position >= clip->samples) clip = nullptr;
      return result;
    }
    void start(Sfx sound, uint8_t rank = 0, uint16_t ageMs = 0) {
      clip = &audioClips[uint8_t(sound)];
      decoder = ImaState{};
      position = 0;
      priority = rank;
      uint32_t skip = uint32_t(ageMs) * AUDIO_SAMPLE_RATE / 1000;
      if (skip >= clip->samples) { clip = nullptr; return; }
      while (skip--) sample();
    }
  };
  Voice music, effects[2];
  makers::Game game=makers::SMASH;ThemeSynth synth;
  uint32_t tonePhase = 0, toneStep = 0, toneRemaining = 0;
  static uint8_t priority(Sfx sound) {
    if (sound == Sfx::VICTORY || sound == Sfx::DEFEAT) return 4;
    if (sound == Sfx::SCORE_LEFT || sound == Sfx::SCORE_RIGHT || sound == Sfx::ROUND) return 3;
    if (sound == Sfx::BOOT || sound == Sfx::LAUNCH || sound == Sfx::MATCH_POINT) return 2;
    return 1;
  }
public:
  AudioMixer() { music.start(Sfx::AMBIENT); }
  void setGame(makers::Game next){if(next>=makers::GAME_COUNT)return;game=next;synth.reset();stopEffects();}
  void play(Sfx sound, uint16_t ageMs = 0) {
    if (sound >= Sfx::COUNT || sound == Sfx::AMBIENT) return;
    if(game==makers::HOCKEY){
      if(sound==Sfx::LAUNCH)beep(1100,100);
      else if(sound==Sfx::SCORE_LEFT||sound==Sfx::SCORE_RIGHT)beep(660,280);
    }else if(game==makers::STRANGE){
      if(sound==Sfx::POWERUP)beep(196,500);
      else if(sound==Sfx::MATCH_POINT)beep(147,350);
    }
    Voice& slot = !effects[0].clip ? effects[0] : !effects[1].clip ? effects[1]
                 : effects[0].priority <= effects[1].priority ? effects[0] : effects[1];
    const uint8_t rank = priority(sound);
    if (!slot.clip || rank >= slot.priority) slot.start(sound, rank, ageMs);
  }
  void beep(uint16_t hz, uint16_t ms) {
    toneStep = uint32_t(uint64_t(hz) * 4294967296ULL / AUDIO_SAMPLE_RATE);
    toneRemaining = hz ? uint32_t(ms) * AUDIO_SAMPLE_RATE / 1000 : 0;
    tonePhase = 0;
  }
  void stopEffects() {
    effects[0].clip = effects[1].clip = nullptr;
    toneRemaining = 0;
  }
  int16_t sample(bool includeMusic=true) {
    if (!music.clip) music.start(Sfx::AMBIENT);
    int32_t result = includeMusic?int32_t(game==makers::SMASH?music.sample():synth.sample(game))*18:0;
    result += (int32_t(effects[0].sample()) + effects[1].sample()) * 35;
    if (toneRemaining) {
      tonePhase += toneStep;
      result += (tonePhase & 0x80000000U) ? 3276 * 100 : -3276 * 100;
      --toneRemaining;
    }
    return int16_t(result / 100); // 18% + 35% + 35% + 10% headroom.
  }
};
