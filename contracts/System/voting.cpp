/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#include "System.hpp"

#include <eosio.token/eosio.token.hpp>
#include <eosiolib/datastream.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/multi_index.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/privileged.hpp>
#include <eosiolib/serialize.hpp>
#include <eosiolib/singleton.hpp>
#include <eosiolib/transaction.hpp>

#include <algorithm>
#include <array>
#include <cmath>

namespace eosiosystem {
using eosio::indexed_by;
using eosio::const_mem_fun;
using eosio::bytes;
using eosio::print;
using eosio::singleton;
using eosio::transaction;

static constexpr uint32_t blocks_per_year =
    52 * 7 * 24 * 2 * 3600; // half seconds per year
static constexpr uint32_t blocks_per_producer = 12;

struct voter_info {
  account_name owner = 0;
  time last_update = 0;
  eosio::asset staked;
  eosio::asset unstaking;
  eosio::asset unstake_per_week;
  std::vector<account_name> producers;
  uint32_t deferred_trx_id = 0;
  time last_unstake_time = 0; // uint32

  uint64_t primary_key() const { return owner; }

  // explicit serialization macro is not necessary, used here only to improve
  // compilation time
  EOSLIB_SERIALIZE(voter_info,
                   (owner)(last_update)(staked)(unstaking)(unstake_per_week)(
                       producers)(deferred_trx_id)(last_unstake_time))
};

typedef eosio::multi_index<N(voters), voter_info> voterstable;

/**
 *  This method will create a producer_config and producer_info object for
 * 'producer'
 *
 *  @pre producer is not already registered
 *  @pre producer to register is an account
 *  @pre authority of producer to register
 *
 */
void system_contract::regproducer(const account_name producer,
                                  const public_key producer_key) {
  // eosio::print("produce_key: ", producer_key.size(), ", sizeof(public_key):
  // ", sizeof(public_key), "\n");
  require_auth(producer);

  producerst producers_tbl(_self, _self);
  auto prod = producers_tbl.find(producer);

  if (prod != producers_tbl.end()) {
    producers_tbl.modify(prod, producer, [&](producer_info &info) {
      info.packed_key = eosio::pack<public_key>(producer_key);
    });
  } else {
    producers_tbl.emplace(producer, [&](producer_info &info) {
      info.owner = producer;
      info.total_votes = 0;
      info.packed_key = eosio::pack<public_key>(producer_key);
    });
  }
}

void system_contract::setprods(eosio::producer_schedule sch) {
  require_auth(_self);
  char buffer[action_data_size()];
  read_action_data( buffer, sizeof(buffer) );
  set_active_producers(buffer, sizeof(buffer));

  config conf;
  get_config(conf);
  conf.is_manual_setprods = true;
  store_config(conf);
}

void system_contract::unsetprods(account_name user) {
  require_auth(_self);
  print("set is_manual_setprods to false, by: ", eosio::name{user});

  config conf;
  get_config(conf);
  conf.is_manual_setprods = false;
  store_config(conf);
}

void system_contract::unregprod(const account_name producer) {
  require_auth(producer);

  producerst producers_tbl(_self, _self);
  auto prod = producers_tbl.find(producer);
  eosio_assert(prod != producers_tbl.end(), "producer not found");

  producers_tbl.modify(prod, 0,
                       [&](producer_info &info) { info.packed_key.clear(); });
}

void system_contract::increase_voting_power(account_name acnt,
                                            const eosio::asset &amount) {
  print("-------increase_voting_power\n");
  print(_self);
  print("---");
  print(N(eosio));
  voterstable voters_tbl(_self, _self);
  auto voter = voters_tbl.find(acnt);

  eosio_assert(0 <= amount.amount, "negative asset");

  if (voter == voters_tbl.end()) {
    print("------not find acnt");
    voter = voters_tbl.emplace(acnt, [&](voter_info &a) {
      a.owner = acnt;
      a.last_update = now();
      a.staked = amount;
    });
  } else {
    print("------find acnt");
    voters_tbl.modify(voter, 0, [&](auto &av) {
      av.last_update = now();
      av.staked += amount;
    });
  }

  const std::vector<account_name> *producers = nullptr;
  producers = &voter->producers;

  if (producers) {
    producerst producers_tbl(_self, _self);
    for (auto p : *producers) {
      auto prod = producers_tbl.find(p);
      eosio_assert(prod != producers_tbl.end(),
                   "never existed producer"); // data corruption
      producers_tbl.modify(
          prod, 0, [&](auto &v) { v.total_votes += uint64_t(amount.amount) / 10000; });
    }
  }
}

void system_contract::decrease_voting_power(account_name acnt,
                                            const eosio::asset &amount) {
  require_auth(acnt);
  voterstable voters_tbl(_self, _self);
  auto voter = voters_tbl.find(acnt);
  eosio_assert(voter != voters_tbl.end(), "stake not found");

  if (0 < amount.amount) {
    eosio_assert(amount <= voter->staked,
                 "cannot unstake more than total stake amount");
    voters_tbl.modify(voter, 0, [&](voter_info &a) {
      a.staked -= amount;
      a.last_update = now();
    });

    const std::vector<account_name> *producers = nullptr;
    producers = &voter->producers;

    if (producers) {
      producerst producers_tbl(_self, _self);
      for (auto p : *producers) {
        auto prod = producers_tbl.find(p);
        eosio_assert(prod != producers_tbl.end(),
                     "never existed producer"); // data corruption
        producers_tbl.modify(prod, 0, [&](auto &v) {
          v.total_votes -= uint64_t(amount.amount) /10000;
        });
      }
    }
    INLINE_ACTION_SENDER(eosio::token, transfer)
    (N(eosio.token), {N(eosio), N(active)},
     {N(eosio), acnt, amount, std::string("unstake")});
  }
}

eosio_global_state system_contract::get_default_parameters() {
  eosio_global_state dp;
  get_blockchain_parameters(dp);
  return dp;
}
  
void system_contract::update_elected_producers() {
  producerst producers_tbl(_self, _self);
  auto idx = producers_tbl.template get_index<N(prototalvote)>();
  eosio::producer_schedule schedule;
  schedule.producers.reserve(21);
  std::vector<uint64_t> count(21, 0);
  size_t n = 0;
  for (auto it = idx.crbegin();
       it != idx.crend() && 0 < it->total_votes; ++it) {
    if (it->active()) {
      for (auto i = 0; i < 21; ++i) {
          if (count[i] < it->total_votes) {
              eosio::producer_key key;
              key.producer_name = it->owner;
              key.block_signing_key = eosio::unpack<public_key>(it->packed_key);
              auto it2 = schedule.producers.begin();
              schedule.producers.insert( it2 + i, key);
              count.insert(count.begin() + i, it->total_votes);
              print(it->total_votes);
              n++;
              break;
          }
      }
    }
  }
  if (n == 0) { // no active producers with votes > 0
    return;
  }
  if (schedule.producers.size() > 21) {
    schedule.producers.resize(21);
  }
  print("---update schedule"); 
  // should use producer_schedule_type from
  // libraries/chain/include/eosio/chain/producer_schedule.hpp
  bytes packed_schedule = pack(schedule);
  set_active_producers(packed_schedule.data(), packed_schedule.size());
}

/**
 *  @pre producers must be sorted from lowest to highest
 *  @pre voter must authorize this action
 *  @pre voter must have previously staked some EOS for voting
 */
void system_contract::voteproducer(const account_name voter,
                                   const std::vector<account_name> &producers) {
  require_auth(voter);

  eosio_assert(producers.size() <= 30,
               "attempt to vote for too many producers");
  for (size_t i = 1; i < producers.size(); ++i) {
    eosio_assert(producers[i - 1] < producers[i],
                 "producer votes must be unique and sorted");
  }

  voterstable voters_tbl(_self, _self);
  auto voter_it = voters_tbl.find(voter);

  eosio_assert(0 <= voter_it->staked.amount, "negative stake");
  eosio_assert(voter_it != voters_tbl.end() && (0 < voter_it->staked.amount),
               "no stake to vote");

  const std::vector<account_name> *old_producers = nullptr;
  old_producers = &voter_it->producers;

  const std::vector<account_name> *new_producers = nullptr;
  new_producers = &producers;

  producerst producers_tbl(_self, _self);
  uint64_t votes = uint64_t(voter_it->staked.amount) / 10000;
  if (old_producers) {
    // revoke votes only from no longer elected
    std::vector<account_name> revoked(old_producers->size());
    auto end_it = std::set_difference(
        old_producers->begin(), old_producers->end(), new_producers->begin(),
        new_producers->end(), revoked.begin());
    for (auto it = revoked.begin(); it != end_it; ++it) {
      auto prod = producers_tbl.find(*it);
      eosio_assert(prod != producers_tbl.end(),
                   "never existed producer"); // data corruption
      producers_tbl.modify(prod, 0, [&](auto &pi) { pi.total_votes -= votes; });
    }
  }

  // update newly elected
  std::vector<account_name> elected(new_producers->size());
  auto end_it = elected.begin();
  if (old_producers) {
    end_it = std::set_difference(new_producers->begin(), new_producers->end(),
                                 old_producers->begin(), old_producers->end(),
                                 elected.begin());
  } else {
    end_it = std::copy(new_producers->begin(), new_producers->end(),
                       elected.begin());
  }
  for (auto it = elected.begin(); it != end_it; ++it) {
    auto prod = producers_tbl.find(*it);
    eosio_assert(prod != producers_tbl.end(), "producer is not registered");
    producers_tbl.modify(prod, 0, [&](auto &pi) { pi.total_votes += votes; });
  }

  // save new values to the account itself
  voters_tbl.modify(voter_it, 0, [&](voter_info &a) {
    a.last_update = now();
    a.producers = producers;
  });

  config conf;
  get_config(conf);
  if (!conf.is_manual_setprods) {
    update_elected_producers();
  }
}
void system_contract::bond(const account_name acc, const asset token) {
  if (asset(0) < token) {
    INLINE_ACTION_SENDER(eosio::token, transfer)
    (N(eosio.token), {acc, N(active)},
     {acc, N(eosio), token, std::string("stake token")});
    increase_voting_power(acc, token);
  }
}
static constexpr time refund_delay = 3600;
static constexpr time refund_expiration_time = 3600;

struct refund_request {
  account_name owner;
  time request_time;
  eosio::asset amount;

  uint64_t primary_key() const { return owner; }

  // explicit serialization macro is not necessary, used here only to improve
  // compilation time
  EOSLIB_SERIALIZE(refund_request, (owner)(request_time)(amount))
};
typedef eosio::multi_index<N(refunds), refund_request> refunds_table;

void system_contract::unbond(const account_name acc, const asset token) {
  if (asset(0) < token) {

    decrease_voting_power(acc, token);
  }
}
}
