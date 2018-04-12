#include <string>

#include "eosiolib/asset.hpp"
#include "eosiolib/eosio.hpp"
#include "eosio.token/eosio.token.hpp"

namespace eosio {

using std::string;

class chainex : public token {
 public:
  chainex(account_name self) : _this_contract(self), _extokens(self), _accounts(self) {}

  void deposit(account_name from, extended_asset quantity);

  void withdraw(account_name from, extended_asset quantity);

  void trade(account_name name, string side, uint128 price, string symbol, uint64 quantity, time expiration);

  void cancel_order(account_name name, uint64_t id);

 private:
  account_name _this_contract;
  token _extokens;
  chainex_accounts _accounts;

  struct market_order {
    extended_symbol quote;
    extended_symbol base;
    uint64_t price; // original price * 10000
    uint64_t quantity; // original quantity * 100
    account_name name;
    time expiration;
    time created_at; // delete too old orders according to created_at time
    uint64_t id; // order id
    uint64_t primary_key() const {
      return id;
    }
  };

  struct orderbook {
    extended_symbol quote;
    extended_symbol base;
    std::vector <market_order> asks;
    std::vector <market_order> bids;
    uint64_t primary_key() const {
      return quote.name();
    }
  };

  typedef eosio::multi_index<N(fullorderbooks), orderbook> full_orderbooks;
  typedef eosio::multi_index<N(activeorderbooks), orderbook> active_orderbooks;
};

} // namespace eosio
