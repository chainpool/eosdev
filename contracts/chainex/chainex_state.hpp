#pragma once

#include "eosiolib/asset.hpp"

namespace eosio {
typedef double real_type;

struct chainex_state {
  account_name manager;
  extended_asset supply;

  uint32_t fee = 0;

  struct connector {
    extended_asset balance;
    uint32_t weight = 500;

    EOSLIB_SERIALIZE(connector, (balance)(weight))
  };

  connector base;
  connector quote;
  uint64_t primary_key() const { return supply.symbol.name(); }

  extended_asset convert_to_exchange(connector &c, extended_asset in);
  extended_asset convert_from_exchange(connector &c, extended_asset in);
  extended_asset convert(extended_asset from, extended_symbol to);

  EOSLIB_SERIALIZE(chainex_state, (manager)(supply)(fee)(base)(quote))
};

typedef eosio::multi_index<N(markets), chainex_state> markets;

} // namespace eosio