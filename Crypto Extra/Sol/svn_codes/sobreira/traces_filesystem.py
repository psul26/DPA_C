# traces_filesystem.py allows to access a filesystem database.
# TODO: How to derive a class from an abstract one in python?
#       Indeed, traces_filesystem and traces_database share the same interface
#
# Copyright (C) 2009 Sylvain GUILLEY (sylvain.guilley@telecom-paristech.fr)
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
import os # For open / read / etc.
import sys # For exit

# DPACONTEST modules
from   parse_binary import *
import sorting

def hex_to_raw( s ):
	"""
	Converts an string hexadecimal representation into a binary raw string.
	Input ``s'' has to be of a pair size. Returns the converted raw string.
	"""
	raw= ""
	for i in range( 0, len(s)/2 ):
		raw += chr( int( s[2*i:2*(i+1)], 16 )) # 16 is the hexadecimal base
	return raw

def raw_to_hex( s ):
	""" Inverse function of hex_to_raw """
	hex= ""
	for c in range( 0, len(s) ): # The bytes are considered chars
		hex += ( "%x"%ord( s[c] )).zfill( 2 )
	return hex

def parse_filename( fname, size ):
	"""
	Parses filename to retrieve the value of the key, the message and the
	ciphertext. Returns a (key, msg, cipher) triplet, each of size size.
	"""
	markers= ("k=", "m=", "c=")
	indices= map( lambda x:fname.find(x)         , markers )
	hexas  = map( lambda x:fname[ x+2: x+2+size ], indices )
	return hexas;
	#return   map( hex_to_raw                     , hexas   )

def get_key( filename ):
	return parse_filename( filename, 16 )[0];

def get_msg( filename ):
	return parse_filename( filename, 16 )[1];

def get_cph( filename ):
	return parse_filename( filename, 16 )[2];

# Class managing the opening and reading of files in a directory
class traces_filesystem:
	""" Returning the file contents """
	# Some class constants
	__db   = "production_traces"

	# Private variables
	__curs = None

	def __init__(self, directory):
		""" No arguments needed """
		if not os.path.isdir( directory ):
			print directory + " does not exist !!"
			sys.exit(1)
		else:
			print "Founded filesystem directory " + directory
			self.__dn = directory

		print "Building files list, please wait..."
		self.__dd = os.listdir( self.__dn )
		self.__dd = sorting.order( self.__dd )
		self.__i  = 0; # File index, in [0, len(dd)[

	def __del__(self):
		""" Automatically called method """

	def get_trace(self):
		"""
		Do not take any argument.
		Returns the next triplet (message, cipher, trace), where:
		 - message is an ascii string containing a 64 bits clear message in hexadecimal,
		 - cipher is an ascii string containing a 64 bits ciphered message in hexadecimal,
		 - trace is a float vector containing a trace during the cipher operation.
		"""
		if self.__i == len( self.__dd ):
			return None, None, None # Error, since we have reached the last file

		trace_name= self.__dd[self.__i]
		self.__i+= 1

		self.__fd= open( self.__dn + '/' + trace_name, 'r' )
		raw_data= self.__fd.read()

		return get_msg( trace_name ), get_cph( trace_name ), parse_binary( raw_data )

def test():
	tfs= traces_filesystem("/home/traces/secmatv1_2006_04_0809"); # /!\ Not the table to use for the contest
	for i in range(10):
		msg, crypt, data= tfs.get_trace()
		print msg, crypt

if __name__ == "__main__":
	test()
