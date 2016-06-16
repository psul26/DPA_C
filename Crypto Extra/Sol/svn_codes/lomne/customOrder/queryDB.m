function [key,PTI,CTO,trace] = queryDB(queryType,query)
% use : queryDB(queryType,query)
%
% Author : Victor LOMNE - victor.lomne@lirmm.fr
%
% queryType : message / cryptogram / index
%
% query : value of the query

DBname = 'production_traces';
user = 'guest';
pass = 'guest';
driver = 'org.postgresql.Driver';
url = 'jdbc:postgresql://dpa.enst.fr/';
table = 'secmatv1_2006_04_0809';

% open a connection to the database
conn = database(DBname,user,pass,driver,url);

% test the connection
if(isconnection(conn) == 0)
    disp('connection to the database failed !');
    exit;
end;

% create a string for the request
if(strcmp('index',queryType) == 1)
    request = sprintf('select key,message,cryptogram from %s where index=%d',table,query);
    request2 = sprintf('select filecontent from %s where index=%d',table,query);
else
    if(strcmp('message',queryType) == 1)
        request = sprintf('select key,message,cryptogram from %s where message=''%s''',table,query);
        request2 = sprintf('select filecontent from %s where message=''%s''',table,query);
    else
        if(strcmp('message',queryType) == 1)
            request = sprintf('select key,message,cryptogram from %s where cryptogram=''%s''',table,query);
            request2 = sprintf('select filecontent from %s where cryptogram=''%s''',table,query);
        end;
    end;
end;

% get data
data = cell2mat(fetch(conn,request));
traceBin = cell2mat(fetch(conn,request2));

% copy each data
keyHex = data(1,1:16);
PTIHex = data(1,17:32);
CTOHex = data(1,33:48);

key = zeros(1,8);
PTI = zeros(1,8);
CTO = zeros(1,8);

for i = 1 : 8
    key(1,i) = hex2dec(keyHex(1,2*i-1:2*i));
    PTI(1,i) = hex2dec(PTIHex(1,2*i-1:2*i));
    CTO(1,i) = hex2dec(CTOHex(1,2*i-1:2*i));
end;

% initialize the cursor
cursor = 1;
% jump the file header
cursor = cursor + 12;
% get the waveform header
wvHeader = bytesArray2int32(traceBin(cursor:cursor+3,1));
% jump the waveform header
cursor = cursor + wvHeader;
% get the data header
dataHeader = bytesArray2int32(traceBin(cursor:cursor+3,1));
% jump the data header
cursor = cursor + dataHeader;
% get the data size
floatSize = bytesArray2int32(traceBin(cursor-4:cursor-1,1));
% compute the data size in integer
L = floatSize / 4 - 3;

% preallocate memory for trace
trace = zeros(1,L);

% for each point convert it from array of bytes in float
for i = 1 : L
    trace(1,i) = bytesArray2float(traceBin(cursor+(i-1)*4:cursor+i*4,1));
end;

% close the connection
close(conn);

% -------------------------------------------------------------------------
function value = bytesArray2int32(bytesArray)

bytesArray = int32(bytesArray);

% convert bytesArray in bits following little endian representation & signed int8 representation   
if(bytesArray(4,1) < 0)
    bytesArray(4,1) = bytesArray(4,1) + 256;
end;
bytesArrayBin(1,1:8) = dec2bin(abs(bytesArray(4,1)),8);

if(bytesArray(3,1) < 0)
    bytesArray(3,1) = bytesArray(3,1) + 256;
end;
bytesArrayBin(1,9:16) = dec2bin(abs(bytesArray(3,1)),8);

if(bytesArray(2,1) < 0)
    bytesArray(2,1) = bytesArray(2,1) + 256;
end;
bytesArrayBin(1,17:24) = dec2bin(abs(bytesArray(2,1)),8);

if(bytesArray(1,1) < 0)
    bytesArray(1,1) = bytesArray(1,1) + 256;
end;
bytesArrayBin(1,25:32) = dec2bin(abs(bytesArray(1,1)),8);

% convert 32 bits array in signed int32
value = bin2dec(num2str(bytesArrayBin(1,2:32)));
if(str2double(bytesArrayBin(1,1)) == 1)
    value = - value;
end;

% -------------------------------------------------------------------------
function value = bytesArray2float(bytesArray)

bytesArray = int32(bytesArray);

% convert bytesArray in bits following little endian representation & signed int8 representation   
if(bytesArray(4,1) < 0)
    bytesArray(4,1) = bytesArray(4,1) + 256;
end;
bytesArrayBin(1,1:8) = dec2bin(abs(bytesArray(4,1)),8);

if(bytesArray(3,1) < 0)
    bytesArray(3,1) = bytesArray(3,1) + 256;
end;
bytesArrayBin(1,9:16) = dec2bin(abs(bytesArray(3,1)),8);

if(bytesArray(2,1) < 0)
    bytesArray(2,1) = bytesArray(2,1) + 256;
end;
bytesArrayBin(1,17:24) = dec2bin(abs(bytesArray(2,1)),8);

if(bytesArray(1,1) < 0)
    bytesArray(1,1) = bytesArray(1,1) + 256;
end;
bytesArrayBin(1,25:32) = dec2bin(abs(bytesArray(1,1)),8);

% convert 32 bits array in single precision float
exponent = bin2dec(num2str(bytesArrayBin(1,2:9)));

% initialize value following value of exponent
if(exponent == 0)
    value = 0;
else
    value = 1;
end;

% compute the mantissa
buf = 1;
for i = 1 : 23
    buf = buf / 2;
    value = value + str2double(bytesArrayBin(1,9+i)) * buf;
end;

% check exponent
if(exponent == -127)
    exponent = -126;
end;
if(exponent == 128)
    exponent = 127;
end;

% compute the value
value = value * 2^(exponent - 127);

% add the sign
if(str2double(bytesArrayBin(1,1)) == 1)
    value = - value;
end;