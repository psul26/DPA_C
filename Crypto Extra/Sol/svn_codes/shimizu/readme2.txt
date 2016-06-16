1. Summary

We add 3 attack files of category 1 of dpacontest.org.

2. How to run

(1) Download db-order.txt (about 7.5M bytes) from www.dpacontest.org.
(2) Modify the line of the script file to indicate the directory where the measured data are stored
as follows :

TRACEDIR = '../secmatv1_2006_04_0809'; % modify this.

(3) After that, execute either 'exec_dpa.m', 'exec_cpa.m', or 'exec_bcpa1.m' on Matlab.

3. File Descriptions

exec_cpa.m    execute CPA with random trace order.  (Matlab script file)
exec_dpa.m    execute DPA with random trace order. (Matlab script file)
exec_bcpa1.m  execute BS-CPA with random trace order. (Matlab script file)

cpa.m            CPA with pre-signal processing. (Matlab script file)
dpa.m            multi-bit DPA with pre-signal processing. (Matlab script file)
bcpa1.m          BS-CPA with pre-signal processing. (Matlab script file)

cpa-output.txt   result file of exec_cpa.m
dpa-output.txt   result file of exec_dpa.m
bcpa1-output.txt result file of exec_bcpa1.m

4. How to change parameters

The exec_??.m have some parameters for attack.

5. Some results

There are 10 attacks results with random trace order.

CPA needs 367.7 traces on average for whole key recovery.
DPA needs 349.2 traces on average for whole key recovery.
BS-CPA needs 208.8 traces on average for whole key recovery.


August 17, 2009
Hideo Shimizu
Toshiba Corporation Research & Development Center
