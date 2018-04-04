#include <string>

#include "eosiolib/asset.hpp"
#include "eosiolib/eosio.hpp"
#include "eosio.token/eosio.token.hpp"

namespace eosio {

using std::string;

class chainex : public token {
public:
  chainex(account_name self):_this_contract(self),_extokens(self),_accounts(self){}

  void deposit(account_name from, extended_asset quantity);

  void withdraw(account_name from, extended_asset quantity);

  void trade(account_name name, string side, uint128 price, string symbol, uint64 quantity, time expiration);

  void cancel_order(account_name name, string id);

private:
  account_name _this_contract;
  token _extokens;
  chainex_accounts _accounts;

  struct market_order {
    uint128 price; // original price * 10000
    uint64 quantity; // original quantity * 100
  };

  // TODO: add expire time to each account, can be configured
  struct full_orderbook {
    string symbol; // <quote>_<base>, exchange pair. e.g. EOS_USD
    std::vector<market_order> asks;
    std::vector<market_order> bids;
    string primary_key() const {
      return symbol;
    }
  };

  struct active_orderbook {
    string symbol;
    std::vector<market_order> asks;
    std::vector<market_order> bids;
    string primary_key() const {
      return symbol;
    }
  };

    typedef eosio::multi_index<N(full_orderbook), full_orderbook> full_orderbooks;
    typedef eosio::multi_index<N(active_orderbook), active_orderbook> active_orderbooks;
};

} // namespace eosio
