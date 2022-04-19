// This file is part of NOIR.
//
// Copyright (c) 2022 Haderech Pte. Ltd.
// SPDX-License-Identifier: AGPL-3.0-or-later
//
#pragma once
#include <noir/p2p/types.h>
#include <optional>

namespace noir::consensus {

constexpr int64_t max_block_size_bytes{104857600};

struct block_params {
  int64_t max_bytes;
  int64_t max_gas;

  static block_params get_default() {
    return block_params{22020096, -1};
  }
};

struct evidence_params {
  int64_t max_age_num_blocks;
  int64_t max_age_duration; // todo - use duration
  int64_t max_bytes;

  static evidence_params get_default() {
    return evidence_params{100000, std::chrono::hours(48).count(), 1048576};
  }
};

struct validator_params {
  std::vector<std::string> pub_key_types;

  static validator_params get_default() {
    return validator_params{{"ed25519"}};
  }
};

struct version_params {
  uint64_t app_version;

  static version_params get_default() {
    return version_params{0};
  }
};

struct consensus_params {
  block_params block;
  evidence_params evidence;
  validator_params validator;
  version_params version;

  std::optional<std::string> validate_consensus_params() const {
    if (block.max_bytes <= 0)
      return "block.MaxBytes must be greater than 0.";
    if (block.max_bytes > max_block_size_bytes)
      return "block.MaxBytes is too big.";
    if (block.max_gas < -1)
      return "block.MaxGas must be greater or equal to -1.";
    // check evidence // todo - necessary?
    // if (validator.pub_key_types.empty())
    //  return "validator.pub_key_types must not be empty.";
    // check if key_type is known // todo
    return {};
  }

  static consensus_params get_default() {
    return consensus_params{block_params::get_default(), evidence_params::get_default(),
      validator_params::get_default(), version_params::get_default()};
  }

  bytes hash_consensus_params() {
    // todo
    return bytes{};
  }
};

} // namespace noir::consensus

NOIR_REFLECT(noir::consensus::block_params, max_bytes, max_gas);
NOIR_REFLECT(noir::consensus::evidence_params, max_age_num_blocks, max_age_duration, max_bytes);
NOIR_REFLECT(noir::consensus::validator_params, pub_key_types);
NOIR_REFLECT(noir::consensus::version_params, app_version);
NOIR_REFLECT(noir::consensus::consensus_params, block, evidence, validator, version);
