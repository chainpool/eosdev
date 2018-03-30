#include "chainex_state.hpp"

namespace eosio {
extended_asset chainex_state::convert_to_exchange(connector &c,
                                                  extended_asset in) {
  real_type R(supply.amount);
  real_type C(c.balance.amount + in.amount);
  real_type F(c.weight / 1000.0);
  real_type T(in.amount);
  real_type ONE(1.0);

  real_type E = -R * (ONE - std::pow(ONE + T / C, F));
  int64_t issued = int64_t(E);

  supply.amount += issued;
  c.balance.amount += in.amount;

  return extended_asset(issued, supply.get_extended_symbol());
}

extended_asset chainex_state::convert_from_exchange(connector &c,
                                                    extended_asset in) {
  eosio_assert(in.contract == supply.contract,
               "unexpected asset contract input");
  eosio_assert(in.symbol == supply.symbol, "unexpected asset symbol input");

  real_type R(supply.amount - in.amount);
  real_type C(c.balance.amount);
  real_type F(1000.0 / c.weight);
  real_type E(in.amount);
  real_type ONE(1.0);

  real_type T = C * (std::pow(ONE + E / R, F) - ONE);
  int64_t out = int64_t(T);

  supply.amount -= in.amount;
  c.balance.amount -= out;

  return extended_asset(out, c.balance.get_extended_symbol());
}

extended_asset chainex_state::convert(extended_asset from, extended_symbol to) {
  auto sell_symbol = from.get_extended_symbol();
  auto ex_symbol = supply.get_extended_symbol();
  auto base_symbol = base.balance.get_extended_symbol();
  auto quote_symbol = quote.balance.get_extended_symbol();

  if (sell_symbol != ex_symbol) {
    if (sell_symbol == base_symbol) {
      from = convert_to_exchange(base, from);
    } else if (sell_symbol == quote_symbol) {
      from = convert_to_exchange(quote, from);
    } else {
      eosio_assert(false, "invalid sell");
    }
  } else {
    if (to == base_symbol) {
      from = convert_from_exchange(base, from);
    } else if (to == quote_symbol) {
      from = convert_from_exchange(quote, from);
    } else {
      eosio_assert(false, "invalid conversion");
    }
  }

  if (to != from.get_extended_symbol()) {
    return convert(from, to);
  }

  return from;
}
} // namespace eosio