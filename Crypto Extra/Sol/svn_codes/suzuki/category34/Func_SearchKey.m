%-- (C) COPYRIGHT 2009 MITSUBISHI ELECTRIC CORPORATION --
%-- File Name : Func_SearchKey.m --
%-- Type : Function M-file --
%-- Author : :Daisuke SUZUKI --
%-- Version : 0 --

function [FoundRange FoundWaveNum FoundKeyLog] = Func_SearchKey...
(WaveDir, FileListName, TargetRange, TraceCount, IterationThreshold, CompRatio, ...
TargetSbox, SearchMode, FixedKeyFlag, FixedKey, BindingFlag, BindingData)

wd = WaveDir;
fln = FileListName;
tc = TraceCount;
fc = TraceCount + IterationThreshold - 1;
cr = CompRatio;
s = TargetSbox;
sm = SearchMode;
tr = TargetRange;
fk  = FixedKey;
fkf = FixedKeyFlag;
bf = BindingFlag;
bd = BindingData;

tl = tr(1,2) - tr(1,1) + 1;
if(cr~=1)
	tl = floor(tl/cr);
end

if(sm(1,4)==1)
	b_flag(1,:) = bf(2,:);
	b_data(1,:) = bd(2,:);
elseif(sm(1,4)==0)
	b_flag(1,:) = bf(1,:);
	b_data(1,:) = bd(1,:);
end

Trace = Func_RunPA(wd, fln, tr, fc, tc, sm, s, fkf, fk);

for c=1:1:tl
	temp_cnt = 0;
	temp_ck = 0;
	for i=1:1:fc
		if(cr==1)
			temp(:,1) = Trace(i,:,c);
			Score(i,:) = temp';
		else
			temp(:,:) = Trace(i,:,(c-1)*cr+1:c*cr);
			Score(i,:) = mean(temp');
		end
		[rank,ck] = sort(Score(i,:));
		a = 64;
		check = 0;
		while(check==0)
			if(bitxor(bitand(ck(1,a)-1,b_flag(1,s)),b_data(1,s))==0)
				key = ck(1,a)-1;
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
	ScoreComp(1,c) = temp_cnt;
	FoundKey(1,c) = temp_ck;
end

[found_num c] = max(ScoreComp);

for i=1:1:fc
	temp(:,:) = Trace(i,:,(c-1)*cr+1:c*cr);
	Score(i,:) = mean(temp');
	[rank,ck] = sort(Score(i,:));
	a = 64;
	check = 0;
	while(check==0)
		if(bitxor(bitand(ck(1,a)-1,b_flag(1,s)),b_data(1,s))==0)
			FoundKeyLog(1,i) = ck(1,a)-1;
			check = 1;
		else
			a = a - 1;
		end
	end
end

FoundRange =  [tr(1,1)+(c-1)*cr tr(1,1)+c*cr-1];
FoundWaveNum = fc - found_num;
