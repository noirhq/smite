// This file is part of NOIR.
//
// Copyright (c) 2022 Haderech Pte. Ltd.
// SPDX-License-Identifier: AGPL-3.0-or-later
//
#pragma once
#include <noir/consensus/types/block.h>
#include <noir/consensus/types/validator.h>

namespace noir::consensus {

/// \brief SignedHeader is a header along with the commits that prove it.
struct signed_header {
  std::shared_ptr<noir::consensus::block_header> header;
  std::optional<noir::consensus::commit> commit;

  result<void> validate_basic(std::string chain_id) {
    return {}; // TODO
  }

  std::shared_ptr<::tendermint::types::SignedHeader> to_proto() {
    auto ret = std::make_shared<::tendermint::types::SignedHeader>();
    *ret->mutable_header() = *header->to_proto();
    if (commit.has_value())
      *ret->mutable_commit() = *commit->to_proto();
    return ret;
  }
};

struct light_block {
  std::shared_ptr<signed_header> s_header;
  std::shared_ptr<validator_set> val_set;

  result<void> validate_basic(std::string chain_id) {
    if (!s_header)
      return make_unexpected("missing signed header");
    if (!val_set)
      return make_unexpected("missing validator set");
    if (auto ok = s_header->validate_basic(chain_id); !ok)
      return make_unexpected(fmt::format("invalid signed header: {}", ok.error()));
    if (auto ok = val_set->validate_basic(); !ok)
      return make_unexpected(fmt::format("invalid validator set: {}", ok.error()));
    if (auto val_set_hash = val_set->get_hash(); val_set_hash != s_header->header->validators_hash)
      return make_unexpected(fmt::format("miss match of hash between validator set hash"));
    return {};
  }

  result<std::shared_ptr<::tendermint::types::LightBlock>> to_proto() {
    auto ret = std::make_shared<::tendermint::types::LightBlock>();
    if (!s_header)
      *ret->mutable_signed_header() = *s_header->to_proto();
    if (!val_set) {
      auto val_set_proto = val_set->to_proto();
      if (!val_set_proto)
        return make_unexpected(val_set_proto.error());
      *ret->mutable_validator_set() = *val_set_proto.value();
    }
    return ret;
  }
};

} // namespace noir::consensus