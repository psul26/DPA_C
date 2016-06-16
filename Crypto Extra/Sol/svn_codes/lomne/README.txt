Author : Victor Lomne - 17/05/2009 - victor.lomne@lirmm.fr

This file describes how to use my functions implemented in Matlab to launch my different Side-Channel Attack implementations on the acquisition campaign secmatv1, and how to check my results for the 2008-2009 DPAcontest.


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Requirement for Fixed and Custom Orders %%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

You just need Matlab, which is OS-independent and an Internet connection, because my programs download waves stored on the ENST database.
You need also to copy a JDBC driver in the directory : "$Matlab_root_directory$/java/jar/" for Linux or in "$Matlab_root_directory$\java\jar\" for Windows.
(you can find it at http://jdbc.postgresql.org/download.html, and download postgresql-8.3-604.jdbc4.jar).
Then add the absolute path "$Matlab_root_directory$/java/jar/postgresql-8.3-604.jdbc4.jar" for Linux or "$Matlab_root_directory$\java\jar\postgresql-8.3-604.jdbc4.jar" for Windows using the command javaaddpath('your_absolute_path').


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%               Fixed Order               %%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

folder "fixedOrder" : I have proposed 4 attacks

1/ DPA DoM 4bits on the 16th round of the DES using the database order.
To use this attack, just launch the script "launchDPA_DoM_4bits_R16.m"
This attack finds the key with 360 waves according a stability of 100 iterations.
the result's file will be written in your current Matlab directory.

2/ DPA DoM HW on the 16th round of the DES using the database order.
To use this attack, just launch the script "launchDPA_DoM_HW_R16.m"
This attack finds the key with 327 waves according a stability of 100 iterations.
the result's file will be written in your current Matlab directory.

3/ CPA on the 16th round of the DES selecting the good temporal window using the database order.
To use this attack, just launch the script "launchCPA_R16.m"
This attack finds the key with 322 waves according a stability of 100 iterations.
the result's file will be written in your current Matlab directory.

4/ CPA enhancement (from the work of [1]) on the 16th round of the DES selecting the good temporal window using the database order.
To use this attack, just launch the script "launchCPAenhancement_R16.m"
This attack finds the key with 310 waves according a stability of 100 iterations.
the result's file will be written in your current Matlab directory.


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%               Custom Order              %%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

folder "customOrder" : I have proposed 2 attacks

Instead of using the database order, I have generated a statistically optimized order inspired from the work of Yongdae KIM.
This order is written in the file filelistStatOrder.txt

How did I generate this order :
a) with a SPA (Simple Power Analysis) of one trace from the acquisition campaign secmatv1,
I select the good temporal window corresponding to the round 16 : time indexes 14450 to 14550.
b) for each trace, I search the time index corresponding to the maximum of the peak which is the power consumption of the round 16.
c) I compute the average of all time indexes, and I obtain 14500.
d) then, among all the traces, I select the traces with the greatest or the smallest power consumption value at time index 14500.

Then I obtain 2 interessant results with this file order :

5/ CPA enhancement (from the work of [1]) on the 16th round of the DES selecting the good temporal window using the statistically optimized order.
To use this attack, just launch the script "launchCPAenhancement_R16_statOrder.m"
This attack finds the key with 116 waves according a stability of 100 iterations.
the result's file will be written in your current Matlab directory.

6/ DPA DoM 4bits on the 16th round of the DES selecting the good temporal window using the statistically optimized order.
To use this attack, just launch the script "launchDPA_DoM_4bits_R16_statOrder.m"
This attack finds the key with 112 waves according a stability of 100 iterations.
the result's file will be written in your current Matlab directory.

P.S. : sometimes when the function "queryDB.m" etablishes the connection with the database, Matlab can crash because of an unfixed bug.
If this problem happens, relaunch Matlab and the attack. If the problem remains, contact me at victor.lomne@lirmm.fr


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Requirement for Fixed and Custom Orders %%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

You just need Matlab, which is OS-independent and a folder where all the traces of the acquisition campaign secmatv1 are stored in binary.
Change the absolute path in the M-file "launchBSCPArepresentativeOrder.m" (in the subfolder "lomne/representativeOrder/") with the absolute path of the locval folder where you have stored the traces to launch the attack.


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%           Representative Order          %%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

I use another attack for the representative order, called : BS-enhanced-CPA with sbox attack order determined "on-the-fly"

This attack is based on [1] and [2], but instead of using always the same sbox attack order, I compute it "on-the-fly".
I obtain an average result of 176,62 according a stability of 100 iterations for cracking the round-key 16 estimated with 100 attacks where traces are randomly chosen among all the secmatv1 traces.

The algorithm works following these steps :

a) At each increment of the number of traces, each subkey is attacked independently following the enhanced-CPA of [1] on time indexes 14470 to 14520 of traces, targeting the last round of the DES.

b) For a given subkey, if the guess value is the same when increasing the number of used traces during threshold times, then consider that this subkey is cracked, so remove the corresponding sbox number from the list "sboxesToCrack" and put it in the list "sboxCracked".

c) Restart the attack (reinitialize the increment) only on the updated list "sboxesToCrack", but using also the value of the cracked subkey to increase the correlation.

d) Follow these steps until cracking all the subkeys. The recursive idea will decrease significantly the number of traces to crack each subkey. When all the subkeys are cracked (i.e. the guessing round key 16 has held the stability threshold), check if the obtained full round key 16 is really the good one (this step could be done even if you do not know the master key, by computing the master key from the guess round key 16 and by ciphering a plaintext with the computed master key and comparing the obtained ciphertext and the real ciphertext provided).

   If the round key 16 obtained is not the good, restart the attack but remove the sbox corresponding to the first cracked subkey from the list "sboxesToCrack" and put it in quarantine by adding it in the list 'sboxStandBy". As soon as another subkey is cracked, remove the sbox in quarantine and reintegrate it in the list "sboxesToCrack". If you obtain again a bad guess round key 16, then the 2 first sboxes cracked during the 2 first trys will be in quarantine, then you will restart the attack on only 6 sboxes. The 2 sboxes in quarantine will be reintegrated in the list "sboxesToCrack" as soon as one of the remaining 6 sboxes will be cracked.

e) If the increment is equal to the parameter "ceil", and, if the algorithm has tried all the sboxes in first and the round key 16 is not cracked or no subkey has held the stability threshold, then increase the paramter "ceil" and restart the attack from a).

Notice that this attack is a "known-ciphertext attack", and generic, without preprocessed constants. You just have to choose the threshold (in the range of the dpacontest this value is fixed at 100) and the ceil (the number of used traces). I have arbitrarily chosen the initial ceil at 200.


[1] Thanh-Ha LE et al., "A Proposition for Correlation Power Analysis Enhancement", CHES 2006
[2] Yuichi Komano and Hideo Shimizu and Shinichi Kawamura, "Build-in Determined Sub-key Correlation Power Analysis", eprint 2009/161

