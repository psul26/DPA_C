%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%             convert_data_format.m
%  DATE : Last Modified : 27 Feb 09 Kim Yongdae
%  DESCRIPTION : This script convert power traces and intermediate
%  power prediction value to Matlab .mat format.
%  Before launch this script convert Agilent .bin format to .csv
%  format using example C program from www.dpacontest.com
%  
%  Part of this code is based Victor Lomne's code.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

clear all
%the number of power traces
no_of_curves = 500;
%power traces folder name
csvfolder = './traces';
%destination of .mat format files
matfolder = 'data';
%create subfolder
mkdir(matfolder);


% matrix for plaintexts
plaintexts(1:no_of_curves,1:8) = 0;
% matrix for ciphertexts
ciphertexts(1:no_of_curves,1:8) = 0;
% power traces
measured_trace = zeros(no_of_curves,20000);
pred_trace = zeros(no_of_curves,64*8);
% end allocate memory -----------------------------------------------------


% open file containing trace filenames
fid = fopen('500_csvfilename.csv','r');

% loop over no_of_curves power traces
for i = 1 : no_of_curves
  i
  % read a line
  line = fgetl(fid);
  
  % extract key, plaintext and ciphertext from filename
  [header,body] = strtok(line,'=');
  % find the ciphertext
  CTO = body(1,40:55);
  
  ciphertexts(i,1) = hex2dec(CTO(1,1:2));
  ciphertexts(i,2) = hex2dec(CTO(1,3:4));
  ciphertexts(i,3) = hex2dec(CTO(1,5:6));
  ciphertexts(i,4) = hex2dec(CTO(1,7:8));
  ciphertexts(i,5) = hex2dec(CTO(1,9:10));
  ciphertexts(i,6) = hex2dec(CTO(1,11:12));
  ciphertexts(i,7) = hex2dec(CTO(1,13:14));
  ciphertexts(i,8) = hex2dec(CTO(1,15:16));
  
  % read the generated file in csv format
  filename = fullfile(csvfolder,line);
  one_curve = csvread(filename);
  % get rid of the timing info, keep only power values
  one_curve = one_curve(:,2)';
  measured_trace(i,:) = one_curve(:,1:20000);
  for sbox=1:8
    pred = zeros(1,64);
    for k=0:63
      IV_tmp = computeIV2('HD',sbox,ciphertexts(i,:),k);
      pred(k+1) = sum(IV_tmp(1:4));
    end
    pred_trace(i,(sbox-1)*64+1:sbox*64) = pred;
  end
end % loop over no_of_curves power traces

filename = fullfile(matfolder,'pred_trace.mat');
save(filename,'pred_trace');
filename = fullfile(matfolder,'measured_trace.mat');
save(filename,'measured_trace');

% all done, clean up and exit
text = sprintf('%d power traces successfully converted', i);
disp(text);
clear

