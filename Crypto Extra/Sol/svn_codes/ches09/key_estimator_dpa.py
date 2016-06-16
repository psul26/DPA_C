# key_estimator.py provides some methods to evaluate the correctness
# of a key used to encipher some clear text with the Data Encryption
# Standard, according to the provided side-channel traces.
# Copyright (C) 2008 Sylvain GUILLEY (Sylvain.GUILLEY@TELECOM-ParisTech.fr)
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# DPACONTEST modules
import des_block
from   constants import *

# Modules only for test
import traces_database

def weight(bit_vector):
	"""
	This private class function maps a multi-bit selection function into a scalar,
	according to targeted bits (see BITS in constants.py).
	We choose here, as in the CPA by Brier, the Hamming weight reduction.
	"""
	w= 0
	for i in BITS:
		if bit_vector[i]: w+= 1
	return w

class key_estimator:
	"""
	Provides methods to give a mark to the key relatively to the probability of
	the correctness of the key.

	Notice: w.r.t. the `difference of means' method, there is no difference in
	        terms of usage of trace's length registers.
	        The `0' and `1' accumulators are replaced by `w' and `hw'.
	"""
	__sbox= None
	__key = None
	__n   = 0    # The accumulated traces count for all partitions
	__h   = 0    # The accumulated Hamming weights
	__w   = None # The accumulated waves
	__hw  = None # The accumulated product `Hamming weights' x `waves'
	__diff= None # The differential trace

	def __partition(self, msg):
		"""
		Return the estimated partition (\in [0,4]) of the message,
		according to the current sbox and the current key.
		The partitioning is done with respect to Brier's original CPA.
		"""
		ip= des_block.des_block(msg, 64).ip()
		l0= ip.subblock(0,32)
		r0= ip.subblock(32,64)
		e0= r0.e().subblock(self.__sbox*6, (self.__sbox+1)*6)
		s0= l0.xor(r0).p(-1).subblock(self.__sbox*4, (self.__sbox+1)*4)

		hd= e0.xor(self.__key).s(self.__sbox).xor(s0) # Full hamming distance vector
		return weight(hd)

	def __init__(self, sbox, key):
		"""
		Initialize the key estimator.
		sbox is a number between 0 and 7 (included)
		key is a number between 0 and 63 (included)
		"""
		self.__sbox= sbox
		self.__key = des_block.__from_int__(key, 6)

	def process(self, msg, trace):
		"Accumulate the given trace according to the given message (msg)"
		h= self.__partition( msg ) # This trace's weight
		self.__n+= 1
		self.__h+= h
		# Work-around to make up for the impossibility to initialize a zero vector of unknown length
		if not self. __w: self. __w= trace
		else            : self. __w= map( lambda a,     b: a+b,       self. __w, trace )
		if not self.__hw: self.__hw= map( lambda      new:     h*new,            trace )
		else            : self.__hw= map( lambda sum, new: sum+h*new, self.__hw, trace )
		self.__diff= None
	
	def get_differential(self):
		"""
		Returns the differential trace accumulated since now
		"""
		if self.__diff == None:
			e_hw= map( lambda w: self.__h*w/self.__n, self.__w ); # Must be divided by n once more
			self.__diff= map( lambda cov0, cov1: (cov0 - cov1)/self.__n, self.__hw, e_hw )
		return self.__diff
	
	def get_mark(self):
		"""
		Return the mark of the key.
		Just the max of the differential trace.
		"""
		# # ----- <DEBUG> -----
		# fd= open("wave_s"+str(self.__sbox)+"_k"+str(self.__key), "w")
		# fd.write( str( self.__diff ) ) # Dumping the differential trace in a file
		# fd.close()
		# # ----- </DEBUG> -----
		if not self.__w: return 0
		return max( map(abs, self.get_differential()) )

def test():
	ke= key_estimator(0, 56) # 56 is the correct key for the sbox #0
	tdb= traces_database.traces_database(TABLE)
	for i in range(10):
		msg, crypt, trace= tdb.get_trace()
		ke.process(msg, trace)
		print "processed trace:", i, "- mark is:", ke.get_mark()

	fd= open("output.csv", "w")
	for f in ke.get_differential():
		fd.write(str(f)+"\n")
	fd.close()

if __name__ == "__main__":
	test()
