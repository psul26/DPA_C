
How to execute our code and reproduce our results
-------------------------------------------------

Our method is provided with the following files:
 - a single C source file (e.g. "dpa_contest.representative.3.c")
 - a file named "input.txt" which contains all 81089 plaintexts (in the temporal order)
 - a file named "output.txt" which contains all 81089 ciphertexts
 - a file named "AllNames.txt" which contains all 81089 trace file names
 
There should be no particular problem for compiling the source file. The following command should be successfull in most cases:

  gcc -O3 dpa_contest.representative.3.c -o dpa_contest.representative.3 -lm

In order to make the program work properly, you must place the three files "input.txt", "output.txt" and "AllNames.txt" in the parent directory of that where the program is executed. In case you are not happy with this constraint (e.g. you want these files to be searched in the current directory instead of the parent one), it is possible to edit the source file and make the suitable changes in the parts of the code where the respective files are opened.

A constant named "CURVE_DIRECTORY" is defined at the very beginning of the source file. This is the directory where the traces will be searched by the program. Note however that the traces should not be placed directly in this directory. Rather, they must be splitted into 9 different subdirectories named "0", "1", ..., "8" in the following way:
 - <CURVE_DIRECTORY>/0 should contain the 10,000 first traces (in the temporal order)
 - <CURVE_DIRECTORY>/1 should contain the 10,000 next ones
 - ...
 - <CURVE_DIRECTORY>/8 should contain the 1,089 last traces
The value of CURVE_DIRECTORY defaults to the parent directory. You are welcome to modify this value and recompile if you need so.

The program may be launched either with 0 or 1 argument:
    (a) dpa_contest.representative.3
 or (b) dpa_contest.representative.3 seed
where seed is a value which governs that of the initial seed to be used by the program.
If no seed value is explicitely provided on the command line (case (a)) then its initial value is set to that of the constant SEED_CHES defined at the beginning of the source file. If a positive value is provided for the seed on the command line, then this value is used as the initial seed. Finally, if you provide the value 0 as seed argument on the command line, then the initial seed will be initialized with the current time.

Note that the SEED_CHES value (1252305000) is the value returned by the function time() at the time of the beginning of the CHES 2009 workshop (September 7, at 08:30:00). This is the initial seed we used in our experiments that give our official representative scores averaged on 100 runs.
We decided to commit on this particularly meaningful value in order to give some evidence that we did not try different initial seeds until obtaining a particular one which gives some exceptionally good results.

Our official results are easily reproducible in the following way:
 - put the 3 text files and the 81,089 traces in the respective directories they will be searched in (see above),
 - simply run the program with no argument (SEED_CHES = 1252305000 will be used)
 - wait about 4 days until the 100 successive runs have been achieved !

Note that the pseudo-random number generator (PRNG) is reseeded at the beginning of each run. The seed value of run #i (i=0,...,99) is defined to be the initial seed + i. We adopted this way to define the seed of each run so that the 100 runs may be easily parallelized. It is so possible to reproduce our results in a matter of one day if you launch the different parts of the 100 runs on for instance four CPUs.

Note also that the PRNGs available on Windows and Linux platforms are different. As a consequence, they do not produce exactly the same results. Our results have been obtained with the Linux PRNG. At the time of writing of this document we do not know what results would have been obtained by using Windows PRNG and the exact same set of parameters.

Enjoy !
