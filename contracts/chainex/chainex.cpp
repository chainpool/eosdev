#include "chainex/chainex.hpp"
#include "eosio.token/eosio.token.hpp"

namespace eosio {

// @abi action
void chainex::deposit(account_name from, extended_asset quantity) {
  eosio_assert(quantity.is_valid(), "invalid quantity");
  token::inline_transfer(from, _this_contract, quantity, "deposit");
  _accounts.adjust_balance(from, quantity, "deposit");
}

// @abi action
void chainex::withdraw(account_name from, extended_asset quantity) {
  require_auth(from);
  eosio_assert(quantity.is_valid(), "invalid quantity");
  eosio_assert(quantity.amount >= 0, "cannot withdraw negative balance");
  _accounts.adjust_balance(from, -quantity, "withdraw");
  token::inline_transfer(_this_contract, from, quantity, "withdraw");
}

/*
void chainex::trade(account_name name, std::string side, uint64_t price,
                    extended_symbol quote, extended_symbol base,
                    int64_t quantity, time expiration) {
  require_auth(name);

  // TODO: check balance enough

  // get symbol order book
  full_orderbooks full_orderbook_table(_self, base);
  auto existing = full_orderbook_table.find(quote);

  // first order in the orderbook
  if (existing == full_orderbook_table.end()) {
    if (side == "buy") {
      full_orderbook_table.emplace(_self, [&](auto &s) {
        s.quote = quote;
        s.base = base;
        s.bids.push_back({
                             .price = price,
                             .quantity = quantity,
                             .name = name,
                             .expiration = expiration,
                             .created_at = now(),
                             .id = now(),
                         });
      });
    } else if (side == "sell") {
      full_orderbook_table.emplace(_self, [&](auto &s) {
        s.quote = quote;
        s.base = base;
        s.asks.push_back({
                             .price = price,
                             .quantity = quantity,
                             .name = name,
                             .expiration = expiration,
                             .created_at = now(),
                             .id = now(),
                         });
      });
    }
    return;
  }

  // order match, if not matched, add to order book
  auto order_book = full_orderbook_table.get(quote);
  if (side == "buy") {
    auto itBid = order_book.bids.begin();
    if (price > itBid->price) {
      auto itAsk = order_book.asks.begin();
      for (; itAsk != order_book.asks.end(); itAsk++) {
        if (price >= itAsk->price) {
          // match ask order, manage order, fee and account balance
          // TODO: fee, now no fees.
          if (itAsk->quantity >= quantity) {
            itAsk->quantity -= quantity;
            // modify seller balance
            _accounts.adjust_balance(
                itAsk->name,
                extended_asset(quantity, base), "trade");
            _accounts.adjust_balance(
                itAsk->name,
                -extended_asset(quantity * int64_t(price), quote), "trade");
            // modify buyer balance
            _accounts.adjust_balance(name, -extended_asset(quantity, base),
                                     "trade");
            _accounts.adjust_balance(
                name, extended_asset(quantity * int64_t(price), quote), "trade");
            quantity = 0;
          } else {
            quantity -= itAsk->quantity;
            // delete order
            itAsk->status = "filled";
            // modify seller balance
            _accounts.adjust_balance(
                itAsk->name,
                extended_asset(
                    itAsk->quantity,
                    base),
                "trade");
            _accounts.adjust_balance(
                itAsk->name,
                -extended_asset(
                    itAsk->quantity * int64_t(price),
                    quote),
                "trade");

            // modify buyer balance
            _accounts.adjust_balance(
                name,
                -extended_asset(
                    itAsk->quantity,
                    base),
                "trade");
            _accounts.adjust_balance(
                name,
                extended_asset(
                    itAsk->quantity * int64_t(price),
                    quote),
                "trade");
          }
          if (quantity == 0) {
            full_orderbook_table.modify(existing, 0, [&](auto &a) {
              a.asks = order_book.asks;
              a.bids = order_book.bids;
            });
            return;
          }
        } else {
          order_book.bids.insert(order_book.bids.begin(), {
              .price = price,
              .quantity = quantity,
              .name = name,
              .expiration = expiration,
              .created_at = now(),
              .id = now(),
          });
        }
      }
      // sold out all ask orders
      if (itAsk == order_book.asks.end()) {
        order_book.bids.insert(order_book.bids.begin(), {
            .price = price,
            .quantity = quantity,
            .name = name,
            .expiration = expiration,
            .created_at = now(),
            .id = now(),
        });
      }
    }
    // insert sort
    itBid++;
    for (; itBid != order_book.bids.end(); itBid++) {
      if (price > itBid->price) {
        // insert here
        order_book.bids.insert(itBid, {
            .price = price,
            .quantity = quantity,
            .name = name,
            .expiration = expiration,
            .created_at = now(),
            .id = now(),
        });
      } else {
        continue;
      }
    }
    if (itBid == order_book.bids.end()) {
      order_book.bids.push_back({
                                    .price = price,
                                    .quantity = quantity,
                                    .name = name,
                                    .expiration = expiration,
                                    .created_at = now(),
                                    .id = now(),
                                });
    }
  } else if (side == "sell") {
    auto itAsk = order_book.asks.begin();
    if (price < itAsk->price) {
      auto itBid = order_book.bids.begin();
      for (; itBid != order_book.bids.end();) {
        if (price <= itBid->price) {
          // match ask order, manage order, fee and account balance
          // TODO: fee, now no fees.
          if (itBid->quantity >=
              quantity) {
            itBid->quantity -= quantity;
            // modify seller balance
            _accounts.adjust_balance(
                itBid->name,
                extended_asset(quantity, base), "trade");
            _accounts.adjust_balance(
                itBid->name,
                -extended_asset(quantity * int64_t(price), quote), "trade");
            // modify buyer balance
            _accounts.adjust_balance(name, -extended_asset(quantity, base),
                                     "trade");
            _accounts.adjust_balance(
                name, extended_asset(quantity * int64_t(price), quote), "trade");
            quantity = 0;
            itBid++;
          } else {
            quantity -= itBid->quantity;
            // delete order
            //itBid = order_book.bids.erase(quote);
            itBid->status = "filled";
            // modify seller balance
            _accounts.adjust_balance(
                itBid->name,
                extended_asset(
                    itAsk->quantity,
                    base),
                "trade");
            _accounts.adjust_balance(
                itBid->name,
                -extended_asset(
                    itAsk->quantity *
                        int64_t(price),
                    quote),
                "trade");

            // modify buyer balance
            _accounts.adjust_balance(
                name,
                -extended_asset(
                    itBid->quantity,
                    base),
                "trade");
            _accounts.adjust_balance(
                name,
                extended_asset(
                    itBid->quantity *
                        int64_t(price),
                    quote),
                "trade");
          }
          if (quantity == 0) {
            full_orderbook_table.modify(existing, 0, [&](auto &a) {
              a.asks = order_book.asks;
              a.bids = order_book.bids;
            });
            return;
          }
        } else {
          order_book.asks.insert(order_book.asks.begin(), {
              .price = price,
              .quantity = quantity,
              .name = name,
              .expiration = expiration,
              .created_at = now(),
              .id = now(),
          });
        }
      }
      // sold out all ask orders
      if (itBid == order_book.bids.end()) {
        order_book.asks.insert(order_book.asks.begin(), {
            .price = price,
            .quantity = quantity,
            .name = name,
            .expiration = expiration,
            .created_at = now(),
            .id = now(),
        });
      }
    }
    // insert sort
    // TODO: merge same price order
    itAsk++;
    for (; itAsk != order_book.asks.end(); itAsk++) {
      if (price < itAsk->price) {
        // insert here
        order_book.asks.insert(itAsk, {
            .price = price,
            .quantity = quantity,
            .name = name,
            .expiration = expiration,
            .created_at = now(),
            .id = now(),
        });
      } else {
        continue;
      }
    }
    if (itAsk == order_book.asks.end()) {
      order_book.asks.push_back({
                                    .price = price,
                                    .quantity = quantity,
                                    .name = name,
                                    .expiration = expiration,
                                    .created_at = now(),
                                    .id = now(),
                                });
    }
  }
  full_orderbook_table.modify(existing, 0, [&](auto &a) {
    a.asks = order_book.asks;
    a.bids = order_book.bids;
  });
}

// @abi action
void chainex::cancel_order(account_name name, uint64_t id) {
  std::ignore = id;
  require_auth(name);
}
*/

} // namespace eosio

// EOSIO_ABI(eosio::chainex, (deposit)(withdraw)(trade)(cancel_order))

EOSIO_ABI(eosio::chainex, (deposit)(withdraw))
