#include "chainex.hpp"
#include "eosio.token/eosio.token.hpp"

namespace eosio {

void chainex::deposit(account_name from, extended_asset quantity) {
  eosio_assert(quantity.is_valid(), "invalid quantity");
  currency::inline_transfer(from, _this_contract, quantity, "deposit");
  _accounts.adjust_balance(from, quantity, "deposit");
}

void chainex::withdraw(account_name from, extended_asset quantity) {
  require_auth(from);
  eosio_assert(quantity.is_valid(), "invalid quantity");
  eosio_assert(quantity.amount >= 0, "cannot withdraw negative balance");
  _accounts.adjust_balance(from, -quantity);
  currency::inline_transfer(_this_contract, from, quantity, "withdraw");
}

void chainex::trade(account_name name, string side, uint128 price, string symbol, uint64 quantity, time expiration) {
  require_auth(name);
  
}

void chainex::cancel_order(account_name name, string id) {
  require_auth(name);
}

} // namespace eosio

EOSIO_ABI(eosio::chainex, (deposit)(withdraw))
