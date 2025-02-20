#include "buffer.hpp"
#include "video.hpp"

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace {

std::vector<int64_t> slice_to_indexes(const py::slice &s, int64_t max_size) {
  ssize_t start = 0, stop = 0, step = 0, slicelength = 0;
  if (!s.compute(max_size, &start, &stop, &step, &slicelength)) {
    throw py::error_already_set();
  }
  std::vector<int64_t> indices;
  indices.reserve(slicelength);
  for (ssize_t i = start; i < stop; i += step) {
    indices.push_back(i);
  }
  return indices;
}

std::vector<int64_t>
compute_indexes(const std::variant<int64_t, pybind11::slice, pybind11::list,
                                   pybind11::tuple> &index,
                int64_t max_size) {
  return std::visit(
      [max_size]<typename T>(const T &val) -> std::vector<int64_t> {
        if constexpr (std::is_same_v<T, int64_t>) {
          return {val};
        } else if constexpr (std::is_same_v<T, pybind11::slice>) {
          return slice_to_indexes(val, max_size);
        } else {
          std::vector<int64_t> indices;
          indices.reserve(val.size());
          for (auto &i : val) {
            indices.push_back(pybind11::cast<int64_t>(i));
          }
          return indices;
        }
      },
      index);
}

} // namespace

PYBIND11_MODULE(pyvframe, m) {
  py::class_<core::buffer, std::shared_ptr<core::buffer>>(m, "Buffer",
                                                          py::buffer_protocol())
      .def_buffer([](core::buffer &buf) -> py::buffer_info {
        auto dims = buf.dims();
        auto stride = buf.stride();
        return py::buffer_info(
            buf.data(),                               /* Pointer to buffer */
            sizeof(uint8_t),                          /* Size of one scalar */
            py::format_descriptor<uint8_t>::format(), /* Python struct-style
                                                         format descriptor */
            dims.size(),                              /* Number of dimensions */
            dims,                                     /* Buffer dimensions */
            stride, /* Strides (in bytes) for each index */
            true    /* Buffer is read-only */
        );
      });

  m.def(
      "extract_video_frames_from_video_at_url",
      [](const std::string &url,
         std::variant<int64_t, py::slice, py::list, py::tuple> &index,
         int64_t max_size = std::numeric_limits<int64_t>::max()) {
        auto indices = compute_indexes(index, max_size);
        return codecs::extract_video_frames_from_video_at_url(url, indices);
      },
      py::arg("url"), py::arg("index"),
      py::arg("max_size") = std::numeric_limits<int64_t>::max());

  m.def(
      "get_video_info",
      [](const std::string &url) { return codecs::get_video_info(url); },
      py::arg("url"));
}
