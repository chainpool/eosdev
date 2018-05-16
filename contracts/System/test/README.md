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

## proposal

Creating wallet: default
Save password to use in the future to unlock this wallet.
Without password imported keys will not be retrievable.
"PW5JkqGEwPv66VmArwuZAnfiawbiqVkAAnS8YY4aefgqDDtidAVo1"

❯ cleos create key
Private key: 5Ju6ujv8PHVsxbKy5ew3qfz8NSkjrwatw3M1khTChAJ9ZCYkD5q
Public key: EOS5ucksZfrXAA1d5ErwDVNSryCggp5zn8o5EJPSxEk69CCPbinpm

eosio

EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3

cleos wallet open && cleos wallet unlock --password=PW5JkqGEwPv66VmArwuZAnfiawbiqVkAAnS8YY4aefgqDDtidAVo1

cleos wallet import 5Ju6ujv8PHVsxbKy5ew3qfz8NSkjrwatw3M1khTChAJ9ZCYkD5q

cleos set contract eosio contracts/System

cleos create account eosio user1 EOS5ucksZfrXAA1d5ErwDVNSryCggp5zn8o5EJPSxEk69CCPbinpm EOS5ucksZfrXAA1d5ErwDVNSryCggp5zn8o5EJPSxEk69CCPbinpm

cleos create account eosio user2 EOS5ucksZfrXAA1d5ErwDVNSryCggp5zn8o5EJPSxEk69CCPbinpm EOS5ucksZfrXAA1d5ErwDVNSryCggp5zn8o5EJPSxEk69CCPbinpm

cleos create account eosio user3 EOS5ucksZfrXAA1d5ErwDVNSryCggp5zn8o5EJPSxEk69CCPbinpm EOS5ucksZfrXAA1d5ErwDVNSryCggp5zn8o5EJPSxEk69CCPbinpm

cleos push action eosio createp "$(cat createp.json)" -p user1

cleos push action eosio proproposal '["user1", "user1", "upgradesys"]' -p user1

cleos push action eosio proproposal '["user1", "user1", "upgradesys"]' -p user1 // should error

cleos push action eosio proproposal '["user2", "user1", "upgradesys"]' -p user2

cleos push action eosio conproposal '["user3", "user1", "upgradesys"]' -p user3

cleos push action eosio conproposal '["user2", "user1", "upgradesys"]' -p user2

cleos get table eosio eosio precord
cleos get table eosio eosio contractstat
