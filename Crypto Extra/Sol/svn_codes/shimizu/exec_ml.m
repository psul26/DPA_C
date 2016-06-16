% Alleged maximum likelihood method
%
% by Hideo Shimizu
% Copyright(C) 2009 by Toshiba Corporation

tic;

rand('twister', 923475); % random seed
N = 1000;   % Trace Limit
NT = 100;   % Number of Attacks
RATIO = 20; % Number of point to sum
SUBKEY = [60 11 56 46 22 50 16 44];    % correct answer
TRACEDIR = '../secmatv1_2006_04_0809'; % <--- modify this
LOGFILE = 'ml-output.txt'; % log file

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

H = [0 1 1 2 1 2 2 3 1 2 2 3 2 3 3 4];

% 14480
M1 = [
    7.010769e-002 7.117416e-002 7.216878e-002 7.320065e-002 7.415702e-002 % s1
    7.057447e-002 7.138699e-002 7.218081e-002 7.293804e-002 7.372408e-002 % s2
    7.060101e-002 7.135674e-002 7.216873e-002 7.297261e-002 7.375509e-002 % s3
    7.041803e-002 7.126491e-002 7.217497e-002 7.309443e-002 7.390966e-002 % s4
    7.054353e-002 7.132988e-002 7.217051e-002 7.301503e-002 7.384701e-002 % s5
    7.001085e-002 7.111619e-002 7.218177e-002 7.324947e-002 7.429636e-002 % s6
    7.047989e-002 7.134482e-002 7.218165e-002 7.300594e-002 7.381891e-002 % s7
    7.001828e-002 7.110815e-002 7.218351e-002 7.322742e-002 7.425245e-002 % s8
];
S1 = [
    2.923727e-003 2.904975e-003 2.838266e-003 2.773971e-003 2.749651e-003 % s1
    2.992673e-003 2.961079e-003 2.907682e-003 2.871706e-003 2.828148e-003 % s2
    2.974357e-003 2.939905e-003 2.926848e-003 2.851387e-003 2.822670e-003 % s3
    2.952398e-003 2.906428e-003 2.906417e-003 2.830207e-003 2.741443e-003 % s4
    3.018631e-003 2.940742e-003 2.901035e-003 2.845046e-003 2.773315e-003 % s5
    2.891967e-003 2.892837e-003 2.806930e-003 2.786856e-003 2.629893e-003 % s6
    2.997657e-003 2.957943e-003 2.880466e-003 2.873406e-003 2.769412e-003 % s7
    2.871577e-003 2.873290e-003 2.832778e-003 2.783003e-003 2.695624e-003 % s8
];

% 15220
M2 = [
    3.632143e-002 3.688715e-002 3.740501e-002 3.793007e-002 3.840520e-002 % s1
    3.638248e-002 3.688501e-002 3.739931e-002 3.790678e-002 3.842197e-002 % s2
    3.634172e-002 3.686712e-002 3.740745e-002 3.792356e-002 3.842073e-002 % s3
    3.642292e-002 3.687773e-002 3.740256e-002 3.793081e-002 3.837568e-002 % s4
    3.637378e-002 3.688942e-002 3.739669e-002 3.792126e-002 3.844130e-002 % s5
    3.634887e-002 3.690756e-002 3.739798e-002 3.791323e-002 3.844440e-002 % s6
    3.640278e-002 3.692244e-002 3.741047e-002 3.788409e-002 3.834338e-002 % s7
    3.634473e-002 3.689673e-002 3.740466e-002 3.790305e-002 3.842224e-002 % s8
];
S2 = [
    1.840846e-003 1.835475e-003 1.804278e-003 1.810131e-003 1.823832e-003 % s1
    1.831398e-003 1.819853e-003 1.821607e-003 1.820729e-003 1.806075e-003 % s2
    1.839705e-003 1.818372e-003 1.821245e-003 1.803058e-003 1.812769e-003 % s3
    1.839981e-003 1.812156e-003 1.834203e-003 1.815786e-003 1.789666e-003 % s4
    1.823187e-003 1.814868e-003 1.827649e-003 1.812073e-003 1.798153e-003 % s5
    1.808446e-003 1.837997e-003 1.809559e-003 1.820118e-003 1.812601e-003 % s6
    1.852999e-003 1.847074e-003 1.819239e-003 1.822838e-003 1.806526e-003 % s7
    1.818056e-003 1.825171e-003 1.824161e-003 1.813614e-003 1.816533e-003 % s8
];

% 15740
M3 = [
    6.896119e-002 7.043422e-002 7.178992e-002 7.319996e-002 7.444851e-002 % s1
    6.897579e-002 7.036300e-002 7.180856e-002 7.317064e-002 7.456787e-002 % s2
    6.916395e-002 7.040552e-002 7.179399e-002 7.314577e-002 7.440858e-002 % s3
    6.903859e-002 7.032668e-002 7.179952e-002 7.326026e-002 7.456899e-002 % s4
    6.917796e-002 7.052324e-002 7.178915e-002 7.309215e-002 7.434272e-002 % s5
    6.903864e-002 7.044288e-002 7.178962e-002 7.317186e-002 7.455369e-002 % s6
    6.919540e-002 7.055919e-002 7.181787e-002 7.301768e-002 7.429959e-002 % s7
    6.907279e-002 7.049032e-002 7.176761e-002 7.313288e-002 7.443246e-002 % s8
];
S3 = [
    4.501918e-003 4.407595e-003 4.257149e-003 4.117787e-003 3.993606e-003 % s1
    4.507895e-003 4.408534e-003 4.255748e-003 4.112848e-003 3.912869e-003 % s2
    4.538123e-003 4.408414e-003 4.309826e-003 4.089057e-003 3.957725e-003 % s3
    4.534869e-003 4.399684e-003 4.255074e-003 4.097764e-003 3.843991e-003 % s4
    4.641842e-003 4.445576e-003 4.302162e-003 4.099730e-003 3.918691e-003 % s5
    4.561816e-003 4.423820e-003 4.249398e-003 4.124250e-003 3.899697e-003 % s6
    4.578177e-003 4.391329e-003 4.279692e-003 4.245811e-003 3.995181e-003 % s7
    4.460688e-003 4.416969e-003 4.310403e-003 4.130619e-003 3.908393e-003 % s8
];

CPOS = 73;
NS = 8;
NKEY = 64;

% read db-order.txt
fp = fopen('db-order.txt', 'r');
if fp < 0, fprintf(2, 'db-order.txt not found\n'); return; end
db = fscanf(fp, '%s', 1);
FSIZE = length(db);
frewind(fp);
db = fscanf(fp, '%s');
fclose(fp);
NFILE = length(db) / FSIZE;
db = reshape(db, FSIZE, NFILE)';
WMAX = length(db);

% open log file
flog = fopen(LOGFILE, 'w');
if flog < 0, fprintf(2, '%s : open error\n', LOGFILE); return; end

% arrays
countlog = zeros(1, NT);
minkey = zeros(1, NS);
klog = zeros(N, NS);
scount = zeros(NT, NS);

% start
for trycount=1:NT

    fprintf(flog, '----- attack %d -----\n', trycount);
    
    c = zeros(1, 64);
    d1 = zeros(NS, NKEY);
    d2 = zeros(NS, NKEY);
    d3 = zeros(NS, NKEY);
    count = 0;

    for wave=1:N
        wn = floor(WMAX * rand);
        filename = db(wn + 1,:);

        % read trace
        fp = fopen([TRACEDIR '/' filename], 'r', 'ieee-le');
        if fp < 0, fprintf(2, '%s : open error\n', filename); return; end
        fseek(fp, 164 + 4 * 14480, 'bof');
        w1 = mean(fread(fp, [1 RATIO], 'float'));
        fseek(fp, 164 + 4 * 15220, 'bof');
        w2 = mean(fread(fp, [1 RATIO], 'float'));
        fseek(fp, 164 + 4 * 15740, 'bof');
        w3 = mean(fread(fp, [1 RATIO], 'float'));
        fclose(fp);

        % make reference
        for i=1:16, c(i*4-3:i*4) = bitget(hex2dec(filename(CPOS + i)), [4 3 2 1]); end
        c = c(IP);
        t = bitxor(c(1:32), c(33:64));
        lr = t(PINV);
        e0 = c(32 + E);

        for s=1:8
            so = SBOX(s, bitxor(e0(s*6-5:s*6) * [32 16 8 4 2 1]', 0:63) + 1);
            ref = H(bitxor(so, lr(s*4-3:s*4) * [8 4 2 1]') + 1);

            % calculate liklihood function
            t = (w1 - M1(s, ref+1)) ./ S1(s, ref+1);
            d1(s,:) = d1(s,:) + t .^ 2;
            t = (w2 - M2(s, ref+1)) ./ S2(s, ref+1);
            d2(s,:) = d2(s,:) + t .^ 2;
            t = (w3 - M3(s, ref+1)) ./ S3(s, ref+1);
            d3(s,:) = d3(s,:) + t .^ 2;
            
            [t minkey(s)] = min(d1(s,:).^2 + d2(s,:).^2 + d3(s,:).^2);
        end
        minkey = minkey - 1;
        klog(wave,:) = minkey;

        if sum(SUBKEY == minkey) == 8
            count = count + 1;
        else
            count = 0;
        end
        fprintf(flog, '%d\t%d', wave, count);
        fprintf(flog, '\t%d', minkey);
        fprintf(flog, '\n');

        if count >= 100, break; end

    end

    for s=1:8
        scount(trycount, s) = max(find(klog(1:wave,s) ~= SUBKEY(s))) + 100;
    end
    countlog(trycount) = wave;
    fprintf(1, 'attack %d\t%d traces', trycount, wave);
    fprintf(1, '\t%d', scount(trycount,:));
    fprintf(1, '\n');

    fprintf(flog, '--- attack %d result ---\n', trycount);
    fprintf(flog, '%d traces are needed.\n', wave);
    fprintf(flog, 'S1-S8 results');
    fprintf(flog, '\t%d', scount(trycount,:));
    fprintf(flog, '\n\n\n');

end
avg = mean(countlog);
fprintf(1, 'average %f\n', avg);

fprintf(flog, '--- average %f traces ---\n', avg);
fprintf(flog, '--- summary (No., #all, #S1, ..., #S8) ---\n');
for i=1:NT
    fprintf(flog, '%d\t%d', i, countlog(i));
    fprintf(flog, '\t%d', scount(i,:));
    fprintf(flog, '\n');
end
fprintf(flog, '--- end of file ---\n');

fclose(flog);

toc;