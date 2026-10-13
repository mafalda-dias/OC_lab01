# %%%% This part was kept as it was before %%%%
TARGETS = cm1
PAPI_DIR ?= ../../papi-7.2.0/src
PAPILIB ?= $(PAPI_DIR)/libpapi.a

# %%%% This part was added: explicit default compilation options %%%%
CFLAGS ?= -O2 -std=c11 -Wall -Wextra

# %%%% This part was kept as it was before %%%%
.PHONY: all clean
all: $(TARGETS)

cm1: cm1.c Makefile $(PAPILIB)
	$(CC) $(CPPFLAGS) -I$(PAPI_DIR) $(CFLAGS) $(LDFLAGS) cm1.c $(PAPILIB) -ldl -lpthread $(LDLIBS) -o $@

$(PAPILIB):
	@echo "Build PAPI first: cd $(PAPI_DIR) && ./configure && make" >&2
	@exit 1

# %%%% This part was updated with a build-only clean target %%%%
clean:
	rm -f $(TARGETS) *.o

# %%%% This part was removed hence the commented bits %%%%
# Former clean commands could delete experimental evidence:
# rm -f $(TARGETS) *.o *.stderr *.stdout core *~
# rm -f cm1.out cm1_plot.dat cm1_plot.png
