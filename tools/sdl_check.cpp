#include <SDL.h>
#include <cmath>
#include <vector>

int main() {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  SDL_Window *w;
  SDL_Renderer *r;
  SDL_CreateWindowAndRenderer(320, 240, 0, &w, &r);
  SDL_SetRenderDrawColor(r, 30, 90, 160, 255);
  SDL_RenderClear(r);
  SDL_RenderPresent(r);

  SDL_AudioSpec want{}, have{};
  want.freq = 48000;
  want.format = AUDIO_F32SYS;
  want.channels = 2;
  want.samples = 1024;
  SDL_AudioDeviceID dev = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
  if (dev) {
    std::vector<float> buf(size_t(have.samples) * have.channels * 24); // ~0.5s
    for (size_t i = 0; i < buf.size(); i += have.channels) {
      float s = 0.2f * std::sin(2 * 3.14159265f * 440.f *
                                float(i / have.channels) / have.freq);
      for (int c = 0; c < have.channels; ++c)
        buf[i + c] = s;
    }
    SDL_QueueAudio(dev, buf.data(), buf.size() * sizeof(float));
    SDL_PauseAudioDevice(dev, 0);
  } else {
    SDL_Log("audio open failed: %s", SDL_GetError());
  }
  SDL_Delay(1500);
  if (dev)
    SDL_CloseAudioDevice(dev);
  SDL_Quit();
}