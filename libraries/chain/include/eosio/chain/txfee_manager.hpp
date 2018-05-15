/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once

#include <eosio/chain/types.hpp>
#include <eosio/chain/asset.hpp>

namespace eosio { namespace chain {

   class controller;

   class txfee_manager {
      public:

         explicit txfee_manager();

         bool check_transaction( const transaction& trx, account_name actor)const;

         asset get_required_fee( const transaction& trx)const;


      private:
        std::map<action_name, asset> fee_map;
   };

} } /// namespace eosio::chain
