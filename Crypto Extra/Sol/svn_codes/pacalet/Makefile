ROOTDIR		= $(HOME)/Projets/dpacontest/code/pacalet

# Do not change anything after this line unless you know what you're doing
CC		= gcc
CFLAGS		= -Wall -ansi -Wno-long-long -c -O3
# CFLAGS		= -g -Wall -ansi -Wno-long-long -O0 -c
INCLUDES	= -I.
LD		= gcc
LDFLAGS		=
LIBS		=
AR		= ar
ARFLAGS		= -r
RANLIB		= ranlib
RANLIBFLAGS	=
DOXYGEN		= doxygen
DOXYGENFLAGS	=
DOCDIR		= docs

SRCS	= $(wildcard *.c)
HEADERS	= $(wildcard *.h)
OBJS	= $(patsubst %.c,%.o,$(SRCS))
EXECS	= fetch cpa_cop

.PHONY: help all doc clean ultraclean

help:
	@echo "Type:"
	@echo "<make> or <make help> to get this help message"
	@echo "<make all> to compile everything"
	@echo "<make doc> to build the documentation"
	@echo "<make clean> to remove object files"
	@echo "<make ultraclean> to remove object files and executables"

all: $(EXECS)

doc: .doc

libhwsec.a: utils.o des.o km.o traces.o tr_pcc.o cop.o
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $(RANLIBFLAGS) $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@
des.o: utils.h
km.o: utils.h des.h km.h
traces.o: utils.h traces.h
tr_pcc.o: utils.h tr_pcc.h
cop.o: utils.h cop.h

fetch.o: utils.h traces.h
cpa_cop.o: utils.h traces.h des.h tr_pcc.h km.h cop.h

.doc: $(HEADERS) doxygen.cfg
	$(DOXYGEN) $(DOXYGENFLAGS) doxygen.cfg
	touch $@

$(EXECS): libhwsec.a

fetch: LIBS += -lpq -lm
fetch: fetch.o
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBS)

cpa_cop: LIBS += -lm
cpa_cop: cpa_cop.o
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f libhwsec.a $(OBJS)

ultraclean:
	rm -rf libhwsec.a $(OBJS) $(EXECS) $(DOCDIR) .doc
