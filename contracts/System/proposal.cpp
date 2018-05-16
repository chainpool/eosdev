#include "System.hpp"

#include <eosiolib/action.h>
#include <eosiolib/chain.h>
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
#include <unordered_set>
#include <vector>

namespace eosiosystem {
void system_contract::createp(account_name creator, eosio::name proposal, proposal_data data, time expiration) {
  require_auth(creator);

  account_name producers[23] = {};
  get_active_producers(producers, sizeof(account_name)*23);

  eosio_assert(std::find(std::begin(producers), std::end(producers), creator) != std::end(producers), "the creator should be in the active producers");

  eosio_assert(expiration > now(), "expiration cannot be earlier than current");

  const uint32_t SEVEN_DAYS = 60*60*24*7;
  eosio_assert(expiration < now() + SEVEN_DAYS, "expiration cannot be longer than 7 days");

  precords record_table(_self, _self);

  eosio_assert(record_table.find(proposal) == record_table.end(), "proposal with the same name already exists" );

  record_table.emplace(_self, [&](auto& row) {
    row.creator = creator;
    row.proposal = proposal;
    row.expiration = expiration;
    row.data = data;
  });
}

void system_contract::proproposal(account_name proposer, account_name creator, eosio::name proposal) {
  require_auth(proposer);

  account_name producers[23] = {};
  get_active_producers(producers, sizeof(account_name)*23);

  eosio_assert(std::find(std::begin(producers), std::end(producers), proposer) != std::end(producers), "the proposer should be in the active producers");

  precords record_table(_self, _self);
  auto record_itr = record_table.find(proposal);
  eosio_assert(record_itr != record_table.end(), "proposal with the name not found");
  eosio_assert(record_itr->creator == creator, "proposal creator not match");

  eosio_assert(record_itr->expiration > now(), "the proposal has expired");
  eosio_assert(std::find(record_itr->pros.begin(), record_itr->pros.end(), proposer) == record_itr->pros.end(), "the proposer already in the pros");

  record_table.modify(record_itr, proposer, [&]( auto& row ) {
    row.pros.push_back(proposer);
    // remove if exits
    row.cons.erase(remove(row.cons.begin(), row.cons.end(), proposer), row.cons.end());
  });

  check_proposal(proposal);
}

void system_contract::conproposal(account_name proposer, account_name creator, eosio::name proposal) {
  require_auth(proposer);

  account_name producers[23] = {};
  get_active_producers(producers, sizeof(account_name)*23);

  eosio_assert(std::find(std::begin(producers), std::end(producers), proposer) != std::end(producers), "the proposer should be in the active producers");

  precords record_table(_self, _self);
  auto record_itr = record_table.find(proposal);
  eosio_assert(record_itr != record_table.end(), "proposal with the name not found");
  eosio_assert(record_itr->creator == creator, "proposal creator not match");

  eosio_assert(record_itr->expiration > now(), "the proposal has expired");
  eosio_assert(std::find(record_itr->cons.begin(), record_itr->cons.end(), proposer) == record_itr->cons.end(), "the proposer already in the cons");

  record_table.modify(record_itr, proposer, [&]( auto& row ) {
    row.cons.push_back(proposer);
    // remove if exits
    row.pros.erase(remove(row.pros.begin(), row.pros.end(), proposer), row.pros.end());
  });

  check_proposal(proposal);
}

int system_contract::intersection_count(const std::vector<account_name>& a1, const std::vector<account_name>& a2) {
  int count = 0;
  for (auto i:a1) {
    if (std::find(std::begin(a2), std::end(a2), i) != std::end(a2)) {
      count++;
    }
  }

  return count;
}

// check pros and cons in proposal with active producers
void system_contract::check_proposal(eosio::name proposal) {
  precords record_table(_self, _self);
  auto record_itr = record_table.find(proposal);

  if (record_itr == record_table.end()) {
    // proposal with the name not found
    return;
  }

  if (record_itr->expiration < now()) {
    // remove expired proposal
    record_table.erase(record_itr);
    return;
  }

  account_name producers[23] = {};
  get_active_producers(producers, sizeof(account_name)*23);

  std::vector<account_name> ac_producers(std::begin(producers), std::end(producers));

  int pro_count = intersection_count(record_itr->pros, ac_producers);
  int con_count = intersection_count(record_itr->cons, ac_producers);

  if (pro_count >= 16) {
    // pro passed
    if (proposal == eosio::name{N(upgradesys)}) {
      contractstats stats_table(_self, _self);
      auto stats_itr = stats_table.find(eosio::name{N(eosio)});
      if (stats_itr == stats_table.end()) {
        // insert
        stats_table.emplace(_self, [&](auto& row){
          row.contract = eosio::name{N(eosio)};
          row.code = std::move(record_itr->data);
        });
      } else {
        // modify
        stats_table.modify(stats_itr, _self, [&](auto& row){
          row.code = std::move(record_itr->data);
        });
      }
    }
    // remove proposal
    record_table.erase(record_itr);
    return;
  }

  if (con_count >= 8) {
    // con passed
    // remove proposal
    record_table.erase(record_itr);
    return;
  }
}
}
