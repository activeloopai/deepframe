#include "video.hpp"

#include <algorithm>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

namespace {
core::buffer_ptr extract_video_frames_from_video_at_url_(
    const std::string &url, const std::vector<int64_t> &original_indices,
    AVPixelFormat force_pixel_format = AV_PIX_FMT_NONE) {

  // Pair each element with its original position
  // this is used to keep track of the original index after sorting
  std::vector<std::pair<int, int>> indexed_list;
  for (size_t i = 0; i < original_indices.size(); i++) {
    indexed_list.emplace_back(i, original_indices[i]);
  }

  // Sort by the second element (value) while keeping track of the original
  // index
  std::sort(indexed_list.begin(), indexed_list.end(),
            [](const std::pair<int, int> &a, const std::pair<int, int> &b) {
              return a.second < b.second; // Sort based on value
            });

  auto start_frame = indexed_list.begin()->second;

  if (start_frame < 0) {
    start_frame = 0;
  }

  const char *filename = url.c_str();

  // Open the video file
  AVFormatContext *formatContext = nullptr;
  if (avformat_open_input(&formatContext, filename, nullptr, nullptr) != 0) {
    std::cerr << "Could not open video file: " << filename << std::endl;
    return nullptr;
  }

  // av_dump_format(formatContext, 0, filename, 0);

  int videoStreamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_VIDEO,
                                             -1, -1, nullptr, 0);
  if (videoStreamIndex < 0) {
    std::cerr << "Could not find video stream" << std::endl;
    avformat_close_input(&formatContext);
    return nullptr;
  }
  AVStream *videoStream = formatContext->streams[videoStreamIndex];
  AVCodecParameters *codecParams = videoStream->codecpar;

  if (codecParams == nullptr) {
    std::cerr << "Could not find codec parameters" << std::endl;
    avformat_close_input(&formatContext);
    return nullptr;
  }

  // Find the decoder
  const AVCodec *codec = avcodec_find_decoder(codecParams->codec_id);
  if (!codec) {
    std::cerr << "Could not find codec" << std::endl;
    avformat_close_input(&formatContext);
    return nullptr;
  }

  // Create codec context
  AVCodecContext *codecContext = avcodec_alloc_context3(codec);
  if (!codecContext) {
    std::cerr << "Could not allocate codec context" << std::endl;
    avformat_close_input(&formatContext);
    return nullptr;
  }

  if (avcodec_parameters_to_context(codecContext, codecParams) < 0) {
    std::cerr << "Could not copy codec parameters to context" << std::endl;
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    return nullptr;
  }

  // Validate video dimensions
  if (codecContext->width <= 0 || codecContext->height <= 0) {
    std::cerr << "Invalid video dimensions: " << codecContext->width << "x"
              << codecContext->height << std::endl;
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    return nullptr;
  }

  // Open the codec
  if (avcodec_open2(codecContext, codec, nullptr) < 0) {
    std::cerr << "Could not open codec" << std::endl;
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    return nullptr;
  }

  // Initialize packet and frame
  AVPacket *packet = av_packet_alloc();
  AVFrame *frame = av_frame_alloc();
  if (!packet || !frame) {
    std::cerr << "Error: Could not allocate packet or frame" << std::endl;
    av_packet_free(&packet);
    av_frame_free(&frame);
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    return nullptr;
  }

  // Ensure pixel format is initialized
  if (codecContext->pix_fmt == AV_PIX_FMT_NONE) {
    std::cerr << "Pixel format is not set in codec context. Setting default "
                 "pixel format to AV_PIX_FMT_YUV420P."
              << std::endl;
    // Set the pixel format explicitly based on the input file information
    // TODO: check if there is a faster option
    codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
  }

  if (force_pixel_format != AV_PIX_FMT_NONE) {
    codecContext->pix_fmt = force_pixel_format;
  }

  // Initialize SwsContext for converting frames
  SwsContext *swsContext =
      sws_getContext(codecContext->width, codecContext->height,
                     codecContext->pix_fmt, // Input dimensions and format
                     codecContext->width, codecContext->height,
                     AV_PIX_FMT_RGB24, // Output dimensions and format
                     SWS_BILINEAR, nullptr, nullptr, nullptr);

  if (!swsContext) {
    std::cerr << "Could not initialize SwsContext" << std::endl;
    av_packet_free(&packet);
    av_frame_free(&frame);
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    return nullptr;
  }

  auto frames_buffer =
      core::buffer::create({static_cast<ssize_t>(original_indices.size()),
                            codecContext->height, codecContext->width, 3});

  int64_t tried_to_seek_to_frame_ = -1;

  // Calculate frame rate and time base
  float frame_rate = av_q2d(videoStream->avg_frame_rate);
  float time_base = av_q2d(videoStream->time_base);

  auto start_pts = static_cast<int64_t>(start_frame / frame_rate / time_base);
  av_seek_frame(formatContext, videoStreamIndex, start_pts,
                AVSEEK_FLAG_BACKWARD);

  int64_t start_pts_in_source =
      (videoStream->start_time == AV_NOPTS_VALUE) ? 0 : videoStream->start_time;

  auto indices_it = indexed_list.begin();

  auto processFrame = [&](AVFrame *frame) -> bool {
    if (indices_it == indexed_list.end()) {
      return true;
    }
    auto current_frame = static_cast<int64_t>(
        (frame->pts - start_pts_in_source) * time_base * frame_rate);
    if (current_frame > start_frame) {
      std::cerr << "invalid frame position: " << current_frame << " > "
                << start_frame << std::endl;
      assert(false);
      return true;
    }

    bool needs_seek =
        ((start_frame - current_frame) > 300) && tried_to_seek_to_frame_ == -1;

    if (needs_seek) {
      // Seek to the approximate starting point
      auto start_pts =
          static_cast<int64_t>(start_frame / frame_rate / time_base) +
          start_pts_in_source;
      av_seek_frame(formatContext, videoStreamIndex, start_pts,
                    AVSEEK_FLAG_BACKWARD);
      avcodec_flush_buffers(codecContext);

      tried_to_seek_to_frame_ = start_frame;
      return false;
    }

    if (current_frame == start_frame) {
      if (indices_it->second != current_frame) {
        std::cerr << "invalid state: current_frame: " << current_frame
                  << " was not found in the indices" << std::endl;
        return true;
      }

      // Check the frame details
      if (frame->width != codecContext->width ||
          frame->height != codecContext->height) {
        std::cerr << "Frame resolution mismatch!" << std::endl;
        return false; // Continue processing next frames
      }

      int64_t offset =
          indices_it->first * codecContext->height * codecContext->width * 3;

      uint8_t *chunks[AV_NUM_DATA_POINTERS] = {nullptr};
      chunks[0] = frames_buffer->buffer_at_offset(offset);

      int linesize[AV_NUM_DATA_POINTERS] = {0};
      linesize[0] = 3 * codecContext->width;

      sws_scale(swsContext, frame->data, frame->linesize, 0,
                codecContext->height, chunks, linesize);

      ++indices_it;

      while (indices_it != indexed_list.end() &&
             indices_it->second == current_frame) {
        auto current_offset =
            indices_it->first * codecContext->height * codecContext->width * 3;
        std::memcpy(frames_buffer->buffer_at_offset(current_offset), chunks[0],
                    codecContext->height * codecContext->width * 3);
        ++indices_it;
      }
      if (indices_it != indexed_list.end()) {
        start_frame = indices_it->second;
        tried_to_seek_to_frame_ = -1;
      }
    }

    if (indices_it == indexed_list.end()) {
      return true;
    }

    return false; // Continue processing
  };

  // Process frames
  while (av_read_frame(formatContext, packet) >= 0) {
    if (packet->stream_index == videoStreamIndex) {
      if (avcodec_send_packet(codecContext, packet) == 0) {
        while (avcodec_receive_frame(codecContext, frame) == 0) {
          if (processFrame(frame)) {
            break;
          }
        }
      }
    }
    av_packet_unref(packet);
    if (indices_it == indexed_list.end()) {
      break;
    }
  }

  // from https://github.com/FFmpeg/FFmpeg/blob/master/libavcodec/avcodec.h
  //
  // docs https://ffmpeg.org/doxygen/5.1/group__lavc__encdec.html
  /**
   * End of stream situations. These require "flushing" (aka draining) the
   * codec, as the codec might buffer multiple frames or packets internally
   for
   * performance or out of necessity (consider B-frames).
   * This is handled as follows:
   * - Instead of valid input, send NULL to the avcodec_send_packet()
   (decoding)
   *   or avcodec_send_frame() (encoding) functions. This will enter draining
   *   mode.
   * - Call avcodec_receive_frame() (decoding) or avcodec_receive_packet()
   *   (encoding) in a loop until AVERROR_EOF is returned. The functions will
   *   not return AVERROR(EAGAIN), unless you forgot to enter draining mode.
   * - Before decoding can be resumed again, the codec has to be reset with
   *   avcodec_flush_buffers().
   */
  if (indices_it != indexed_list.end()) {
    avcodec_send_packet(codecContext, NULL);
    while (avcodec_receive_frame(codecContext, frame) >= 0) {
      if (processFrame(frame)) {
        break;
      }
    }
  }

  // Clean up
  sws_freeContext(swsContext);
  av_packet_free(&packet);
  av_frame_free(&frame);
  avcodec_free_context(&codecContext);
  avformat_close_input(&formatContext);

  return frames_buffer;
}
} // namespace

namespace codecs {
std::map<std::string, double, std::less<>>
get_video_info(const std::string &url) {
  std::map<std::string, double, std::less<>> info;
  AVFormatContext *format_ctx = nullptr;

  // Open the input video
  if (avformat_open_input(&format_ctx, url.c_str(), nullptr, nullptr) != 0) {
    std::cerr << "Could not open video file: " << url << std::endl;
    return {};
  }

  // Retrieve stream information
  if (avformat_find_stream_info(format_ctx, nullptr) != 0) {
    std::cerr << "Could not find stream info for video file: " << url
              << std::endl;
    avformat_close_input(&format_ctx);
    return {};
  }

  // Find the video stream
  int video_stream_index = -1;
  for (unsigned int i = 0; i < format_ctx->nb_streams; i++) {
    if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      video_stream_index = i;
      break;
    }
  }

  if (video_stream_index == -1) {
    std::cerr << "No video stream found" << std::endl;
    avformat_close_input(&format_ctx);
    return {};
  }

  AVStream *video_stream = format_ctx->streams[video_stream_index];
  AVCodecParameters *codecpar = video_stream->codecpar;

  // Fill video_info
  info["width"] = codecpar->width;
  info["height"] = codecpar->height;
  info["num_channels"] = 3; // Currently RGB is supported
  info["num_frames"] = video_stream->nb_frames;

  if (info["num_frames"] == 0) {
    // Estimate frames if nb_frames is unavailable
    AVRational framerate = video_stream->avg_frame_rate;
    auto duration = video_stream->duration;
    if (duration > 0 && framerate.num > 0 && framerate.den > 0) {
      info["num_frames"] = static_cast<uint64_t>(
          av_q2d(framerate) * duration * av_q2d(video_stream->time_base));
    }
  }

  AVRational fps_rational = video_stream->avg_frame_rate;
  info["fps"] = fps_rational.den > 0
                    ? static_cast<double>(fps_rational.num) / fps_rational.den
                    : 0;
  info["duration"] = video_stream->duration;
  info["time_base_num"] = video_stream->time_base.num;
  info["time_base_den"] = video_stream->time_base.den;

  // Cleanup
  avformat_close_input(&format_ctx);

  return info;
}

core::buffer_ptr
extract_video_frames_from_video_at_url(const std::string &url,
                                       const std::vector<int64_t> &indices) {
  return extract_video_frames_from_video_at_url_(url, indices,
#if __APPLE__
                                                 AV_PIX_FMT_YUV420P
#else
                                                 AV_PIX_FMT_NONE
#endif
  );
}

} // namespace codecs
