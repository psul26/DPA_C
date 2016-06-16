# n_sbox_breaker.py implements some methods to find the subkey used in a sbox
# to encrypt some messages, using the DES algorithm, according to the
# provided side channel traces.
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

# Global modules 
from operator import itemgetter # For sorting list

# DPACONTEST modules
from constants import *
if   PPA=="dpa":
	from n_key_estimator_dpa import n_key_estimator 
elif PPA=="cpa":
	from n_key_estimator_cpa import n_key_estimator 

# Modules only for test
from traces_database import traces_database

class n_sbox_breaker:
	"""
	Provides methods to break n sbox, given some traces.
	"""

	def __init__(self, sbox_key_list):
		"""
		Builds n_key_estimators according to unknown keys (key=None) of "sbox_key_list"
		"""
		if DEBUG:
			print "Constructor", self, " - sbox_key_list=", sbox_key_list

		self.__n_key_estimators= []
		self.__best_key= None

		# Splits "sbox_key_list" in two lists: known and unknown keys
		known=   []
		unknown= []

		for sbox_key in sbox_key_list:
			if sbox_key[1] == None:
				unknown.append( sbox_key )
			else:
				known.append( sbox_key )

		# Create initial sbox_key vector
		num_unknown= []
		for s,k in unknown:
		   num_unknown.append( [s,0] )

		# Create key_estimators
		for e in range(64**len(unknown)):
			full_vector= []
			for elem in known:
				full_vector.append( elem )
			for elem in num_unknown:
				full_vector.append( list(elem) ) # @warning As "num_unknown" is updated later,
															#          list() is mandatory to avoid
															#          pointer usage

			self.__n_key_estimators.append( [ full_vector, n_key_estimator( full_vector ) ] )

			# Update sbox_key vector
			num_unknown[0][1]+= 1
			for i in range( len(unknown) ):
				if num_unknown[i][1] == 64:
					num_unknown[i][1]= 0
					if i != len(unknown)-1:
						num_unknown[i+1][1]+= 1

	def __del__(self):
		if DEBUG:
			print "Destructor", self
		for fv, nke in self.__n_key_estimators:
			del nke
		self.__n_key_estimators= []

	def process(self, msg, trace):
		"""
		Process the given trace for the given message.
		Updates the best key (instance member)
		"""
		# Computing differential traces and the best key at that point
		# (i.e. given the number of traces consumed from the database so far)
		for fv, nke in self.__n_key_estimators:
			nke.process( msg, trace )
		self.__best_key= None
	
	def get_keys(self):
		"Return the keys sorted by best candidates"
		keys_marks= []
		if self.__best_key == None:
			for fv, nke in self.__n_key_estimators:
				keys_marks.append( list( [fv, nke.get_mark()] ) )
			keys_marks.sort( reverse=True, key=itemgetter(1) ) # Sort by sbox

		return map( lambda km: km[0], keys_marks )

if __name__ == "__main__":
	sb= n_sbox_breaker( [ [0,56], [1,None] ] ) # Here, subkey #0 is known, we look for subkey #1
	tdb= traces_database(TABLE)
	for i in range(100):
		msg, crypt, trace= tdb.get_trace()
		sb.process(msg, trace)
		best_keys= sb.get_keys()
		print "Processed trace:", i, " - Best key is:", best_keys[0], " - True key is at position: ", best_keys.index( [ [0,56], [1,11] ] )
