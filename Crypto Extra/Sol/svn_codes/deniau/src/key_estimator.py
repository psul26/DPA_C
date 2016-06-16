# key_estimator.py provides some methods to evaluate the correctness
# of a key used to encipher some clear text with the Data Encryption
# Standard, according to the provided side-channel traces.
# Copyright (C) 2008 Florent Flament (florent.flament@telecom-paristech.fr)
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

import des_block
from constants import __BIT__
# Only for test:
import traces_database
from constants import __TABLE__

from NativeDes import Trace

class key_estimator:
	"""
	Provides methods to give a mark to the key relatively to the probability of
	the correctness of the key.
	"""
	sbox= None
	key = None
	
	__p0  = None # The bit=0 estimated partition
	__p1  = None # The bit=1 estimated partition
	__diff= None # The differential trace

	def partition(self, msg):
		"""
		Return the estimated partition (True or False) of the message,
		according to the current sbox and the current key.
		The partitioning is done with respect to P. Kocher's original
		`difference of means' algorithm applied on the first round
		(using the sbox output bit #0 (MSB) if __BIT__=0).
		"""
		ip= des_block.des_block(msg, 64).ip()
		l0= ip.subblock(0,32)
		r0= ip.subblock(32,64)
		e0= r0.e().subblock(self.sbox*6, (self.sbox+1)*6)
		s0= l0.xor(r0).p(-1).subblock(self.sbox*4, (self.sbox+1)*4)
		return e0.xor(self.key).s(self.sbox).xor(s0).get(__BIT__)

	def __init__(self, sbox, key):
		"""
		Initialize the key estimator.
		sbox is a number between 0 and 7 (included)
		key is a number between 0 and 63 (included)
		"""
		self.sbox= sbox
		self.key = des_block.__from_int__(key, 6)

	def process(self, msg, trace):
		"Accumulate the given trace according to the given message (msg)"
		if self.partition(msg):
			if not self.__p1: self.__p1 = Trace(trace)
			else            : self.__p1 += trace
		else:
			if not self.__p0: self.__p0 = Trace(trace)
			else            : self.__p0 += trace
		self.__diff= None
	
	def get_differential(self):
		"""
		Returns the differential trace accumulated since now
		"""
		if self.__diff == None:
			self.__diff= self.__p1.getDifferential(self.__p0)
			self.__diff.thisown = True
		return self.__diff
	
	def get_mark(self):
		"""
		Return the mark of the key.
		Just the max of the differential trace.
		"""
		if not self.__p0 or not self.__p1 : return 0
		return self.get_differential().getMark()

def test():
	ke= key_estimator(0, 56) # 56 is the correct key for the sbox #0
	tdb= traces_database.traces_database(__TABLE__)
	for i in range(10):
		msg, crypt, trace= tdb.get_trace()
		ke.process(msg, trace)
		print "processed trace:", i, "- mark is:", ke.get_mark()

	fd= open("output.csv", "w")
        values = ke.get_differential().getValues()
        values.thisown= True
	fd.write(values)
	fd.close()

if __name__ == "__main__":
	test()
