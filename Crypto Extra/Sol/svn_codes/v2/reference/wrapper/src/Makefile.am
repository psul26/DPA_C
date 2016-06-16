bin_PROGRAMS = attack_wrapper compute_results

attack_wrapper_SOURCES = attack_wrapper.c aes.c db_postgresql.c trace_files.c utils.c dpa_contest.h dpa_contest_results.h aes.h db_postgresql.h trace_files.h utils.h
attack_wrapper_CPPFLAGS = $(POSTGRESQL_CFLAGS)
attack_wrapper_CFLAGS = $(PTHREAD_CFLAGS)
attack_wrapper_LDADD = $(POSTGRESQL_LDFLAGS) $(PTHREAD_LIBS)
attack_wrapper_LDFLAGS = $(PTHREAD_CFLAGS)

compute_results_SOURCES = compute_results.c dpa_contest.h dpa_contest_results.h
