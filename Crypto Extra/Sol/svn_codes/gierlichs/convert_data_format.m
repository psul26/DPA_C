%
% This script converts power traces from Agilent .bin format to Matlab .mat
% format. Additionally it extracts plaintexts, ciphertexts and the key from
% the file names. Input files have to reside in a subfoler data (binfolder),
% output files are written to a subfolder data2 (matfolder).
%
% Make sure the binfolder contains a file filelist.txt containing the file
% names of the input files, one per line! Can be generated with
% ls *.bin > filelist.txt 
% 
% Part of this code is based on Victor Lomn�'s code (SVN revision 65)
%
% Last change: 14/01/2009 Benedikt Gierlichs
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% define constants --------------------------------------------------------

% number of traces to convert
no_of_curves = 400;

% where is this project's base folder?
folder = '/data/users/bgierlic/dpacontest/';
% where is the Agilent .bin data?
binfolder = fullfile(folder,'data');
% where to store the output data in .mat format?
matfolder = fullfile(folder,'data2');
% create subfolder for output data, nothing happens if it already exists
mkdir(matfolder);

% end define constants ----------------------------------------------------

% allocate memory ---------------------------------------------------------

% matrix for plaintexts
plaintexts(1:no_of_curves,1:8) = 0;

% matrix for ciphertexts
ciphertexts(1:no_of_curves,1:8) = 0;

% end allocate memory -----------------------------------------------------


% open file containing trace filenames
fid = fopen(fullfile(binfolder,'filelist.txt'),'r');

% loop over no_of_curves power traces
for i = 1 : no_of_curves
    
    % read a line
    line = fgetl(fid);
    
    % extract key, plaintext and ciphertext from filename
    if(i == 1)
        
        [header,body] = strtok(line,'=');
        K = body(1,2:17);
        key(1,1) = hex2dec(K(1,1:2));
        key(1,2) = hex2dec(K(1,3:4));
        key(1,3) = hex2dec(K(1,5:6));
        key(1,4) = hex2dec(K(1,7:8));
        key(1,5) = hex2dec(K(1,9:10));
        key(1,6) = hex2dec(K(1,11:12));
        key(1,7) = hex2dec(K(1,13:14));
        key(1,8) = hex2dec(K(1,15:16));
        
        % find the plaintext
        PTI = body(1,21:36);
        plaintexts(i,1) = hex2dec(PTI(1,1:2));
        plaintexts(i,2) = hex2dec(PTI(1,3:4));
        plaintexts(i,3) = hex2dec(PTI(1,5:6));
        plaintexts(i,4) = hex2dec(PTI(1,7:8));
        plaintexts(i,5) = hex2dec(PTI(1,9:10));
        plaintexts(i,6) = hex2dec(PTI(1,11:12));
        plaintexts(i,7) = hex2dec(PTI(1,13:14));
        plaintexts(i,8) = hex2dec(PTI(1,15:16));
        
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
    
    else
    
        % find the plaintext
        [header,body] = strtok(line,'=');
        PTI = body(1,21:36);
        plaintexts(i,1) = hex2dec(PTI(1,1:2));
        plaintexts(i,2) = hex2dec(PTI(1,3:4));
        plaintexts(i,3) = hex2dec(PTI(1,5:6));
        plaintexts(i,4) = hex2dec(PTI(1,7:8));
        plaintexts(i,5) = hex2dec(PTI(1,9:10));
        plaintexts(i,6) = hex2dec(PTI(1,11:12));
        plaintexts(i,7) = hex2dec(PTI(1,13:14));
        plaintexts(i,8) = hex2dec(PTI(1,15:16));
        
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
        
    end % if
    
%     % generate a Windows batch file which we will call to convert the
%     % current power trace from .bin to .csv
%     filename = fullfile(folder,'temp.bat');
%     text = sprintf('%s %s %s"', 'agilent_bin_reader.exe', fullfile('data',line), 'temp.csv');
%     dlmwrite(filename, text, '');
%     % call the batch file
%     !temp.bat
    
    % alternatively, generate a Linux shell script
    filename = fullfile(folder,'temp.bat');
    text = sprintf('%s\n%s %s %s', '#!/bin/sh', './agilent_bin_reader.exe', fullfile('data',line), 'temp.csv');
    dlmwrite(filename, text, '');
    % make the batch file executable and call the batch file
    !chmod u+x temp.bat
    !./temp.bat
    
    % read the generated file in csv format
    filename = fullfile(folder,'temp.csv');
    one_curve = csvread(filename);
    % get rid of the timing info, keep only power values
    one_curve = one_curve(:,2)';
    
    % write this power trace to matfolder in .mat format
    filename = sprintf('trace%d.mat',i);
    filename = fullfile(matfolder,filename);
    save(filename, 'one_curve');
    
end % loop over no_of_curves power traces

% write the plaintexts, ciphertexts and the key to matfolder in .mat format
filename = fullfile(matfolder,'plaintexts.mat');
save(filename, 'plaintexts');
filename = fullfile(matfolder,'ciphertexts.mat');
save(filename, 'ciphertexts');
filename = fullfile(matfolder,'key.mat');
save(filename, 'key');

% all done, clean up and exit
delete(fullfile(folder,'temp.bat'));
delete(fullfile(folder,'temp.csv'));
text = sprintf('%d power curves successfully converted', i);
disp(text);
clear

