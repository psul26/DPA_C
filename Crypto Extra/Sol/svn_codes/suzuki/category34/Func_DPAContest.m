%-- (C) COPYRIGHT 2009 MITSUBISHI ELECTRIC CORPORATION --
%-- File Name : Func_DPAContest.m --
%-- Type : M-file --
%-- Author : :Daisuke SUZUKI --
%-- Version : 0 --

function Func_DPAContest(WaveDir, FileListName, OutFileIndex, TargetRange, TraceCount,...
IterationThreshold, CompRatio, SearchMode, SearchOrder, SearchTimes)

ofi = OutFileIndex;
wd = WaveDir;
fln = FileListName;
tc = TraceCount;
it = IterationThreshold;
cr = CompRatio;
fc = tc + it - 1;
sf = SearchOrder(1,:);
sn = SearchOrder(2,:);
st = SearchTimes;

sm01 = [SearchMode 0];
tr01 = TargetRange(1,1:2);
sm16 = [SearchMode 1];
tr16 = TargetRange(2,1:2);
bf = zeros(2,8);
bd = zeros(2,8);

fk  = zeros(2,8);
fkf = zeros(2,8);
fp = zeros(2,8);

for i=1:1:st
	Status = sprintf('%s %02d %s%02d%s%d','Step:',i,'Target Key: K',sf(1,i)*15+1,'-S',sn(1,i))
	s = sn(1,i);
	if(sf(1,i)==1)
		ss = 6 - sum(bitget(bf(2,s),6:-1:1));
		[fr fwn skl] = Func_SearchKey(wd, fln, tr16, tc, it, cr, s, sm16, fkf, fk, bf, bd);
		attack_log(i,:)=[16 s fr fwn ss];
		keylog16(s,:) = skl;
		fkf(2,s) = 1;
		fk(2,s) = skl(1,tc);
		[bf, bd] = Func_UpdateBC(fkf,fk);
	elseif(sf(1,i)==0)
		ss = 6 - sum(bitget(bf(1,s),6:-1:1));
		[fr fwn skl] = Func_SearchKey(wd, fln, tr01, tc, it, cr, s, sm01, fkf, fk, bf, bd);
		attack_log(i,:)=[1 s fr fwn ss];
		keylog01(s,:) = skl;
		fkf(1,s) = 1;
		fk(1,s) = skl(1,tc);
		[bf, bd] = Func_UpdateBC(fkf,fk);
	end
end

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

FoundKeyWithParity(1:fc,1:64) = -1;

for i=1:1:fc
	for s=st:-1:1
		j = sn(1,s);
		if(sf(1,s)==0)
			for k=1:1:6
				FoundKeyWithParity(i,K01BP(1,6*j-6+k))= bitget(keylog01(j,i),6-k+1:-1:6-k+1);
			end
		elseif(sf(1,s)==1)
			for k=1:1:6
				FoundKeyWithParity(i,K16BP(1,6*j-6+k))= bitget(keylog16(j,i),6-k+1:-1:6-k+1);
			end
		end
	end
end

for i=1:1:fc
	temp = '';
	for j=1:1:64
		if(mod(j,8)~=0)
			if(FoundKeyWithParity(i,j)==1)
				temp = strcat(temp,'1');
			elseif(FoundKeyWithParity(i,j)==0)
				temp = strcat(temp,'0');
			else
				temp = strcat(temp,'*');
			end
		elseif(j~=64)
			temp = strcat(temp,'_');
		end
	end
	FoundKey(i,:) = temp;
end

for i=1:1:fc
	for j=1:1:8
		if(fkf(1,j)==1)
			R01Key(:,j) = keylog01(j,:)';
		else
			R01Key(:,j) = ones(fc,1)'*-1;
		end
	end
end

for i=1:1:fc
	for j=1:1:8
		if(fkf(2,j)==1)
			R16Key(:,j) = keylog16(j,:)';
		else
			R16Key(:,j) = ones(fc,1)'*-1;
		end
	end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Write Log Files
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
key01 = 'E0BEE600D677';
key16 = 'F0BE2E5B242C';
key01b(1,:) = bitget(hex2dec(key01),48:-1:1);
key16b(1,:) = bitget(hex2dec(key16),48:-1:1);

bit6_dec(1,1:6) = [32,16,8,4,2,1];

for i=1:1:8
	key01d(1,i) = bit6_dec(1,1:6) * key01b(1,6*i-5:6*i)';
	key16d(1,i) = bit6_dec(1,1:6) * key16b(1,6*i-5:6*i)';
end

fn = strcat(ofi,'_subkey.txt');
fid1 = fopen(fn','w');
fn = strcat(ofi,'_masterkey.txt');
fid2 = fopen(fn,'w');
fprintf(fid1, 'Stability threshold: 100\n');
fprintf(fid2, 'Stability threshold: 100\n');
fprintf(fid1, '# Columns: [Iteration] | [Stability] | R1 [Subkey0] ... [Subkey7] | R16 [Subkey0] ... [Subkey7] \n');
fprintf(fid2, '# Columns: [Iteration] | [Stability] | 56bit Master Key\n');
fprintf(fid1, '# ** is not a search object\n');
fprintf(fid2, '# * is not a search object\n');

cnt_c = 0;
cnt_s = 0;
for i=1:1:fc
	fprintf(fid1, '%03d ', i);
	fprintf(fid2, '%03d ', i);
	for j=1:1:8
		if(R01Key(i,j)==key01d(1,j))
			cnt_c = cnt_c + 1;
		end
		if(R16Key(i,j)==key16d(1,j))
			cnt_c = cnt_c + 1;
		end
	end
	if(cnt_c==st)
		cnt_s = cnt_s + 1;
		fprintf(fid1, '| %03d ', cnt_s);
		fprintf(fid2, '| %03d ', cnt_s);
	else
		cnt_s = 0;
		fprintf(fid1, '| %03d ', cnt_s);
		fprintf(fid2, '| %03d ', cnt_s);
	end
	cnt_c = 0;
	fprintf(fid1, '| ');
	fprintf(fid2, '| ');
	fprintf(fid2, '%s\n', FoundKey(i,:));
	for j=1:1:8
		if(R01Key(i,j)==-1)
			fprintf(fid1, '** ');
		else
			fprintf(fid1, '%02d ',R01Key(i,j));
		end
	end
	fprintf(fid1, '| ');
	for j=1:1:8
		if(R16Key(i,j)==-1)
			fprintf(fid1, '** ');
		else
			fprintf(fid1, '%02d ',R16Key(i,j));
		end
	end
	fprintf(fid1, '\n');
end

fprintf(fid1, '# Answer  | 56 11 59 38 00 13 25 55 | 60 11 56 46 22 50 16 44 \n');
fprintf(fid2, '# Answer  | 0110101_0110010_0111100_0110101_0110010_0111100_0110101_0110010\n');
fclose(fid1);
fclose(fid2);

fn = strcat(ofi,'_log.txt');
fid3 = fopen(fn,'w');
fprintf(fid1, 'Analysis process\n');
for i=1:1:st
	fprintf(fid3, 'Step %02d :',i);
	fprintf(fid3, 'TargetRound=%02d | ',attack_log(i,1));
	fprintf(fid3, 'TargetSbox=%d | ',attack_log(i,2));
	fprintf(fid3, 'AdoptedInterval=%05d:%05d | ',attack_log(i,3),attack_log(i,4));
	fprintf(fid3, 'SearchSpace=2^%d | ',attack_log(i,6));
	fprintf(fid3, 'FileNum=%02d \n',attack_log(i,5));
end
fclose(fid3);

