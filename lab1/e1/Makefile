CC ?= gcc
CFLAGS ?= -O2 -std=gnu11 -Wall -Wextra

.PHONY: all run table plots clean

all: cache_profile

cache_profile: cache_profile.c
	$(CC) $(CFLAGS) $< -o $@

run: cache_profile
	./cache_profile > results.csv

table: results.csv
	python3 plot_results.py results.csv

plots: results.csv
	python3 plot_results.py results.csv --plot

clean:
	rm -f cache_profile results.csv cache_table.csv cache_table.md cache_time.png
