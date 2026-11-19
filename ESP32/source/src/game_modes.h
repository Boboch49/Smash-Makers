#pragma once
#include <stdint.h>

enum GameMode : uint8_t { MODE_DEMO, MODE_1PLAYER, MODE_2PLAYERS, MODE_SOLO_J2 };

inline GameMode nextGameMode(GameMode mode) {
  if (mode == MODE_1PLAYER) return MODE_SOLO_J2;
  if (mode == MODE_SOLO_J2) return MODE_2PLAYERS;
  return MODE_1PLAYER;
}

inline uint8_t soloPlayer(GameMode mode) {
  if (mode == MODE_1PLAYER) return 1;
  if (mode == MODE_SOLO_J2) return 2;
  return 0;
}

inline bool humanControls(GameMode mode, uint8_t player) {
  return mode == MODE_2PLAYERS || soloPlayer(mode) == player;
}

inline bool aiControls(GameMode mode, uint8_t player) {
  return mode == MODE_DEMO || (soloPlayer(mode) && soloPlayer(mode) != player);
}

inline bool playerPressed(GameMode mode, uint8_t server, bool one, bool two) {
  const uint8_t player = soloPlayer(mode) ? soloPlayer(mode) : server;
  return player == 1 ? one : two;
}
