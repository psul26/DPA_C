# Function allowing conversion from C binary data
from struct import unpack
import os


def parse_binary( raw_data ):
	"""
	Takes a raw binary string containing data from our oscilloscope.
	Returns the corresponding float vector.
	"""
	ins =  4   # Size of int stored if the raw binary string
	cur =  0   # Cursor walking in the string and getting data
	cur += 12  # Skipping the global raw binary string header
	whs =  unpack("i", raw_data[cur:cur+ins])[0] # Storing size of the waveform header
	cur += whs # Skipping the waveform header
	dhs =  unpack("i", raw_data[cur:cur+ins])[0] # Storing size of the data header
	cur += dhs # Skipping the data header
	bfs =  unpack("i", raw_data[cur-ins:cur])[0] # Storing the data size
	sc  =  bfs/ins # Samples Count - How much samples compose the wave
	dat =  unpack("f"*sc, raw_data[cur:cur+bfs])
	return dat

class traces_database:
	""" Class providing database IOs """
	__folder_name = None

	def __init__(self, folder_name):
		""" No arguments needed """
		self.__folder_name = folder_name;

	def get_trace(self):
		"""
		Do not take any argument.
		Returns the next triplet (message, cipher, trace), where:
		 - message is an ascii string containing a 64 bits clear message in hexadecimal,
		 - cipher is an ascii string containing a 64 bits ciphered message in hexadecimal,
		 - trace is a float vector containing a trace during the cipher operation.
		"""
		# List all files in the folder containing traces
		files = [f for f in os.listdir('../' + self.__folder_name) if not os.path.isdir(f)]

		for f in files:
			name, info = f.split('__') # split the file name into name, and info
			key, msg, crypt = info.split('_') # split the file name at the "_" mark
			key = key[2:] # remove "k=" from the key portion of the name
			msg = msg[2:] # remove "m=" from the message portion of the name
			crypt = crypt[2:-4] # remove "c=" and ".bin" from the crypt portion of the name
			raw_data = None

			full_path = '../' + self.__folder_name + '/' + f
			with open(full_path, 'rb') as file_content: # the "rb" flag is crucial (read binary)
				raw_data = file_content.read()  # read contents of the binary file

			# This creates a generator from our function so that we perform the file listing step only once.
			# Calling the get_trace() only executes one loop of the for loop every time.
			yield msg, crypt, parse_binary(str(raw_data))

def test():
	tdb = traces_database("secmatv1_2006_04_0809")

	traces = tdb.get_trace()
	for i in range(10):
		msg, crypt, data = traces.next()
		print ("msg=%s c=%s") % (msg, crypt)


if __name__ == "__main__":
	test()
