function guess = keyBrokenChecker(vectorRoundKey,vectorResults)
% use : keyBrokenChecker(vectorRoundKey,vectorResults)
%
% Author : Victor LOMNE - victor.lomne@lirmm.fr
%
% vectorRoundKey : vector containing good roundKey
%
% vectorResults : vector containing results of the last iteration

% fill guess
if(vectorResults == vectorRoundKey)
    guess = 1;
else
    guess = 0;
end;