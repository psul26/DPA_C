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

from constants import __SB_LST__
from constants import __SK_R1__

class des_breaker:
	"""
	Provides tool to break a DES algorithm from given power consumption traces.
	"""
	__sbox_breakers= None
	
	def __init__(self):
		"""
		Builds one breaker per attacked sbox
		(depends on the parameter SB_LST, see constant.py).
		"""
		self.__sbox_breakers= []
		self.__sbox_broken= []
		for i in __SB_LST__:
			self.__sbox_breakers.append( sbox_breaker(i) )
			self.__sbox_broken.append( False )

	def process(self, msg, trace):
		"""
		Processes the given trace.
		Returns None as long as the key as not been found.
		Returns the key as a hex string when found.
		"""
		for sbox_breaker in self.__sbox_breakers:
			sbox_breaker.process( msg, trace )
	
	def get_subkeys(self):
		"""
		Returns a vector of currently best sboxes subkeys.
		This is an array of 8 integers.
		"""
		subkeys= map( lambda sb_b: sb_b.get_keys(), self.__sbox_breakers )
		# <TEST>
		#fd=open("right_R1subkeys_pos.txt",'a') # Opens file to store right subkeys positions
		#rkpos= [] # Empty list to store right subkeys position for current trace
		#for sb in range(8):
		#	rkpos.append( subkeys[sb].index( __SK_R1__[sb] ) )
		#fd.write( str(rkpos)+'\n' )
		#fd.close()
		# <\TEST>
		return map( lambda sks: sks[0], subkeys ) # Return only one best candidate per SBox
		
	def get_key(self, msg, crypt):
		"""
		Guess the 8 last bits of the full key, using cryptogram.
		Return the key if found, None elsewhere.
		"""
		# Building turn_key
		turn_key= des_block.des_block()
		for sb in self.__sbox_breakers:
			turn_key= turn_key.concat( des_block.__from_int__(sb.get_keys()[0],6) )
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
			if cip == des_block.des_block(crypt,64):
				return key
		return None

def test():
	db= des_breaker()
	# Setting good subkeys
	db._des_breaker__sbox_breakers[0]._sbox_breaker__best_key= 56
	db._des_breaker__sbox_breakers[1]._sbox_breaker__best_key= 11
	db._des_breaker__sbox_breakers[2]._sbox_breaker__best_key= 59
	db._des_breaker__sbox_breakers[3]._sbox_breaker__best_key= 38
	db._des_breaker__sbox_breakers[4]._sbox_breaker__best_key= 0
	db._des_breaker__sbox_breakers[5]._sbox_breaker__best_key= 13
	db._des_breaker__sbox_breakers[6]._sbox_breaker__best_key= 25
	db._des_breaker__sbox_breakers[7]._sbox_breaker__best_key= 55
	# Key should be 6A64786A64786A64
	print "key found:", db.get_key("993fa9b70fe852af", "09b1a3ea6377adf2")

if __name__ == "__main__":
	test()
