#pragma once
// src/raii.hpp — FFmpeg C API 자원의 RAII 래퍼 + 공통 유틸
#include <memory>
#include <string>

// FFmpeg 헤더는 반드시 이 진입점을 통해서만 include한다(C 링키지 유지).
#include "ffmpeg.hpp"

namespace ff {

struct PacketDel {
  void operator()(AVPacket *p) const noexcept { av_packet_free(&p); }
};
struct FrameDel {
  void operator()(AVFrame *f) const noexcept { av_frame_free(&f); }
};
struct FmtDel {
  void operator()(AVFormatContext *c) const noexcept {
    avformat_close_input(&c);
  }
};
struct CodecDel {
  void operator()(AVCodecContext *c) const noexcept {
    avcodec_free_context(&c);
  }
};

using PacketPtr = std::unique_ptr<AVPacket, PacketDel>;
using FramePtr = std::unique_ptr<AVFrame, FrameDel>;
using FormatCtx = std::unique_ptr<AVFormatContext, FmtDel>;
using CodecCtx = std::unique_ptr<AVCodecContext, CodecDel>;

// AVERROR 코드 → 문자열 (av_err2str의 이식 가능한 대체 — 설계 문서 §10 규칙)
inline std::string errstr(int err) {
  char buf[AV_ERROR_MAX_STRING_SIZE] = {0};
  av_strerror(err, buf, sizeof(buf));
  return buf;
}

} // namespace ff