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

void chainex::createx(account_name creator, asset initial_supply, uint32_t fee,
                      extended_asset base_deposit,
                      extended_asset quote_deposit) {
  require_auth(creator);
  eosio_assert(initial_supply.is_valid(), "invalid initial supply");
  eosio_assert(initial_supply.amount > 0, "initial supply must be positive");
  eosio_assert(base_deposit.is_valid(), "invalid base deposit");
  eosio_assert(base_deposit.amount > 0, "base deposit must be positive");
  eosio_assert(quote_deposit.is_valid(), "invalid quote deposit");
  eosio_assert(quote_deposit.amount > 0, "quote deposit must be positive");
  eosio_assert(base_deposit.get_extended_symbol() !=
                   quote_deposit.get_extended_symbol(),
               "must exchange between two different currencies");

  print("base: ", base_deposit.get_extended_symbol());
  print("quote: ", quote_deposit.get_extended_symbol());

  auto exchange_symbol = initial_supply.symbol.name();
  print("marketid: ", exchange_symbol, " \n");

  markets exstates(_this_contract, exchange_symbol);
  auto existing = exstates.find(exchange_symbol);

  eosio_assert(existing == exstates.end(), "market already exists");
  exstates.emplace(creator, [&](auto &s) {
    s.manager = creator;
    s.supply = extended_asset(initial_supply, _this_contract);
    s.base.balance = base_deposit;
    s.quote.balance = quote_deposit;
  });

  // TODO: After token contract respects maximum supply limits, the maximum
  // supply here needs to be set appropriately.
  _extokens.create(_this_contract, asset(0, initial_supply.symbol), false,
                   false, false);

  _extokens.issue(_this_contract, initial_supply,
                  string("initial exchange tokens"));

  _accounts.adjust_balance(creator,
                           extended_asset(initial_supply, _this_contract),
                           "new exchange issue");
  _accounts.adjust_balance(creator, -base_deposit, "new exchange deposit");
  _accounts.adjust_balance(creator, -quote_deposit, "new exchange deposit");
}

} // namespace eosio

EOSIO_ABI(eosio::chainex, (deposit)(withdraw)(createx))
