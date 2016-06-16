%-- (C) COPYRIGHT 2009 MITSUBISHI ELECTRIC CORPORATION --
%-- File Name : Run_RepresentativeOrder.m --
%-- Type : Script M-file --
%-- Author : :Daisuke SUZUKI --
%-- Version : 1 --

clear

WaveDir = 'K:\DPAContest\secmatv1_2006_04_0809\'; % modify here
BaseFileList = 'db-order.txt'; % copy of http://projets.comelec.enst.fr/dpacontest/db-order.txt

mkdir('list_files');
mkdir('log_files');
TargetRange = [5725 5775;14475 14525];
TraceCount = 901;
IterationThreshold = 100;
CompFlag = 0;
SearchMode = [1 1 1];
SearchOrder(1,:) = [1 0 1 0 1 0 1 0 1 1 1 0];
SearchOrder(2,:) = [6 6 8 8 1 1 5 5 7 4 2 7];

SearchTimes = 12;
N = 1000;

OutFileIndex = 'log_files\\RepresentativeOrder';


FileListName = 'list_files\\PartitionedList';
for n=1:1:80
	n
	fn1 = sprintf('%s%s%04d%s',FileListName,'_',n,'.txt');
	Func_GenListFile(BaseFileList, fn1,n);
	fn2 = sprintf('%s%s%04d%',OutFileIndex,'_',n,'_');
	[MaxWaveNum(1,n) StepWaveNum(n,:)] = Func_DPAContest(WaveDir, fn1, fn2, TargetRange, TraceCount,...
 	IterationThreshold, CompFlag, SearchMode, SearchOrder, SearchTimes);
end

rand('twister', 1234567);

FileListName = 'list_files\\RandomList';
for n=81:1:N
	n
	fn1 = sprintf('%s%s%04d%s',FileListName,'_',n,'.txt');
	Func_GenListFile(BaseFileList, fn1,n);
	fn2 = sprintf('%s%s%04d%',OutFileIndex,'_',n,'_');
	[MaxWaveNum(1,n) StepWaveNum(n,:)] = Func_DPAContest(WaveDir, fn1, fn2, TargetRange, TraceCount,...
 	IterationThreshold, CompFlag, SearchMode, SearchOrder, SearchTimes);
end

save('MaxWaveNum','MaxWaveNum');
at80 = mean(MaxWaveNum(1,1:80));
at1000 = mean(MaxWaveNum);

temp = histc(MaxWaveNum(1,1:80),1:1000);
rate80 = cumsum(temp)/80;
temp = histc(MaxWaveNum,1:1000);
rate1000 = cumsum(temp)/1000;

plot(rate80,'r')
hold on
plot(rate1000)
legend('Our attack (80 exp.)','Our attack (1000 exp.)','Location','SouthEast')
xlabel('Number of queries');
ylabel('Success rate of our attack');
saveas(gcf,'Results.fig')

fid1 = fopen('Results.txt','w');
fprintf(fid1, 'Average number of traces (80 exp.)  : %d (DPA contest policy: %d)\n',ceil(at80),ceil(at80)+99);
fprintf(fid1, 'Average number of traces (1000 exp.): %d (DPA contest policy: %d)\n',ceil(at1000),ceil(at1000)+99);
fclose(fid1);
