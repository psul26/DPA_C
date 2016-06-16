function check = writeResults(inputOrder,model,method,ceil,L,xMin,xMax,folderDest,vectorRoundKey,matrixResults,stability,round,selection,floor,threshold)
% use : writeResults(inputOrder,model,method,ceil,L,xMin,xMax,folderDest,vectorRoundKey,matrixResults,stability,round,selection,floor,threshold)
%
% Author : Victor LOMNE - victor.lomne@lirmm.fr
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
% ceil : iteration where we stop the DPA, even if key is not broken
%
% L : number of samples per trace
%
% xMin & xMax : values where searching maxPeak or maxIntegral on DPA traces
%
% folderDest : path of folder where results & pictures will be stored.
%
% vectorRoundKey : vector containing good roundKey
%
% matrixResults : matrix containing results
%
% stability : matrix containing stability values
%
% round : attacked round
%
% selection : selection function
%
% floor : iteration from where we begin keySearch
%
% threshold : stability threshold to consider that key is broken

% create filename
output = sprintf('results_%s_%s_%s_round%d_%s.txt',inputOrder,model,method,round,selection);

% open results file
fid = fopen(fullfile(folderDest,output),'w');

% write informations in fid
fprintf(fid,'DPA DES - %d traces of length %d points\n\npower consumption model : %s\nkeySearch method        : %s\nkeySearch interval      : %d to %d\nattacked round          : %d\nselection function      : %s\n',...
        ceil,L,model,method,xMin,xMax,round,selection);
fprintf(fid,'\n');

% write roundKey
fprintf(fid,'roundKey %d : %d %d %d %d %d %d %d %d\n',round,vectorRoundKey);
fprintf(fid,'\n');

% write header
fprintf(fid,'Iteration Stability Subkey0 ... Subkey7\n');

% loop on the number of traces
for i = 1 : ceil

    % increment until floor
    if(i < floor)

         % write iteration, stability & guess subkeys
        fprintf(fid,'%d\n',i);

    else

        % write success if stability > threshold
        if(stability(i,1) == (threshold+1))
            
            % write iteration, stability & guess subkeys
            fprintf(fid,'%d %d %d %d %d %d %d %d %d %d - key guess in %d iterations !!!\n',i,stability(i,1),matrixResults(i,:),i);
        
        else
            
            % check that results line is fullfilled
            if(sum(matrixResults(i,:)) ~= 0)
            
                % write iteration, stability & guess subkeys
                fprintf(fid,'%d %d %d %d %d %d %d %d %d %d\n',i,stability(i,1),matrixResults(i,:));
                
            end;
            
        end;

    end;

end;

% close file
fclose(fid);

check = 0;