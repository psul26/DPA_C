This implementation consists of a collection of Matlab scripts and an
external program to read the power traces in agilent .bin format. The
Matlab scripts are:
* convert_data_format == processes agilent .bin files, extracts
  plaintexts, ciphertexts and key from the filename and converts
  agilent .bin format to Matlab .mat format
* agilent_bin_reader.exe == used to convert from agilent .bin to .csv
  format, compile the C code provided by dpacontest.org; independent
  of the OS you work on, name the executable .exe or change the call
  in the above Matlab script
* computeIV2.m == computes intermediate results of the last DES round,
  the code was extracted from Victor Lomne's implementation
* ComputeRoundKey == computes the correct DES round keys for
  verification, the code was extracted from Victor Lomne's
  implementation
* dpa_ttest_DES_R16.m == performs the DPA attack; parameters can be set at the
  beginning of the script
* dpa_dom_DES_R16.m == performs the DPA attack; parameters can be set at the 
  beginning of the script
* result_DPA_ttest4bit.txt == contains the round keys for DES round 16 as guessed by
  the attack
* resultDPA4_dom.txt == contains the round keys for DES round 16 as guessed by the
  attack

The entire project has to reside in a folder structure as follows:
--- base directory == Matlab variable >folder<, contains all of the
|   above listed files
|
|-- data == contains agilent .bin files as provided by dpacontest.org,
|   Matlab variable >binfiles<
|
|-- data2 == contains the plaintexts, ciphertexts, the correct key and
    the power traces in Matlab format, Matlab variable >matfiles<,
    this directory will be generated and filled by convert_data_format.m


To run this code
* place e.g. 400 power traces in Agilent .bin format in the subdirectory
  data
* cd into the data directory and issue ls *.bin > filelist.txt
* open Matlab and cd into the base directory
* run the script convert_data_format.m in Matlab (adapt parameters)
* run the script dpa_ttest_DES_R16.m
* or run the script dpa_dom_DES_R16.m

To check your result, open result.txt.


Added on 23/01/09

Things that can go wrong... make sure that
- the binary agilent_bin_reader.exe is executable
- the filelist.txt you generated looks ok
- the Matlab variable >folder< has to be an absolute path

The hall of fame result >382 traces< (i.e. the key guess is stable from
282 traces onwards) for DPA with a t-test can be reproduced using the
filelist filelist_dpa_ttest_382.txt.

The hall of fame result >329 traces< (i.e. the key guess is stable from
229 traces onwards) for DPA with a DoM-test can be reproduced using the
filelist filelist_dpa_ttest_382.txt (this is not a typo!).
