// tools/probe.cpp — M0: 컨테이너/스트림 정보 출력 (ffprobe 대조 검증용)
#include "ffmpeg.hpp"
#include "raii.hpp"

#include <cinttypes>
#include <cstdio>

namespace {

const char *type_name(AVMediaType t) {
  switch (t) {
  case AVMEDIA_TYPE_VIDEO:
    return "video";
  case AVMEDIA_TYPE_AUDIO:
    return "audio";
  case AVMEDIA_TYPE_SUBTITLE:
    return "subtitle";
  case AVMEDIA_TYPE_ATTACHMENT:
    return "attachment";
  case AVMEDIA_TYPE_DATA:
    return "data";
  default:
    return "other";
  }
}

void print_stream(const AVStream *st) {
  const AVCodecParameters *par = st->codecpar;
  std::printf("[stream #%d] %s\n", st->index, type_name(par->codec_type));

  std::printf("  codec      : %s\n", avcodec_get_name(par->codec_id));
  if (const char *prof = avcodec_profile_name(par->codec_id, par->profile))
    std::printf("  profile    : %s\n", prof);
  if (par->bit_rate > 0)
    std::printf("  bit_rate   : %" PRId64 " bps\n", par->bit_rate);

  if (par->codec_type == AVMEDIA_TYPE_VIDEO) {
    std::printf("  resolution : %dx%d\n", par->width, par->height);
    std::printf("  pix_fmt    : %s\n",
                av_get_pix_fmt_name((AVPixelFormat)par->format));
    if (st->avg_frame_rate.num)
      std::printf("  frame_rate : %.3f fps\n", av_q2d(st->avg_frame_rate));
  } else if (par->codec_type == AVMEDIA_TYPE_AUDIO) {
    std::printf("  sample_rate: %d Hz\n", par->sample_rate);
    std::printf("  sample_fmt : %s\n",
                av_get_sample_fmt_name((AVSampleFormat)par->format));
    char lb[64] = {0};
    av_channel_layout_describe(&par->ch_layout, lb,
                               sizeof(lb)); // FFmpeg 6.x 신규 API
    std::printf("  ch_layout  : %s (%d ch)\n", lb, par->ch_layout.nb_channels);
  }

  std::printf("  time_base  : %d/%d\n", st->time_base.num, st->time_base.den);
  if (st->duration != AV_NOPTS_VALUE)
    std::printf("  duration   : %.3f s\n",
                st->duration * av_q2d(st->time_base));

  if (st->disposition & AV_DISPOSITION_DEFAULT)
    std::printf("  disposition: default\n");
  if (st->disposition & AV_DISPOSITION_FORCED)
    std::printf("  disposition: forced\n");

  for (const AVDictionaryEntry *e = nullptr;
       (e = av_dict_iterate(st->metadata, e));)
    std::printf("  meta       : %s=%s\n", e->key, e->value);
  std::printf("\n");
}

void print_format(const AVFormatContext *fc) {
  std::printf("[format]\n");
  std::printf("  file       : %s\n", fc->url ? fc->url : "(n/a)");
  std::printf("  format     : %s (%s)\n", fc->iformat->name,
              fc->iformat->long_name);
  if (fc->duration != AV_NOPTS_VALUE)
    std::printf("  duration   : %.3f s\n", fc->duration / double(AV_TIME_BASE));
  if (fc->start_time != AV_NOPTS_VALUE)
    std::printf("  start_time : %.3f s\n",
                fc->start_time / double(AV_TIME_BASE));
  if (fc->bit_rate > 0)
    std::printf("  bit_rate   : %" PRId64 " bps\n", fc->bit_rate);
  std::printf("\n");
}

} // namespace

int main(int argc, char **argv) {
  if (argc < 2) {
    std::fprintf(stderr, "usage: %s <media file>\n", argv[0]);
    return 2;
  }
  const char *path = argv[1];

  AVFormatContext *raw = nullptr;
  int ret = avformat_open_input(&raw, path, nullptr,
                                nullptr); // 실패 시 내부에서 free됨
  if (ret < 0) {
    std::fprintf(stderr, "open failed '%s': %s\n", path,
                 ff::errstr(ret).c_str());
    return 1;
  }
  ff::FormatCtx fmt{raw}; // 이 지점부터 RAII 소유

  // 컨테이너 헤더에 없는 파라미터를 일부 구간 파싱으로 보완 (MKV는 대부분
  // 헤더에 있어 저비용)
  ret = avformat_find_stream_info(fmt.get(), nullptr);
  if (ret < 0) {
    std::fprintf(stderr, "find_stream_info failed: %s\n",
                 ff::errstr(ret).c_str());
    return 1;
  }

  print_format(fmt.get());
  for (unsigned i = 0; i < fmt->nb_streams; ++i)
    print_stream(fmt->streams[i]);

  // 플레이어 트랙 선택 로직 미리보기 (M1에서 실제 사용)
  int vi =
      av_find_best_stream(fmt.get(), AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
  int ai =
      av_find_best_stream(fmt.get(), AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
  std::printf("[selection]\n");
  if (vi >= 0)
    std::printf("  video      : stream #%d\n", vi);
  else
    std::printf("  video      : (none)\n");
  if (ai >= 0)
    std::printf("  audio      : stream #%d\n", ai);
  else
    std::printf("  audio      : (none)\n");
  return 0;
}