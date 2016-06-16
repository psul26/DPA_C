+-------------------------------------------------+
| Using matlab with the attack wrapper under UNIX |
+-------------------------------------------------+

mkfifo matlab_i;
mkfifo matlab_o;

0>matlab_i;
matlab_o>1;

/comelec/softs/opt/matlab/2009b/bin/matlab -nodisplay -nosplash -r attack.m

# attack.m:
# Example of prototyping code...
>> fi = fopen( 'matlab_i', 'r' );
>> fo = fopen( 'matlab_o', 'w' );
>> fprintf( fo, 'Hello world!' );
>> fread( fi, 5 )                
>> quit
