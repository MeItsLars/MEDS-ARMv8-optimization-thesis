SHELL := /bin/bash

PARAM = toy

LIBS = -lssl -lcrypto
CC := gcc
AS := as

ifdef DEBUG
CFLAGS := -g -Wall -D$(PARAM) -DDEBUG -Wno-unused-function
OBJDIR := debug/$(PARAM)
else ifdef BENCHMARK
CFLAGS := -O3 -Wall -D$(PARAM) -DBENCHMARK -Wno-unused-function
OBJDIR := build/$(PARAM)
else
#CFLAGS := -O3 -march=native -Wall -D$(PARAM)
CFLAGS := -O3 -Wall -D$(PARAM)
OBJDIR := build/$(PARAM)
endif

ifdef LOG
CFLAGS += -DDEBUG
endif

CFLAGS += -I$(OBJDIR) -INIST

EXES = test bench benchmark KAT_test benchmatmul benchrref

TARGETS := ${EXES:%=$(OBJDIR)/%}

.PHONY: default all clean

default: $(EXES)

ASM_SOURCES := $(wildcard *.s)
ASM_OBJECTS := $(patsubst %.s,%.o,$(ASM_SOURCES))
C_OBJECTS = meds.o util.o seed.o osfreq.o fips202.o matrixmod.o bitstream.o randombytes.o cyclecounter.o
HEADERS = $(wildcard *.h) $(OBJDIR)/params.h $(OBJDIR)/api.h

# Define all object files
BUILDOBJ := ${ASM_OBJECTS:%=$(OBJDIR)/%} ${C_OBJECTS:%=$(OBJDIR)/%}

# Build executable
$(EXES) : % :
	@make $(OBJDIR)/$(@F)

# Generate params.h and api.h
$(OBJDIR)/params.h: params.py $(OBJDIR)
	python3 $< -p $(PARAM) > params.h

$(OBJDIR)/api.h: params.py $(OBJDIR)
	python3 $< -a $(PARAM) > api.h

# Create build directory if it doesn't exist
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Compile ASM files
$(OBJDIR)/%.o: %.s $(HEADERS) $(OBJDIR)
	$(AS) $< -o $@

# Compile C files
$(OBJDIR)/%.o: %.c $(HEADERS) $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link all objects
$(TARGETS) : $(OBJDIR)/%: %.c $(BUILDOBJ)
	$(CC) $(@F).c $(BUILDOBJ) $(CFLAGS) $(LIBS) -o $@

RUN: test
	@echo ""
	@$(OBJDIR)/test

RUN_ALL:
	for p in `./params.py -l`; do \
	echo "\nRunning par set" $$p; \
	make RUN PARAM=$$p; \
	echo ""; \
	done;

.PHONY: debug KAT

KAT: 
	$(MAKE) KAT_test PARAM=$(PARAM)
	$(OBJDIR)/KAT_test

KAT_ALL:
	@for p in `./params.py -l`; do \
	echo "Running par set" $$p; \
	make PARAM=$$p KAT; \
	echo ""; \
	done

BENCH:
	make bench PARAM=$(PARAM)
	$(OBJDIR)/bench | ./proc_bench.py >> bench.txt

BENCH_ALL:
	rm -f bench.txt
	for p in `./params.py -l`; do \
	echo "Running par set" $$p; \
	make PARAM=$$p BENCH; \
	echo ""; \
	cat bench.txt; \
	done

BENCHMARK:
	make benchmark PARAM=$(PARAM) BENCHMARK=true
	$(OBJDIR)/benchmark sign 1

BENCH_MATMUL:
	make benchmatmul PARAM=$(PARAM) BENCHMARK=true
	$(OBJDIR)/benchmatmul

BENCH_RREF:
	make benchrref PARAM=$(PARAM) BENCHMARK=true
	$(OBJDIR)/benchrref

all:
	for p in `./params.py -l`; do \
	make PARAM=$$p; \
	make PARAM=$$p DEBUG=true; \
	done


clean:
	rm -rf build/ debug/ PQCsignKAT_*

