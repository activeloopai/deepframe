#pragma once

#include "buffer.hpp"

#include <map>
#include <vector>

namespace codecs {
core::buffer_ptr extract_video_frames_from_video_at_url(const std::string& url,
                                                        const std::vector<int64_t>& original_indices);
std::map<std::string, double, std::less<>> get_video_info(const std::string& url);
} // namespace codecs
