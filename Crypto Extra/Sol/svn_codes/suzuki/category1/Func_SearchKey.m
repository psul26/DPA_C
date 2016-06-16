%-- (C) COPYRIGHT 2009 MITSUBISHI ELECTRIC CORPORATION --
%-- File Name : Func_SearchKey.m --
%-- Type : Function M-file --
%-- Author : :Daisuke SUZUKI --
%-- Version : 1 --

function [FoundWaveNum FoundKeyLog] = Func_SearchKey...
(WaveDir, FileListName, TargetRange, TraceCount, IterationThreshold, CompFlag, ...
TargetSbox, SearchMode, FixedKeyFlag, FixedKey, BindingFlag, BindingData, SelectionFunction)

wd = WaveDir;
fln = FileListName;
tc = TraceCount;
fc = TraceCount + IterationThreshold - 1;
cf = CompFlag;
s = TargetSbox;
sm = SearchMode;
tr = TargetRange;
fk  = FixedKey;
fkf = FixedKeyFlag;
bf = BindingFlag;
bd = BindingData;
sf = SelectionFunction;

if(sm(1,4)==1)
	b_flag(1,:) = bf(2,:);
	b_data(1,:) = bd(2,:);
elseif(sm(1,4)==0)
	b_flag(1,:) = bf(1,:);
	b_data(1,:) = bd(1,:);
end

Score = Func_RunPA(wd, fln, tr, fc, tc, cf, sm, s, fkf, fk, sf);

temp_cnt = 0;
temp_ck = 0;
for i=1:1:fc
	[rank,ck] = sort(Score(i,:));
	a = 64;
	check = 0;
	while(check==0)
		if(bitxor(bitand(ck(1,a)-1,b_flag(1,s)),b_data(1,s))==0)
			key =  ck(1,a)-1;
			FoundKeyLog(1,i) = key;
			check = 1;
		else
			a = a - 1;
		end
	end
	if(key==temp_ck)
		temp_cnt = temp_cnt + 1;
	else
		temp_cnt = 0;
		temp_ck = key;
	end
end

FoundWaveNum = fc - temp_cnt;
