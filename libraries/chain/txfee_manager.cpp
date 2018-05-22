/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

#include <eosio/chain/exceptions.hpp>
#include <eosio/chain/txfee_manager.hpp>

namespace eosio { namespace chain {

   txfee_manager::txfee_manager(){
        fee_map[N(newaccount)] = asset(1000);
        fee_map[N(setcode)] = asset(1000);
        fee_map[N(setabi)] = asset(1000);
        fee_map[N(transfer)] = asset(1000);
        fee_map[N(updateprod)] = asset(1000);
        fee_map[N(addvote)] = asset(1000);
        fee_map[N(minusvote)] = asset(1000);
        fee_map[N(unfreeze)] = asset(1000);
        fee_map[N(claim)] = asset(1000);
        fee_map[N(createp)] = asset(1000);
        fee_map[N(proproposal)] = asset(1000);
        fee_map[N(conproposal)] = asset(1000);
   }

   bool txfee_manager::check_transaction( const transaction& trx)const
   {
      for( const auto& act : trx.actions ) {
          for (const auto & perm : act.authorization) {
            if (perm.actor != trx.actions[0].authorization[0].actor) {
                return false;
            }
          }
      }
      return true;
   }

   asset txfee_manager::get_required_fee( const transaction& trx)const
   {
      auto fee = asset(0);
      for (const auto& act : trx.actions ) {
        auto it = fee_map.find(act.name);
        EOS_ASSERT(it != fee_map.end(), action_validate_exception, "action name not include in feemap");
        fee += it->second;
      }

      return fee;
   }

} } /// namespace eosio::chain
