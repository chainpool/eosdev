/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once

#include <eosiolib/action.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/contract.hpp>
#include <eosiolib/datastream.hpp>
#include <eosiolib/optional.hpp>
#include <eosiolib/privileged.hpp>
#include <eosiolib/singleton.hpp>
#include <eosiolib/vector.hpp>

#include <string>

namespace eosiosystem {

using eosio::asset;
using eosio::indexed_by;
using eosio::const_mem_fun;

uint32_t endian_reverse_u32( uint32_t x )
{
   return (((x >> 0x18) & 0xFF)        )
        | (((x >> 0x10) & 0xFF) << 0x08)
        | (((x >> 0x08) & 0xFF) << 0x10)
        | (((x        ) & 0xFF) << 0x18)
        ;
}

struct block_header {
  checksum256 previous;
  time timestamp;
  checksum256 transaction_mroot;
  checksum256 action_mroot;
  checksum256 block_mroot;
  account_name producer;
  uint32_t schedule_version;
  eosio::optional<eosio::producer_schedule> new_producers;

  // explicit serialization macro is not necessary, used here only to improve
  // compilation time
  EOSLIB_SERIALIZE(block_header,
                   (previous)(timestamp)(transaction_mroot)(action_mroot)(
                       block_mroot)(producer)(schedule_version)(new_producers))
};

struct eosio_parameters : eosio::blockchain_parameters {
  uint64_t max_storage_size = 10 * 1024 * 1024;
  uint32_t percent_of_max_inflation_rate = 0;
  uint32_t storage_reserve_ratio = 1000; // ratio * 1000

  // explicit serialization macro is not necessary, used here only to improve
  // compilation time
  EOSLIB_SERIALIZE_DERIVED(
      eosio_parameters, eosio::blockchain_parameters,
      (max_storage_size)(percent_of_max_inflation_rate)(storage_reserve_ratio))
};

struct eosio_global_state : eosio_parameters {
  uint64_t total_storage_bytes_reserved = 0;
  eosio::asset total_storage_stake;
  eosio::asset payment_per_block;
  eosio::asset payment_to_eos_bucket;
  time first_block_time_in_cycle = 0;
  uint32_t blocks_per_cycle = 0;
  time last_bucket_fill_time = 0;
  eosio::asset eos_bucket;

  // explicit serialization macro is not necessary, used here only to improve
  // compilation time
  EOSLIB_SERIALIZE_DERIVED(
      eosio_global_state, eosio_parameters,
      (total_storage_bytes_reserved)(total_storage_stake)(payment_per_block)(
          payment_to_eos_bucket)(first_block_time_in_cycle)(blocks_per_cycle)(
          last_bucket_fill_time)(eos_bucket))
};

struct producer_info {
  account_name owner;
  uint64_t total_votes = 0;
  eosio_parameters prefs;
  eosio::bytes packed_key; /// a packed public key object
  eosio::asset per_block_payments;
  time last_rewards_claim = 0;
  time time_became_active = 0;
  time last_produced_block_time = 0;

  uint64_t primary_key() const { return owner; }
  uint64_t by_votes() const { return total_votes; }
  bool active() const { return 0 < packed_key.size(); }

  // explicit serialization macro is not necessary, used here only to improve
  // compilation time
  EOSLIB_SERIALIZE(
      producer_info,
      (owner)(total_votes)(prefs)(packed_key)(per_block_payments)(
          last_rewards_claim)(time_became_active)(last_produced_block_time))
};

typedef eosio::multi_index<
    N(producerinfo), producer_info,
    indexed_by<N(prototalvote), const_mem_fun<producer_info, uint64_t,
                                              &producer_info::by_votes>>>
    producerst;

// typedef eosio::multi_index<N(producerinfo), producer_info> producerst;

typedef eosio::singleton<N(global), eosio_global_state> global_state_singleton;

static constexpr uint32_t max_inflation_rate = 5; // 5% annual inflation
static constexpr uint32_t seconds_per_day = 24 * 3600;
static constexpr uint64_t system_token_symbol = S(4, EOS);

class system_contract : private eosio::contract {
public:
  using eosio::contract::contract;

  struct proposal_data {
    proposal_data() {}

    std::string code_id;
    std::string abi_id;

    friend bool operator == (const proposal_data &a, const proposal_data &b) {
      return a.code_id == b.code_id && a.abi_id == b.abi_id;
    }

    EOSLIB_SERIALIZE(proposal_data, (code_id)(abi_id))
  };

  // functions defined in voting.cpp
  void regproducer(const account_name producer,
                  const public_key producer_key);

  void unregprod(const account_name producer);

  eosio::asset payment_per_block(uint32_t percent_of_max_inflation_rate);

  void update_elected_producers();

  void voteproducer(const account_name voter,
                    const std::vector<account_name> &producers);

  void nonce(const std::string & /*value*/) {}

  void bond(const account_name acc, const asset token);
  void unbond(const account_name acc, const asset token);

  void refund(const account_name owner);

  // functions defined in producer_pay.cpp

  void onblock(const block_header &header);

  void setprods(eosio::producer_schedule sch);

  void unsetprods(account_name user);

  // @abi action
  void createp(account_name creator, eosio::name proposal, proposal_data data, time expiration);

  // @abi action
  void proproposal(account_name proposer, account_name creator, eosio::name proposal);

  // @abi action
  void conproposal(account_name proposer, account_name creator, eosio::name proposal);

//  void claimrewards(const account_name &owner);

private:
  struct config {
    config() {}
    constexpr static uint64_t key = N(config);
    bool is_manual_setprods = false;
  };

  void store_config(const config &conf) {
    auto it = db_find_i64(_self, _self, N(config), config::key);
    if (it != -1) {
      db_update_i64(it, _self, (const char *)&conf, sizeof(config));
    } else {
      db_store_i64(_self, N(config), _self, config::key, (const char *)&conf, sizeof(config));
    }
  }

  bool get_config(config &conf) {
    auto it = db_find_i64(_self, _self, N(config), config::key);
    if (it != -1) {
      auto size = db_get_i64(it, (char*)&conf, sizeof(config));
      eosio_assert(size == sizeof(config), "Wrong record size");
      return true;
    }
    return false;
  }
  // Implementation details:

  // defined in voting.hpp
  static eosio_global_state get_default_parameters();

  // defined in voting.cpp
  void increase_voting_power(account_name acnt, const eosio::asset &amount);

  void decrease_voting_power(account_name acnt, const eosio::asset &amount);

  // @abi table
  struct precord {
    account_name creator;
    eosio::name proposal;
    proposal_data data;
    std::vector<account_name> pros;
    std::vector<account_name> cons;
    time expiration;

    eosio::name primary_key() const {
      return proposal;
    }
  };

  // @abi table
  struct contractstat {
    account_name contract;
    proposal_data code;
    std::string status;

    account_name primary_key() const {
      return contract;
    }
  };

  typedef eosio::multi_index<N(precord), precord> precords;
  typedef eosio::multi_index<N(contractstat), contractstat> contractstats;

  void check_proposal(eosio::name proposal);
  int intersection_count(const std::vector<account_name>& a1, const std::vector<account_name>& a2);
};

} /// eosiosystem
