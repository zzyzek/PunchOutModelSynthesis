# To the extent possible under law, the person who associated CC0 with
# this file has waived all copyright and related or neighboring rights
# to this file.
#    
# You should have received a copy of the CC0 legalcode along with this
# work.  If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
#

CPP := g++
CFLAGS := -lm -lyajl # -Wall
OPT := -O3 -I.
DEBUGOPT := -g

LIBDIR := .lib
DEBUGLIBDIR := .debuglib
BINDIR := ../bin
SRC_DIR := "./"

BIN_CPP := 'test_poms|main_poms|le_poms|mcmc_poms|lace_poms|ir_poms|investigate'

#SRCFILES := $(shell find $(SRC_DIR) -maxdepth 1 -name '*.cpp' -or -name '*.c' | grep -P -v -i 'test_poms|main_poms|quilt' )
#SRCFILES := $(shell find $(SRC_DIR) -maxdepth 1 -name '*.cpp' -or -name '*.c' | grep -P -v -i 'test_poms|main_poms|le_poms|mcmc_poms|lace_poms|ir_poms|bms_poms' )
SRCFILES := $(shell find $(SRC_DIR) -maxdepth 1 -name '*.cpp' -or -name '*.c' | grep -P -v -i $(BIN_CPP) )

CPPFILES := $(wildcard *.cpp)
CFILES := $(wildcard *.c)
HPPFILES := $(wildcard *.hpp)
HFILES := $(wildcard *.h)

OBJS := $(SRCFILES:%=$(LIBDIR)/%.o)
DEBUGOBJS := $(SRCFILES:%=$(DEBUGLIBDIR)/%.o)

### prod

all: poms

poms: $(BINDIR)/poms

$(BINDIR)/poms: $(OBJS) $(LIBDIR)/main_poms.cpp.o main_poms.cpp poms.hpp ac4_tier.hpp
	@mkdir -p $(BINDIR)
	@$(CPP) $(OBJS) $(LIBDIR)/main_poms.cpp.o -o $@ $(CFLAGS) $(OPT)

$(LIBDIR)/main_poms.cpp.o: main_poms.cpp poms.hpp
	@$(CPP) -c $< -o $@ $(CFLAGS) $(OPT)

$(LIBDIR)/test_poms.cpp.o: test_poms.cpp poms.hpp
	@$(CPP) -c $< -o $@ $(CFLAGS) $(OPT)

$(LIBDIR)/%.cpp.o: %.cpp poms.hpp
	@mkdir -p $(LIBDIR)
	@$(CPP) -c $< -o $@ $(CFLAGS) $(OPT)

$(LIBDIR)/%.c.o: %.c %.h poms.hpp
	@mkdir -p $(LIBDIR)
	@$(CPP) -c $< -o $@ $(CFLAGS) $(OPT)

### debug

$(DEBUGLIBDIR)/main_poms.cpp.o: main_poms.cpp
	@mkdir -p $(DEBUGLIBDIR)
	@$(CPP) -c $< -o $@ $(CFLAGS) $(DEBUGOPT)

$(DEBUGLIBDIR)/test_poms.cpp.o: test_poms.cpp
	@mkdir -p $(DEBUGLIBDIR)
	@$(CPP) -c $< -o $@ $(CFLAGS) $(DEBUGOPT)

$(DEBUGLIBDIR)/%.cpp.o: %.cpp poms.hpp
	@mkdir -p $(DEBUGLIBDIR)
	@$(CPP) -c $< -o $@ $(CFLAGS) $(DEBUGOPT)

$(DEBUGLIBDIR)/%.c.o: %.c %.h
	@mkdir -p $(DEBUGLIBDIR)
	@$(CPP) -c $< -o $@ $(CFLAGS) $(DEBUGOPT)

.PHONY: clean poms.debug poms.test poms.le debug test le mcmc poms.mcmc bms poms.bms

clean:
	@rm -f $(OBJS) $(DEBUGOBJS) ../bin/poms ../bin/poms.debug $(DEBUGLIBDIR)/*.cpp.o $(DEBUGLIBDIR)/*.c.o

debug: poms.debug
poms.debug: $(DEBUGLIBDIR)/main_poms.cpp.o $(DEBUGOBJS)
	@$(CPP) $(DEBUGOBJS) $(DEBUGLIBDIR)/main_poms.cpp.o -o $(BINDIR)/poms.debug $(CFLAGS) $(DEBUGOPT)

test: poms.test
poms.test: $(DEBUGLIBDIR)/test_poms.cpp.o $(DEBUGOBJS) tests/road2d_json.h
	@$(CPP) $(DEBUGOBJS) $(DEBUGLIBDIR)/test_poms.cpp.o -o $(BINDIR)/poms.test $(CFLAGS) $(DEBUGOPT)

# local entropy
le: poms.le
poms.le: $(LIBDIR)/le_poms.cpp.o $(OBJS)
	@$(CPP) $(OBJS) $(LIBDIR)/le_poms.cpp.o -o $(BINDIR)/poms.le $(CFLAGS) $(OPT)

mcmc: poms.mcmc
poms.mcmc: $(LIBDIR)/mcmc_poms.cpp.o $(OBJS)
	@$(CPP) $(OBJS) $(LIBDIR)/mcmc_poms.cpp.o -o $(BINDIR)/poms.mcmc  $(CFLAGS) $(OPT)

# local arc consistent entropy
lace: poms.lace
poms.lace: $(LIBDIR)/lace_poms.cpp.o $(OBJS)
	@$(CPP) $(OBJS) $(LIBDIR)/lace_poms.cpp.o -o $(BINDIR)/poms.lace $(CFLAGS) $(OPT)

ir: poms.ir
poms.ir: $(LIBDIR)/ir_poms.cpp.o $(OBJS)
	@$(CPP) $(OBJS) $(LIBDIR)/ir_poms.cpp.o -o $(BINDIR)/poms.ir $(CFLAGS) $(OPT)
 
# breakout model synthesis
bms: poms.bms
poms.bms: $(LIBDIR)/bms_poms.cpp.o $(OBJS)
	@$(CPP) $(OBJS) $(LIBDIR)/bms_poms.cpp.o -o $(BINDIR)/poms.bms $(CFLAGS) $(OPT)

bms.debug: poms.bms.debug
poms.bms.debug: $(DEBUGLIBDIR)/bms_poms.cpp.o $(DEBUGOBJS)
	@$(CPP) $(DEBUGOBJS) $(DEBUGLIBDIR)/bms_poms.cpp.o -o $(BINDIR)/poms.bms.debug $(CFLAGS) $(DEBUGOPT)

# block solver
block: poms.block
poms.block: $(LIBDIR)/block_poms.cpp.o $(OBJS)
	@$(CPP) $(OBJS) $(LIBDIR)/block_poms.cpp.o -o $(BINDIR)/poms.block $(CFLAGS) $(OPT)

block.debug: poms.block.debug
poms.block.debug: $(DEBUGLIBDIR)/block_poms.cpp.o $(DEBUGOBJS)
	@$(CPP) $(DEBUGOBJS) $(DEBUGLIBDIR)/block_poms.cpp.o -o $(BINDIR)/poms.block.debug $(CFLAGS) $(DEBUGOPT)

# blocking
investigate: investigate.block
investigate.block: $(LIBDIR)/block_investigate.cpp.o $(OBJS)
	@$(CPP) $(OBJS) $(LIBDIR)/block_investigate.cpp.o -o $(BINDIR)/investigate.block $(CFLAGS) $(OPT)


#lace: poms.lace
#poms.lace: $(DEBUGLIBDIR)/lace_poms.cpp.o $(DEBUGOBJS)
#	@$(CPP) $(DEBUGOBJS) $(DEBUGLIBDIR)/lace_poms.cpp.o -o $(BINDIR)/poms.lace $(CFLAGS) $(DEBUGOPT)


