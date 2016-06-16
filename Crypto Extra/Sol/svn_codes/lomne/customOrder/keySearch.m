function [keyGuess,margin,abscissa] = keySearch(method,xMin,xMax,selection)
% use : keySearch(method,xMin,xMax,selection)
%
% Author : Victor LOMNE - victor.lomne@lirmm.fr
%
% method : 'maxPeak' / 'maxIntegral'
%
% xMin & xMax : values where searching maxPeak or maxIntegral on DPA traces
%
% selection : selection function

global matrixDPA0
global matrixDPA1
global matrixDPA2
global matrixDPA3
global matrixDPA4
global matrixDPAsum
global matrixDPAhw
global matrixCPA

matrixMax = zeros(64,1);
matrixPoint = zeros(64,1);

% attribute the matrixDPA following selection
switch(selection)
    case 'bit0'
        matrixDPA = matrixDPA0;
    case 'bit1'
        matrixDPA = matrixDPA1;
    case 'bit2'
        matrixDPA = matrixDPA2;
    case 'bit3'
        matrixDPA = matrixDPA3;
    case '4bits'
        matrixDPA = matrixDPA4;
    case 'sum'
        matrixDPA = matrixDPAsum;
    case 'hw'
        matrixDPA = matrixDPAhw;
    case 'cpa'
        matrixDPA = matrixCPA;
end;

% search the max of each DPA trace
for i = 1 : 64
    
    if(strcmp(method,'maxPeak') == 1)
        [matrixMax(i,1),I] = max(abs(matrixDPA(i,xMin:xMax)));
        matrixPoint(i,1) = I;
    end;
    if(strcmp(method,'maxIntegral') == 1)
        [matrixMax(i,1),I] = trapz(abs(matrixDPA(i,xMin:xMax)));
        matrixPoint(i,1) = I;
    end;
    
end;

% get max of all DPA traces max and its indice
[keyGuessValue,I] = max(matrixMax(:,1));
keyGuess = I-1;

% get second max of all DPA traces max and its indice
if(keyGuess == 0)
    keyGuessValue2 = max(matrixMax(2:64,1));
else
    if(keyGuess == 63)
        keyGuessValue2 = max(matrixMax(1:63,1)); 
    else
        keyGuessValue2 = max(matrixMax(1:keyGuess,1));
        keyGuessValue3 = max(matrixMax(keyGuess+2:64,1));   
        if(keyGuessValue2 < keyGuessValue3)
            keyGuessValue2 = keyGuessValue3;
        end;
    end;
end;

% compute margin between keyGuess and keyGuess2
margin = ceil(100 * ((keyGuessValue - keyGuessValue2) / keyGuessValue));

% compute abscissa of the greatest peak
abscissa = matrixPoint(keyGuess+1,1) + xMin;