# decode-render

SDL2 기반 디코드/렌더링 실험 저장소.

## SDL2 스모크 테스트 (`tools/sdl_check.cpp`)

SDL2 창/렌더러를 열고, 48kHz 스테레오 F32 오디오 디바이스에 440Hz 사인파 버퍼를
큐잉해서 영상·오디오 출력이 정상 동작하는지 확인하는 최소 예제입니다.

### 빌드 및 실행

`tools/sdl_check.cpp`는 저장소 루트의 `build_sdl_check.sh`를 실행하면 빌드됩니다.

```bash
./build_sdl_check.sh
```

이 스크립트는 내부적으로 다음을 수행합니다.

```bash
g++ -std=c++20 tools/sdl_check.cpp $(pkg-config --cflags --libs sdl2) -o /tmp/sdl_check
/tmp/sdl_check
```

### 실행 파일 경로

빌드 결과인 **실행 파일은 `/tmp/sdl_check` 로 생성**됩니다. 스크립트가 빌드 직후
이 파일을 바로 실행하므로, 재빌드 없이 다시 돌려보고 싶으면 다음 한 줄이면 됩니다.

```bash
/tmp/sdl_check
```

### 요구 사항

- `g++` (C++20 지원)
- SDL2 개발 패키지와 `pkg-config`

```bash
sudo apt install build-essential pkg-config libsdl2-dev
```

### 참고

- 창이 뜬 뒤 약 1.5초(`SDL_Delay(1500)`) 동안 사인파를 재생하고 종료합니다.
- 디스플레이/오디오 장치가 없는 헤드리스 환경에서는 SDL 더미 드라이버로 로직만
  검증할 수 있습니다.

  ```bash
  SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy /tmp/sdl_check
  ```

- 오디오 디바이스 오픈에 실패하면 `SDL_Log`로 `audio open failed: ...` 를 출력하고
  영상 부분만 수행합니다.
