function [key_trace] = dpa_correlation(measured_trace, pred_trace, ...
				       first,last, method)

% [key_trace] = dpa_correlation(messured_trace, pred_trace,first,last,method);
%
% Calculating 3 different correlation coefficient
%
% measured_trace : measured power trace. it has to be (m x n)
%                  m : number of traces
%                  n : time index
% pred_trace : intermediate value (HD model). it has to be (m x 64)
%                  m : same order as m in 'measured_trace'
%                 64 : intermediate value on each 64 key condidates 
% method  : 
%  'pearson' :  calculate Pearson's linnear correlation
%  'spearman' :  calculate Spearman's rho (need statistics toolbox)
%  'kendall' : calculate Kendall's tau (need statistics toolbox)
%
%  first : the first index of key candidates, usually 1.
%  last : the last index of key candidates, usually 64.
% 
% key_trace : the result of correlation coefficient using 'method',
%             This metix has (64 x n)
%             64 : 64 key candidates index
%             n : time index
%

[m_a m_b] = size(measured_trace);
[p_a p_b] = size(pred_trace);
if(m_a ~= p_a)
  error('Error - different size')
end

if (m_b < 50)
  chunksize = 1;
  chunks = m_b;
else
  chunksize = 50;
  chunks = m_b / chunksize;
end


if(strcmp(method,'pearson'))
  for i=first:last
    if(rem(i,100)==0)
    fprintf(1,'Key %d ......processing..\n',i);    
    end
    for j=1:chunks
    corr_x = measured_trace(:,1+(j-1)*chunksize : j*chunksize);
    corr_y = pred_trace(:,i);
    cmatrix = corrcoef([corr_x corr_y]);
    key_trace(i,1+(j-1)*chunksize:j*chunksize) = cmatrix(chunksize+ ...
						  1,1:chunksize);
    end
  end
end


if(strcmp(method,'spearman'))
  for i=first:last
    [rho p] = corr(measured_trace, pred_trace(:,i), 'type','spearman');
    key_trace(i,:) = rho';
  end
end

if(strcmp(method,'kendall'))
  for i=first:last
    [rho p] = corr(measured_trace, pred_trace(:,i), 'type','kendall');
    key_trace(i,:) = rho';
  end
end
