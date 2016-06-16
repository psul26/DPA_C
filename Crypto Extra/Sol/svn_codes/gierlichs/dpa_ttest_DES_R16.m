%
% This script performs a DPA attack with a more sophisticated Difference of
% Means test (t-test, e.g. http://en.wikipedia.org/wiki/Student%27s_t-test)
%
% It requires input data in the matfolder in .mat format.
% 
% Part of this code is based on Victor Lomn�'s code (SVN revision 65) 
%
% This reference code is not optimized for efficiency.
%
% Last change: 23/01/2009
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


% define constants and load input data ------------------------------------

% where is this project's base folder?
folder = '/data/users/bgierlic/dpacontest/';
% where is the data in .mat format?
matfolder = fullfile(folder,'data2');

% load plaintexts, ciphertexts and the key
cd(matfolder);
load plaintexts.mat;
load ciphertexts.mat;
load key.mat

% what's the max number of power curves to use?
max_no_of_curves = 385;
% what's the min number of power curves to use?
min_no_of_curves = 270;
% how fine grained do we step from min to max?
stepsize = 1;

% read first trace
load trace1.mat;

% get size of traces
L = size(one_curve,2);

% which chunk of the curves do we analyze? 1:L means everything
data_start = 00001;
data_end = L;

% allocate space to store power curves
curves(1:max_no_of_curves,1:data_end - data_start + 1) = 0;

% read power traces
for i = 1 : max_no_of_curves
    filename = sprintf('trace%d.mat',i);
    load(filename);
    curves(i,:) = one_curve(data_start:data_end);
end
clear one_curve
cd(folder);

% initialize matrix to store results, the matrix will store the best
% candidates for all 8 subkeys in columns 2 to 9, column 1 holds the
% number of power curves that was used
matrixResults4(1:max_no_of_curves,1:9) = 0;
matrixResults4(1:max_no_of_curves,1) = 1:max_no_of_curves;

% end define constants and load input data --------------------------------


% trig the clock
t1 = cputime;

% pre-compute the correct roundKey
matrixRoundKey = computeRoundKey(key);


% -------------------------------------------------------------------------
% ---------------------- DPA attack on round 16 ---------------------------

disp(' ');
disp('Launching DPA on round 16 ...');
disp(' ');

tic;
    
% loop, how many traces to use for the attack
for no_of_curves = min_no_of_curves:stepsize:max_no_of_curves

    % loop over the 8 sboxes
    for i = 1 : 8

        output = sprintf('attacking Sbox%d using %d traces ...',i, no_of_curves);
        disp(output);
        
        % allocate memory for correlation matrix
        matrixDPA(1:64,1:data_end - data_start + 1) = 0;
        % loop on subkey hypothesis
        for k = 0 : 63

            %allocate memory
            IV2_4bit(1:no_of_curves) = 0;
            
            % loop over all traces used for this attack
            for j = 1 : no_of_curves

                % compute Intermediate value of round 16
                % HD for Hamming distance of L15 and L16
                % HW for Hamming weight of L15
                temp = computeIV2('HD',i,ciphertexts(j,:),k);
                IV2_4bit(j) = sum(temp(1:4));
                %IV2_3bit(j) = sum(temp(1:3));
                %IV2_2bit(j) = sum(temp(1:2));
                %IV2_1bit(j) = temp(1);
                
            % end loop over all traces used in this attack
            end
            
            % compute differential trace using 4 bits
            setA = find(IV2_4bit < 2);
            setB = find(IV2_4bit > 2);
            meanA = mean(curves(setA,:));
            varA = var(curves(setA,:));
            varA = varA ./ length(setA);  % precompute directly here to avoid
            meanB = mean(curves(setB,:)); % doing it repeatedly later during t-test
            varB = var(curves(setB,:));
            varB = varB ./ length(setB);  % dito
            matrixDPA(k+1,:) = (meanA - meanB) ./ sqrt(varA + varB);
            
            % extract highest/lowest peak for this key candidate
            maxDPA4bit(no_of_curves,k+1) = max(abs(matrixDPA(k+1,:)));
            

        end % loop on key hypotheses

        % extract best subkey guess for this sbox (and number of traces)
        % and store it in result matrix
        matrixResults4(no_of_curves,i+1) = find(maxDPA4bit(no_of_curves,:) == max(maxDPA4bit(no_of_curves,:))) -1;

    end % loop over the 8 sboxes
    
end % loop on number of traces to use

output = sprintf('time for computing all differentials');
disp(output);
toc;

% write result matrix to output file result.txt in ASCII
dlmwrite(fullfile(folder,'resultDPA4.txt'),matrixResults4,'delimiter','\t','newline','pc');

% stop the clock
t2 = cputime - t1;
output = sprintf('Total time for everything : %d',t2);
disp(' ');
disp(output);
disp(' ');
disp('---------------------------------------------------------------------');
disp('---------------------------------------------------------------------');
disp(' ');

clear
exit
