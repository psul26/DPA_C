% BS-CPA for random order
%
% by Hideo Shimizu
% Copyright(C) 2009 by Toshiba Corporation

function [successcount] = bcpa1(TRACEDIR, FILELIST, FLOG, VERBOSE, RATIO, SBOXORDER, FIRST, LAST, SIDE, SUBKEY, IterationThreshold, StabilityThreshold, IterationLimit)

% DES tables
IP = [
    58 50 42 34 26 18 10 2 ...
    60 52 44 36 28 20 12 4 ...
    62 54 46 38 30 22 14 6 ...
    64 56 48 40 32 24 16 8 ...
    57 49 41 33 25 17  9 1 ...
    59 51 43 35 27 19 11 3 ...
    61 53 45 37 29 21 13 5 ...
    63 55 47 39 31 23 15 7];
E = [
    32  1  2  3  4  5 ...
     4  5  6  7  8  9 ...
     8  9 10 11 12 13 ...
    12 13 14 15 16 17 ...
    16 17 18 19 20 21 ...
    20 21 22 23 24 25 ...
    24 25 26 27 28 29 ...
    28 29 30 31 32 1];
SBOX = [
    14  0  4 15 13  7  1  4  2 14 15  2 11 13  8  1  3 10 10  6  6 12 12 11  5  9  9  5  0  3  7  8  4 15  1 12 14  8  8  2 13  4  6  9  2  1 11  7 15  5 12 11  9  3  7 14  3 10 10  0  5  6  0 13 ;
    15  3  1 13  8  4 14  7  6 15 11  2  3  8  4 14  9 12  7  0  2  1 13 10 12  6  0  9  5 11 10  5  0 13 14  8  7 10 11  1 10  3  4 15 13  4  1  2  5 11  8  6 12  7  6 12  9  0  3  5  2 14 15  9 ;
    10 13  0  7  9  0 14  9  6  3  3  4 15  6  5 10  1  2 13  8 12  5  7 14 11 12  4 11  2 15  8  1 13  1  6 10  4 13  9  0  8  6 15  9  3  8  0  7 11  4  1 15  2 14 12  3  5 11 10  5 14  2  7 12 ;
     7 13 13  8 14 11  3  5  0  6  6 15  9  0 10  3  1  4  2  7  8  2  5 12 11  1 12 10  4 14 15  9 10  3  6 15  9  0  0  6 12 10 11  1  7 13 13  8 15  9  1  4  3  5 14 11  5 12  2  7  8  2  4 14 ;
     2 14 12 11  4  2  1 12  7  4 10  7 11 13  6  1  8  5  5  0  3 15 15 10 13  3  0  9 14  8  9  6  4 11  2  8  1 12 11  7 10  1 13 14  7  2  8 13 15  6  9 15 12  0  5  9  6 10  3  4  0  5 14  3 ;
    12 10  1 15 10  4 15  2  9  7  2 12  6  9  8  5  0  6 13  1  3 13  4 14 14  0  7 11  5  3 11  8  9  4 14  3 15  2  5 12  2  9  8  5 12 15  3 10  7 11  0 14  4  1 10  7  1  6 13  0 11  8  6 13 ;
     4 13 11  0  2 11 14  7 15  4  0  9  8  1 13 10  3 14 12  3  9  5  7 12  5  2 10 15  6  8  1  6  1  6  4 11 11 13 13  8 12  1  3  4  7 10 14  7 10  9 15  5  6  0  8 15  0 14  5  2  9  3  2 12 ;
    13  1  2 15  8 13  4  8  6 10 15  3 11  7  1  4 10 12  9  5  3  6 14 11  5  0  0 14 12  9  7  2  7  2 11  1  4 14  1  7  9  4 12 10 14  8  2 13  0 15  6 12 10  9 13  0 15  3  3  5  5  6  8 11
];
PINV = [
     9 17 23 31 ...
    13 28  2 18 ...
    24 16 30  6 ...
    26 20 10  1 ...
     8 14 25  3 ...
     4 29 11 19 ...
    32 12 22  7 ...
     5 27 15 21];

% constant
NS = 8;    % number of sbox
NKEY = 64; % number of key candidate
h = [0 1 1 2 1 2 2 3 1 2 2 3 2 3 3 4]; % hamming weight

% adjust length
pos = FIRST;
len = LAST - FIRST + 1;
len0 = floor(len / RATIO);
len = len0 * RATIO;

% arrays
w = zeros(IterationLimit, len0);
cc = zeros(64, len0);  % correlation coefficient
maxkey = zeros(1, NS);
ref = zeros(NKEY, IterationLimit, NS);
cipher = zeros(1, 64);

% opening message
if VERBOSE
    fprintf(1, '# Stability threshold: %d\n', StabilityThreshold);
    fprintf(1, '# IteRATIOn threshold: %d\n', IterationThreshold);
end
fprintf(FLOG, '# Stability threshold: %d\n', StabilityThreshold);
fprintf(FLOG, '# IteRATIOn threshold: %d\n', IterationThreshold);

CPOS = findstr(FILELIST(1,:), [SIDE '=']) + 2;
count = 0;

for wave=1:IterationLimit
    % read trace
    filename = [TRACEDIR '/' FILELIST(wave,:)];
    fp = fopen(filename, 'r', 'ieee-le'); % ieee little endian format
    if fp < 0, fprintf(2, '%d %s : open error\n', wave, filename); return; end
    fseek(fp, 164 + 4 * pos, 'bof'); % 164 : file header size
    t = fread(fp, [1 len], 'float');
    fclose(fp);
    if RATIO == 1
        w(wave,:) = t;
    else
        w(wave,:) = mean(reshape(t, RATIO, len0)); % integration
    end

    % calculate reference
    filename = FILELIST(wave,:);
    for i=1:16, cipher(i*4-3:i*4) = bitget(hex2dec(filename(CPOS + i - 1)), [4 3 2 1]); end
    x0 = cipher(IP);  % x0 = calculate IP
    l = x0(1:32);     % l = left part of x0
    r = x0(33:64);    % r = right part of x0
    t = bitxor(l, r);
    lr = t(PINV);     % lr = invP(L ^ R)
    e0 = r(E);        % e0 = calculate E
    for s=1:NS
        si = e0(s*6-5:s*6) * [32 16 8 4 2 1]'; % si = sbox input
        so = SBOX(s, bitxor(si, 0:63) + 1);    % so = sbox output
        lr0 = lr(s*4-3:s*4) * [8 4 2 1]';
        ref(:,wave,s) = h(bitxor(so, lr0) + 1);
    end

    % save computing time
    if wave < IterationThreshold
        if VERBOSE, fprintf(1, '%d\n', wave); end
        fprintf(FLOG, '%d\n', wave);
        continue;
    end
    
    r = zeros(1, wave); % build-up buffer
    for s=SBOXORDER
        % calculate correlation
        for point=1:len0,
            for candidate=1:NKEY
                t = corrcoef(w(1:wave, point), r + ref(candidate, 1:wave, s));
                cc(candidate, point) = t(1, 2);
            end
        end
        % search peak
        [t k] = max(max(abs(cc), [], 2));
        maxkey(s) = k(1) - 1;
        r = r + ref(k(1), 1:wave, s);
    end
    % Are all SUBKEYs correct?
    if sum(maxkey == SUBKEY) == NS
        count = count + 1;
    else
        count = 0;
    end
    
    % output log
    if VERBOSE
        fprintf(1, '%d\t%d', wave, count);
        fprintf(1, '\t%d', maxkey);
        fprintf(1, '\n');
    end
    fprintf(FLOG, '%d\t%d', wave, count);
    fprintf(FLOG, '\t%d', maxkey);
    fprintf(FLOG, '\n');

    if count == StabilityThreshold, break; end
end

successcount = wave;
