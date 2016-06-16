# des_breaker.py implements some methods to extract the key used to encipher
# some messages, using de DES algorithm, according to the provided
# side channel traces.
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

from sbox_breaker import sbox_breaker
import des_block

class des_breaker:
	"""
	Provides tool to break a DES algorithm from given power consumption traces.
	"""
	__sbox_breakers= None
	
	def __init__(self):
		"Builds 8 sbox breakers. One for each sbox."
		self.__sbox_breakers= []
		self.__sbox_broken= []
		for i in range(4):
			self.__sbox_breakers.append( sbox_breaker(2*i) )
			self.__sbox_broken.append( False )
	
	def process(self, msg, trace):
		"""
		Processes the given trace.
		Returns None as long as the key as not been found.
		Returns the key as a hex string when found.
		"""
		for i in range( len(self.__sbox_breakers) ):
			self.__sbox_breakers[i].process( msg, trace )
	
	def get_subkeys(self):
		"""
		Returns a vector of currently best sboxes subkeys.
		This is an array of 4 integers.
		"""
		return map( lambda i:self.__sbox_breakers[i].get_key(), range(4) )
		
	def get_key(self, msg, crypt):
		"""
		Guess the 8 last bits of the full key, using cryptogram.
		Return the key if found, None elsewhere.
		"""
		# Building turn_key
		turn_key= des_block.des_block()
		for sb in self.__sbox_breakers:
			turn_key= turn_key.concat( des_block.__from_int__(sb.get_key(),12) )
		# Computing uncomplete initial_key
		cd1= turn_key.pc2(-1)
		c0 = cd1.subblock(0,28).rs(1)
		d0 = cd1.subblock(28,56).rs(1)
		uncomp_cd0= c0.concat(d0)
		# Trying encipherement with the remaining 256 possible keys
		for i in range(256):
			cd0= uncomp_cd0.fill( des_block.__from_int__(i, 8) )
			key= cd0.pc1(-1).fill( des_block.__from_int__(0, 8) )
			cip= des_block.des_block(msg, 64).encipher(key)
			if cip.value() == des_block.des_block(crypt,64).value():
				return key
		return None
	
	def get_indexes():
		for sb in __self.sbox_breakers:
			sb.get_index()

def test():
	db= des_breaker()
	# Setting good subkeys
	db._des_breaker__sbox_breakers[0]._sbox_breaker__best_key= 56*64+11
	db._des_breaker__sbox_breakers[1]._sbox_breaker__best_key=  59*64+38  #11
	db._des_breaker__sbox_breakers[2]._sbox_breaker__best_key= 0*64+13 #59
	db._des_breaker__sbox_breakers[3]._sbox_breaker__best_key= 25*64+55  # 38
	#db._des_breaker__sbox_breakers[4]._sbox_breaker__best_key= 0
	#db._des_breaker__sbox_breakers[5]._sbox_breaker__best_key= 13
	#db._des_breaker__sbox_breakers[6]._sbox_breaker__best_key= 25
	#db._des_breaker__sbox_breakers[7]._sbox_breaker__best_key= 55
	# Key should be 6A64786A64786A64
	print "key found:", db.get_key("993fa9b70fe852af", "09b1a3ea6377adf2")

if __name__ == "__main__":
	test()
