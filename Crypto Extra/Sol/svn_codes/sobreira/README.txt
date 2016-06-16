					DPA CONTEST 09

A SIMPLE IMPROVEMENT OF CPA BY CONSIDERING THE LEFT SIDE OF THE MESSAGE REGISTER
(August 2009)

Authors: Antonio Almeida (almeida@ira.uka.de)
 	 - Institut für Kryptographie und Sicherheit (IKS)
           Universität Karlsruhe (TH)
           http://avalon.ira.uka.de/eiss/

         Dejan Lazich (lazic@ira.uka.de)
     	 - Institut für Kryptographie und Sicherheit (IKS)
           Universität Karlsruhe (TH)
	   http://avalon.ira.uka.de/eiss/

---------
Implementation:

Our implementation is based on the reference code (SVN ches09 revision 252) provided by the DPA Contest organizers. 
The only change is the addition of the Hamming distance for the left side of the message register in the key_estimator.py file.


--------
Method:

We propose a simple method that improves the reference CPA when considering the influence of the  message register left side (L) in the first round. The  reference CPA implementation checks only the four output bits of a S-BOX under inspection, which correspond to four bits in the right side (R) of the message register. Only the Hamming distance (dH (SBOX(i))) between the known old (subset of R0) and guessed new values (subset of R1, if the guess is right) of these four bits is included in the computation of the correlation value.
However, the old content (L0) and the new content (L1 = R0) of the message register left side are completely known in the first round. By considering the message register left side, a higher signal-to-noise ratio (SNR) can be attained. Therefore, in the computation of the correlation value we simply added to dH (SBOX(i)) the Hamming distance dH (L0, L1) between the known old and new content of L. In this way, we reduced the number of traces needed to guess the key.


---------
Results:
The results show the obtained improvement: without considering L, the average number of traces needed to obtain the key was 530.454 (see DPA Contest's Hall of Fame); by including L, we reduced it to 367.025, estimated with 200 attacks.


--------
Note: We tried our approach in the last round, but it has not shown the expected improvement in results.

We consider that there must be some hardware-related reason for this. We suppose that some other procedure and/or register loading is occuring simultaneously, e.g. in the key scheduling, that somehow masks the current consumption in the last round and disables the use of our CPA expansion method.

