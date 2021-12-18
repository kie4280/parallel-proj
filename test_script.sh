#!/bin/bash

# valgrind --leak-check=full -s 
echo "[test single]"
build/MCTSsingle.out < test_input.txt
echo "[test root]"
build/MCTSroot.out < test_input.txt