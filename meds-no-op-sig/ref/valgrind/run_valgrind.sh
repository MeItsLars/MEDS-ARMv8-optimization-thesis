#!/bin/bash

valgrind --tool=callgrind --dump-instr=yes --simulate-cache=yes --collect-jumps=yes ../build/toy/benchmark