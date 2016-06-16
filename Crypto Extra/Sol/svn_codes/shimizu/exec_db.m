% bcpa batch file for database order
%
% by Hideo Shimizu
% Copyright(C) 2009 by Toshiba Corporation

tracedir = '../secmatv1_2006_04_0809'; % modify this.

ordfile = 'db-order.txt';
reffile = 'db-ref.mat';
outfile = 'db-output.txt';
subkey = [60 11 56 46 22 50 16 44];
sboxorder = [1 4 7 5 6 8 3 2];
first = 14450;
last  = 14550;
ratio = 26;

if exist(reffile', 'file') ~= 2
    makeref(reffile, ordfile, 500);
end

bcpa(tracedir, ordfile, reffile, outfile, ratio, first, last, sboxorder, subkey, 10, 100, 500);
