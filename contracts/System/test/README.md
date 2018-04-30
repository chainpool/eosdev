## wallet

```
root@05f33080b74f:~# cleos wallet create
Creating wallet: default
Save password to use in the future to unlock this wallet.
Without password imported keys will not be retrievable.
"PW5KWYFBXQ3tNgPCmTf73fSuk3m1FgUA78EoFARsoEzAjf2c3pbjk"
```

cleos wallet open

cleos wallet unlock --password PW5KWYFBXQ3tNgPCmTf73fSuk3m1FgUA78EoFARsoEzAjf2c3pbjk

## System contract

cleos set contract eosio contracts/System -p eosio

cleos push action eosio regproducer "$(cat regproducer.json)" -p eosio

cleos get table eosio eosio producerinfo

cleos push action eosio unregprod '["eosio"]' -p eosio

root@05f33080b74f:/tmp/build# cleos create key
Private key: 5JroXnZ6wcfQGj1ow34zT7bPNq5TYuQGBFPGxrFshyeQcLFAZQn
Public key: EOS85qZDwLwfKwUQMk1WZvmZJZYHQAH7HBV9sZYnHSWnq2oVBJE85

cleos create account eosio producera EOS85qZDwLwfKwUQMk1WZvmZJZYHQAH7HBV9sZYnHSWnq2oVBJE85 EOS85qZDwLwfKwUQMk1WZvmZJZYHQAH7HBV9sZYnHSWnq2oVBJE85

cleos push action eosio regproducer "$(cat regproducera.json)" -p producera

### bond

cleos push action eosio bond "$(cat bonda.json)" -p usera

cleos push action eosio bond "$(cat bondb.json)" -p userb

cleos get table eosio eosio voters

cleos push action eosio voteproducer "$(cat voteproducera.json)" -p usera

cleos push action eosio voteproducer "$(cat voteproducerb.json)" -p userb

## token.map contract

eosio.token

cleos create account eosio eosio.token EOS85qZDwLwfKwUQMk1WZvmZJZYHQAH7HBV9sZYnHSWnq2oVBJE85 EOS85qZDwLwfKwUQMk1WZvmZJZYHQAH7HBV9sZYnHSWnq2oVBJE85

cleos create account eosio usera EOS85qZDwLwfKwUQMk1WZvmZJZYHQAH7HBV9sZYnHSWnq2oVBJE85 EOS85qZDwLwfKwUQMk1WZvmZJZYHQAH7HBV9sZYnHSWnq2oVBJE85

cleos create account eosio userb EOS85qZDwLwfKwUQMk1WZvmZJZYHQAH7HBV9sZYnHSWnq2oVBJE85 EOS85qZDwLwfKwUQMk1WZvmZJZYHQAH7HBV9sZYnHSWnq2oVBJE85

cleos set contract eosio.token contracts/token.map -p eosio.token

cleos push action eosio.token init '[ "eosio", 0, 0, 0]' -p eosio.token

cleos push action eosio.token maping '["usera", 18668058327]' -p eosio

cleos push action eosio.token maping '["userb", 18668057238]' -p eosio

cleos get table eosio.token usera accountmap

cleos get table eosio.token userb accountmap

cleos get table eosio.token usera accounts

cleos get table eosio.token userb accounts

cleos get table eosio.token EOS stat
