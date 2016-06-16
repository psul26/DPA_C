function check = readBinaryTraces(folderSrc,fid,nbTraces,xMin,xMax)
% use : readBinaryTraces(folderSrc,fid,nbTraces,xMin,xMax)
%
% Author : Victor LOMNE - victor.lomne@lirmm.fr

global matrixTraces
global matrixCTO

% allocate memory for M and matrixCTO
matrixTraces = zeros(nbTraces,xMax-xMin+1);
matrixCTO = zeros(nbTraces,8);

% jump header
fgetl(fid);

% loop over the number of traces
for i = 1 : nbTraces
    
    % get filename of trace, CTO and read selected part at ieee little endian format
    tracename = fgetl(fid);
    [header,body] = strtok(tracename,'=');
    CTOhex = body(1,40:55);
    matrixCTO(i,1) = hex2dec(CTOhex(1,1:2));
    matrixCTO(i,2) = hex2dec(CTOhex(1,3:4));
    matrixCTO(i,3) = hex2dec(CTOhex(1,5:6));
    matrixCTO(i,4) = hex2dec(CTOhex(1,7:8));
    matrixCTO(i,5) = hex2dec(CTOhex(1,9:10));
    matrixCTO(i,6) = hex2dec(CTOhex(1,11:12));
    matrixCTO(i,7) = hex2dec(CTOhex(1,13:14));
    matrixCTO(i,8) = hex2dec(CTOhex(1,15:16));
    fid2 = fopen(fullfile(folderSrc,tracename),'r','ieee-le');
    if(fid2 == -1)
        disp('traces can not be read !');
        disp('check path !');
        quit;
    end;
    fseek(fid2, 164 + 4 * xMin, 'bof');
    matrixTraces(i,:) = fread(fid2, [1 (xMax-xMin+1)], 'float');
    fclose(fid2);
    
end;

check = 0;