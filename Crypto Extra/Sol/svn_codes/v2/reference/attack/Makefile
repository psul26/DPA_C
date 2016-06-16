LDLIBS = -lm
#CFLAGS = -Wall -Werror -I/usr/include/postgresql
#CFLAGS += -O0 -g
CPPFLAGS = -Wall -Werror -O3
#LDFLAGS = 

all: attack_reference

attack_reference: dpa_contest.h attack_last_round.o

attack_last_round.o: attack_last_round.h attack_last_round.cpp

.PHONY: clean

clean:
	rm -f attack_reference *.o
	rm -f log.txt results
	rm -f *~
