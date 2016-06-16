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
from sbox_breaker2 import sbox_breaker2
import des_block

class des_breaker:
	"""
	Provides tool to break a DES algorithm from given power consumption traces.
	"""
	__sbox_breakers= None
	__sbox_breakers2= None
	__first_subkey= None
	
	def __init__(self):
		"Builds 8 sbox breakers: One for each sbox of the first round, and four more for the second round."
		self.__sbox_breakers= []
		self.__sbox_breakers2= []
		self.__sbox_broken= []
		for i in range(8):
			self.__sbox_breakers.append( sbox_breaker(i) )
			self.__sbox_broken.append( False )
	
	def process(self, msg, trace):
		"""
		Processes the given trace.
		Returns None as long as the key as not been found.
		Returns the key as a hex string when found.
		"""
		for i in range( len(self.__sbox_breakers) ):
			self.__sbox_breakers[i].process( msg, trace )
	
	def process2(self, msg, trace):
	  	"""
		Idem, for second round
		"""
		intermediary= des_block.des_block(msg, 64)
  		l0= intermediary.subblock(0, 32)
  		r0= intermediary.subblock(32,64)
		l0= r0.concat( l0.xor( r0.f(self.__first_subkey) ) )
		intermediary= l0.subblock(32, 64).concat( l0.subblock(0, 32) )

		for i in range( len(self.__sbox_breakers2) ):
			self.__sbox_breakers2[i].process( intermediary, trace )
	
	def get_subkeys(self):
		"""
		Returns a vector of currently best sboxes subkeys.
		This is an array of 8 integers.
		"""
		return map( lambda i:self.__sbox_breakers[i].get_key(), range(8) )

	def get_subkeys2(self):
	  	"""
		Idem for round 2
		"""
		return map( lambda i:self.__sbox_breakers2[i].get_key(), range(6) )

	def __begin_second_round__(self):
		"""
		Calcul des bits deja connus au tour 2
		"""
		turn_key= des_block.des_block()
		for sb in self.__sbox_breakers:
			turn_key= turn_key.concat( des_block.__from_int__(sb.get_key(),6) )
#		turn_key= des_block.des_block("FFFFFFFFFFFF", 48)
		print("Premiere sous-cle : " + str(turn_key))
		self.__first_subkey= turn_key
		cd1= turn_key.pc2(-1)
		c0 = cd1.subblock(0,28).ls(1)
		d0 = cd1.subblock(28,56).ls(1)
		subkey2= c0.concat(d0).pc2(1)
		for i in [0,1,2,4,6,7]:
			print("Attaque de la boite S" + str(i) + " avec la cle partielle " + str(subkey2.subblock(6*i, 6*i+6).value))
		  	self.__sbox_breakers2.append( sbox_breaker2(i, subkey2.subblock(6*i, 6*i+6).value) )
	
	def recover_full_key(self, subkeys, subkeys2):
  		"""
		Builds the master key using the first two subkeys
		"""
		print("Construction de la cle")
		# Building an uncomplete key using subkeys
		turn_key= des_block.des_block()
		for sb in subkeys:
			turn_key= turn_key.concat( des_block.__from_int__(sb, 6) )
		cd1= turn_key.pc2(-1)
		c0 = cd1.subblock(0,28).rs(1)
		d0 = cd1.subblock(28,56).rs(1)
		uncomp1= c0.concat(d0)
  		# Building another uncomplete key using subkeys2
  		turn_key= des_block.des_block()
  		turn_key= turn_key.concat(des_block.__from_int__(subkeys2[0], 6))
  		turn_key= turn_key.concat(des_block.__from_int__(subkeys2[1], 6))
  		turn_key= turn_key.concat(des_block.__from_int__(subkeys2[2], 6))
		turn_key= turn_key.concat(des_block.des_block("0", 6) )
  		turn_key= turn_key.concat(des_block.__from_int__(subkeys2[3], 6))
		turn_key= turn_key.concat(des_block.des_block("0", 6) )
  		turn_key= turn_key.concat(des_block.__from_int__(subkeys2[4], 6))
  		turn_key= turn_key.concat(des_block.__from_int__(subkeys2[5], 6))
  		cd1= turn_key.pc2(-1)
		c0 = cd1.subblock(0,28).rs(2)
		d0 = cd1.subblock(28,56).rs(2)
		uncomp2= c0.concat(d0)
  		# Merging the two subkeys
  		print("Premiere cle incomplete : " + des_block.int2bin(uncomp1.value,56))
		print("Deuxieme cle incomplete : " + des_block.int2bin(uncomp2.value,56))
  		key= des_block.des_block()
  		key= uncomp1.key_or( uncomp2)
  		print("Cle finale :              " + des_block.int2bin(key.value,    56))
  		return key


def test():
	db= des_breaker()
	# Setting good subkeys
	db._des_breaker__sbox_breakers[0].best_key= 56
	db._des_breaker__sbox_breakers[1].best_key= 11
	db._des_breaker__sbox_breakers[2].best_key= 59
	db._des_breaker__sbox_breakers[3].best_key= 38
	db._des_breaker__sbox_breakers[4].best_key= 0
	db._des_breaker__sbox_breakers[5].best_key= 13
	db._des_breaker__sbox_breakers[6].best_key= 25
	db._des_breaker__sbox_breakers[7].best_key= 55
	# Key should be 6A64786A64786A64
	print "key found:", db.get_key("993fa9b70fe852af", "09b1a3ea6377adf2")

if __name__ == "__main__":
	test()
