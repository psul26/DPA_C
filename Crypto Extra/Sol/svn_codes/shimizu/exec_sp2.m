% bcpa batch file for Lomne's special order
% method : Advance BS-CPA
%
% by Hideo Shimizu
% Copyright(C) 2009 by Toshiba Corporation

tracedir = '../secmatv1_2006_04_0809'; % modify this.

ordfile = 'lomne141.txt';
reffile = 'sp2-ref.mat';
outfile = 'sp2-output.txt';
subkey = [60 11 56 46 22 50 16 44];
% Explanation of sboxorder
% For example, BS-CPA using S-box order is [4 3 5 8 1 2 6 7], 
% 1st element 4 means sbox4.
% 2nd element 3 means sbox3, and sbox3 depends on sbox4.
% 3rd element 5 means sbox5, and sbox5 depends on both sbox4 and sbox3.
% 8th element 7 means sbox7, and sbox7 depends on sbox4, 3, 5, 8, 1, 2, 6.
%
% Unlike BS-CPA, Advanced BS-CPA use the subset of past S-box.
% 1st element 4 means sbox4.
% 2nd element [4 3] means sbox3 depends on sbox4.
% 3rd element [3 5] means sbox5 depends on sbox3. Unlike BS-CPA, sbox5 does
% not depend on sbox4. In other word, sbox5 depends on the subset {3} of
% the past sbox set {4 3}.
% 8th element [1 3 4 6 7] means sbox 7 depends on sbox1, sbox3, sbox4, and
% sbox6.
% It is notice that sboxorder is a structured list using brace ({, }).
sboxorder = {4 [4 3] [3 5] [5 8] [8 1] [5 2] [4 6] [1 3 4 6 7]};
first = 14482; % temporal window first
last  = 14484; % temporal window last
ratio = 3;     % integrate ratio. We add each 'ratio' elements, then analyze.

if exist(reffile', 'file') ~= 2
    makeref(reffile, ordfile, 150);
end

bcpa2(tracedir, ordfile, reffile, outfile, ratio, first, last, sboxorder, subkey, 3, 100, 150);
