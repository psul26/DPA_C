


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
str = sprintf('%s/measured_trace.mat',matfolder);
load(str);
measured_trace1 = measured_trace(:,time_idx11:time_idx12);

%-----------------------------------
%        Load IV 
%-----------------------------------
str = sprintf('%s/pred_trace.mat',matfolder);
load(str);

%-------------------------------------
%        Execute DPA
%-------------------------------------
fid = 1;
stability = 0;
iteration = 10;

trg_dir = sprintf('./output');
mkdir(trg_dir);
output_filename = sprintf('%s/output_spearman.txt',trg_dir);
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
      m1= measured_trace1(1:i,:);
      m2 = pred_trace(1:i,64*(sbox-1)+1:64*sbox);
      dpa_result = dpa_correlation(m1,m2,1,64,'spearman');
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


