#include <ctime>

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

void chainex::trade(account_name name, string side, uint64_t price,
                    extended_symbol quote, extended_symbol base,
                    uint64_t quantity, time expiration) {
  require_auth(name);
  eosio_assert(quantity >= 0, "quantity must be positive");
  eosio_assert(price >= 0, "price must be positive");
  eosio_assert(symbol == "buy" || symbol == "sell",
               "symbol must be buy or sell")
      // TODO: check balance enough

      // get symbol order book
      full_orderbooks full_orderbook_table(_self, base);
  auto existing = full_orderbook_table.find(quote);

  // first order in the orderbook
  if (existing == full_orderbook_table.end()) {
    switch
      side {
      case "buy":
        full_orderbook_table.emplace(_self, [&](auto &s) {
          s.quote = quote;
          s.base = base;
          s.bids.push_back({
            price : price,
            quantity : quantity,
            name : name,
            expiration : expiration,
            created_at : now(),
            id : eosio::utilities::rand::random(uint64_t(std::time(nullptr)))
                .next(),
          });
        });
        break;
      case "sell":
        full_orderbook_table.emplace(_self, [&](auto &s) {
          s.quote = quote;
          s.base = base;
          s.asks.push_back({
            price : price,
            quantity : quantity,
            name : name,
            expiration : expiration,
            created_at : now(),
            id : eosio::utilities::rand::random(uint64_t(std::time(nullptr)))
                .next(),
          });
        });
        break;
      default:
        // assert error, and return
        return;
      }
    return;
  }

  // order match, if not matched, add to order book
  switch
    side {
    case "buy":
      auto itBid = full_orderbook_table[existing].bids.begin();
      if (price > *itBid.price) {
        auto itAsk = full_orderbook_table[existing].asks.begin();
        for (; itAsk != full_orderbook_table[existing].asks.end();) {
          if (price >= *itAsk.price) {
            // match ask order, manage order, fee and account balance
            // TODO: fee, now no fees.
            if (full_orderbook_table[existing].asks[itAsk].quantity >=
                quantity) {
              full_orderbook_table[existing].asks[itAsk].quantity -= quantity;
              // modify seller balance
              _accounts.adjust_balance(
                  full_orderbook_table[existing].asks[itAsk].name,
                  extended_asset(quantity, base), "trade");
              _accounts.adjust_balance(
                  full_orderbook_table[existing].asks[itAsk].name,
                  -extended_asset(quantity * price, quote), "trade");
              // modify buyer balance
              _accounts.adjust_balance(name, -extended_asset(quantity, base),
                                       "trade");
              _accounts.adjust_balance(
                  name, extended_asset(quantity * price, quote), "trade");
              quantity = 0;
              itAsk++;
            } else {
              quantity -= full_orderbook_table[existing].asks[itAsk].quantity;
              // delete order
              itAsk = full_orderbook_table[existing].asks.erase(existing);
              // modify seller balance
              _accounts.adjust_balance(
                  full_orderbook_table[existing].asks[itAsk].name,
                  extended_asset(
                      full_orderbook_table[existing].asks[itAsk].quantity,
                      base),
                  "trade");
              _accounts.adjust_balance(
                  full_orderbook_table[existing].asks[itAsk].name,
                  -extended_asset(
                      full_orderbook_table[existing].asks[itAsk].quantity *
                          price,
                      quote),
                  "trade");

              // modify buyer balance
              _accounts.adjust_balance(
                  name,
                  -extended_asset(
                      full_orderbook_table[existing].asks[itAsk].quantity,
                      base),
                  "trade");
              _accounts.adjust_balance(
                  name,
                  extended_asset(
                      full_orderbook_table[existing].asks[itAsk].quantity *
                          price,
                      quote),
                  "trade");
            }
            if (quantity == 0) {
              return;
            }
          } else {
            full_orderbook_table[existing].bids.push_front({
              price : price,
              quantity : quantity,
              name : name,
              expiration : expiration,
              created_at : now(),
              id : eosio::utilities::rand::random(uint64_t(std::time(nullptr)))
                  .next(),
            });
          }
        }
        // sold out all ask orders
        if (itAsk == full_orderbook_table[existing].asks.end()) {
          full_orderbook_table[existing].bids.push_front({
            price : price,
            quantity : quantity,
            name : name,
            expiration : expiration,
            created_at : now(),
            id : eosio::utilities::rand::random(uint64_t(std::time(nullptr)))
                .next(),
          });
        }
      }
      // insert sort
      itBid++;
      for (; itBid != full_orderbook_table[existing].bids.end(); itBid++) {
        if (price > *itBid.price) {
          // insert here
          full_orderbook_table[existing].bids.insert(itBid, {
            price : price,
            quantity : quantity,
            name : name,
            expiration : expiration,
            created_at : now(),
            id : eosio::utilities::rand::random(uint64_t(std::time(nullptr)))
                .next(),
          });
        } else {
          continue;
        }
      }
      if (itBid == full_orderbook_table[existing].bids.end()) {
        full_orderbook_table[existing].bids.push_back({
          price : price,
          quantity : quantity,
          name : name,
          expiration : expiration,
          created_at : now(),
          id : eosio::utilities::rand::random(uint64_t(std::time(nullptr)))
              .next(),
        });
      }
      break;
    case "sell":
      auto itAsk = full_orderbook_table[existing].asks.begin();
      if (price < *itAsk.price) {
        auto itBid = full_orderbook_table[existing].bids.begin();
        for (; itBid != full_orderbook_table[existing].bids.end();) {
          if (price <= *itBid.price) {
            // match ask order, manage order, fee and account balance
            // TODO: fee, now no fees.
            if (full_orderbook_table[existing].bids[itBid].quantity >=
                quantity) {
              full_orderbook_table[existing].bids[itBid].quantity -= quantity;
              // modify seller balance
              _accounts.adjust_balance(
                  full_orderbook_table[existing].bids[itBid].name,
                  extended_asset(quantity, base), "trade");
              _accounts.adjust_balance(
                  full_orderbook_table[existing].bids[itBid].name,
                  -extended_asset(quantity * price, quote), "trade");
              // modify buyer balance
              _accounts.adjust_balance(name, -extended_asset(quantity, base),
                                       "trade");
              _accounts.adjust_balance(
                  name, extended_asset(quantity * price, quote), "trade");
              quantity = 0;
              itBid++;
            } else {
              quantity -= full_orderbook_table[existing].bids[itBid].quantity;
              // delete order
              itBid = full_orderbook_table[existing].bids.erase(existing);
              // modify seller balance
              _accounts.adjust_balance(
                  full_orderbook_table[existing].bids[itBid].name,
                  extended_asset(
                      full_orderbook_table[existing].asks[itAsk].quantity,
                      base),
                  "trade");
              _accounts.adjust_balance(
                  full_orderbook_table[existing].bids[itBid].name,
                  -extended_asset(
                      full_orderbook_table[existing].asks[itAsk].quantity *
                          price,
                      quote),
                  "trade");

              // modify buyer balance
              _accounts.adjust_balance(
                  name,
                  -extended_asset(
                      full_orderbook_table[existing].bids[itBid].quantity,
                      base),
                  "trade");
              _accounts.adjust_balance(
                  name,
                  extended_asset(
                      full_orderbook_table[existing].bids[itBid].quantity *
                          price,
                      quote),
                  "trade");
            }
            if (quantity == 0) {
              return;
            }
          } else {
            full_orderbook_table[existing].asks.push_front({
              price : price,
              quantity : quantity,
              name : name,
              expiration : expiration,
              created_at : now(),
              id : eosio::utilities::rand::random(uint64_t(std::time(nullptr)))
                  .next(),
            });
          }
        }
        // sold out all ask orders
        if (itBid == full_orderbook_table[existing].bids.end()) {
          full_orderbook_table[existing].asks.push_front({
            price : price,
            quantity : quantity,
            name : name,
            expiration : expiration,
            created_at : now(),
            id : eosio::utilities::rand::random(uint64_t(std::time(nullptr)))
                .next(),
          });
        }
      }
      // insert sort
      // TODO: merge same price order
      itAsk++;
      for (; itAsk != full_orderbook_table[existing].asks.end(); itAsk++) {
        if (price < *itAsk.price) {
          // insert here
          full_orderbook_table[existing].asks.insert(itAsk, {
            price : price,
            quantity : quantity,
            name : name,
            expiration : expiration,
            created_at : now(),
            id : eosio::utilities::rand::random(uint64_t(std::time(nullptr)))
                .next(),
          });
        } else {
          continue;
        }
      }
      if (itAsk == full_orderbook_table[existing].asks.end()) {
        full_orderbook_table[existing].asks.push_back({
          price : price,
          quantity : quantity,
          name : name,
          expiration : expiration,
          created_at : now(),
          id : eosio::utilities::rand::random(uint64_t(std::time(nullptr)))
              .next(),
        });
      }
      break;
    }
}

void chainex::cancel_order(account_name name, uint64_t id) {
  require_auth(name);
}

} // namespace eosio

EOSIO_ABI(eosio::chainex, (deposit)(withdraw)(trade)(cancel_order))
