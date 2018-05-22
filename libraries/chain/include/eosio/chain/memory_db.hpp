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
      struct account_info {
         account_name     name;
         asset            balance;

         uint64_t primary_key()const { return name; }
      };

      struct producer_info {
         account_name      name;
         asset             total_votes;
         public_key_type   producer_key;
         asset             rewards_pool;
         uint32_t          commission_rate;
         int64_t           total_votes_age;
         uint32_t          update_votes_age;

         uint64_t primary_key() const { return name; }
      };
};
} } // namespace eosio::chain

FC_REFLECT(eosio::chain::memory_db::account_info, (name)(balance))
FC_REFLECT(eosio::chain::memory_db::producer_info, (name)(total_votes)
  (producer_key)(rewards_pool)(commission_rate)(total_votes_age)(update_votes_age))

