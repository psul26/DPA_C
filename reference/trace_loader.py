# Function allowing conversion from C binary data
from struct import unpack
import os
import random
from constants import HW_val
from constants import HW_count
a_list = []
newFiles =[]
listOfLists =[]

for i in range(64+1):
	listOfLists.append([a_list])



def hamming_weight_fcn(str_message):
	str_message = int(str_message, 16)
	hamming_weight = 0
	bin_rep = str(bin(str_message))
	bin_rep = bin_rep[2:]
	for i in range(len(bin_rep)):
		if bin_rep[i] == '1':
			hamming_weight += 1
	return hamming_weight

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
	#print str(dat) + '\n'
	return dat

class traces_database:
	""" Class providing database IOs """
	__folder_name = None

	def __init__(self, folder_name):
		""" No arguments needed """
		self.__folder_name = folder_name;

	def get_trace(self):
		first = -1
		for h in os.listdir('/Users/patricksullivan/Documents/DPAcontest/secmatv1_2006_04_0809'):
			first += 1
			if first > 0:# and first%3 == 0:

				#newFiles.append(h)
				name, info = h.split('__') # split the file name into name, and info
				key, msg, crypt = info.split('_') # split the file name at the "_" mark
				msg = msg[2:]
				HW = hamming_weight_fcn(msg)
				
				if HW == 31:
					newFiles.append(h)
				# elif HW < 21:
				# 	newFiles.append(h)
				# elif HW == 28:
				# 	newFiles.append(h)
				elif HW == 34:
					newFiles.append(h)
				# elif HW == 37:
				# 	newFiles.append(h)
				# elif HW == 33:
				# 	newFiles.append(h)

		
			

		"""
		Do not take any argument.
		Returns the next triplet (message, cipher, trace), where:
		 - message is an ascii string containing a 64 bits clear message in hexadecimal,
		 - cipher is an ascii string containing a 64 bits ciphered message in hexadecimal,
		 - trace is a float vector containing a trace during the cipher operation.
		"""
		# List all files in the folder containing traces
		# first = -1
		# for k in os.listdir('/Users/patricksullivan/Documents/DPAcontest/secmatv1_2006_04_0809'):
		# 	first += 1
		# 	if first > 0 and first % 3 == 0:
		# 		newFiles.append(k)
		print ('new files length '+str(len(newFiles)))
		# for i in newFiles:
		# 	if i == '[]':
		# 		newFiles.remove(i)
		# for i in newFiles:
		# 	print i
		#random.shuffle(newFiles)
		for f in newFiles:
			name, info = f.split('__') # split the file name into name, and info
			key, msg, crypt = info.split('_') # split the file name at the "_" mark
			key = key[2:] # remove "k=" from the key portion of the name
			msg = msg[2:] # remove "m=" from the message portion of the name
			crypt = crypt[2:-4] # remove "c=" and ".bin" from the crypt portion of the name
			raw_data = None
			trace_data = None
			full_path = '/Users/patricksullivan/Documents/DPAcontest/secmatv1_2006_04_0809' + '/' + f
			with open(full_path, 'rb') as file_content: # the "rb" flag is crucial (read binary)
				raw_data = file_content.read()  # read contents of the binary file
				trace_data = parse_binary(str(raw_data))
			yield msg, crypt, trace_data[5700:5900]

def test():
	tdb = traces_database("secmatv1_2006_04_0809")

	traces = tdb.get_trace()
	print "done"

if __name__ == "__main__":
	test()
