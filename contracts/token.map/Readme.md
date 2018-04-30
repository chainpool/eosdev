### token.map 合约的name 为eosio.token
- 0, create eosio.token account
```
./cleos create account eosio eosio.token 
./cleos set contract eosio.token ../../contracts/token.map
```

- 1, init 10亿EOS给eosio
```
./cleos push action eosio.token init '[ "eosio", 0, 0, 0]' -p eosio.token
```

- 2, map 
```
./cleos push action eosio.token maping '["force", 15168355842]' -p eosio
```
- 3, 查询
```
./cleos get table eosio.token force accountmap
```
