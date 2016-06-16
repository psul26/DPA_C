%module NativeDes
%{
#include "des_block.h"
#include "trace.h"
%}

%include "des_block.h"
%include "trace.h"

%newobject Trace::getDifferential(Trace);