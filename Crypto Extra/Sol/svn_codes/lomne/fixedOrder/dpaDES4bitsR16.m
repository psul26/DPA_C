function y = dpaDES4bitsR16(folderDest,inputOrder,model,method,xMin16,xMax16,floor,step,ceil,threshold)
% use : dpaDES4bitsR16(folderDest,inputOrder,model,method,xMin16,xMax16,floor,step,ceil,threshold)
%
% Author : Victor LOMNE - victor.lomne@lirmm.fr
%
% folderDest : path of folder where results & pictures will be stored.
%
% inputOrder : 'chrono' / 'db'
%               chrono : chronological / alphanumerical order
%               db : pseudo random order following the SQL 'SELECT' function
%
% model : power consumption model
%         'HW'   -> Hamming Weight
%         'HD'   -> Hamming Distance
%
% method : 'maxPeak' / 'maxIntegral'
%
% xMin16 & xMax16 : values where searching maxPeak or maxIntegral on DPA traces for round 16
%
% floor : iteration from where we begin keySearch
%
% step : step where we compute keySearch
%
% ceil : iteration where we stop the DPA, even if key is not broken
%
% threshold : stability threshold to consider that key is broken

global matrixDPA0
global matrixDPA1
global matrixDPA2
global matrixDPA3
global matrixDPAsum

disp(' ');
disp('---------------------------------------------------------------------');
disp('---------------------------------------------------------------------');
disp(' ');
disp('                          Launching DPA ...');
disp(' ');

% trig the clock
t1 = cputime;

% call function queryDB()
% read key, PTI, CTO & trace
[key,PTI,CTO,trace] = queryDB('index',1);
L = size(trace,2);

% open filelistStatOrder.txt
if(strcmp(inputOrder,'stat') == 1)
    fid = fopen('filelistStatOrder.txt','r');
end;

% call function computeRoundKeysDES()
% compute roundKey
matrixRoundKey = computeRoundKeysDES(key);

% initialize matrixResults
matrixResultssum = zeros(ceil,8,3);

% allocate space for selection on bit0
matrixA0 = zeros(64,L,8);
matrixB0 = zeros(64,L,8);
counterA0 = zeros(64,1,8);
counterB0 = zeros(64,1,8);
matrixDPA0 = zeros(64,L);

% allocate space for selection on bit1
matrixA1 = zeros(64,L,8);
matrixB1 = zeros(64,L,8);
counterA1 = zeros(64,1,8);
counterB1 = zeros(64,1,8);
matrixDPA1 = zeros(64,L);

% allocate space for selection on bit2
matrixA2 = zeros(64,L,8);
matrixB2 = zeros(64,L,8);
counterA2 = zeros(64,1,8);
counterB2 = zeros(64,1,8);
matrixDPA2 = zeros(64,L);

% allocate space for selection on bit3
matrixA3 = zeros(64,L,8);
matrixB3 = zeros(64,L,8);
counterA3 = zeros(64,1,8);
counterB3 = zeros(64,1,8);
matrixDPA3 = zeros(64,L);

% allocate space for selection on sum of monobit
matrixDPAsum = zeros(64,L);

% initialise traceCounter & stability
i = 0;
stability = zeros(ceil,1);
minStability = 0;
tic;
% -------------------------------------------------------------------------
% ----------------------------- DPA algorithm -----------------------------

% loop on the number of traces
while((i < ceil) && (minStability <= threshold))
    
    % increment traceCounter
    i = i + 1;
    
    % call function queryDB()
    % read key, PTI, CTO & trace
    switch(inputOrder)
        case 'db'
            [key,PTI,CTO,vectorTrace] = queryDB('index',i);
        case 'stat'
            line = fgetl(fid);
            message = line(1,55:70);
            [key,PTI,CTO,vectorTrace] = queryDB('message',message);
    end;
    
    % loop on the number of sboxes
    for j = 1 : 8
        
        % -----------------------------------------------------------------
        % --------------------------- round 16 ----------------------------
        
        % loop on the number of key hypothesis
        for k = 0 : 63
            
            % call function computeIV2DES()
            % compute Intermediate value of round 16
            IV2 = computeIV2DES(model,j,CTO,k);
            
            % apply selection on bit0 of IV2
            if(IV2(1,4) == 0)
                matrixA0(k+1,:,j) = matrixA0(k+1,:,j) + vectorTrace(1,:);
                counterA0(k+1,1,j) = counterA0(k+1,1,j) + 1;
            end;
            if(IV2(1,4) == 1)
                matrixB0(k+1,:,j) = matrixB0(k+1,:,j) + vectorTrace(1,:);
                counterB0(k+1,1,j) = counterB0(k+1,1,j) + 1;
            end;
            
            % apply selection on bit1 of IV2
            if(IV2(1,3) == 0)
                matrixA1(k+1,:,j) = matrixA1(k+1,:,j) + vectorTrace(1,:);
                counterA1(k+1,1,j) = counterA1(k+1,1,j) + 1;
            end;
            if(IV2(1,3) == 1)
                matrixB1(k+1,:,j) = matrixB1(k+1,:,j) + vectorTrace(1,:);
                counterB1(k+1,1,j) = counterB1(k+1,1,j) + 1;
            end;
            
            % apply selection on bit2 of IV2
            if(IV2(1,2) == 0)
                matrixA2(k+1,:,j) = matrixA2(k+1,:,j) + vectorTrace(1,:);
                counterA2(k+1,1,j) = counterA2(k+1,1,j) + 1;
            end;
            if(IV2(1,2) == 1)
                matrixB2(k+1,:,j) = matrixB2(k+1,:,j) + vectorTrace(1,:);
                counterB2(k+1,1,j) = counterB2(k+1,1,j) + 1;
            end;
            
            % apply selection on bit3 of IV2
            if(IV2(1,1) == 0)
                matrixA3(k+1,:,j) = matrixA3(k+1,:,j) + vectorTrace(1,:);
                counterA3(k+1,1,j) = counterA3(k+1,1,j) + 1;
            end;
            if(IV2(1,1) == 1)
                matrixB3(k+1,:,j) = matrixB3(k+1,:,j) + vectorTrace(1,:);
                counterB3(k+1,1,j) = counterB3(k+1,1,j) + 1;
            end;       
            
            % compute DPA difference when iteration is greater than floor & when mod(i,step) == 0
            if( ((i == floor) || (i > floor)) && (mod(i,step) == 0) )
                
                matrixDPA0(k+1,:) = (matrixB0(k+1,:,j) / counterB0(k+1,1,j)) - (matrixA0(k+1,:,j) / counterA0(k+1,1,j));
                
                matrixDPA1(k+1,:) = (matrixB1(k+1,:,j) / counterB1(k+1,1,j)) - (matrixA1(k+1,:,j) / counterA1(k+1,1,j));
                
                matrixDPA2(k+1,:) = (matrixB2(k+1,:,j) / counterB2(k+1,1,j)) - (matrixA2(k+1,:,j) / counterA2(k+1,1,j));
                
                matrixDPA3(k+1,:) = (matrixB3(k+1,:,j) / counterB3(k+1,1,j)) - (matrixA3(k+1,:,j) / counterA3(k+1,1,j));
                
                matrixDPAsum(k+1,:) = (matrixDPA0(k+1,:) + matrixDPA1(k+1,:) + matrixDPA2(k+1,:) + matrixDPA3(k+1,:)) / 4;

            end;      
            
        % end loop on the number of key hypothesis
        end;
        
        % -----------------------------------------------------------------
        % -------------------- keySearch on round 16 ----------------------
        
        % apply keySearch when iteration is greater than floor & when mod(i,step) == 0
        if( (i >= floor) && (mod(i,step) == 0) )
            
            % call function keySearch()
            % launch keySearch for each selection function
            [keyGuess,margin,abscissa] = keySearch(method,xMin16,xMax16,'sum');
            matrixResultssum(i,j,:) = [keyGuess,margin,abscissa];
        end;
        
        % ------------------------- end round 16 --------------------------
        % -----------------------------------------------------------------
        
    % end loop on sboxes
    end;

    % display
    if((i >= floor) && (mod(i,100) == 0))
        output = sprintf('%d traces processed !',i);
        disp(output);
        toc;
        tic;
    end;
    
    % apply keyBrokenChecker when iteration is greater than floor & when mod(i,step) == 0
    if( (i >= floor) && (mod(i,step) == 0) )
    
        % hw - round 16
        stability(i,1) = (stability(i-1,1) + 1) * keyBrokenChecker(matrixRoundKey(16,:),matrixResultssum(i,:,1));
         if(stability(i,1) == (threshold+1))
            output = sprintf('---> DPA on 4bits at round16 : key found in %d traces',i);
            disp(output);
        end;
        
    end;
    
    minStability = stability(i,1);
    
% end loop on ceil
end;

toc;

% call function writeResults()
% write results in files
writeResults(inputOrder,model,method,ceil,L,xMin16,xMax16,folderDest,matrixRoundKey(16,:),matrixResultssum(:,:,1),stability(:,1),16,'sum',floor,threshold);

% close filelistStatOrder.txt
if(strcmp(inputOrder,'stat') == 1)
    fclose(fid);
end;

% stop the clock
t2 = cputime - t1;
output = sprintf('Total time : %d',t2);
disp(' ');
disp(output);
disp(' ');
disp('---------------------------------------------------------------------');
disp('---------------------------------------------------------------------');
disp(' ');

y = 'DPA done !';

% clean workspace
clear all;