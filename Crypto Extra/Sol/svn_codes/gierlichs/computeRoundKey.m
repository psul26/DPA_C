%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%--------------------------------------------------------------------------
% compute roundKey
%
% Part of this code is based on Victor Lomn�'s code (SVN revision 65)
%
%--------------------------------------------------------------------------
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function matrixRoundKey = computeRoundKey(key)

% permutation choice 1
PC_1 = [ 57, 49, 41, 33, 25, 17,  9,  1, ...
         58, 50, 42, 34, 26, 18, 10,  2, ...
         59, 51, 43, 35, 27, 19, 11,  3, ...
         60, 52, 44, 36, 63, 55, 47, 39, ...
         31, 23, 15,  7, 62, 54, 46, 38, ...
         30, 22, 14,  6, 61, 53, 45, 37, ...
         29, 21, 13,  5, 28, 20, 12,  4];

% permutation choice 2
PC_2 = [ 14, 17, 11, 24,  1,  5,  3, 28, ...
         15,  6, 21, 10, 23, 19, 12,  4, ...
         26,  8, 16,  7, 27, 20, 13,  2, ...
         41, 52, 31, 37, 47, 55, 30, 40, ...
         51, 45, 33, 48, 44, 49, 39, 56, ...
         34, 53, 46, 42, 50, 36, 29, 32];

% shift key
Shift = [1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1];

keyBin = zeros(1,64);
keyAfterPC_1 = zeros(1,56);
CN = zeros(1,28);
DN = zeros(1,28);
CNN = zeros(1,28);
DNN = zeros(1,28);
keyBeforePC_2 = zeros(1,56);
roundKeyBin = zeros(1,48);
matrixRoundKey = zeros(16,8);

% convert DES key in binary
for i = 1 : 8
    for j = 0 : 7
        keyBin(1,8*(i-1)+j+1) = bitand(1,bitshift(key(1,i),j-7));
    end;
end;

% apply Permutation Choice 1
for i = 1 : 56
    keyAfterPC_1(1,i) = keyBin(PC_1(1,i));
end;

% fill C0 & D0
for i = 1 : 28
    CN(1,i) = keyAfterPC_1(1,i);
    DN(1,i) = keyAfterPC_1(1,i+28);
end;

% loop on the number of rounds
for i = 1 : 16

    % apply left shift(s)
    if(Shift(1,i) == 1)
        for j = 1 : 27
            CNN(1,j) = CN(1,j+Shift(1,i));
            DNN(1,j) = DN(1,j+Shift(1,i));
        end;
        CNN(1,28) = CN(1,1);
        DNN(1,28) = DN(1,1);
    else
        for j = 1 : 26
            CNN(1,j) = CN(1,j+Shift(1,i));
            DNN(1,j) = DN(1,j+Shift(1,i));
        end;
        CNN(1,28) = CN(1,2);
        DNN(1,28) = DN(1,2);
        CNN(1,27) = CN(1,1);
        DNN(1,27) = DN(1,1);
    end;

    % concatenation of C1 & D1
    for j = 1 : 28
        keyBeforePC_2(1,j)    = CNN(1,j);
        keyBeforePC_2(1,j+28) = DNN(1,j);
    end;

    % apply Permutation Choice 2
    for j = 1 : 48
        roundKeyBin(1,j) = keyBeforePC_2(1,PC_2(1,j));
    end;

    % compute roundKey in decimal
    for j = 1 : 8
        for k = 1 : 6
            matrixRoundKey(i,j) = matrixRoundKey(i,j) + roundKeyBin(1,6*(j-1) + k) * 2^(6-k);
        end;
    end;
    
    CN = CNN;
    DN = DNN;
    
 % end loop rounds
end;
