// This file is part of NOIR.
//
// Copyright (c) 2022 Haderech Pte. Ltd.
// SPDX-License-Identifier: AGPL-3.0-or-later
//
#pragma once
#include <noir/common/check.h>
#include <span>
#include <stdexcept>
#include <vector>

/// \defgroup codec Codec
/// \brief Data (de)serialization

/// \brief codec namespace
/// \ingroup codec
namespace noir::codec {

/// \addtogroup codec
/// \{

/// \brief wraps a buffer of characters and provides high-level input/output interface
template<typename T>
class basic_datastream {
  static_assert(std::is_same_v<std::remove_cv_t<T>, char> || std::is_same_v<std::remove_cv_t<T>, unsigned char>,
    "datastream can be initialized by char type only");

public:
  /// \brief constructs new datastream object
  /// \param s contiguous sequence of characters
  basic_datastream(std::span<T> s): buf(s), pos(s.begin()) {}

  /// \brief constructs new datastream object
  /// \param s pointer to the first element of the sequence
  /// \param count number of elements in the sequence
  basic_datastream(T* s, size_t count): buf(s, count), pos(buf.begin()) {}

  /// \brief move forward the position indicator without extracting
  /// \param s offset from the current position
  /// \return `*this`
  auto& skip(size_t s) {
    pos += s;
    return *this;
  }

  /// \brief extracts characters from stream
  /// \param reference to the range of characters to store the characters to
  /// \return `*this`
  auto& read(std::span<char> s) {
    check<std::out_of_range>(remaining() >= s.size(), "datastream attempted to read past the end");
    std::copy(pos, pos + s.size(), s.begin());
    pos += s.size();
    return *this;
  }

  /// \brief extracts characters from stream
  /// \param s pointer to the character array to store the characters to
  /// \param count  number of characters to read
  /// \return `*this`
  auto& read(char* s, size_t count) {
    return read({s, count});
  }

  /// \brief extracts characters from stream in reverse order
  /// \param reference to the range of characters to store the characters to
  /// \return `*this`
  auto& reverse_read(std::span<char> s) {
    check<std::out_of_range>(remaining() >= s.size(), "datastream attempted to read past the end");
    std::reverse_copy(pos, pos + s.size(), s.begin());
    pos += s.size();
    return *this;
  }

  /// \brief reads the next character without extracting it
  /// \return the next character
  int peek() {
    check<std::out_of_range>(remaining() >= 1, "datastream attempted to read past the end");
    return *pos;
  }

  /// \brief extracts a character
  /// \return the extracted character
  int get() {
    auto c = peek();
    ++pos;
    return c;
  }

  /// \brief extracts a character
  /// \param reference to the character to write the result to
  /// \return `*this`
  auto& get(char& c) {
    c = get();
    return *this;
  }

  /// \brief makes the most recently extracted character available again
  /// \return `*this`
  auto& unget() {
    check<std::out_of_range>(tellp() >= 1, "datastream attempted to read past the beginning");
    --pos;
    return *this;
  }

  /// \brief inserts characters to stream
  /// \param s contiguous sequence of characters
  /// \return `*this`
  auto& write(std::span<const char> s) {
    check<std::out_of_range>(remaining() >= s.size(), "datastream attempted to write past the end");
    std::copy(s.begin(), s.end(), pos);
    pos += s.size();
    return *this;
  }

  /// \brief inserts characters to stream
  /// \param s pointer to the character string to write
  /// \param count number of characters to write
  /// \return `*this`
  auto& write(const void* c, size_t count) {
    return write({(const char*)c, count});
  }

  /// \brief inserts characters to stream in reverse order
  /// \param s contiguous sequence of characters
  /// \return `*this`
  auto& reverse_write(std::span<const char> s) {
    check<std::out_of_range>(remaining() >= s.size(), "datastream attempted to write past the end");
    std::reverse_copy(s.begin(), s.end(), pos);
    pos += s.size();
    return *this;
  }

  /// \brief inserts a character
  /// \param c character to write
  /// \return `*this`
  auto& put(char c) {
    check<std::out_of_range>(remaining() >= 1, "datastream attempted to write past the end");
    *pos++ = c;
    return *this;
  }

  /// \brief sets the position indicator
  /// \param p offset from the first element of underlying buffer
  /// \return `*this`
  auto& seekp(size_t p) {
    pos = buf.begin() + p;
    return *this;
  }

  /// \brief returns the position indicator
  /// \return offset from the first element of underlying buffer
  size_t tellp() const {
    return std::distance(buf.begin(), pos);
  }

  /// \brief returns the remaining size
  /// \return offset to the end of underlying buffer
  size_t remaining() const {
    return std::distance(pos, buf.end());
  }

private:
  std::span<T> buf;
  typename std::span<T>::iterator pos;
};

/// \brief calculates the size needed for serialization
/// \ingroup codec
template<>
class basic_datastream<size_t> {
public:
  /// \brief constructs new datastream object
  /// \param init_size initial size
  constexpr basic_datastream(size_t init_size = 0): size(init_size) {}

  /// \brief increase the size indicator
  /// \param s value to be added to the calculated size
  /// \return `*this`
  constexpr auto& skip(size_t s) {
    size += s;
    return *this;
  }

  /// \brief increase the calculated size by the size of the given range
  /// \param s contiguous sequence of characters
  constexpr auto& write(std::span<const char> s) {
    size += s.size();
    return *this;
  }

  /// \brief increase the calculated size by the given value
  /// \param s value to be added to the calculated size
  constexpr auto& write(const void*, size_t s) {
    size += s;
    return *this;
  }

  /// \brief increase the calculated size by the size of the given range
  /// \param s contiguous sequence of characters
  constexpr auto& reverse_write(std::span<const char> s) {
    size += s.size();
    return *this;
  }

  /// \brief increase the calculated size by 1
  constexpr auto& put(char) {
    ++size;
    return *this;
  }

  /// \brief set the size indicator
  /// \param p value to be set to the calculated size
  /// \return `*this`
  constexpr auto& seekp(size_t p) {
    size = p;
    return *this;
  }

  /// \brief returns the calculated size
  /// \return calculated size
  constexpr size_t tellp() const {
    return size;
  }

  /// \brief always returns 0
  constexpr size_t remaining() const {
    return 0;
  }

private:
  size_t size;
};

/// \}

} // namespace noir::codec

#define NOIR_CODEC(CODEC) \
  namespace noir::codec::CODEC { \
    template<typename T> \
    class datastream : public basic_datastream<T> { \
    public: \
      using basic_datastream<T>::basic_datastream; \
    }; \
    template<typename T> \
    constexpr size_t encode_size(const T& v) { \
      datastream<size_t> ds; \
      ds << v; \
      return ds.tellp(); \
    } \
    template<typename T> \
    std::vector<char> encode(const T& v) { \
      auto buffer = std::vector<char>(encode_size(v)); \
      datastream<char> ds(buffer); \
      ds << v; \
      return buffer; \
    } \
    template<typename T> \
    T decode(std::span<const char> s) { \
      T v; \
      datastream<const char> ds(s); \
      ds >> v; \
      return v; \
    } \
  } \
  namespace noir::codec::CODEC
