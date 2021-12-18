#!/bin/bash

valgrind --leak-check=full -s build/MCTSsingle.out < test_input.txt