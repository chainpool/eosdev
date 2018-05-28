#include <fc/exception/exception.hpp>
#include <fc/io/json.hpp>
#include <fc/variant_object.hpp>

#include <eosio/chain/genesis_state.hpp>

//using namespace eosio;
//using namespace eosio::chain;


int main(int argc, const char **argv) {
  const eosio::chain::genesis_state gs;
  const std::string path = "./genesis.json";
  fc::json::save_to_file<eosio::chain::genesis_state>( gs, path, true );

  return 0;
}
