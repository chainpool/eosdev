#include "System.hpp"

#include <eosio.token/eosio.token.hpp>
#include <eosiolib/print.hpp>

namespace eosiosystem {

static const uint32_t num_of_payed_producers = 121;

void system_contract::onblock(const block_header &header) {
  // update parameters if it's a new cycle
//  if (endian_reverse_u32(header.previous._hash[0]) % 21 == 0)
  //  prints("----onblock update elected producers.");
  // update_elected_producers();
 // }
}
} // namespace eosiosystem
