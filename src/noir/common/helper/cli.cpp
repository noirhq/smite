// This file is part of NOIR.
//
// Copyright (c) 2022 Haderech Pte. Ltd.
// SPDX-License-Identifier: AGPL-3.0-or-later
//
#include <noir/common/helper/cli.h>

namespace CLI::detail {

template<>
bool lexical_conversion<noir::uint256_t, noir::uint256_t>(
  const std::vector<std::string>& strings, noir::uint256_t& output) {
  output = noir::uint256_t(strings[0]);
  return true;
}

template<>
bool lexical_conversion<noir::bytes32, noir::bytes32>(const std::vector<std::string>& strings, noir::bytes32& output) {
  output = noir::bytes32(strings[0]);
  return true;
}

} // namespace CLI::detail
