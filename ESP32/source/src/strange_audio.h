#pragma once
#include "strange_media_catalog.h"
#include "audio_types.h"

struct StrangeAudioState {
  bool enabled=false,blocked=true,clock=false,intro=false,inverted=false,defeat=false;
  uint16_t eventId=0,clockAge=0,eventAge=65535;
  uint8_t event=255;uint32_t session=0,sent=0;
};

// One owner: the existing audio worker. Assets are immutable PSRAM views.
class StrangeAudio {
  const strange::View* media=nullptr;
  StrangeAudioState state;
  uint32_t music=0,clock=0,effect=0;strange::Slot effectSlot=strange::POINT;
  bool effectPlaying=false,seen=false;
  int16_t at(strange::Slot slot,uint32_t sample)const{
    if(!media||!media->ready||sample>=media->bytes[slot]/2)return 0;
    auto p=media->data[slot]+sample*2;return int16_t(uint16_t(p[0])|(uint16_t(p[1])<<8));
  }
public:
  void begin(const strange::View* m){media=m;}
  void control(const StrangeAudioState& next,uint32_t received){
    bool fresh=!seen||next.session!=state.session;
    if(fresh||next.enabled!=state.enabled||next.inverted!=state.inverted)music=0;
    if(next.blocked||!next.enabled){effectPlaying=false;clock=0;}
    else if(next.clock||next.intro){
      uint32_t target=(uint32_t(next.clockAge)+(received-next.sent))*16;
      // Correct only material drift. Normal packets leave the stream continuous.
      uint32_t difference=clock>target?clock-target:target-clock;
      if((!state.clock&&!state.intro)||next.intro!=state.intro||fresh||difference>640)clock=target;
      effectPlaying=false;
    }else{
      if(state.defeat&&!next.defeat)effectPlaying=false;
      if(next.defeat&&(!state.defeat||fresh)){effectSlot=strange::DEFEAT;effect=0;effectPlaying=true;}
      else if(!next.defeat&&(fresh||next.eventId!=state.eventId)&&next.eventAge<=150&&
              (next.event==uint8_t(Sfx::SCORE_LEFT)||next.event==uint8_t(Sfx::SCORE_RIGHT))){
        effectSlot=strange::POINT;effect=uint32_t(next.eventAge)*16;effectPlaying=true;
      }
    }
    state=next;seen=true;
  }
  bool ownsMusic()const{return media&&media->ready&&state.enabled;}
  int16_t sample(){
    if(!ownsMusic()||state.blocked)return 0;
    if(state.clock||state.intro)return int16_t(int32_t(at(state.intro?strange::INTRO_AUDIO:strange::CLOCK_AUDIO,clock++))*55/100);
    auto slot=state.inverted?strange::INVERSE:strange::NORMAL;
    uint32_t length=media->bytes[slot]/2;
    if(music>=length)music=0;
    int32_t result=int32_t(at(slot,music++))*(effectPlaying?10:24)/100;
    if(effectPlaying){
      result+=int32_t(at(effectSlot,effect++))*55/100;
      if(effect>=media->bytes[effectSlot]/2)effectPlaying=false;
    }
    return int16_t(result);
  }
};
