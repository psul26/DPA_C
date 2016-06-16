% build-up cpa
%
% by Hideo Shimizu
% Copyright(C) 2009 by Toshiba Corporation

function bcpa2(tracedir, filelist, reffile, logfile, ratio, first, last, sboxorder, key, IterationThreshold, StabilityThreshold, IterationLimit);

% adjust length
pos = first;
len = last - first + 1;
len0 = floor(len / ratio);
len = len0 * ratio;

% read trace
flst = fopen(filelist, 'r');
if flst < 0, fprintf(2, '%s : open error\n', filelist); return; end
w = zeros(IterationLimit, len0);
for wave=1:IterationLimit,
    t = fscanf(flst, '%s', 1);
    filename = sprintf('%s/%s', tracedir, t);
    fp = fopen(filename, 'r', 'ieee-le'); % ieee little endian format
    if fp < 0, fprintf(2, '%d %s : open error\n', wave, filename); return; end
    fseek(fp, 164 + 4 * pos, 'bof'); % 164 : file header size
    t = fread(fp, len, 'float');
    fclose(fp);
    if ratio == 1
        w(wave,:) = t';
    else
        w(wave,:) = mean(reshape(t, ratio, len0)); % integration
    end
end
fclose(flst);

% read reference
load(reffile);

flog = fopen(logfile, 'w');
if flog < 0, fprintf(2, '%s : open error\n', logfile); return; end

fprintf(1, 'Stability threshold: %d\n', StabilityThreshold);
fprintf(1, 'Iteration threshold: %d\n', IterationThreshold);
fprintf(flog, 'Stability threshold: %d\n', StabilityThreshold);
fprintf(flog, 'Iteration threshold: %d\n', IterationThreshold);

c = zeros(64, len0);  % correlation coefficient
keylog = zeros(1, 8);
count = 0;

for wave=1:IterationLimit,
    if wave < IterationThreshold
        fprintf(1, '%d\n', wave);
        fprintf(flog, '%d\n', wave);
        continue;
    end
    
    for s=1:8
        % get sbox own ancient list
        sord = sboxorder{s};
        sbox = sord(length(sord));
        % calculate built-up
        r = zeros(1, wave);
        for i=1:length(sord)-1
            r = r + ref(keylog(sord(i)) + 1, 1:wave, sord(i));
        end
        % calculate correlation
        for point=1:len0,
            for candidate=1:64,
                t = corrcoef(w(1:wave, point), r + ref(candidate, 1:wave, sbox));
                c(candidate, point) = t(1, 2);
            end
        end
        % search peak
        c(isnan(c)) = 0;
        [t k] = max(max(abs(c), [], 2));
        keylog(sbox) = k(1) - 1;
    end
    % Are all keys correct?
    if sum(keylog == key) == 8
        count = count + 1;
    else
        count = 0;
    end
    
    % output log
    fprintf(1, '%d\t%d', wave, count);
    fprintf(1, '\t%d', keylog);
    fprintf(1, '\n');

    fprintf(flog, '%d\t%d', wave, count);
    fprintf(flog, '\t%d', keylog);
    fprintf(flog, '\n');

    if count == StabilityThreshold, break; end
end
fclose(flog);
