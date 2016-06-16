import des_block
from constants import __BIT__
# Only for test:
import traces_database
from constants import __TABLE__

from NativeDes import Trace
from key_estimator import key_estimator

class key_estimator2 (key_estimator):
	"""
	Provides methods to give a mark to the key relatively to the probability of
	the correctness of the key.
	"""
	
	def partition(self, msg):
		"""
		Return the estimated partition (True or False) of the message,
		according to the current sbox and the current key.
		The partitioning is done with respect to P. Kocher's original
		`difference of means' algorithm applied on the first round
		(using the sbox output bit #0 (MSB) if __BIT__=0).
		"""
		ip= msg
		l0= ip.subblock(0,32)
		r0= ip.subblock(32,64)
		e0= r0.e().subblock(self.sbox*6, (self.sbox+1)*6)
		s0= l0.xor(r0).p(-1).subblock(self.sbox*4, (self.sbox+1)*4)
		return e0.xor(self.key).s(self.sbox).xor(s0).get(__BIT__)