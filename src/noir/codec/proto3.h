// This file is part of NOIR.
//
// Copyright (c) 2022 Haderech Pte. Ltd.
// SPDX-License-Identifier: AGPL-3.0-or-later
//
#pragma once
#include <noir/codec/datastream.h>
#include <noir/codec/proto3/types.h>
#include <noir/common/concepts.h>
#include <noir/common/refl.h>

NOIR_CODEC(proto3) {

template<typename Stream, integral T>
datastream<Stream>& operator<<(datastream<Stream>& ds, const T& v) {
  write_uleb128(ds, varint<T>(v));
  return ds;
}

template<typename Stream, integral T>
datastream<Stream>& operator>>(datastream<Stream>& ds, T& v) {
  v = 0;
  varint<T> val;
  read_uleb128(ds, val);
  v = val;
  return ds;
}

template<typename Stream, typename T>
datastream<Stream>& operator<<(datastream<Stream>& ds, const sint<T>& v) {
  write_zigzag(ds, v);
  return ds;
}

template<typename Stream, typename T>
datastream<Stream>& operator>>(datastream<Stream>& ds, sint<T>& v) {
  v = 0;
  read_zigzag(ds, v);
  return ds;
}

template<typename Stream, typename T>
datastream<Stream>& operator<<(datastream<Stream>& ds, const fixed<T>& v) {
  ds.write((const char*)&v.value, sizeof(T));
  return ds;
}

template<typename Stream, typename T>
datastream<Stream>& operator>>(datastream<Stream>& ds, fixed<T>& v) {
  v = 0;
  ds.read((char*)&v.value, sizeof(T));
  return ds;
}

template<typename Stream>
datastream<Stream>& operator<<(datastream<Stream>& ds, const std::string& v) {
  ds.write(v.data(), v.size());
  return ds;
}

template<typename Stream>
datastream<Stream>& operator>>(datastream<Stream>& ds, std::string& v) {
  auto size = ds.remaining();
  v.resize(size);
  ds.read(v.data(), size);
  return ds;
}

template<typename Stream>
datastream<Stream>& operator<<(datastream<Stream>& ds, const bytes& v) {
  ds.write(v.data(), v.size());
  return ds;
}

template<typename Stream>
datastream<Stream>& operator>>(datastream<Stream>& ds, bytes& v) {
  auto size = ds.remaining();
  v.resize(size);
  ds.read(v.data(), size);
  return ds;
}

template<typename Stream, typename T>
datastream<Stream>& operator<<(datastream<Stream>& ds, const std::optional<T>& v) {
  if (v) {
    ds << *v;
  }
  return ds;
}

template<typename Stream, typename T>
datastream<Stream>& operator>>(datastream<Stream>& ds, std::optional<T>& v) {
  T tmp;
  ds >> tmp;
  v = std::forward<T>(tmp);
  return ds;
}

template<typename Stream, integral T>
datastream<Stream>& operator<<(datastream<Stream>& ds, const std::vector<T>& v) {
  for (const auto& e : v) {
    ds << e;
  }
  return ds;
}

template<typename Stream, integral T>
datastream<Stream>& operator>>(datastream<Stream>& ds, std::vector<T>& v) {
  while (ds.remaining()) {
    T tmp;
    ds >> tmp;
    v.push_back(tmp);
  }
  return ds;
}

template<typename Stream, reflection T>
datastream<Stream>& operator<<(datastream<Stream>& ds, const T& v) {
  auto fields_count = refl::fields_count_v<T>;
  auto tag = 1;
  while (fields_count && tag < max_tag) {
    refl::for_each_field(
      [&](const auto& desc, const auto& value) {
        if (desc.tag == tag) {
          auto type = wire_type_v<std::decay_t<decltype(value)>>;
          ds.put((desc.tag << 3) | type);
          if (type == 2) {
            ds << encode_size(value);
          }
          ds << value;
          fields_count -= 1;
          return false;
        }
        return true;
      },
      v);
    tag += 1;
  }
  return ds;
}

template<typename Stream, reflection T>
datastream<Stream>& operator>>(datastream<Stream>& ds, T& v) {
  auto fields_count = refl::fields_count_v<T>;
  auto processed = 0;
  while (fields_count) {
    varuint32 key;
    read_uleb128(ds, key);
    auto tag = key.value >> 3;
    // for canonical representation, original specification doesn't have this limitation.
    check(tag > processed, "not encoded in canonical sequence");
    processed = tag;
    auto type = key & 0b111;
    auto ret = refl::for_each_field(
      [&](const auto& desc, auto& value) {
        if (desc.tag == tag) {
          check(type == wire_type_v<std::decay_t<decltype(value)>>, "wire type unmatched");
          if (type == 2) {
            size_t size = 0;
            ds >> size;
            datastream<Stream> subds(ds.subspan(ds.tellp(), size));
            subds >> value;
            ds.skip(size);
          } else {
            ds >> value;
          }
          fields_count -= 1;
          return false;
        }
        return true;
      },
      v);
    check(!ret, "field not found with tag: {}", tag);
    if (ds.remaining() == 0) {
      break;
    }
  }
  return ds;
}

} // NOIR_CODEC(proto3)
