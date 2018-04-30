/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

#include <token.map/token.map.hpp>

namespace eosio {

void token::init( account_name issuer,
                    uint8_t      issuer_can_freeze,
                    uint8_t      issuer_can_recall,
                    uint8_t      issuer_can_whitelist )
{
    require_auth( _self );

    asset sym(10000000000000);
    eosio_assert( sym.symbol.is_valid(), "invalid symbol name" );
    eosio_assert( sym.is_valid(), "invalid supply");
    eosio_assert( sym.amount > 0, "max-supply must be positive");

    stats statstable( _self, sym.symbol.name() );
    auto existing = statstable.find( sym.symbol.name() );
    eosio_assert( existing == statstable.end(), "token with symbol already exists" );

    statstable.emplace( _self, [&]( auto& s ) {
       s.supply.symbol = sym.symbol;
       s.max_supply    = sym;
       s.issuer        = issuer;
       s.can_freeze    = issuer_can_freeze;
       s.can_recall    = issuer_can_recall;
       s.can_whitelist = issuer_can_whitelist;
    });
}

void token::maping( account_name to, uint64_t id )
{
    accountmap accmap(_self, id);
    auto existing = accmap.find(id);
    eosio_assert(existing == accmap.end(), "this id already exists");

    print( "issue" );
    asset quantity(100000000);
    auto sym = quantity.symbol.name();
    stats statstable( _self, sym );
    const auto& st = statstable.get( sym );

    require_auth( st.issuer );
    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must issue positive quantity" );
    eosio_assert( quantity <= st.max_supply - st.supply, "quantity exceeds available supply");

    statstable.modify( st, 0, [&]( auto& s ) {
       s.supply += quantity;
    });

    accmap.emplace(_self, [&](auto& s){s.name = to; s.id = id;});
    add_balance( st.issuer, quantity, st, st.issuer );

    if( to != st.issuer )
    {
       SEND_INLINE_ACTION( *this, transfer, {st.issuer,N(active)}, {st.issuer, to, quantity, "EOS"} );
    }
}

void token::transfer( account_name from,
                      account_name to,
                      asset        quantity,
                      string       /*memo*/ )
{
    print( "transfer" );
    require_auth( from );
    auto sym = quantity.symbol.name();
    stats statstable( _self, sym );
    const auto& st = statstable.get( sym );

    require_recipient( from );
    require_recipient( to );

    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must transfer positive quantity" );

    sub_balance( from, quantity, st );
    add_balance( to, quantity, st, from );
}

void token::sub_balance( account_name owner, asset value, const currency_stats& st ) {
   accounts from_acnts( _self, owner );

   const auto& from = from_acnts.get( value.symbol.name() );
   eosio_assert( from.balance.amount >= value.amount, "overdrawn balance" );

   if( has_auth( owner ) ) {
      eosio_assert( !st.can_freeze || !from.frozen, "account is frozen by issuer" );
      eosio_assert( !st.can_freeze || !st.is_frozen, "all transfers are frozen by issuer" );
      eosio_assert( !st.enforce_whitelist || from.whitelist, "account is not white listed" );
   } else if( has_auth( st.issuer ) ) {
      eosio_assert( st.can_recall, "issuer may not recall token" );
   } else {
      eosio_assert( false, "insufficient authority" );
   }

   from_acnts.modify( from, owner, [&]( auto& a ) {
       a.balance -= value;
   });
}

void token::add_balance( account_name owner, asset value, const currency_stats& st, account_name ram_payer )
{
   accounts to_acnts( _self, owner );
   auto to = to_acnts.find( value.symbol.name() );
   if( to == to_acnts.end() ) {
      eosio_assert( !st.enforce_whitelist, "can only transfer to white listed accounts" );
      to_acnts.emplace( ram_payer, [&]( auto& a ){
        a.balance = value;
      });
   } else {
      eosio_assert( !st.enforce_whitelist || to->whitelist, "receiver requires whitelist by issuer" );
      to_acnts.modify( to, 0, [&]( auto& a ) {
        a.balance += value;
      });
   }
}

} /// namespace eosio

EOSIO_ABI( eosio::token, (init)(maping)(transfer) )
