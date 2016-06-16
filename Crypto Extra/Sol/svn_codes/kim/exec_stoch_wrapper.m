
%----------------------------------
%        Clear 
%----------------------------------
clear all

%----------------------------------
%        Set Up
%----------------------------------

% attack time index range
time_idx1 = 14450;
time_idx2 = 14550;

% measured power trace/hamming distance value folder
matfolder = './data';

% correct subkeys of the last round
roundkey = [60 11 56 46 22 50 16 44];

% number of interesting points
int_num = 50;

% number of trial number
try_num = 100;

% result output directory
trg_dir = sprintf('./output');

% result filename
output_filename = sprintf('%s/output_stochastic_rep.txt',trg_dir);

% number of traces for attack
num_trace_attack = 500;
%------------------------------------
%        Load Measured Traces
%------------------------------------
disp('Loading data.............');
str = sprintf('%s/measured_trace_10000.mat',matfolder);
load(str);

%-----------------------------------
%        Load IV 
%-----------------------------------
str = sprintf('%s/pred_trace_10000.mat',matfolder);
load(str);
pred_trace = pred_trace2;

%-------------------------------
%         Main
%-------------------------------
fid = fopen(output_filename,'w');
all_mtd = zeros(1,try_num);
for i=1:try_num
  % all power trace number vector
  trace_all = [1:10000];
  
  % first, define traces for attack from all power trace
  tmp_idx = randperm(length(trace_all));
  tmp_idx = tmp_idx(1:num_trace_attack);
  trace_N3 = tmp_idx;
  trace_all(tmp_idx) = [];

  % the rest of traces are used for profiling phase
  mid_idx = floor(length(trace_all)/2);
  trace_N1 = trace_all(1:mid_idx);
  trace_N2 = trace_all(mid_idx+1:end);


  [mtd] = func_stoch_mtd(trace_N1,trace_N2,trace_N3,int_num,measured_trace,pred_trace,time_idx1,time_idx2,fid);
  all_mtd(i) = mtd;

  fprintf(1,'Attacking trial #%d MTD : %d Average : %d\n',i,mtd,mean(all_mtd(1:i)));
  fprintf(fid,'trial number\t mtd with 100 stability\n');
  fprintf(fid,'%d\t %d\n',i,mtd);
end



