% launchBSCPArepresentativeOrder.m
% script launching BS-CPA on representative order for the dpacontest
%
% Author : Victor LOMNE - victor.lomne@lirmm.fr

global matrixTraces
global matrixCTO

t1 = cputime;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% change "folderSrc" with the absolute path of your local folder where traces of secmatv1 are stored in binary
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
folderSrc = '~/measuresDPAcontest/secmatv1bin';

% parameters for attacks
nbAttacks = 100;
nbTraces = 500;
xMin = 14421;
xMax = 14520;
floor = 2;
ceil = 200;
threshold = 100;
epsilon = 1;

% open pointer on filelists.txt
fid = fopen('filelistRepresentativeOrder.txt','r');

% open pointer for results.txt
fid2 = fopen('resultsBSCPA_100randomAttacksBIS.txt','w');

% create matrixResults
matrixResults = zeros(nbAttacks,1);

% launch attack(s)
for i = 1 : nbAttacks
    
    % display
    output = sprintf('attack %d on %d',i,nbAttacks);
    disp(' ');
    disp(output);
    tic;
    
    % read traces
    readBinaryTraces(folderSrc,fid,nbTraces,xMin,xMax);
    
    % launch attack
    result = BScpaDES(fid2,0,xMax-xMin+1,floor,ceil,threshold,epsilon);
    matrixResults(i,1) = result;
    toc;
    
end;

% close pointer for filelists.txt
fclose(fid);

% display results
result = sum(matrixResults) / nbAttacks;
disp(' ');
output = sprintf('-----> average result : %03.3d',result);
disp(output);
fprintf(fid2,'\n\n%s\n\n',output);
disp(' ');
output = sprintf('global elapsed time : %04.4d',cputime-t1);
disp(output);
disp(' ');

% close pointer for results.txt
fclose(fid2);
