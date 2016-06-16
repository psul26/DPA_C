%-- (C) COPYRIGHT 2009 MITSUBISHI ELECTRIC CORPORATION --
%-- File Name : Run_CustomOrder_r140.m --
%-- Type : Script M-file --
%-- Author : :Daisuke SUZUKI --
%-- Version : 0 --

clear

WaveDir = 'D:\DPAContest\secmatv1_2006_04_0809\'; % modify here
FileListName = 'SVNr140.txt';  % modify here

OutFileIndex = 'CustomOrder_r140';
TargetRange = [5725 5775;14475 14525];
TraceCount = 13;
IterationThreshold = 100;
CompRatio = 5;
SearchMode = [1 1 1];
SearchOrder(1,:) = [1 1 1 1 1 1 0 0 0 1 0 0];
SearchOrder(2,:) = [3 1 6 7 4 8 8 5 6 5 3 1];
SearchTimes = 12;

Func_DPAContest(WaveDir, FileListName, OutFileIndex, TargetRange, TraceCount,...
 IterationThreshold, CompRatio, SearchMode, SearchOrder, SearchTimes);