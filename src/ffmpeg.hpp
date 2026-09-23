#pragma once
// src/ffmpeg.hpp — FFmpeg C API 단일 진입점 (C 링키지 보장)
//
// FFmpeg 공개 헤더는 `extern "C"` 가드를 넣지 않은 순수 C 헤더다. C++ TU에서
// 그대로 include하면 컴파일러가 함수명을 C++ 규칙으로 망글링해서
// (_Z16avcodec_get_name9AVCodecID) 링크 단계에서 전부
// `undefined reference to 'avcodec_get_name(AVCodecID)'` 로 터진다.
// 라이브러리가 없어서가 아니라 심볼 이름이 달라져서 나는 에러다.
//
// 따라서 FFmpeg 헤더는 개별 .cpp/.hpp에서 직접 include하지 말고 항상 이
// 헤더를 통해서만 include한다(include guard 덕에 중복 include는 무해).
//
// 주의: extern "C" 블록 안에 C++ 표준 헤더(<memory>, <string> 등)를 넣지
//       않는다. FFmpeg 헤더가 끌어오는 것은 glibc C 헤더뿐이라 안전하다.
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavcodec/packet.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/channel_layout.h>
#include <libavutil/dict.h>
#include <libavutil/frame.h>
#include <libavutil/pixdesc.h>
#include <libavutil/rational.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}
