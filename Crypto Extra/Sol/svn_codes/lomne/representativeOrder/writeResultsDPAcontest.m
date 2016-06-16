function nbTracesToCrack = writeResultsDPAcontest(fid,matrixResults,floor,ceil,threshold,sboxCracked,subKeyCracked,nbTracesSubKeyCracked)
% use : writeResultsDPAcontest(fid,matrixResults,floor,ceil,threshold)

nbTracesToCrack = 0;
stability = zeros(ceil,1);

fprintf(fid,'\n');
fprintf(fid,'\n');
fprintf(fid,'order in which sboxes have been cracked :\n');
fprintf(fid,'%d %d %d %d %d %d %d %d\n',sboxCracked);
fprintf(fid,'value of each sbox cracked :\n');
fprintf(fid,'%d %d %d %d %d %d %d %d\n',subKeyCracked);
fprintf(fid,'number of traces required to crack each sbox :\n');
fprintf(fid,'%d %d %d %d %d %d %d %d\n',nbTracesSubKeyCracked);
fprintf(fid,'\n');
fprintf(fid,'iteration stability subKey0 ... subKey7\n');

% loop on the number of traces
for i = 1 : ceil
    
    % apply check stability when iteration is greater than floor
    if(i >= floor)
        
        if(matrixResults(i,:) == matrixResults(i-1,:))
            stability(i,1) = stability(i-1,1) + 1;
        else
            stability(i,1) = 0;
        end;
        
        if(stability(i,1) == threshold)
            nbTracesToCrack = i;
            output = sprintf('---> key guess in %d traces',i);
            disp(output);
            fprintf(fid,'%03.3d %03.3d %d %d %d %d %d %d %d %d - key guess in %d iterations !!!\n',i,stability(i,1),matrixResults(i,:),i);
            break;
        else
            fprintf(fid,'%03.3d %03.3d %d %d %d %d %d %d %d %d\n',i,stability(i,1),matrixResults(i,:));
        end;
        
    else
        
        fprintf(fid,'%d\n',i);
        
    end;
    
end;