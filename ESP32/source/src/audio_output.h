#pragma once
#include <Arduino.h>
#include <driver/i2s.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <atomic>
#include "config.h"
#include "audio_mixer.h"
#include "es8311.h"
#include "strange_audio.h"
#include "strange_fast.h"
#include "hockey_audio.h"

class AudioOutput {
  static constexpr i2s_port_t PORT = I2S_NUM_0;
  static constexpr size_t FRAMES = 64;
  enum Action : uint8_t { EFFECT, TONE, STOP, THEME };
  struct Message {
    Action action;
    Sfx sound;
    uint16_t age, hz, ms;
    uint32_t sent;
  };
  QueueHandle_t messages = nullptr;
  ES8311 codec;
  std::atomic<unsigned> masterVolume{65};
  QueueHandle_t mediaStates=nullptr;
  const strange::View* mediaView=nullptr;
  QueueHandle_t hockeyStates=nullptr;
  const hockey::View* hockeyView=nullptr;
  std::atomic<uint32_t> lastWrite{0};
  std::atomic<bool> failed{false};
  bool send(Action action, Sfx sound, uint16_t age, uint16_t hz, uint16_t ms) {
    if (!messages || failed.load()) return false;
    Message message{action, sound, age, hz, ms, millis()};
    if (xQueueSend(messages, &message, 0) == pdTRUE) return true;
    if (action != STOP) return false;
    xQueueReset(messages);
    return xQueueSend(messages, &message, 0) == pdTRUE;
  }
  static void worker(void* context) { static_cast<AudioOutput*>(context)->run(); }
  void run() {
    AudioMixer mixer;
    StrangeAudio media;media.begin(mediaView);
    HockeyAudio hockeyMedia;hockeyMedia.begin(hockeyView);
    int16_t pcm[FRAMES * 3 * 2];
    for (;;) {
      StrangeAudioState next;
      if(xQueueReceive(mediaStates,&next,0)==pdTRUE)media.control(next,millis());
      HockeyAudioState hockeyNext;
      if(xQueueReceive(hockeyStates,&hockeyNext,0)==pdTRUE)hockeyMedia.control(hockeyNext,millis());
      Message message;
      for (unsigned n = 0; n < 8 && xQueueReceive(messages, &message, 0) == pdTRUE; ++n) {
        const uint32_t age = uint32_t(message.age) + uint32_t(millis() - message.sent);
        if (message.action == THEME) mixer.setGame(makers::Game(message.hz));
        else if (message.action == STOP) mixer.stopEffects();
        else if (age <= 150 && message.action == EFFECT) mixer.play(message.sound, age);
        else if (age < message.ms && message.action == TONE) mixer.beep(message.hz, message.ms - age);
      }
      for (size_t n = 0; n < FRAMES; ++n) {
        int32_t mixed=hockeyMedia.ownsMusic()?int32_t(hockeyMedia.sample())+int32_t(mixer.sample(false))/4:
          media.ownsMusic()?int32_t(media.sample())+int32_t(mixer.sample(false))/4:int32_t(mixer.sample());
        const int16_t value=strange_fast::level(mixed,masterVolume.load());
        for (size_t i = 0; i < 6; ++i) pcm[n * 6 + i] = value;
      }
      size_t offset = 0;
      while (offset < sizeof(pcm)) {
        size_t written = 0;
        const esp_err_t result = i2s_write(PORT, reinterpret_cast<uint8_t*>(pcm) + offset,
                                         sizeof(pcm) - offset, &written, pdMS_TO_TICKS(25));
        if (result != ESP_OK || !written) {
          failed.store(true);
          digitalWrite(AUDIO_ENABLE_PIN, LOW);
          vTaskDelete(nullptr);
          return;
        }
        offset += written;
        lastWrite.store(millis());
      }
    }
  }
public:
  bool begin(const strange::View* assets=nullptr,const hockey::View* hockeyAssets=nullptr) {
    mediaView=assets;
    hockeyView=hockeyAssets;
    pinMode(AUDIO_ENABLE_PIN, OUTPUT);
    digitalWrite(AUDIO_ENABLE_PIN, LOW);
    if (!codec.begin(AUDIO_SDA_PIN, AUDIO_SCL_PIN, 400000) ||
        !codec.setVolume(85) || !codec.setBitsPerSample(16) ||
        !codec.enableMicrophone(false)) return false;
    i2s_config_t config{};
    config.mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_TX);
    config.sample_rate = 48000;
    config.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
    config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
    config.communication_format = I2S_COMM_FORMAT_STAND_I2S;
    config.dma_buf_count = 6;
    config.dma_buf_len = FRAMES * 3;
    config.tx_desc_auto_clear = true;
    config.fixed_mclk = 12288000;
    config.mclk_multiple = I2S_MCLK_MULTIPLE_256;
    if (i2s_driver_install(PORT, &config, 0, nullptr) != ESP_OK) return false;
    i2s_pin_config_t pins{};
    pins.mck_io_num = AUDIO_MCLK_PIN;
    pins.bck_io_num = AUDIO_BCLK_PIN;
    pins.ws_io_num = AUDIO_LRCLK_PIN;
    pins.data_out_num = AUDIO_DATA_PIN;
    pins.data_in_num = I2S_PIN_NO_CHANGE;
    if (i2s_set_pin(PORT, &pins) == ESP_OK && i2s_zero_dma_buffer(PORT) == ESP_OK) {
      messages = xQueueCreate(8, sizeof(Message));
      mediaStates=xQueueCreate(1,sizeof(StrangeAudioState));
      hockeyStates=xQueueCreate(1,sizeof(HockeyAudioState));
      lastWrite.store(millis());
      if (messages && mediaStates && hockeyStates && xTaskCreatePinnedToCore(worker, "smash-audio", 4096, this, 3, nullptr, 0) == pdPASS) {
        digitalWrite(AUDIO_ENABLE_PIN, HIGH);
        return true;
      }
      if (messages) vQueueDelete(messages);
      messages = nullptr;
      if(mediaStates)vQueueDelete(mediaStates);mediaStates=nullptr;
      if(hockeyStates)vQueueDelete(hockeyStates);hockeyStates=nullptr;
    }
    i2s_driver_uninstall(PORT);
    return false;
  }
  void mediaControl(StrangeAudioState state){state.sent=millis();if(mediaStates)xQueueOverwrite(mediaStates,&state);}
  void hockeyControl(HockeyAudioState state){state.sent=millis();if(hockeyStates)xQueueOverwrite(hockeyStates,&state);}
  void setMasterVolume(unsigned value){masterVolume.store(value>100?100:value);}
  unsigned volume()const{return masterVolume.load();}
  bool healthy() const {
    return messages && !failed.load() && uint32_t(millis() - lastWrite.load()) < 200;
  }
  void play(Sfx sound, uint16_t age = 0) { send(EFFECT, sound, age, 0, 0); }
  void beep(uint16_t hz, uint16_t ms, uint16_t age) { send(TONE, Sfx::HOVER, age, hz, ms); }
  void stopEffects() { send(STOP, Sfx::HOVER, 0, 0, 0); }
  bool setGame(uint8_t game){return game<makers::GAME_COUNT&&send(THEME,Sfx::HOVER,0,game,0);}
};
