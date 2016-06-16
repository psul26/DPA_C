%-- (C) COPYRIGHT 2009 MITSUBISHI ELECTRIC CORPORATION --
%-- File Name : Func_RunPA.m --
%-- Type : Function M-file --
%-- Author : :Daisuke SUZUKI --
%-- Version : 1 --

function Score = Func_RunPA(WaveDir, FileListName, TargetRange, ...
FileCount, TraceCount, CompFlag, SearchMode, ...
TargetSbox, FixedKeyFlag, FixedKey, SelectionFunction)

bit6_dec(1,1:6) = [32,16,8,4,2,1];

tl = TargetRange(1,2) - TargetRange(1,1) + 1;

K01BP = ...
[ 10,51,34,60,49,17, ...
  33,63,02,09,19,42, ...
  03,35,26,25,44,58, ...
  59,01,36,27,18,41, ...
  22,28,39,54,37,04, ...
  47,30,05,53,23,29, ...
  61,21,38,57,15,20, ...
  45,14,13,62,55,31];

K16BP = ...
[ 18,59,42,03,57,25, ...
  41,36,10,17,27,50, ...
  11,43,34,33,52,01, ...
  02,09,44,35,26,49, ...
  30,05,47,62,45,12, ...
  55,38,13,61,31,37, ...
  06,29,46,04,23,28, ...
  53,22,21,07,63,39 ];

K02BP = ...
[  2,43,26,52,41, 9, ...
  25,55,59, 1,11,34, ...
  60,27,18,17,49,50, ...
  51,58,63,19,10,33, ...
  14,20,31,46,29,44, ...
  39,22,28,45,15,21, ...
  53,13,30,36, 7,12, ...
  37, 6, 5,54,47,23 ];

K15BP = ...
[ 26, 2,50,11,52,33, ...
  49,44,18,25,35,58, ...
  19,51,42,41,60, 9, ...
  10,17, 4,43,34,57, ...
  38,13,55, 7,53,20, ...
  63,46,21, 6,39,45, ...
  14,37,54,12,31, 5, ...
  61,30,29,15,36,47 ];

Sigma_H  = zeros(1,64);
Sigma_H2 = zeros(1,64);
Sigma_W  = zeros(1,tl);
Sigma_W2 = zeros(1,tl);
Sigma_WH = zeros(64,tl);
data_w = zeros(1,tl);

if(SearchMode(1,2)==1)
	npk(1,1:64) = -1;
	for i=1:1:8
		if(FixedKeyFlag(1,i)==1)
			for j=1:1:6
				npk(1,K01BP(1,6*i-6+j))	= bitget(FixedKey(1,i),6-j+1:-1:6-j+1);
			end
		end
	end
	for i=1:1:8
		if(FixedKeyFlag(2,i)==1)
			for j=1:1:6
				npk(1,K16BP(1,6*i-6+j))	= bitget(FixedKey(2,i),6-j+1:-1:6-j+1);
			end
		end
	end
	if(SearchMode(1,4)==1)
		for i=1:1:64
			for j=1:1:6
				npk(1,K16BP(1,6*TargetSbox-6+j)) = bitget(i-1,6-j+1:-1:6-j+1);
			end
			dk(1,i) = 0;
			for j=1:1:48
				if((npk(1,K16BP(1,j))+npk(1,K15BP(1,j)))>0)
					dk(1,i) = dk(1,i) + 1;
				end
			end
		end
	elseif(SearchMode(1,4)==0)
		for i=1:1:64
			for j=1:1:6
				npk(1,K01BP(1,6*TargetSbox-6+j)) = bitget(i-1,6-j+1:-1:6-j+1);
			end
			dk(1,i) = 0;
			for j=1:1:48
				if((npk(1,K01BP(1,j))+npk(1,K02BP(1,j)))>0)
					dk(1,i) = dk(1,i) + 1;
				end
			end
		end
	end
end


fid1 = fopen(FileListName,'r');

for i=1:1:FileCount

	fn = fgets(fid1);
	fn = strcat(WaveDir,fn);
	fid2 = fopen(fn,'r');

	header = fread(fid2,[164 1], 'uchar');
	wave = fread(fid2,[20003 1], 'float32');
	fclose(fid2);
	temp_wave = wave(1:20003,1)';

	data_w(1,:) = temp_wave(1,TargetRange(1,1):TargetRange(1,2));

	if(SearchMode(1,4)==1)
		temp_hc(:,:) = SelectionFunction(2,i,:,:);
		data_h(1,:) = temp_hc(TargetSbox,:);
		if(SearchMode(1,3)==1)
			for j=1:1:8
				if(FixedKeyFlag(2,j)==1)
					data_h(1,:) = data_h(1,:) + temp_hc(j,FixedKey(2,j)+1);
				else
					data_h(1,:) = data_h(1,:);
				end
			end
		end
	elseif(SearchMode(1,4)==0)
		temp_hm(:,:)= SelectionFunction(1,i,:,:);
		data_h(1,:) = temp_hm(TargetSbox,:);
		if(SearchMode(1,3)==1)
			for j=1:1:8
				if(FixedKeyFlag(1,j)==1)
					data_h(1,:) = data_h(1,:) + temp_hm(j,FixedKey(1,j)+1);
				else
					data_h(1,:) = data_h(1,:);
				end
			end
		end
	end

	if(SearchMode(1,2)==1)
		data_h(1,:) = data_h(1,:) + dk(1,:);
	end

	Sigma_W(1,:)  = Sigma_W(1,:) + data_w(1,:);
	Sigma_W2(1,:) = Sigma_W2(1,:) + (data_w(1,:) .^2);
	Sigma_H(1,:) = Sigma_H(1,:) + data_h(1,:);
	Sigma_H2(1,:) = Sigma_H2(1,:) + (data_h(1,:).^2);
	temp_h  = data_h(1,:)';
	temp_wh = temp_h(:,1) * data_w(1,:); 
	Sigma_WH(:,:) = Sigma_WH(:,:) + temp_wh;

	Cii(1,:)=sqrt((Sigma_W2(1,:)/i)-((Sigma_W(1,:)/i).^2));
	Cij(:,:) = ((Sigma_WH/i)-(Sigma_H(1,:)'/i)*(Sigma_W(1,:)/i));
	Cjj(1,:)=sqrt((Sigma_H2(1,:)/i)-((Sigma_H(1,:)/i) .^2));

	CPATrace = Cij ./ (Cjj(1,:)' * Cii(1,:));

	if(CompFlag==0)
		Score(i,:) = max(CPATrace');
	else
		Score(i,:) = mean(CPATrace');
	end

end

fclose(fid1);
