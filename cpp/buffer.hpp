#pragma once

#include <assert.h>
#include <iostream>
#include <numeric>
#include <vector>

namespace core {
class exception : public std::exception {
public:
  exception(const std::string &what) : what_(what) {}
  const char *what() const noexcept override { return what_.c_str(); }

private:
  std::string what_;
};

class buffer_overflow_exception : public exception {
public:
  buffer_overflow_exception(int64_t size, size_t max_size)
      : exception(std::string(
            "buffer overflow exception, tried to get: " + std::to_string(size) +
            " allowed max size: " + std::to_string(max_size))) {}
};

class buffer;
typedef std::shared_ptr<buffer> buffer_ptr;

class buffer : public std::enable_shared_from_this<buffer> {
public:
  static buffer_ptr create(const std::vector<ssize_t> &dims) {
    return std::shared_ptr<buffer>(new buffer(dims));
  }

  int64_t size() const { return allocated_size_; }
  const std::vector<ssize_t> &stride() const { return stride_; }
  const std::vector<ssize_t> &dims() const { return dims_; }

  uint8_t *data() const { return data_; }
  uint8_t *buffer_at_offset(int64_t offset) const {
    assert(offset < allocated_size_);
    if (offset >= allocated_size_) {
      throw buffer_overflow_exception(offset, allocated_size_);
    }
    return data_ + offset;
  }

  ~buffer() { delete[] data_; }

private:
  buffer(const std::vector<ssize_t> &dims) {
    assert(dims.size() > 0);

    dims_ = dims;
    allocated_size_ = std::accumulate(dims_.begin(), dims_.end(), 1,
                                      std::multiplies<ssize_t>()) *
                      sizeof(uint8_t);
    
    stride_.resize(dims_.size());
    stride_.back() = sizeof(uint8_t);
    for (auto i = dims_.size() - 1; i > 0; --i) {
      stride_[i - 1] = stride_[i] * dims_[i];
    }

    data_ = new uint8_t[allocated_size_];
  }

  std::vector<ssize_t> dims_;
  std::vector<ssize_t> stride_;
  uint8_t *data_ = nullptr;
  ssize_t allocated_size_ = 0;
};
} // namespace core