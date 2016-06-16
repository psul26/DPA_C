


%----------------------------------
%        Setup
%----------------------------------
clear all

%attack time index range
time_idx11 = 14450;
time_idx12 = 14550;

%measured power trace/hamming distance value folder
matfolder = './data';

%correct subkeys of the last round
roundkey = [60 11 56 46 22 50 16 44];

%------------------------------------
%        Load Measured Traces
%------------------------------------
str = sprintf('%s/measured_trace_10000.mat',matfolder);
load(str);
measured_trace = measured_trace(:,time_idx11:time_idx12);

%-----------------------------------
%        Load IV 
%-----------------------------------
str = sprintf('%s/pred_trace_10000.mat',matfolder);
load(str);

%------------------------------------------
%     Choose Plaintext Order
% Assume that an adversary know the key.
%------------------------------------------
% Calculate all-bit-long HD
sim_trace = zeros(10000,1);
for sbox=1:8
  m2 = pred_trace(:,64*(sbox-1)+1:64*sbox);
  x = m2(:,roundkey(sbox)+1);
  sim_trace = sim_trace + x;
end

% Find the most correlated point
sim_result = dpa_correlation(measured_trace,sim_trace,1,1,'pearson');
[dummy trg_time] = find(sim_result == max(sim_result));

% Calculate mean and variance of each HD
for sbox=1:8
  key_idx = roundkey(sbox)+1;
  sbox_mean = zeros(5,1);
  sbox_var = zeros(5,1);
  sbox_p = pred_trace(:,64*(sbox-1)+1:64*sbox);
  sbox_p = sbox_p(:,key_idx);
  for hd=0:4
    tmp_idx = find(hd==sbox_p);
    tmp_mean = mean(measured_trace(tmp_idx,trg_time));
    tmp_var = var(measured_trace(tmp_idx,trg_time));
    sbox_mean(hd+1,:) = tmp_mean;
    sbox_var(hd+1,:) = tmp_var;
  end
  s(sbox) = struct('mean_vec',sbox_mean,'var_vec',sbox_var);
end


% Derive new order
all_sbox_f = zeros(10000,1);
for sbox=1:8
  m2 = pred_trace(:,64*(sbox-1)+1:64*sbox);
  hd= m2(:,roundkey(sbox)+1);
  temp = s(sbox).mean_vec;
  mu = temp(hd+1);
  temp = sqrt(s(sbox).var_vec);
  sigma = temp(hd+1);
  
  x = measured_trace(:,trg_time);
  z = (x-mu)./sigma;
  f = normpdf(z);
  all_sbox_f = all_sbox_f+ f;
end
[sorted_sbox_f sorted_idx] = sort(all_sbox_f,'ascend');
new_idx = sorted_idx;

% Replace trace order
measured_trace = measured_trace(new_idx,:);
pred_trace = pred_trace(new_idx,:);

%-------------------------------------
%        Execute DPA
%-------------------------------------
stability = 0;
iteration = 10;

trg_dir = sprintf('./output');
mkdir(trg_dir);
output_filename = sprintf('%s/output_cpa_chosen.txt',trg_dir);
disp('-------------------------------------------------');
fprintf(1,'OUTPUT FILENAME : %s\n',output_filename);
disp('-------------------------------------------------');
fid = fopen(output_filename,'w');
fprintf(fid,'# TABLE : secmatv1_2006_04_0809\n');
fprintf(fid,'# Time Index Range : %d - %d \n',time_idx11,time_idx12);
fprintf(fid,'# Target Round Key : 16 round \n');
fprintf(fid,'# Find Peak Value of CPA Result\n');
fprintf(fid,'# RoundKey : %d %d %d %d %d %d %d %d\n',roundkey(1),roundkey(2),roundkey(3),roundkey(4),roundkey(5),roundkey(6),roundkey(7),roundkey(8));
fprintf(fid,'# Stability threshold : 100\n');
fprintf(fid,'iteration Stability Subkey0 ... Subkey7\n');

found_key = zeros(1,8);
for i=1:500
  if(i<iteration)
    fprintf(fid,'%d\n',i);
  else
    if(rem(i,50)==0)
      fprintf(1,'Attacking using %d traces............\n',i);
    end
    correct_subkey = 0;
    for sbox=1:8
      m1= measured_trace(1:i,:);
      m2 = pred_trace(1:i,64*(sbox-1)+1:64*sbox);
      dpa_result = dpa_correlation(m1,m2,1,64,'pearson');
      [key max_time] = find(dpa_result == max(max(dpa_result)));
      % Just in case two possible choice
      key = key(1); max_time = max_time(1);
      key_idx = key - 1;
      sbox_found_key(i-iteration+1,sbox) = key_idx;
      if(roundkey(sbox)==key_idx)
	correct_subkey = correct_subkey + 1;
      end
    end % end of sbox

    if(correct_subkey==8)
      stability = stability + 1;
    else
      stability = 0;
    end
    
    fprintf(fid,'%d %d',i,stability);
    for sbox=1:8
      fprintf(fid,' %d',sbox_found_key(i-iteration+1,sbox));
    end
    %check stability is 100 or not
    if(stability==100)
      fprintf(fid,' <---------- ');
    end
    fprintf(fid,'\n');
  end
end


