function IV2 = computeIV2DES(model,sbox,CTO,keyHyp)
% use : computeIV2DES(model,sbox,CTO,keyHyp)
%
% Author : Victor LOMNE - victor.lomne@lirmm.fr
%
% model : power consumption model
%         'HW'   -> Hamming Weight
%         'HD'   -> Hamming Distance
%
% sbox : number of attacked sbox
%
% CTO : array 1x8 containing in decimal Ciphertext Output
%
% keyHyp : value between 0 & 63 corresponding to the key Hypothesis


% initial permutation
IP = [ 58, 50, 42, 34, 26, 18, 10,  2, ...
       60, 52, 44, 36, 28, 20, 12,  4, ...
       62, 54, 46, 38, 30, 22, 14,  6, ...
       64, 56, 48, 40, 32, 24, 16,  8, ...
       57, 49, 41, 33, 25, 17,  9,  1, ...
       59, 51, 43, 35, 27, 19, 11,  3, ...
       61, 53, 45, 37, 29, 21, 13,  5, ...
       63, 55, 47, 39, 31, 23, 15,  7 ];

% expansion function
Expansion = [ 32,  1,  2,  3,  4,  5, ...
               4,  5,  6,  7,  8,  9, ...
               8,  9, 10, 11, 12, 13, ...
              12, 13, 14, 15, 16, 17, ...
              16, 17, 18, 19, 20, 21, ...
              20, 21, 22, 23, 24, 25, ...
              24, 25, 26, 27, 28, 29, ...
              28, 29, 30, 31, 32,  1 ];

% the (in)famous DES sboxes
sboxDES1 = [14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7;
             0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8;
             4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0;
            15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13 ];

sboxDES2 = [15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10;
             3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5;
             0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15;
            13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9 ];

sboxDES3 = [10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8;
            13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1;
            13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7;
             1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12 ];

sboxDES4 = [ 7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15;
            13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9;
            10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4;
             3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14 ];

sboxDES5 = [ 2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9;
            14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6;
             4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14;
            11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3 ];

sboxDES6 = [12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11;
            10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8;
             9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6;
             4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13 ];

sboxDES7 = [ 4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1;
            13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6;
             1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2;
             6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12 ];

sboxDES8 = [13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7;
             1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2;
             7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8;
             2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11 ];

ciphertextBin = zeros(1,64);
ciphertextBeforeIPinv = zeros(1,64);
L16 = zeros(1,32);
R16 = zeros(1,32);
inputAfterExpansion = zeros(1,48);
keyHypBin = zeros(1,6);
%xored = zeros(1,6);
sboxOutputBin = zeros(1,4);
L15 = zeros(1,4);

% convert plaintext in binary
for i = 1 : 8
    for j = 0 : 7
        ciphertextBin(1,8*(i-1)+j+1) = bitand(1,bitshift(CTO(1,i),j-7));
    end;
end;

% apply Initial Permutation
for i = 1 : 64
    ciphertextBeforeIPinv(1,i) = ciphertextBin(IP(1,i));
end;

% get L0 & R0
for i = 1 : 32
    R16(1,i) = ciphertextBeforeIPinv(1,i);
    L16(1,i) = ciphertextBeforeIPinv(1,i+32);
end;

% apply Expansion
for i = 1 : 48
    inputAfterExpansion(1,i) = L16(Expansion(1,i));
end;

% convert keyHyp in binary
for i = 0 : 5
    keyHypBin(1,i+1) = bitand(1,bitshift(keyHyp,i-5));
end;

% xor between data & roundKey
xored = xor(inputAfterExpansion(1+6*(sbox-1):6+6*(sbox-1)),keyHypBin);

% compute line & column for sbox submit
line = 2 * xored(1) + xored(6) + 1;
column = 8 * xored(2) + 4 * xored(3) + 2 * xored(4) + xored(5) + 1;

% sbox submit
switch(sbox)
    case 1
        sboxOutput = sboxDES1(line,column);
    case 2
        sboxOutput = sboxDES2(line,column);
    case 3
        sboxOutput = sboxDES3(line,column);
    case 4
        sboxOutput = sboxDES4(line,column);
    case 5
        sboxOutput = sboxDES5(line,column);
    case 6
        sboxOutput = sboxDES6(line,column);
    case 7
        sboxOutput = sboxDES7(line,column);
    case 8
        sboxOutput = sboxDES8(line,column);
end;

% convert sboxOutput in binary
for i = 0 : 3
    sboxOutputBin(1,i+1) = bitand(1,bitshift(sboxOutput,i-3));
end;

% apply permutation P and Xor with R16 to obtain L15
switch(sbox)
    case 1
        L15(1,4) = bitxor(sboxOutputBin(1,4),R16(1,31));
        L15(1,3) = bitxor(sboxOutputBin(1,3),R16(1,23));
        L15(1,2) = bitxor(sboxOutputBin(1,2),R16(1,17));
        L15(1,1) = bitxor(sboxOutputBin(1,1),R16(1,9));
    case 2
        L15(1,4) = bitxor(sboxOutputBin(1,4),R16(1,18));
        L15(1,3) = bitxor(sboxOutputBin(1,3),R16(1,2));
        L15(1,2) = bitxor(sboxOutputBin(1,2),R16(1,28));
        L15(1,1) = bitxor(sboxOutputBin(1,1),R16(1,13));
    case 3
        L15(1,4) = bitxor(sboxOutputBin(1,4),R16(1,6));
        L15(1,3) = bitxor(sboxOutputBin(1,3),R16(1,30));
        L15(1,2) = bitxor(sboxOutputBin(1,2),R16(1,16));
        L15(1,1) = bitxor(sboxOutputBin(1,1),R16(1,24));
    case 4
        L15(1,4) = bitxor(sboxOutputBin(1,4),R16(1,1));
        L15(1,3) = bitxor(sboxOutputBin(1,3),R16(1,10));
        L15(1,2) = bitxor(sboxOutputBin(1,2),R16(1,20));
        L15(1,1) = bitxor(sboxOutputBin(1,1),R16(1,26));
    case 5
        L15(1,4) = bitxor(sboxOutputBin(1,4),R16(1,3));
        L15(1,3) = bitxor(sboxOutputBin(1,3),R16(1,25));
        L15(1,2) = bitxor(sboxOutputBin(1,2),R16(1,14));
        L15(1,1) = bitxor(sboxOutputBin(1,1),R16(1,8));
    case 6
        L15(1,4) = bitxor(sboxOutputBin(1,4),R16(1,19));
        L15(1,3) = bitxor(sboxOutputBin(1,3),R16(1,11));
        L15(1,2) = bitxor(sboxOutputBin(1,2),R16(1,29));
        L15(1,1) = bitxor(sboxOutputBin(1,1),R16(1,4));
    case 7
        L15(1,4) = bitxor(sboxOutputBin(1,4),R16(1,7));
        L15(1,3) = bitxor(sboxOutputBin(1,3),R16(1,22));
        L15(1,2) = bitxor(sboxOutputBin(1,2),R16(1,12));
        L15(1,1) = bitxor(sboxOutputBin(1,1),R16(1,32));
    case 8
        L15(1,4) = bitxor(sboxOutputBin(1,4),R16(1,21));
        L15(1,3) = bitxor(sboxOutputBin(1,3),R16(1,15));
        L15(1,2) = bitxor(sboxOutputBin(1,2),R16(1,27));
        L15(1,1) = bitxor(sboxOutputBin(1,1),R16(1,5));

% end case
end;

% compute IV2 for HW model
if(strcmp(model,'HW'))
    
    IV2 = L15;
    
end;

% compute IV2 for HD model
if(strcmp(model,'HD'))

    % compute HD between L15 and L16
    switch(sbox)
        case 1
            IV2(1,4) = bitxor(L15(1,4),L16(1,31));
            IV2(1,3) = bitxor(L15(1,3),L16(1,23));
            IV2(1,2) = bitxor(L15(1,2),L16(1,17));
            IV2(1,1) = bitxor(L15(1,1),L16(1,9));
        case 2
            IV2(1,4) = bitxor(L15(1,4),L16(1,18));
            IV2(1,3) = bitxor(L15(1,3),L16(1,2));
            IV2(1,2) = bitxor(L15(1,2),L16(1,28));
            IV2(1,1) = bitxor(L15(1,1),L16(1,13));
        case 3
            IV2(1,4) = bitxor(L15(1,4),L16(1,6));
            IV2(1,3) = bitxor(L15(1,3),L16(1,30));
            IV2(1,2) = bitxor(L15(1,2),L16(1,16));
            IV2(1,1) = bitxor(L15(1,1),L16(1,24));
        case 4
            IV2(1,4) = bitxor(L15(1,4),L16(1,1));
            IV2(1,3) = bitxor(L15(1,3),L16(1,10));
            IV2(1,2) = bitxor(L15(1,2),L16(1,20));
            IV2(1,1) = bitxor(L15(1,1),L16(1,26));
        case 5
            IV2(1,4) = bitxor(L15(1,4),L16(1,3));
            IV2(1,3) = bitxor(L15(1,3),L16(1,25));
            IV2(1,2) = bitxor(L15(1,2),L16(1,14));
            IV2(1,1) = bitxor(L15(1,1),L16(1,8));
        case 6
            IV2(1,4) = bitxor(L15(1,4),L16(1,19));
            IV2(1,3) = bitxor(L15(1,3),L16(1,11));
            IV2(1,2) = bitxor(L15(1,2),L16(1,29));
            IV2(1,1) = bitxor(L15(1,1),L16(1,4));
        case 7
            IV2(1,4) = bitxor(L15(1,4),L16(1,7));
            IV2(1,3) = bitxor(L15(1,3),L16(1,22));
            IV2(1,2) = bitxor(L15(1,2),L16(1,12));
            IV2(1,1) = bitxor(L15(1,1),L16(1,32));
        case 8
            IV2(1,4) = bitxor(L15(1,4),L16(1,21));
            IV2(1,3) = bitxor(L15(1,3),L16(1,15));
            IV2(1,2) = bitxor(L15(1,2),L16(1,27));
            IV2(1,1) = bitxor(L15(1,1),L16(1,5));

    % end case
    end;
    
end;