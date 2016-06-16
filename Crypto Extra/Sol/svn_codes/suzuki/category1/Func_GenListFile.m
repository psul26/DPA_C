%-- (C) COPYRIGHT 2009 MITSUBISHI ELECTRIC CORPORATION --
%-- File Name : Func_GenListFile.m --
%-- Type : Function M-file --
%-- Author : :Daisuke SUZUKI --
%-- Version : 0 --

function Func_GenListFile(BaseFileList,FileListName,N)

fid1 = fopen(BaseFileList,'r');
fid2 = fopen(FileListName ,'w');

if(N>80)
	for n=1:1:1000
		r = ceil(80000*rand);
		r = r - 1;
		fseek(fid1,94*r,'bof');
		fn = fgets(fid1);
		fprintf(fid2, '%s', fn);
	end
else
	fseek(fid1,94*1000*(N-1),'bof');
	for n=1:1:1000
		fn = fgets(fid1);
		fprintf(fid2, '%s', fn);
	end
end

fclose(fid1);
fclose(fid2);
