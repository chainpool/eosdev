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
};

} // namespace eosio
