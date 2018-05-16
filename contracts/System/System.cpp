#include "System.hpp"
#include <eosiolib/dispatcher.hpp>

#include "producer_pay.cpp"
#include "voting.cpp"
#include "proposal.cpp"

EOSIO_ABI(eosiosystem::system_contract,
          // voting.cpp
          (bond)(unbond)(regproducer)(unregprod)(voteproducer)(onblock)(setprods)(unsetprods)
          // defined in eosio.system.hpp
          (nonce)
          (createp)(proproposal)(conproposal))
