# sbox_breaker.py implements some methods to find the subkey used in a sbox
# to encrypt some messages, using the DES algorithm, according to the
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

from key_estimator import key_estimator

# Only for test:
from traces_database import traces_database
from constants       import __TABLE__
from parse_binary    import *

class sbox_breaker:
	"""
	Provides methods to break a sbox, given some traces.
	"""
	__key_estimators= None
	__best_key= None

	def __init__(self, sbox):
		"Builds 64 key_estimator, one for each possible subkey for the given sbox"
		self.__key_estimators= map( lambda x:key_estimator(sbox, x), range(64) )

	def process(self, msg, trace):
		"""
		Process the given trace for the given message.
		Updates the best key (instance member)
		"""
		# Computing differential traces and the best key at that point
		# (i.e. given the number of traces consumed from the database so far)
		for key_estimator in self.__key_estimators:
			key_estimator.process( msg, trace )
		self.__best_key= None
	
	def get_keys(self):
		"Return vector of best keys candidates"
		if self.__best_key == None:
			keys_marks= map( lambda i: [i,self.__key_estimators[i].get_mark()], range(64) )
			from  operator import itemgetter # For sorting by key
			keys_marks.sort( reverse=True, key=itemgetter(1) ) # Sort by good hypothesis
			self.__best_keys= map( lambda x: x[0], keys_marks ) # Extracts best keys order
		return self.__best_keys

def test():
	sb= sbox_breaker( 1 )
	tdb= traces_database(__TABLE__)
	for i in range(10):
		msg, crypt, trace= tdb.get_trace()
		sb.process(msg, parse_binary(trace))
		best_key= sb.get_keys()[0]
		print "processed trace:", i, "- best key is:", best_key

if __name__ == "__main__":
	test()
