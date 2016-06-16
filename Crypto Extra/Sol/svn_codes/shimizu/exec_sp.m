% bcpa batch file for special order
%
% by Hideo Shimizu
% Copyright(C) 2009 by Toshiba Corporation

tracedir = '../secmatv1_2006_04_0809'; % modify this.

ordfile = 'sp-order.txt';
reffile = 'sp-ref.mat';
outfile = 'sp-output.txt';
subkey = [60 11 56 46 22 50 16 44];
sboxorder = [2 6 4 7 8 1 3 5];
first = 14400;
last  = 14500;
ratio = 19;

if exist(reffile', 'file') ~= 2
    makeref(reffile, ordfile, 500);
end

bcpa(tracedir, ordfile, reffile, outfile, ratio, first, last, sboxorder, subkey, 10, 100, 500);
