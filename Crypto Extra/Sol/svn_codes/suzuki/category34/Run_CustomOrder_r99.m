%-- (C) COPYRIGHT 2009 MITSUBISHI ELECTRIC CORPORATION --
%-- File Name : Run_CustomOrder_r99.m --
%-- Type : Script M-file --
%-- Author : :Daisuke SUZUKI --
%-- Version : 0 --

clear

WaveDir = 'D:\DPAContest\secmatv1_2006_04_0809\'; % modify here
FileListName = 'SVNr99.txt';  % modify here

OutFileIndex = 'CustomOrder_r99';
TargetRange = [5725 5775;14475 14525];
TraceCount = 16;
IterationThreshold = 100;
CompRatio = 5;
SearchMode = [1 1 1];
SearchOrder(1,:) = [1 1 1 0 1 0 1 1 0 1 0];
SearchOrder(2,:) = [6 8 5 1 1 6 7 2 4 3 2];
SearchTimes = 11;

Func_DPAContest(WaveDir, FileListName, OutFileIndex, TargetRange, TraceCount,...
 IterationThreshold, CompRatio, SearchMode, SearchOrder, SearchTimes);
