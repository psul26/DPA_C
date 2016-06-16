
------- File Description -----------------------------------------

Author : Kim Yongdae - 27 Feb 09 - kimyd@aoki.ecei.tohoku.ac.jp

This file describe how to use the scripts.

My implementation has 4 files including one csv file list
file. I will explain in detail as follows.

500_csvfilename.csv
 Power trace csv file name list. You need to convert 'bin'
 format to 'csv' format.

convert_data_format.m
 To convert power trace to 'mat' format from 'csv'
 format. Also, the last round intermediate value (hamming
 distance) will be written in "data" directory as 'mat'
 format. 
 After launching this file, you can find out two
 'mat' format file in "data" directory as follows.
 measured_trace.mat : Power trace 
 pred_trace.mat : The last round intermediate value (HD model)

computeIV2.m
 This file is used in "convert_data_format.m" to compute
 intermediate value.

exec_cpa_pearson.m
 This file actually launch CPA attack using pearson
 correlation coefficient. Before launch this
 file, you need to launch "convert_data_format.m" first. The
 result's file will be written in "output" directory named
 "output_pearson.txt"

exec_cpa_spearman.m
 This file launch CPA attack Using Spearman's rho. The
 result file will be written in "output" directory named
 "output_spearman.m".

exec_cpa_kendall.m
 This file launch CPA attack Using Kendall's tau. The result
 file will be written in "output" directory named
 "output_kendall".

-------------- How to select special file order -----------------
From now on, I will explain the way to select file
order. 

 Firstly, after conducting CPA using all-bits-long hamming
 distance, then we can find out which time index have the
 most correlation relation with hamming distance and power
 trace. The result was aobut 14500 time index. 

 Second, compute mean(m) and variance(s) power trace of the
 time index using lots of power traces. I used about 20,000
 traces.

 Third, it is well known that power trace follows normal
 distribution(gaussian distribution) as follows,

       Power trace ~ N(m,s) at 14500 time index.

 Fourth, we can calculate probability using PDF of N(m,s) on
 each power traces. After that, we can sort power trace by
 probability of each power traces. My file order is the result
 of sorting among 20,000 traces.

--------------------------------------------------------------------


------------<2009/08/24 : chosen plaintext order>-------------------
10000_csvfilenames.csv 
 Power traces csv filename list by database order.

convert_data_format_10000.m
 This is exactly same as 'convert_to_data_format.m'. The
 only difference is the target of power traces from 500
 traces to 10000 traces. This would take about 30 minutes.

exec_cpa_chosen.m
 This file launch CPA attack and choose 500 power traces
 from 10,0000 traces. Before execute this file,
 'covert_data_format_10000.m' should be launched first. 
 The result file will be written in 'output_cpa_chosen.txt'
 in 'output' directory. 

exec_stoch_mtd.m
 This file launch Stochastic Model Attacks. As described in
 Improving the Rules of the DPA Contest (IACR eprint #
 2008/517), we use different power trace for profiling and
 attack phase. The file names 'output_stochastic.txt' in
 'output' directory will be written. In this implementation,
 we use F5 subspace. And 8000 power traces for profiling and
 500 power traces in database order are used for attack. 
 Before launch this script, 'convert_data_format_10000.'
 should be launched first to get 'MAT' format trace file. 

--------<2009/08/28 : Representative Order>-----------
exec_stoch_wrapper.m 
 This file execute stochastic model attack 100
 times. The traces order for attack is randomly choosen
 among 10,000 power traces. So, 'convert_data_format_10000.m'
 file is needed to be launched first to generate MAT data
 files. The result file 'output_stochastic_rep.txt' is
 written after finishing the analysis. 

func_stoch_mtd.m
 This is file have function which is used in
 'exec_stoch_wrapper.m'

stoch_wrapper.log
 I logged the output of 'exec_stoch_wrapper.m'. I obtained
 the average 230.78 traces for the attacks. 


 
 



