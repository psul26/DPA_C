# sbox_breaker.py implements some methods to find the subkey used in a sbox
# to encipher some messages, using de DES algorithm, according to the
# provided side channel traces.
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

from math import sqrt
# Only for test:
from traces_database import traces_database
from constants       import TABLE
from struct import *
import des_block

def hw( i ):
	"Hamming weight (number of ones in the binary representation) of integer i"
	res= 0
	while( i ): res+= i&1; i>>=1
	return res

def weight(bit_vector):
	"""
	This private class function maps a multi-bit selection function into a scalar;
	We choose here, as in the CPA by Brier, the Hamming weight reduction.
	"""
	return hw( bit_vector.value() ) # Mere Hamming weight

class sbox_breaker:
	"""
	Provides method to break a sbox, given some traces.
	"""
	__best_key= None
	__sbox = None
	__samples = 0. #Number of samples per trace
	__n   = 0.   # The accumulated traces count for all partitions
	__h   = None   # The accumulated Hamming weights
	__w   = None # The accumulated waves
	__hw  = None # The accumulated product `Hamming weights' x `waves'
	__h2  = None   # The accumulated squared Hamming weights
	__w2  = None # The accumulated squared waves
	__diff= None # The differential trace
	             # (now a correlation trace, but we keep the same name to minimize the changed w.r.t. the DPA version)

	def __init__(self, sbox): # Sbox from 0 to 7
		"Initializes the accumulators for a given S-box"
		self.__sbox = sbox
		self.__h = []
		self.__h2 = []
		self.__hw = []
		self.__diff = []
		for i in range(64):
			self.__h.append(0) # Initialize hw arrays with zeros
			self.__h2.append(0)
			self.__hw.append(None)
	def __partition(self, msg,key):
		"""
		Return the estimated partition (\in [0,8]) of the message,
		according to the current sbox and the current key.
		The partitioning is done with respect to Brier's original CPA.
		"""
		ip= des_block.des_block(msg, 64).ip()
		l0= ip.subblock(0,32)
		r0= ip.subblock(32,64)
		key1 = des_block.__from_int__(key,6)
		e0= r0.e().subblock(self.__sbox*6, (self.__sbox+1)*6)
		s0= l0.xor(r0).p(-1).subblock(self.__sbox*4, (self.__sbox+1)*4)
		return float( weight( e0.xor(key1).s(self.__sbox).xor(s0) ) ) 

	def process(self, msg, trace):
		"""
		Process the given trace for the given message.
		Accumulates the data
		"""
		# Computing differential traces and the best key at that point
		# (i.e. given the number of traces consumed from the database so far)
		
		#Annotate new trace
		self.__n +=1
		#Accumulate signal
		if self.__samples == 0:
			self.__samples = len(trace)
		if not self.__w:
			self.__w= trace
		else:
			 self. __w= map( lambda a,     b: a+b,       self. __w, trace )
		if not self.__w2:
			self.__w2 =  map( lambda a       : a**2,                 trace )
		else:
			self.__w2 = map( lambda a,b: a+b**2, self.__w2, trace)

		for i in range(64): #For each candidate
			
			#Accumulate hamming weights
			h = self.__partition(msg,i)
			#print "HW is: ",h
			self.__h[i] += h
			self.__h2[i] += h**2
			#And product wave x hamming weight
			if not self.__hw[i]:	self.__hw[i]= map( lambda      new:     h*new,            trace )
                	else:	self.__hw[i]= map( lambda sum, new: sum+h*new, self.__hw[i], trace )
		self.__best_key= None

	def __get_differential(self):
		"""
		Returns the differential trace accumulated since now
		"""
		self.__diff = []
		for i in range(64):
			# The covariance
			e_hw= map( lambda w: self.__h[i]*w/self.__n, self.__w ) # Must be divided by n once more
			covhw=map( lambda cov0, cov1: (cov0 - cov1), self.__hw[i], e_hw )
			# The variances
			varw= map( lambda x, x2: ( x2 - x**2/self.__n ), self.__w, self.__w2 )
			varh= ( self.__h2[i] - self.__h[i]**2/self.__n )
			# WARNING: varh and/or some points in varw can be null in early iterations! (n=1, n=2: for sure)
			# The correlation coefficient
			self.__diff.append( map(lambda cov,var: cov/sqrt( var)/sqrt( varh ),covhw,varw))
		return self.__diff
	
	def get_key(self):
		"Gives the current best key, based on a sum of correlations above a threshold"
		if self.__best_key == None:
			diff = self.__get_differential()
			
			self.__best_key = 0
			#Obtain max values for each candidate
			marks = []
			for i in range(64):
                                threshold = 1 / sqrt(self.__n)
                                selected = filter(lambda a: abs(a)>threshold, diff[i])
                                num_selected = len(selected)
                                if num_selected==0:     marks.append(0) #Add just 0 if no result above threshold
                                else:
                                        sum =reduce(lambda a,b:abs(a)+abs(b), selected) - num_selected*threshold
                                        marks.append(sum) #Add the sum as the mark for this candidate
			#And now look for the max inside the marks
			self.__best_key = marks.index( max(marks) )
		return self.__best_key


def test():
	sb= sbox_breaker( 0 )
	tdb= traces_database(TABLE)
	for i in range(300):
		msg, crypt, trace= tdb.get_trace()
		print "processing trace: ",i
		sb.process(msg, trace[5095:5800])
		if i>150 :
			best_key= sb.get_key()
			print "- best key is:", best_key
	key=sb.get_key()
	print "Got key ",best_key
if __name__ == "__main__":
	test()
