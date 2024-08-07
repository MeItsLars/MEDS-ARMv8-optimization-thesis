#!/bin/bash

for p in `./params.py -l`; do
  v=Reference_Implementation

  mkdir -p MEDS/$v/$p

  cp *.c MEDS/$v/$p/
  cp *.h MEDS/$v/$p/

  rm MEDS/$v/$p/randombytes.*
  rm MEDS/$v/$p/KAT_test.c

  cp NIST/PQCgenKAT_sign.c MEDS/$v/$p/PQCgenKAT_sign.c
  cp NIST/rng.c MEDS/$v/$p/randombytes.c
  cp NIST/rng.h MEDS/$v/$p/randombytes.h
  cp NIST/rng.h MEDS/$v/$p/rng.h

  ./params.py -p $p > MEDS/$v/$p/params.h

  ./params.py -a $p > MEDS/$v/$p/api.h

  cp NIST.mk MEDS/$v/$p/Makefile
done

ln -s $v MEDS/Optimized_Implementation

