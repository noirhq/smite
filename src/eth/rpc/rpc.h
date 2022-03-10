// This file is part of NOIR.
//
// Copyright (c) 2022 Haderech Pte. Ltd.
// SPDX-License-Identifier: AGPL-3.0-or-later
//
#pragma once
#include <noir/consensus/abci.h>
#include <noir/rpc/jsonrpc.h>
#include <noir/tx_pool/tx_pool.h>
#include <appbase/application.hpp>
#include <eth/common/types.h>
#include <eth/rpc/api.h>

namespace eth::rpc {

class rpc : public appbase::plugin<rpc> {
public:
  rpc();

  APPBASE_PLUGIN_REQUIRES((noir::rpc::jsonrpc)(noir::consensus::abci)(noir::tx_pool::tx_pool))
  void set_program_options(CLI::App& config) override;

  void plugin_initialize(const CLI::App& config);
  void plugin_startup();
  void plugin_shutdown();

private:
  std::unique_ptr<api::api> api;
};

} // namespace eth::rpc
