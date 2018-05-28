#include <fc/exception/exception.hpp>
#include <fc/io/json.hpp>
#include <fc/variant_object.hpp>

#include <eosio/chain/genesis_state.hpp>
#include <eosio/chain/name.hpp>

//using namespace eosio;
using namespace eosio::chain;

struct key_map {
    std::map<account_name, fc::crypto::private_key> keymap;
};
FC_REFLECT(key_map, (keymap))
int main(int argc, const char **argv) {
  eosio::chain::genesis_state gs;
  const std::string path = "./genesis.json";
  key_map my_keymap;
  key_map my_sign_keymap;
  for (int i = 0; i < 23; i++) {
      auto key = fc::crypto::private_key::generate<fc::ecc::private_key_shim>();
      auto pub_key = key.get_public_key();
      eosio::chain::account_tuple tu;
      tu.key = pub_key;
      tu.asset = eosio::chain::asset(10000);
      std::string name("biosbp");
      char mark = 'a' + i;
      name.append(1u, mark);
      tu.name = string_to_name(name.c_str());
      gs.initial_account_list.push_back(tu);

      auto sig_key = fc::crypto::private_key::generate<fc::ecc::private_key_shim>();
      auto sig_pub_key = sig_key.get_public_key();

      gs.initial_producer_map[string_to_name(name.c_str())] = sig_pub_key;
      my_keymap.keymap[string_to_name(name.c_str())] = key; 
      my_sign_keymap.keymap[string_to_name(name.c_str())] = sig_key; 
  }
  
  const std::string keypath = "./key.json";
  const std::string sigkeypath = "./sigkey.json";
  fc::json::save_to_file<eosio::chain::genesis_state>( gs, path, true );
  fc::json::save_to_file<key_map>( my_keymap, keypath, true );
  fc::json::save_to_file<key_map>( my_sign_keymap, sigkeypath, true );

  return 0;
}
