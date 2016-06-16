%-- (C) COPYRIGHT 2009 MITSUBISHI ELECTRIC CORPORATION --
%-- File Name : Func_UpdateBC.m --
%-- Type : Function M-file --
%-- Author : :Daisuke SUZUKI --
%-- Version : 0 --

function [BindingFlag BindingData] = Func_UpdateBC(FixedKeyFlag, FixedKey)

fkf = FixedKeyFlag;
fk = FixedKey;

bit6_dec(1,1:6) = [32,16,8,4,2,1];

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

nk_data(1,1:64) = 0;
nk_flag(1,1:64) = 0;

for i=1:1:8
	if(fkf(1,i)==1)
		for j=1:1:6
			nk_data(1,K01BP(1,6*i-6+j)) = bitget(fk(1,i),6-j+1:-1:6-j+1);
			nk_flag(1,K01BP(1,6*i-6+j)) = 1;
		end
	end
end
for i=1:1:8
	if(fkf(2,i)==1)
		for j=1:1:6
			nk_data(1,K16BP(1,6*i-6+j)) = bitget(fk(2,i),6-j+1:-1:6-j+1);
			nk_flag(1,K16BP(1,6*i-6+j)) = 1;
		end
	end
end

for i=1:1:8
	for j=1:1:6
		temp_nkd01(1,j) = nk_data(1,K01BP(1,6*i-6+j));
		temp_nkf01(1,j) = nk_flag(1,K01BP(1,6*i-6+j));
		temp_nkd16(1,j) = nk_data(1,K16BP(1,6*i-6+j));
		temp_nkf16(1,j) = nk_flag(1,K16BP(1,6*i-6+j));
	end
	bd(1,i) = bit6_dec(1,:) * temp_nkd01(1,:)';
	bd(2,i) = bit6_dec(1,:) * temp_nkd16(1,:)';
	bf(1,i) = bit6_dec(1,:) * temp_nkf01(1,:)';
	bf(2,i) = bit6_dec(1,:) * temp_nkf16(1,:)';
end

BindingFlag = bf;
BindingData = bd;
