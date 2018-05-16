#!/usr/bin/env python
# -*- coding: utf-8 -*-

import subprocess

def cli(cmd):
    try:
        out = subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    except Exception, e:
        out = str(e.output)
    return out

class TestEos:
    def setup_class(cls):
        print("\nsetup_class class:%s\n" % cls.__name__)
    def teardown_class(cls):
        print("\nteardown_class class:%s\n" % cls.__name__)
    def test_wallet_open(self):
        cmd = ["cleos", "wallet", "open"]
        out = cli(cmd)
        assert 'Opened' in out
    def test_wallet_unlock(self):
        cmd = ["cleos", "wallet", "unlock", "--password=PW5JkqGEwPv66VmArwuZAnfiawbiqVkAAnS8YY4aefgqDDtidAVo1"]
        out = cli(cmd)
        assert 'Unlocked' in out
    def test_wallet_import(self):
        cmd = ["cleos", "wallet", "import", "5Ju6ujv8PHVsxbKy5ew3qfz8NSkjrwatw3M1khTChAJ9ZCYkD5q"]
        out = cli(cmd)
        assert 'imported' in out
