function y = dpaDEShwR16(folderDest,inputOrder,model,method,xMin16,xMax16,floor,step,ceil,threshold)
% use : dpaDEShwR16(folderDest,inputOrder,model,method,xMin16,xMax16,floor,step,ceil,threshold)
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

global matrixDPAhw

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
matrixResultshw = zeros(ceil,8,3);

% allocate space for selection on hamming weight
matrixAhw = zeros(64,L,8);
matrixBhw = zeros(64,L,8);
counterAhw = zeros(64,1,8);
counterBhw = zeros(64,1,8);
matrixDPAhw = zeros(64,L);

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
            
            % apply selection on HW(4 bits) of IV2
            if(sum(IV2,2) < 2)
                matrixAhw(k+1,:,j) = matrixAhw(k+1,:,j) + vectorTrace(1,:);
                counterAhw(k+1,1,j) = counterAhw(k+1,1,j) + 1;
            end;
            if(sum(IV2,2) > 2)
                matrixBhw(k+1,:,j) = matrixBhw(k+1,:,j) + vectorTrace(1,:);
                counterBhw(k+1,1,j) = counterBhw(k+1,1,j) + 1;
            end;
            
            % compute DPA difference when iteration is greater than floor & when mod(i,step) == 0
            if( ((i == floor) || (i > floor)) && (mod(i,step) == 0) )
                                
                matrixDPAhw(k+1,:) = (matrixBhw(k+1,:,j) / counterBhw(k+1,1,j)) - (matrixAhw(k+1,:,j) / counterAhw(k+1,1,j));

            end;            
            
        % end loop on the number of key hypothesis
        end;
        
        % -----------------------------------------------------------------
        % -------------------- keySearch on round 16 ----------------------
        
        % apply keySearch when iteration is greater than floor & when mod(i,step) == 0
        if( (i >= floor) && (mod(i,step) == 0) )
            
            % call function keySearch()
            % launch keySearch for each selection function
            [keyGuess,margin,abscissa] = keySearch(method,xMin16,xMax16,'hw');
            matrixResultshw(i,j,:) = [keyGuess,margin,abscissa];
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
        stability(i,1) = (stability(i-1,1) + 1) * keyBrokenChecker(matrixRoundKey(16,:),matrixResultshw(i,:,1));
         if(stability(i,1) == (threshold+1))
            output = sprintf('---> DPA on hw at round16 : key found in %d traces',i);
            disp(output);
        end;
        
    end;
    
    minStability = stability(i,1);
    
% end loop on ceil
end;

toc;

% call function writeResults()
% write results in files
writeResults(inputOrder,model,method,ceil,L,xMin16,xMax16,folderDest,matrixRoundKey(16,:),matrixResultshw(:,:,1),stability(:,1),16,'hw',floor,threshold);

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