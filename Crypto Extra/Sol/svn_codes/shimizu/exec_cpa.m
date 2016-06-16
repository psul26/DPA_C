% cpa batch file for random order
%
% by Hideo Shimizu
% Copyright(C) 2009 by Toshiba Corporation

tic;

% random seed
rand('twister', 1234567);

% constant
TRACEDIR = '../secmatv1_2006_04_0809'; % modify this.
LOGFILE = 'cpa-output.txt';
VERBOSE = false; % true or false. only omit screen display
%SIDE = 'm';
SIDE = 'c';

if SIDE == 'm'
    FIRST = 5700;
    LAST  = 5800;
    RATIO = 20;
    SUBKEY = [56 11 59 38 0 13 25 55];
elseif SIDE == 'c'
    FIRST = 14400;
    LAST  = 14500;
    RATIO = 20;
    SUBKEY  = [60 11 56 46 22 50 16 44];
end

NumberOfTry = 10;
IterationThreshold = 100;
StabilityThreshold = 100;
IterationLimit = 1000;

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

% open log file
FLOG = fopen(LOGFILE, 'w');
if FLOG < 0, fprintf(2, '%s : open error\n', LOGFILE); return; end

countlog = zeros(1, NumberOfTry);

for trycount = 1:NumberOfTry
    % make random order
    ix = floor(NFILE * rand(1, IterationLimit)) + 1;
    FILELIST = db(ix,:);
    % exec cpa
    countlog(trycount) = cpa(TRACEDIR, FILELIST, FLOG, VERBOSE, RATIO, FIRST, LAST, SIDE, SUBKEY, IterationThreshold, StabilityThreshold, IterationLimit);

    fprintf(1,    '# %d\t%d\n', trycount, countlog(trycount));
    fprintf(FLOG, '# %d\t%d\n', trycount, countlog(trycount));
    if VERBOSE, fprintf(1, '################\n\n'); end
    fprintf(FLOG, '################\n\n');

end

fprintf(1,    '# average %f\n', mean(countlog));
fprintf(FLOG, '# average %f\n', mean(countlog));

% close log file
fclose(FLOG);

toc; % show elapse time
