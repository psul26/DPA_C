
 MATLAB script files for DPA Contest

 (C) COPYRIGHT 2009 MITSUBISHI ELECTRIC CORPORATION

 -- Author :D. Suzuki and M. Saeki--


 OVERVIEW
 --------

 Our implementation includes:

 Func_GenSF.m
  Pulls out plaintext or ciphertext information from a waveform file name,
  and generate selection functions in CPA.

 Func_RunPA.m
  Generates CPA trace.

 Func_SearchKey.m
  Extracts candidate keys from the CPA traces.

 Func_UpdateBC.m
  Updates the constraints for the subkeys of the first and last rounds
  according to the analyzed candidate keys.

 Func_DPAContest.m
  The top module of the Function M-files.
  Runs CPA and updates the constraints using the parameters defined in the
  Script M-files, and outputs the final results to 3 files.

 Run_FixedOrder.m
  Script M-file which defines parameters for the database order
  (and then runs analysis).

 Run_CustomOrder_r99.m
  Script M-file which defines parameters for the special order of the SVN r99
  (and then runs analysis).

 Run_CustomOrder_r140.m
  Script M-file which defines parameters for the special order of the SVN r140
  (and then runs analysis).

The file trees are as follows:

 Run_FixedOrder.m/Run_CustomOrder_r99.m/Run_CustomOrder_r140.m (Script M-files)
   |
   +---Func_DPAContest.m (Function M-file)
         |
         +---Func_UpdateBC.m (Function M-file)
         |
         +---Func_SearchKey.m (Function M-file)
               |
               +---Func_RunPA.m (Function M-file)
                     |
                     +---Func_GenSF.m (Function M-file)


 HOW TO RUN OUR M-FILES
 ----------------------
First, change the location paths for the list file and the waveform files
in the Script M-files. Then, run a Script M-file (e.g. Run_FixedOrder).
When finished, the analysis result will be output to 3 text files.

<OutFileIndex>_masterkey.txt: the result as to the master key
<OutFileIndex>_subkey.txt: the result as to the subkeys
<OutFileIndex>_log.txt: the log for each step


 ANALYSIS METHODOLOGY
 ----------------------
 Our analysis method is based on BS-CPA[1], with the below two improvements introduced.

 [1] Y. Komano, H. Shimizu and S. Kawamura,
    ``Build-in Determined Sub-key Correlation Power Analysis,''
      Cryptology ePrint Archive, 2009/161, 2009.

 1 Makes use of the relationship between the subkeys for the 1st and the 16th rounds.

  Our method attacks part of both the 1st and the 16th rounds,
  aiming to finally deduce more than 48 bits of the master key.
  Note here that large part of the subkeys for the 1st and the 16th rounds
  have the same secret information in common.
  For instance, in the "Fixed order" scheme,
  our attack extracts a candidate subkey for the S1
  at the 16th round using CPA.
  This key is denoted as {18 59 42 03 57 25}, where the digits are
  from the bits of the master key (1:64).
  In the second step, we extract a candidate subkey for the S2 at the 1st round
  using CPA. Same as above, this key is denoted as {33 63 02 09 19 42}.
  Now the bit 42 is already fixed in the first step, we deduce the key
  in the second step using the information.
  In this example, the search space in the second step is 2^5 keys:
  We can find a candidate key from a space half the size of that in the
  ordinary CPA.
  We continue the same procedure in the remaining steps, say,
  deduce candidate keys using the constraints for subkeys.

  In the "Fixed order" scheme, the final search space is represented as
  2^6+2^5+2^6+...+2^1+2^2.
  It's noteworthy that the space is smaller than that in the ordinary
  CPA, 8*2^6. (See <OutFileIndex>_log.txt)

 2 More precise modelling of the selection function

 As the selection function, [1] uses the Hamming distance of the data registers
 connected to f-function.
 In order for the selection function to model more precisely
 the behaviour of the actual hardware,
 we introduce two improvements shown below.

 (1) The fanout from the E-function considered
     The number of fanouts from the E-function of DES differs
     from bit to bit: can be 1 for some bits and 2 for the others.
     In our model, the registers whose fanout will be 2 are considered
     to have a Hamming weight of 2, and Hamming distances are calculated
     in accordance with that.

 (2) The transition of the key register considered
     The transition of the key register propagates to the logic circuits
     for the S-boxes, which supposedly affects the power consumption
     to a great extent.
     With that considered, we bring the transition into the selection function
     using the information of the candidate keys already analyzed.

 Nonetheless, it should be mentioned that the attack of [1] can be carried out
 with ciphertexts and waveforms, while our attack needs the plaintexts in addition.


 PARAMETER DEFINITIONS
 ----------------------
Parameters defined in Script M-files (e.g. Run_FixedOrder.m) are as follows:

WaveDir             : the location of the waveform files
FileListName        : the location of the list file
OutFileIndex        : the index of the output file (to distinguish from others)
TargetRange(1,1:2)  : the interval in the waveforms under analysis
                      to deduce the subkey for the 1st round
TargetRange(2,1:2)  : the interval in the waveforms under analysis
                      to deduce the subkey for the 16th round
TraceCount          : the number of waveforms when the attack first succeeds
                      IterationThreshold: the threshold to ensure the stability of the success
CompRatio           : the range of correlation coefficients to be evaluated.
                      If, for instance, CompRatio=5, the maximum of the mean values of coefficients
                      in the range of every 5 points is used for key deduction.
SearchMode(1,1)     : =1 E-function fanout considered, = 0 Not considered
SearchMode(1,2)     : =1 key register transition considered, = 0 Not considered
SearchMode(1,3)     : =1 BS-CPA, = 0 ordinary CPA
SearchOrder(1,:)    : =1 the 16th round, =0 the 1st round
SearchOrder(2,:)    : the S-box number
SearchTimes         : the number of steps of the attack
