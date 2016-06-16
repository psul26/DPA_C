%
% Framework for developping attacks in Matlab under Unix
% for the DPA contest V2
%
% Version 1, 12/05/2010
%
% Guillaume Duc <guillaume.duc@telecom-paristech.fr>
%

% FIFO filenames (TODO: adapt them)

fifo_in_filename = 'fifo_from_wrapper';
fifo_out_filename = 'fifo_to_wrapper';

% Number of the attacked subkey (1: subkey used in the first round, ...,
% 10: subkey used in the last round)
% TODO: adapt it
attacked_subkey = 10;


% Open the two communication FIFO

[fifo_in,msg] = fopen(fifo_in_filename, 'r');
if fifo_in < 0
    error('Cannot open FIFO: %s', msg);
end

[fifo_out,msg] = fopen(fifo_out_filename, 'w');
if fifo_out < 0
    error('Cannot open FIFO: %s', msg);
end

% Retrieve the number of traces

num_traces = fread(fifo_in, 1, '*uint16', 0, 'l');

% Send start of attack string

fwrite(fifo_out, [10 46 10], 'uint8');

% Main iteration
for iteration = 1:num_traces
    % Read trace
    plaintext = fread(fifo_in, 16, '*uint8'); % 16x1 uint8
    ciphertext = fread(fifo_in, 16, '*uint8'); % 16x1 uint8
    samples = fread(fifo_in, 3253, '*int16', 0, 'l'); % 3253x1 int16
    
    % TODO: put your attack code here
    % Your attack code can use:
    % - plaintext (16x1 uint8) : the plaintext
    % - ciphertext (16x1 uint8) : the ciphertext
    % - samples (3253x1 int16) : the samples of the trace
    % And must produce bytes which is a 256 lines x 16 columns array
    % (matrix) where for each byte of the attacked subkey (the columns of
    % the array), all the 256 possible values of this byte are sorted
    % according to their probability (first position: most probable, last:
    % least probable), i.e. if your attack is successful, the value of the
    % key is the first line of the array.
    
    bytes = repmat((0:255)', 1, 16);
    
    
    % Send result
    fwrite(fifo_out, attacked_subkey, 'uint8');
    fwrite(fifo_out, bytes, 'uint8');
end

% Close the two FIFOs
fclose(fifo_in);
fclose(fifo_out);