/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once
#include <eosio/chain/contract_table_objects.hpp>
#include <eosio/chain/controller.hpp>
#include <fc/utility.hpp>
#include <sstream>
#include <algorithm>
#include <set>

namespace chainbase { class database; }

namespace eosio { namespace chain {

class controller;

class memory_db {
   /// Constructor
   public:
      memory_db(controller& con)
      :db(con.db())
      {
      }
   /// Database methods:
   public:
      int  db_store_i64( uint64_t code, uint64_t scope, uint64_t table, const account_name& payer, uint64_t id, const char* buffer, size_t buffer_size );

   private:

      const table_id_object* find_table( name code, name scope, name table );
      const table_id_object& find_or_create_table( name code, name scope, name table, const account_name &payer );
      void                   remove_table( const table_id_object& tid );

   /// Fields:
   public:
      chainbase::database&          db;  ///< database where state is stored
      struct account {
         account_name name;
         asset    balance;
         asset    lock_balance;
         uint64_t primary_key()const { return name; }
      };

      /*struct public_key {
        char data[34];
      };*/

      struct producer_info {
         account_name owner;
         uint64_t total_votes = 0;
         public_key_type producer_key;
         uint32_t last_produced_time = 0;
         uint32_t produced_blocks;
         asset rewards_pool = asset(0);
         uint32_t last_dividend;
         asset settlement_balance;
         double rewards_rate;

         uint64_t primary_key() const { return owner; }
         uint64_t by_votes() const { return total_votes; }
         //TODO:check producer_key
         bool active() const { return 34 == sizeof(producer_key); }
      };
};
} } // namespace eosio::chain

FC_REFLECT(eosio::chain::memory_db::account, (name)(balance)(lock_balance) )
FC_REFLECT(eosio::chain::memory_db::producer_info, (owner)(total_votes)(producer_key)(last_produced_time)(produced_blocks)(rewards_pool)(last_dividend)(settlement_balance)(rewards_rate))

