function result = BScpaDES(fid,xMin,xMax,floor,ceil,threshold,epsilon)
% use : BScpaDES(fid,xMin,xMax,floor,ceil,threshold,epsilon)
%
% Author : Victor LOMNE - victor.lomne@lirmm.fr
%
% fid : file pointer for results
%
% xMin & xMax : temporal window to perform keySearch
%
% floor : iteration from where we begin keySearch
%
% ceil : iteration where we stop the DPA, even if key is not broken
%
% threshold : stability threshold to consider that key is broken
%
% epsilon : positive value for CPA enhancement (cf work of Than-Ha)

global matrixTraces
global matrixCTO

% -------------------------------------------------------------------------
% initialize parameters
% -------------------------------------------------------------------------

realRoundKey16 = [60,11,56,46,22,50,16,44];

nbSbox = 8;
nbKeyHyp = 64;

% allocate memory for matrixResults and other matrixes for cpa
sumXi = zeros(nbKeyHyp,nbSbox);
sumXi2 = zeros(nbKeyHyp,nbSbox);
sumYi = zeros(1,xMax-xMin);
sumYi2 = zeros(1,xMax-xMin);
stdYi = zeros(1,xMax-xMin);
sumXiYi = zeros(nbKeyHyp,xMax-xMin,nbSbox);
matrixDPA = zeros(nbKeyHyp,xMax-xMin);

% allocate memory
matrixMax = zeros(nbKeyHyp,1);
matrixPoint = zeros(nbKeyHyp,1);
matrixResults = zeros(ceil,nbSbox);
stability = zeros(ceil,nbSbox+1);

% initialise traceCounter
i = 0;

% recursive technique
sboxToCrack = [1,2,3,4,5,6,7,8];
sboxCracked = [];
subKeyCracked = [];
nbTracesSubKeyCracked = zeros(1,8);
reInitializeI = 0;
sboxStandBy = [];
quarantine = 0;

% -------------------------------------------------------------------------
% DPA algorithm
% -------------------------------------------------------------------------

disp(' ');
disp('---------------------------------------------------------------------');
output = sprintf('launching BS-CPA ...\n');
disp(output);

% loop on the number of traces
while(i <= ceil)
    
    % reinitialize column(s) corresponding to sbox(es) to crack for recursive processing
    if(i == 0)
        sumXi = zeros(nbKeyHyp,nbSbox);
        sumXi2 = zeros(nbKeyHyp,nbSbox);
        sumYi = zeros(1,xMax-xMin);
        sumYi2 = zeros(1,xMax-xMin);
        stdYi = zeros(1,xMax-xMin);
        sumXiYi = zeros(nbKeyHyp,xMax-xMin,nbSbox);
    end;
    
    % increment traceCounter
    i = i + 1;
    
    % for each sample, compute sumYi & sumYi^2 & stdYi
    for l = 1 : (xMax-xMin)
        % compute sumYi & sumYi^2
        sumYi(1,l) = sumYi(1,l) + matrixTraces(i,xMin+l);
        sumYi2(1,l) = sumYi2(1,l) + matrixTraces(i,xMin+l)^2;
        % compute stdYi
        stdYi(1,l) = sqrt( i * sumYi2(1,l) - sumYi(1,l)^2 );
    end;
    
    % ---------------------------------------------------------------------
    % loop over number of sboxes
    % ---------------------------------------------------------------------
    
    % loop on the number of sboxes
    for j = 1 : size(sboxToCrack,2)
        
        % case of build-up has given a bad roundkey : eliminate first wrong guess subkey
        if(quarantine == 1)
            if(size(sboxCracked,2) ~= 0)
                output1 = 'sbox(es)';
                output2 = '';
                for k = 1 : size(sboxStandBy,2)
                    output2 = sprintf('%s %d',output2,sboxStandBy(1,k));
                end;
                output3 = sprintf('%s%s in quarantine reintegrated in sboxes to crack !',output1,output2);
                disp(output3);
                sboxToCrack = cat(2,sboxToCrack,sboxStandBy);
                sboxToCrack = sort(sboxToCrack);
                quarantine = 0;
            end;
        end;
        
        % -------------------------------------------------------------
        % loop over number of key hypothesis
        % -------------------------------------------------------------
        
        % loop over the number of key hypothesis
        for k = 0 : (nbKeyHyp-1)
            
            % call function computeIV2DES()
            % compute Intermediate value
            IV = computeIV2DES('HD',sboxToCrack(j),matrixCTO(i,:),k);
            
            % build-up case if current sbox is not already cracked
            if(size(find(sboxCracked == sboxToCrack(j)),2) == 0)
                for l = 1 : size(sboxCracked,2)
                    IV2 = computeIV2DES('HD',sboxCracked(l),matrixCTO(i,:),subKeyCracked(l));
                    IV = cat(2,IV,IV2);
                end;
            end;
            
            % build-up case if current sbox is already cracked but not at first
            if( (size(find(sboxCracked == sboxToCrack(j)),2) ~= 0) && (sboxToCrack(j) ~= sboxCracked(1,1)) )
                for l = 1 : (find(sboxCracked == sboxToCrack(j)) - 1)
                    IV2 = computeIV2DES('HD',sboxCracked(l),matrixCTO(i,:),subKeyCracked(l));
                    IV = cat(2,IV,IV2);
                end;
            end;
            
            % apply cpa selection on allBits of IV
            
            % compute buffers for CPA
            sumXi(k+1,sboxToCrack(j)) = sumXi(k+1,sboxToCrack(j)) + sum(IV,2);
            sumXi2(k+1,sboxToCrack(j)) = sumXi2(k+1,sboxToCrack(j)) + sum(IV,2)^2;
            stdXi = sqrt( i * sumXi2(k+1,sboxToCrack(j)) - sumXi(k+1,sboxToCrack(j))^2 );
            
            % for each sample, compute buffers for CPA & compute sampling correlation 
            for l = 1 : (xMax-xMin)
                % compute sumXiYi
                sumXiYi(k+1,l,sboxToCrack(j)) = sumXiYi(k+1,l,sboxToCrack(j)) + sum(IV,2) * matrixTraces(i,xMin+l);
                % compute the covariance between Xi & Yi
                covXiYi = i * sumXiYi(k+1,l,sboxToCrack(j)) - sumXi(k+1,sboxToCrack(j)) * sumYi(1,l);
                % compute the product of standard deviations of Xi
                stdXistdYi = stdXi * (stdYi(1,l) + epsilon);
                % compute the correlation
                if(stdXistdYi == 0)
                    matrixDPA(k+1,l) = 0;
                else
                    matrixDPA(k+1,l) = covXiYi / stdXistdYi;
                end;
            end;        
            
        % end loop on the number of key hypothesis
        end;

        % -------------------------------------------------------------
        % keySearch
        % -------------------------------------------------------------

        % compute key search when iteration is greater than floor & when mod(i,step) == 0
        if(i >= floor)
            
            % loop on the number of key hypothesis
            for k = 1 : nbKeyHyp

                [matrixMax(k,1),I] = max(abs(matrixDPA(k,:)));
                matrixPoint(k,1) = I;

            end;
            
            % get max of all DPA traces max and its indice
            [keyGuessValue,I] = max(matrixMax(:,1));
            matrixResults(i,sboxToCrack(j)) = I-1;
            
        end;
        
        % check stability when iteration is greater than floor
        if(i >= floor)
            
            % fill stability for each subkey
            if(matrixResults(i,sboxToCrack(j)) == matrixResults(i-1,sboxToCrack(j)))
                stability(i,sboxToCrack(j)) = stability(i-1,sboxToCrack(j)) + 1;
            else
                stability(i,sboxToCrack(j)) = 0;
            end;
            
        end;
        
    % end loop on sboxes
    end;
    
    % ---------------------------------------------------------------------
    % check if each subkey is cracked
    % ---------------------------------------------------------------------
    
    % loop on the number of sboxes
    for j = 1 : size(sboxToCrack,2)

        % check only non cracked sboxes
        if(size(find(sboxCracked == sboxToCrack(j)),2) == 0)

            % if one subkey has a stability equal to threshold, consider it as cracked
            % and then apply build-up technique
            if(stability(i,sboxToCrack(j)) == threshold)

                % consider current subkey as cracked
                nbTracesSubKeyCracked(1,sboxToCrack(j)) = i;

                % update sboxCraked, subKeyCracked, and rewind i to 0
                sboxCracked = cat(2,sboxCracked,sboxToCrack(j));
                subKeyCracked = cat(2,subKeyCracked,matrixResults(i,sboxToCrack(j)));
                reInitializeI = 1;
                
                output = sprintf('sbox %d cracked in %d traces - subKey = %d !',sboxToCrack(j),nbTracesSubKeyCracked(1,sboxToCrack(j)),matrixResults(i,sboxToCrack(j)));
                disp(output);
                
            end;

        end;
        
    end;
    
    % ---------------------------------------------------------------------
    % check if roundkey is broken
    % ---------------------------------------------------------------------
        
    % increment stability if current roundkey16 is equal to last roundkey16
    for j = 2 : i
        if(matrixResults(j,:) == matrixResults(j-1,:))
            stability(j,nbSbox+1) = stability(j-1,nbSbox+1) + 1;
        else
            stability(j,nbSbox+1) = 0;
        end;
    end;

    % test if roundkey16 has a stability equal to threshold, then quit
    if(stability(i,nbSbox+1) == threshold)
        if(matrixResults(i,:) == realRoundKey16(1,:))
            i = ceil+2;
            reInitializeI = 0;
            output = sprintf('\nround key 16 fully cracked !!!\n');
            disp(output);
        else
            output = sprintf('\nstability threshold hold and bad round key 16 guess');
            disp(output);
            output = sprintf('bad first subKey %d (sbox %d) -> put it in quarantine and restart attack !\n',subKeyCracked(1,1),sboxCracked(1,1));
            disp(output);
            % first sbox cracked was wrong - put it on stand by
            sboxStandBy = cat(2,sboxStandBy,sboxCracked(1,1));
            quarantine = 1;
            % reinitialize sboxCraked, subKeyCracked and sboxToCrack without first wrong cracked sbox
            sboxToCrack = [];
            sboxCracked = [];
            subKeyCracked = [];
            nbTracesSubKeyCracked = zeros(1,nbSbox);
            for j = 1 : nbSbox
                if(size(find(sboxStandBy == j),2) == 0)
                    sboxToCrack = cat(2,sboxToCrack,j);
                end;
            end;
            % reinitialize matrixResults, stability and i
            matrixResults = zeros(ceil,nbSbox);
            stability = zeros(ceil,nbSbox+1);
            reInitializeI = 1;
        end;
    else
        if(i == ceil)
            if(matrixResults(i,:) == realRoundKey16(1,:))
                output = sprintf('\nceil hold and stability threshold not hold');
                disp(output);
                if(size(subKeyCracked,2) ~= 0)
                    output = sprintf('bad first subKey %d (sbox %d) -> put it in quarantine and restart attack !\n',subKeyCracked(1,1),sboxCracked(1,1));
                    disp(output);
                    % first sbox cracked was wrong - put it on stand by
                    sboxStandBy = cat(2,sboxStandBy,sboxCracked(1,1));
                    quarantine = 1;
                    % reinitialize sboxCraked, subKeyCracked and sboxToCrack without first wrong cracked sbox
                    sboxToCrack = [];
                    sboxCracked = [];
                    subKeyCracked = [];
                    nbTracesSubKeyCracked = zeros(1,nbSbox);
                    for j = 1 : nbSbox
                        if(size(find(sboxStandBy == j),2) == 0)
                            sboxToCrack = cat(2,sboxToCrack,j);
                        end;
                    end;
                    % reinitialize column of matrixResults and stability
                    matrixResults = zeros(ceil,nbSbox);
                    stability = zeros(ceil,nbSbox+1);
                    reInitializeI = 1;
                else
                    output = sprintf('no subKey cracked -> increase ceil and restart attack !\n');
                    disp(output);
                    sboxToCrack = [1,2,3,4,5,6,7,8];
                    sboxCracked = [];
                    subKeyCracked = [];
                    nbTracesSubKeyCracked = zeros(1,nbSbox);
                    sboxStandBy = [];
                    quarantine = 0;
                    matrixResults = zeros(ceil,nbSbox);
                    stability = zeros(ceil,nbSbox+1);
                    reInitializeI = 1;
                    ceil = ceil + threshold;
                end;
            else
                output = sprintf('\nceil hold and roundKey 16 not cracked');
                disp(output);
                if(size(subKeyCracked,2) ~= 0)
                    output = sprintf('bad first subKey %d (sbox %d) -> put it in quarantine and restart attack !\n',subKeyCracked(1,1),sboxCracked(1,1));
                    disp(output);
                    % first sbox cracked was wrong - put it on stand by
                    sboxStandBy = cat(2,sboxStandBy,sboxCracked(1,1));
                    quarantine = 1;
                    % reinitialize sboxCraked, subKeyCracked and sboxToCrack without first wrong cracked sbox
                    sboxToCrack = [];
                    sboxCracked = [];
                    subKeyCracked = [];
                    nbTracesSubKeyCracked = zeros(1,nbSbox);
                    for j = 1 : nbSbox
                        if(size(find(sboxStandBy == j),2) == 0)
                            sboxToCrack = cat(2,sboxToCrack,j);
                        end;
                    end;
                    % reinitialize column of matrixResults and stability
                    matrixResults = zeros(ceil,nbSbox);
                    stability = zeros(ceil,nbSbox+1);
                    reInitializeI = 1;
                else
                    output = sprintf('no subKey cracked -> increase ceil and restart attack !\n');
                    disp(output);
                    sboxToCrack = [1,2,3,4,5,6,7,8];
                    sboxCracked = [];
                    subKeyCracked = [];
                    nbTracesSubKeyCracked = zeros(1,nbSbox);
                    sboxStandBy = [];
                    quarantine = 0;
                    matrixResults = zeros(ceil,nbSbox);
                    stability = zeros(ceil,nbSbox+1);
                    reInitializeI = 1;
                    ceil = ceil + threshold;
                end;
            end;
        end;
    end;
    
    % if all subkeys give bad results, then increase ceil
    if(size(sboxStandBy,2) == 8)
        
        output = sprintf('\nceil hold and all subKeys are in quarantine');
        disp(output);
        output = sprintf('no subKey cracked -> increase ceil and restart attack !\n');
        disp(output);
        sboxToCrack = [1,2,3,4,5,6,7,8];
        sboxCracked = [];
        subKeyCracked = [];
        nbTracesSubKeyCracked = zeros(1,nbSbox);
        sboxStandBy = [];
        quarantine = 0;
        matrixResults = zeros(ceil,nbSbox);
        stability = zeros(ceil,nbSbox+1);
        reInitializeI = 1;
        ceil = ceil + threshold;
        
    end;
    
    % reinitialize i if a sbox has just been cracked
    if(reInitializeI == 1)
        i = 0;
        reInitializeI = 0;
    end;
    
% end loop on ceil
end;

% -------------------------------------------------------------------------
% end DPA algorithm
% -------------------------------------------------------------------------

result = writeResultsDPAcontest(fid,matrixResults,floor,ceil,threshold,sboxCracked,subKeyCracked,nbTracesSubKeyCracked);

disp(' ');
output = sprintf('roundKey 16 : %d %d %d %d %d %d %d %d\n',matrixResults(max(nbTracesSubKeyCracked),:));
disp(output);
disp('BS-CPA finished !');
disp('---------------------------------------------------------------------');