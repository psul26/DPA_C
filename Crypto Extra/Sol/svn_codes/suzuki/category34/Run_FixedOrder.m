%-- (C) COPYRIGHT 2009 MITSUBISHI ELECTRIC CORPORATION --
%-- File Name : Run_FixedOrder.m --
%-- Type : Script M-file --
%-- Author : :Daisuke SUZUKI --
%-- Version : 0 --

clear

WaveDir = 'D:\DPAContest\secmatv1_2006_04_0809\'; % modify here
FileListName = 'db-order.txt';  % modify here

OutFileIndex = 'FixedOrder';
TargetRange = [5725 5775;14475 14525];
TraceCount = 21;
IterationThreshold = 100;
CompRatio = 5;
SearchMode = [1 1 1];
SearchOrder(1,:) = [1 0 0 0 1 1 0 1 1 1 0 0 0];
SearchOrder(2,:) = [1 2 1 8 4 5 6 7 6 8 3 5 7];
SearchTimes = 13;

Func_DPAContest(WaveDir, FileListName, OutFileIndex, TargetRange, TraceCount,...
 IterationThreshold, CompRatio, SearchMode, SearchOrder, SearchTimes);
