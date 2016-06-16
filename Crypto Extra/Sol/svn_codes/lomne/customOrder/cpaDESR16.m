function y = cpaDESR16(folderDest,inputOrder,model,method,xMin16,xMax16,floor,step,ceil,threshold,epsilon)
% use : cpaDESR16(folderDest,inputOrder,model,method,xMin16,xMax16,floor,step,ceil,threshold,stepPic,epsilon)
%
% Author : Victor LOMNE - victor.lomne@lirmm.fr
%
% folderDest : path of folder where results & pictures will be stored.
%
% inputOrder : order dealing with traces
%               'db'   -> database order
%               'stat' -> statistically optimized order
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
%
% epsilon : positive value for CPA enhancement

global matrixCPA

disp(' ');
disp('---------------------------------------------------------------------');
disp('---------------------------------------------------------------------');
disp(' ');
disp('                          Launching CPA ...');
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
matrixResults = zeros(ceil,8,3);

% allocate space for matrixCPAR1 & matrixCPAR16
matrixCPA = zeros(64,xMax16-xMin16);

% allocate space for buffers computing the CPA
sumXiR16 = zeros(64,8);
sumXi2R16 = zeros(64,8);
sumYiR16 = zeros(1,xMax16-xMin16);
sumYi2R16 = zeros(1,xMax16-xMin16);
stdYiR16 = zeros(1,xMax16-xMin16);
sumXiYiR16 = zeros(64,8,xMax16-xMin16);

% initialise traceCounter & stability
i = 0;
stability = zeros(ceil,1);
minStability = 0;
tic;

% -------------------------------------------------------------------------
% ----------------------------- CPA algorithm -----------------------------

% loop on the number of traces
while((i <= ceil) && (minStability <= threshold))
    
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
    
    % for each sample, compute sumYi & sumYi^2 & stdYi
    for l = 1 : (xMax16-xMin16)
        
        % compute sumYi & sumYi^2
        sumYiR16(1,l) = sumYiR16(1,l) + vectorTrace(1,xMin16+l);
        sumYi2R16(1,l) = sumYi2R16(1,l) + vectorTrace(1,xMin16+l)^2;
        % compute stdYi
        stdYiR16(1,l) = sqrt( i * sumYi2R16(1,l) - sumYiR16(1,l)^2 );
        
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
            
            % compute buffers for CPA
            sumXiR16(k+1,j) = sumXiR16(k+1,j) + sum(IV2,2);
            sumXi2R16(k+1,j) = sumXi2R16(k+1,j) + sum(IV2,2)^2;
            stdXi = sqrt( i * sumXi2R16(k+1,j) - sumXiR16(k+1,j)^2 );
            
            % for each sample, compute buffers for CPA & compute sampling correlation 
            for l = 1 : (xMax16-xMin16)
                
                % compute sumXiYi
                sumXiYiR16(k+1,j,l) = sumXiYiR16(k+1,j,l) + sum(IV2,2) * vectorTrace(1,xMin16+l);
                
                % compute the covariance between Xi & Yi
                covXiYi = i * sumXiYiR16(k+1,j,l) - sumXiR16(k+1,j) * sumYiR16(1,l);
                
                % compute the product of standard deviations of Xi
                stdXistdYi = stdXi * (stdYiR16(1,l) + epsilon);
                
                % compute the correlation
                if(stdXistdYi == 0)
                    matrixCPA(k+1,l) = 0;
                else
                    matrixCPA(k+1,l) = covXiYi / stdXistdYi;
                end;

            end;
            
        % end loop on the number of key hypothesis
        end;
        
        % -----------------------------------------------------------------
        % -------------------- keySearch on round 16 ----------------------
        
        % apply keySearch when iteration is greater than floor & when mod(i,step) == 0
        if( (i >= floor) && (mod(i,step) == 0) )
            
            % call function keySearch()
            % launch keySearch for each selection function
            [keyGuess,margin,abscissa] = keySearch(method,1,xMax16-xMin16,'cpa');
            matrixResults(i,j,:) = [keyGuess,margin,abscissa];
            
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
    
        % call function keyBrokenChecker()
        % CPA - round 16
        stability(i,1) = (stability(i-1,1) + 1) * keyBrokenChecker(matrixRoundKey(16,:),matrixResults(i,:,1));
        if(stability(i,1) == (threshold+1))
            output = sprintf('---> CPA at round16 : key found in %d traces',i);
            disp(output);
        end;
        
    end;
    
    % get the minimum value of stability
    minStability = min(stability(i,1));
    
% end loop on ceil
end;

toc;

% call function writeResults()
% write results in files
writeResults(inputOrder,model,method,ceil,L,xMin16,xMax16,folderDest,matrixRoundKey(16,:),matrixResults(:,:,1),stability(:,1),16,'cpa',floor,threshold);

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

y = 'CPA done !';

% clean workspace
clear all;
