# des_block.py implements the DES cryptographic algorithm
# allowing to compute any intermediate value.
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

# Some DES specific constants

import NativeDes

__MSG_SIZE__ = 64 # DES messages are 64 bits long

def __from_int__( i_msg, nbits ):
	"""
	Builds a des_block on nbits bits from the given i_msg int.
	Should only be used by internal functions.
	"""
	res = des_block()
	res.value = i_msg;
	res.length = nbits;
	return res;

def int2bin(n, count=24):
        #
        """returns the binary of integer n, using count number of digits"""
        #
        return "".join([str((n >> y) & 1) for y in range(count-1, -1, -1)])
        
class des_block:
	"""
	Class providing every operations done to a message in the DES datapath.
	"""

	# Some variables
	length = 0
	value = 0L
	native = NativeDes.DesBlock();
			
	def __init__(self, hex_msg=None, nbits=0):
		"""
		Constructor takes the message from which the des_block will be initialized.
		It must be an hexadecimal string representation of a 64 bits message.
		Example: 0f564a334af654ab
		"""
		if hex_msg:
			self.value = int(hex_msg, 16)
			self.length = nbits

	def subblock(self, begin, end):
		"Returns the subblock of data between begin and end (not included)."
		return __from_int__((self.value >> (self.length - end)) & ((1 << (end-begin))-1), end-begin) 

	def ip(self, direc=1):
		"""
		Returns a des_block resulting of the application of the
		IP (Initial Permutation) function on the current des_block
		"""
		return __from_int__(self.native.applyTable(self.value, 64, self.native.bIP1, 64, direc), 64)

	def e(self):
		"""
		Returns a des_block resulting of the application of the
		E (Expansion) function on the current des_block
		"""
		return __from_int__(self.native.applyTable(self.value, 32, self.native.bE, 48, 1),
                                    48)

	def xor(self, db):
		"Returns the results of __data xor db."""
		return __from_int__(self.value ^ db.value, self.length)
		
	def key_or(self, db):
		"Returns the results of __data or db."""
		return __from_int__(self.value | db.value, self.length)
			
	def s(self, n_sbox):
		"""
		Returns the output of the n_sbox sbox with des_block data for
		its inputs.
		"""
		return __from_int__(self.native.applyS(n_sbox, self.value), 4)

	def p(self, direc=1):
		"""
		Apply the P permutation to this des_block and returns the result.
		dir is the direction in which to apply the permutation.
		1 for the forward direction
		-1 for the backward direction
		Defaults to forward.
		"""
		return __from_int__(self.native.applyTable( self.value, 32, self.native.bP, 32, direc), 32)
		

	def pc1(self, direc=1):
		"Apply the PC1 permutation and returns the result."
		if (direc == 1):
			inputLength = 64
			outputLength = 56
		else:
			inputLength = 56
			outputLength = 64
		return __from_int__(self.native.applyTable( self.value, inputLength, self.native.bPC1, outputLength, direc), outputLength)

	def pc2(self, direc=1):
		"Apply the PC2 permutation and returns the result."
		if (direc == 1):
			inputLength = 56
			outputLength = 48
		else:
			inputLength = 48
			outputLength = 56
		return __from_int__(self.native.applyTable( self.value, inputLength, self.native.bPC2, outputLength, direc), outputLength)

	def ls(self, n):
		"Returns the result of left shifting the data by n bits."
		if (self.length != 28):
			print "ls with length ", self.length
		return __from_int__( ((self.value << n) & 0xFFFFFFF) | (self.value >> (28-n)), 28  )

	def rs(self, n):
		"Returns the result of right shifting the data by n bits."
		return __from_int__( ((self.value << (28-n)) & 0xFFFFFFF) | (self.value >> n), 28)

	def f(self, k):
		"Returns the result of the f function for the given k key."
		bs= self.e().xor(k)
		res= des_block()
		for i in range(0,8):
			res= res.concat( bs.subblock(6*i,6*i+6).s(i) )
		return res.p()

	def concat(self, db):
		"Return the concatenated des_block between self and db."
		return __from_int__( (self.value << db.length) | db.value, self.length + db.length )

	def hw(self):
		"Returns the hamming weight of the block"
		res= 0
		v = self.value
		while v != 0:
			res += (v&1)
			v >>= 1
		return res

	def get(self, n):
		"Return the n'th element of the block"
		return (self.value >> (self.length - n - 1)) & 1

	def encipher(self, key):
		"""
		Encipher the block as a clear message into a cryptogram.
		According to the given key, with the DES algorithm.
		"""
		kshifts= ( 0, 1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1 )
		cd= [key.pc1()]
		ki= [0]
		for i in range(1,17):
			c= cd[i-1].subblock(0,28).ls(kshifts[i])
			d= cd[i-1].subblock(28,56).ls(kshifts[i])
			cd.append( c.concat(d) )
			ki.append( cd[i].pc2() )
		lr= self.ip()
		for i in range(1,17):
			l= lr.subblock(0,32)
			r= lr.subblock(32,64)
			lr= r.concat( l.xor(r.f(ki[i])) )
		return lr.subblock(32,64).concat(lr.subblock(0,32)).ip(-1)

	def __len__(self):
		return self.length;
	
	def __str__(self):
		"""
		Returns the string representation in hexadecimal of the data stored
		in the structure.
		"""
		return hex(self.value)

def test():
	msg= des_block("48656c6c6f202121", 64)
	key= des_block("6b65796b65796b65", 64)
	print "msg", msg
	print "key", key
	# Cryptogram should be 71E36B810C097F33
	print "cryptogram", msg.encipher(key)

if __name__ == "__main__":
	test()
