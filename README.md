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

CMake + Ninja로도 빌드할 수 있습니다(`build/sdl_check`). 자세한 방법은 아래
**Asio 코루틴/채널 스모크 테스트** 항목의 `### CMake로 빌드`를 참고하세요.

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

## Asio 코루틴/채널 스모크 테스트 (`tools/hello_coro.cpp`)

C++20 코루틴으로 만든 **생산자-소비자 파이프라인**이 이벤트 루프 위에서 정상 동작하는지
확인하는 최소 예제입니다. `asio::io_context`에 `asio::co_spawn`으로 코루틴 두 개를 올리고,
`asio::experimental::concurrent_channel`(용량 2)로 정수 5개를 흘려보낸 뒤 `close()`로
스트림 종료를 알립니다. 이후에 올 **디코드 → 렌더 큐** 구조의 축소판입니다.

```cpp
using Chan = concurrent_channel<void(asio::error_code, int)>;

asio::awaitable<void> producer(Chan &ch);  // async_send 5회 후 close()
asio::awaitable<void> consumer(Chan &ch);  // async_receive 루프, ec면 종료
```

### CMake로 빌드

`CMakeLists.txt`가 있어 CMake + Ninja로 두 예제를 함께 빌드할 수 있습니다.

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

빌드 결과는 저장소 루트의 `build/` 아래에 생성됩니다(`.gitignore`에 포함).

```bash
./build/hello_coro    # Asio 코루틴/채널 스모크 테스트
./build/sdl_check     # SDL2 영상·오디오 스모크 테스트
```

asio는 **header-only**이므로 링크할 라이브러리가 없습니다. 다만 `find_package(SDL2 CONFIG
REQUIRED)`와 FFmpeg `pkg_check_modules` 때문에 아래 패키지가 필요합니다.

```bash
sudo apt install build-essential cmake ninja-build pkg-config libsdl2-dev \
    libavformat-dev libavcodec-dev libavutil-dev libswscale-dev libswresample-dev
```

### 예상 출력

```text
recv 0
recv 1
recv 2
recv 3
recv 4
```

### 동작 요약

- `Chan = concurrent_channel<void(asio::error_code, int)>`의 시그니처는 **수신 완료
  시그니처**입니다. `async_receive`는 `(error_code, int)`를 그대로 돌려주고,
  `async_send`는 같은 인자를 payload로 받습니다(송신 완료 시그니처는
  `void(asio::error_code)` 하나뿐).
- `async_send`의 첫 인자 `asio::error_code{}`는 **메시지의 에러 슬롯**입니다. 관례상 채널
  오류/종료 신호 자리이며, 수신자는 이 값을 `ec`로 받습니다.
- 용량 `2` → 버퍼가 차면 생산자 코루틴이 멈추는 **백프레셔**가 걸립니다. 용량 `0`은
  수신자가 나타날 때까지 송신자가 기다리는 rendezvous 모드입니다.
- `as_tuple(asio::use_awaitable)`로 완료를 `(ec, value)` 튜플로 받습니다. `close()`로 인한
  종료(`channel_closed`)를 예외가 아닌 정상 루프 탈출(`break`)로 처리하기 위함입니다.
- `ch.close()`는 파이프라인 종료 프로토콜이자 **이벤트 루프 종료 조건**입니다. 대기 중인
  `async_receive`가 남아 있으면 미완료 작업으로 잡혀 `io_context::run()`이 리턴하지
  않습니다. 모든 작업이 끝나야 `run()`이 리턴하고 프로그램이 종료됩니다.
- `asio::detached`로 스폰한 코루틴의 완료 통지(`std::exception_ptr`)는 아무도 받지 않으므로,
  코루틴 안에서 발생한 예외는 조용히 사라집니다. 디버깅할 때는 `asio::use_future`로 바꿔
  `future.get()`으로 예외를 확인하는 편이 좋습니다.
- 두 코루틴은 같은 스레드의 `io_context`에서 번갈아 실행됩니다(동시성 O, 병렬성 X).
  병렬 실행이 필요하면 여러 스레드에서 `io_context::run()`을 호출합니다.

## FFmpeg C API 사용 규칙 (`src/ffmpeg.hpp`)

FFmpeg 공개 헤더(`libavcodec/*.h`, `libavformat/*.h`, `libavutil/*.h`)는
`extern "C"` 가드가 없는 **순수 C 헤더**입니다. C++ 소스에서 그대로 include하면
컴파일러가 함수명을 C++ 규칙으로 망글링해서(`_Z16avcodec_get_name9AVCodecID`)
링크 단계에서 다음처럼 실패합니다.

```text
undefined reference to `avcodec_get_name(AVCodecID)'
undefined reference to `avformat_open_input(AVFormatContext**, char const*, ...)'
```

라이브러리(`pkg-config`/`PkgConfig::FFMPEG`)가 정상 연결돼 있어도 나는 에러이며,
에러 메시지에 **괄호로 인자 타입이 붙어 있는지**가 망글링의 증거입니다
(순수 C 심볼이면 `avcodec_get_name` 만 표시됩니다). 확인 방법:

```bash
nm build/CMakeFiles/probe.dir/tools/probe.cpp.o | grep avcodec_get_name
# 나쁨: U _Z16avcodec_get_name9AVCodecID     ← 망글링됨
# 좋음: U avcodec_get_name
```

따라서 FFmpeg 헤더는 개별 소스에서 직접 include하지 않고 **`src/ffmpeg.hpp`를
통해서만** include합니다. 이 헤더가 모든 FFmpeg include를 `extern "C" { }`로
감싸 C 링키지를 보장합니다.

```cpp
#include "ffmpeg.hpp"   // FFmpeg API
#include "raii.hpp"     // FFmpeg 자원 RAII 래퍼(내부에서 ffmpeg.hpp 포함)
```

`extern "C"` 블록 안에는 C++ 표준 헤더(`<memory>`, `<string>` 등)를 넣지
않습니다. FFmpeg 헤더가 끌어오는 것은 glibc C 헤더(`<errno.h>`, `<inttypes.h>`,
`<math.h>` …)뿐이라 이 방식이 안전합니다.

