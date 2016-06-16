
%----------------------------------
%        Clear 
%----------------------------------
clear all

%----------------------------------
%        Set Up
%----------------------------------

% traces for calculation of h*(t,x,k)
trace_N1_1 =  501;
trace_N1_2 = trace_N1_1+4000;

% traces to compute the covariance matrix C
trace_N2_1 = trace_N1_2+1;
trace_N2_2 = trace_N2_1+4000;

% traces to retrieve key (database order)
trace_N3_1 = 1;
trace_N3_2 = 500;

% attack time index range
time_idx1 = 14450;
time_idx2 = 14550;
% measured power trace/hamming distance value folder
matfolder = './data';

% correct subkeys of the last round
roundkey = [60 11 56 46 22 50 16 44];

% number of interesting points
int_num = 30;

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

%------------------------------------
%    Profiling Phase #1 
% calculation of h*(t,x,k)
%------------------------------------
disp('Calculating h*(t,x,k)........');
m1 = measured_trace(trace_N1_1:trace_N1_2,time_idx1:time_idx2);
for sbox=1:8
  key_idx = roundkey(sbox)+1;
  m2 = pred_trace(trace_N1_1:trace_N1_2,(sbox-1)*64+1:sbox*64);
  N1 = trace_N1_2-trace_N1_1+1;
  A = zeros(N1,5); %Using F5 subspace
  A(:,1) = ones(N1,1);
  for i=1:4
    A(:,i+1) = bitget(double(m2(:,key_idx)),i);
  end
  S = inv(A' * A)*A'; 
  B(:,:,sbox) = S * m1; 
end

% Calculate all-bit-long HD
sim_trace = zeros(size(m1,1),1);
for sbox=1:8
  m2 = pred_trace(trace_N1_1:trace_N1_2,64*(sbox-1)+1:64*sbox);
  x = m2(:,roundkey(sbox)+1);
  sim_trace = sim_trace + x;
end
% Find the most correlated point
sim_result = dpa_correlation(m1,sim_trace,1,1,'pearson');
[sorted_result sorted_index] = sort(sim_result);
int_points = sorted_index(end-int_num+1:end);

B = B(:,int_points,:);  

%------------------------------------
%   R(t)
%------------------------------------
disp('Calculating covariance........');
m1 = measured_trace(trace_N2_1:trace_N2_2,time_idx1:time_idx2);
m1 = m1(:,int_points);
for sbox=1:8
  key_idx = roundkey(sbox)+1;
  m2 = pred_trace(trace_N2_1:trace_N2_2,(sbox-1)*64+1:sbox*64);
  N2 = trace_N2_2 - trace_N2_1 + 1;
  SP = size(m1,2); %sample points number
  h = zeros(N2,SP);
  temp = zeros(N2,4);
  for i=1:4
    temp(:,i) = bitget(double(m2(:,key_idx)),i);
  end
  h = repmat(B(1,:,sbox),N2,1) + temp * B(2:5,:,sbox);
  Rt = m1 - h;
  R_c(:,:,sbox) = cov(Rt); 
end


%------------------------------------
%   Stochastic Attack Phase
%------------------------------------
disp('Attack Phase Start........');
trg_dir = sprintf('./output');
mkdir(trg_dir);
output_filename = sprintf('%s/output_stochastic.txt',trg_dir);
fid = fopen(output_filename,'w');
fprintf(fid,'# TABLE : secmatv1_2006_04_0809\n');
fprintf(fid,'# Time Index Range : %d - %d \n',time_idx1,time_idx2);
fprintf(fid,'# Target Round Key : 16 round \n');
fprintf(fid,'# Find Peak Value of CPA Result\n');
fprintf(fid,'# RoundKey : %d %d %d %d %d %d %d %d\n',roundkey(1),roundkey(2),roundkey(3),roundkey(4),roundkey(5),roundkey(6),roundkey(7),roundkey(8));
fprintf(fid,'# Stability threshold : 100\n');
fprintf(fid,'iteration Stability Subkey0 ... Subkey7\n');

measured_trace = measured_trace(trace_N3_1:trace_N3_2,time_idx1:time_idx2);
pred_trace = pred_trace(trace_N3_1:trace_N3_2,:);

N3 = trace_N3_2 - trace_N3_1 + 1;
dpa_result = zeros(1,64,8);
stability = 0;
for trace_idx=1:N3
  correct_subkey = 0;
  for sbox=1:8
    t_i = measured_trace(trace_idx,int_points);
    pred_i = pred_trace(trace_idx,(sbox-1)*64+1:sbox*64);
    prob = zeros(1,64);
    for k=1:64
      temp = zeros(1,4);
      for i=1:4
	temp(i) = bitget(double(pred_i(:,k)),i);
      end
      h = B(1,:,sbox) + temp * B(2:5,:,sbox);
      z = t_i - h;
      prob(k) = z*inv(R_c(:,:,sbox))*z';
    end
    dpa_result(:,:,sbox) = dpa_result(:,:,sbox) + prob;
    [dummy found_key] = find(min(dpa_result(:,:,sbox)) == dpa_result(:,:,sbox)); 
    found_key = found_key(1)-1;
    all_found_key(trace_idx,sbox) =  found_key;
    if(roundkey(sbox) == found_key)
      correct_subkey = correct_subkey + 1;
    end
  end % end of Sbox
  
  % Update stability
  if(correct_subkey==8)
    stability = stability + 1;
  else
    stability = 0;
  end

  % Print out found keys
  fprintf(fid,' %d %d  ',trace_idx,stability);
  for sbox=1:8
    fprintf(fid,'%d\t',all_found_key(trace_idx,sbox));
  end
  fprintf(fid,'\n');
end




