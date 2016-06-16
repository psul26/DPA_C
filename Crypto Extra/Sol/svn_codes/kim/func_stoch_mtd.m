
% Return MTD with 100 stability.
function [mtd] = func_stoch_mtd(trace_N1,trace_N2,trace_N3,int_num,measured_trace,pred_trace,time_idx1,time_idx2,fid)

% correct subkeys of the last round
roundkey = [60 11 56 46 22 50 16 44];

%------------------------------------
%    Profiling Phase #1 
% assume that an adversary know all keys
% in the profiling phase #1 and #2
%
% calculation of h*(t,x,k)
%------------------------------------
%disp('Calculating h*(t,x,k)........');
m1 = measured_trace(trace_N1,time_idx1:time_idx2);
for sbox=1:8
  key_idx = roundkey(sbox)+1;
  m2 = pred_trace(trace_N1,(sbox-1)*64+1:sbox*64);
  N1 = length(trace_N1);
  A = zeros(N1,5); 
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
  m2 = pred_trace(trace_N1,64*(sbox-1)+1:64*sbox);
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
%disp('Calculating covariance........');
m1 = measured_trace(trace_N2,time_idx1:time_idx2);
m1 = m1(:,int_points);
for sbox=1:8
  key_idx = roundkey(sbox)+1;
  m2 = pred_trace(trace_N2,(sbox-1)*64+1:sbox*64);
  N2 = length(trace_N2);
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
%disp('Attack Phase Start........');
fprintf(fid,'-------------------------------------------------\n');
fprintf(fid,'iteration Stability Subkey0 ... Subkey7\n');
measured_trace = measured_trace(trace_N3,time_idx1:time_idx2);
pred_trace = pred_trace(trace_N3,:);

N3 = length(trace_N3);
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
  if(stability == 100)
    mtd = trace_idx;
    break
  end

end




