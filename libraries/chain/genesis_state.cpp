/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

#include <eosio/chain/genesis_state.hpp>

// these are required to serialize a genesis_state
#include <fc/smart_ref_impl.hpp>   // required for gcc in release mode

namespace eosio { namespace chain {

genesis_state::genesis_state() {
   initial_timestamp = fc::time_point::from_iso_string( "2018-05-02T12:00:00" );
   initial_key = fc::variant(eosio_root_key).as<public_key_type>();

   account_tuple acc_tuple = { fc::variant("EOS5zK6Eo9CrPJRVn2vKaqYRDBo7YMRjqy43DCASdTwryY9sWTRJ8").as<public_key_type>(), asset(1000000000), N(eosforce)};
   initial_account_list.push_back(acc_tuple);
}

chain::chain_id_type genesis_state::compute_chain_id() const {
   digest_type::encoder enc;
   fc::raw::pack( enc, *this );
   return chain_id_type{enc.result()};
}

} } // namespace eosio::chain
