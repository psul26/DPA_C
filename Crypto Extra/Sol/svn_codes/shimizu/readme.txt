1. Summary
2. How to run the script file
3. File Description
4. Analyze Techniques
5. Some tips
6. Change History


--------------------------------------------------
1. Summary

(1) Environment

Matlab 7.7.0 on Windows XP

(2) Attack Side

Ciphertext side

(3) Attack Technique

CPA, Peak detection, Pre-signal Processing, BS-CPA, Advanced BS-CPA

(4) File Order

- Database file order (143 traces)
- Special file order by Tohoku University (119 traces)
- Special file order by Lomne (107traces)

--------------------------------------------------
2. How to run the script file

(1) Change the 6th line of the script file to indicate the directory where the measuring data is stored.
It is described as follows now.

tracedir = '../secmatv1_2006_04_0809'; % modify this.

(2) After that, execute either 'exec_db.m', 'exec_sp.m' or 'exec_sp2.m' on Matlab.

--------------------------------------------------
3. File Descriptions

exec_sp.m     execute BS-CPA with special file order.  (Matlab script file)
exec_db.m     execute BS-CPA with database file order. (Matlab script file)
exec_sp2.m    execute advanced BS-CPA with special trace order 'lomne141.txt'. (Matlab script file)

bcpa.m        BS-CPA. (Matlab function define script)
bcpa2.m       advanced BS-CPA (Matlab function define script)
makeref.m     generate predicated file. (Matlab function define script)

db-ref.mat    predicated file for database file order. (Matlab output format binary file)
sp-ref.mat    predicated file for special file order.  (Matlab output format binary file)

sp-order.txt  file list of special order (text file)
db-order.txt  filelist of database order (text file)
lomne141.txt  file list of special trace order by Lomne. (text file)

sp-output.txt result file of exec_sp.m (text file)
db-output.txt result file of exec_db.m (text file)
sp2-order.txt result file of exec_sp2.m(text file)

--------------------------------------------------
4. Analyze Techniques

(1) Signal Processing

We add a certain number of the measured data. It is also usefule for speed improvement.

(2) BS-CPA

We pile and recycle information on the subkey that has already been obtained.

See Yuichi Komano and Hideo Shimizu and Shinichi Kawamura,
"Build-in Determined Sub-key Correlation Power Analysis, " eprint 2009/161
(http://eprint.iacr.org/2009/161)

(3) advanced BS-CPA

Unlike BS-CPA, we use only the subset of past sbox set.

--------------------------------------------------
5. Some tips

We can directly treat measured data as binary file on Matlab. It is useful for speed improvement.




August 1, 2009
Hideo Shimizu
Toshiba Corporation Research & Development Center


--------------------------------------------------
6. Change History
Apr 7, 2009  Initial version
Aug 1, 2009  Add the result of advanced BS-CPA
