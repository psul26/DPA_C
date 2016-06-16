# des_breaker12.py implements some methods to extract the key used to encipher
# some messages, using de DES algorithm, according to the provided
# side channel traces.
# Copyright (C) 2008 Florent Flament (florent.flament@telecom-paristech.fr)
# Copyright (C) 2009 Laurent SAUVAGE (laurent.sauvage@telecom-paristech.fr)
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
from   n_sbox_breaker import n_sbox_breaker
import des_block
from   constants    import *

class des_breaker12:
	"""
	Provides tool to break a DES algorithm from given power consumption traces.
	"""
	
	def __init__(self, known=[]):
		"""
		Builds one breaker per SBox couple to attack
		(depends on the parameter SB_LST, see constant.py).
		"""
		if DEBUG:
			print "Constructor", self

		# Builds list of SBoxes which have been broken
		self.__sbox_broken= map( lambda sbox_key: sbox_key[0], known )

		# Builds objects
		self.__n_sbox_breakers= [] # List to store objst instances
		self.__sbox_couples=    [] # List to store sbox couples

		if len(known)<7: # More than two sbox to break?
			for i in SB_LST: # For each SBox to break
				for j in range(8): # and for each of its possible sbox partner
					first, second= min(i,j), max(i,j) # Rearrange by increasing order
																 # to discard same couples: (0,1) == (1,0)
	
					known_cp= list( known )# Makes HARD copy of known
					if first != second and [first,second] not in self.__sbox_couples: # If not the same sbox
																											# and not a couple already created
						if first not in self.__sbox_broken and second not in self.__sbox_broken: # If both are not broken...
							known_cp.append( [first, None] ) # ...use both (12 bit attack)
							known_cp.append( [second,None] )
							self.__n_sbox_breakers.append( n_sbox_breaker( known_cp ) ) # Create n_sbox_breakers
							self.__sbox_couples.append( [first,second] )
		
		else: # It lefts only one sbox to break
			known_cp= list( known ) # Makes HARD copy of known
			for i in range(8):
				if i not in self.__sbox_broken:
					known_cp.append( [i,None] )
			self.__n_sbox_breakers.append( n_sbox_breaker( known_cp ) ) # Create n_sbox_breakers

	def __del__(self):
		if DEBUG:
			print "Destructor", self
		for nsb in self.__n_sbox_breakers:
			del nsb
		del self.__n_sbox_breakers, self.__sbox_couples 

	def process(self, msg, trace):
		"""
		Processes the given trace.
		Returns None as long as the key as not been found.
		Returns the key as a hex string when found.
		"""
		for n_sbox_breaker in self.__n_sbox_breakers:
			n_sbox_breaker.process( msg, trace )
	
	def get_subkeys(self):
		"""
		Returns a vector of currently best sboxes subkeys,
		according to targeted SBoxes (see SB_LST of constant.py).
		"""
		subkeys= [] # Empty list to store sbox_subkeys values

		# Retrieves candidates for each sbox
		candidates= []
		for nsb in self.__n_sbox_breakers:
			for sbox_key_list in nsb.get_keys()[:NBK]:
				for sbox_key in sbox_key_list:
					candidates.append( sbox_key )

		# Sorts them
		for sbox in SB_LST:
			stats= {} # Empty dictionnary to store stats on subkey candidates
			for candidate in candidates:
				if candidate[0] == sbox:
					stats[ candidate[1] ]= stats.get( candidate[1], 0 ) + 1				

			# Store best candidate for current SBox
			inv_stats= {} # Empty dictionnary to store inverse of first_stats
			for k,v in stats.items(): # Builds inverse dictionnary
				inv_stats[v]= k
			subkeys.append( [ sbox, inv_stats[ max( stats.values() ) ] ] )

		return subkeys

	def get_key(self, msg, crypt, sk_lst):
		"""
		Guess the 8 last bits of the full key, using cryptogram.
		Return the key if found, None elsewhere.
		"""
		# Building turn_key
		turn_key= des_block.des_block()
		for subkey in sk_lst:
			turn_key= turn_key.concat( des_block.__from_int__(subkey,6) )
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
		print "!!WARNING!! Wrong key !!"
		return key # We return the key, even if bad

if __name__ == "__main__":

	db12_0= des_breaker12()
	db12_1= des_breaker12( [ [0,55] ] ) 
	db12_2= des_breaker12( [ [0,55], [1,11], [2,12], [3,53], [4,21], [5,32] ] ) 
	db12_3= des_breaker12( [ [0,55], [1,11], [2,12], [3,53], [5,21], [6,32], [7,0] ] ) 
