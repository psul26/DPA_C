# traces_database.py allows to access a PostGreSQL database through either the
# PgSQL or pgdb libraries and retrieve side-channel traces from such database.
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
import sys

# DPACONTEST modules
from   parse_binary import *
import sorting
from   constants    import *

# Class managing transaction with the database, renamed "db"
"""
Notice about "pyPgSQL.PgSQL" versus "pgdb":
- "pyPgSQL.PgSQL" has fast SELECTs on large (100k+ rows) tables; it is thus preferred
- both "pyPgSQL.PgSQL" and "pgdb" share the same API (DB API 2.0).
  However, fetchone() on a postGreSQL BYTEA field returns:
   - a PgBytea object in the case of "pyPgSQL.PgSQL" and
   - a string in the case of "pgdb".
  The code is thus necessarily dependent on the "db" used. Hence the "db_name" flag.
"""
try: from pyPgSQL import PgSQL as db
except:
	try: import pgdb as db
	except:
		print "Neither pyPgSQL nor pgdb is installed. Stopping dead."
		print "Please read the instructions on 'http://www.dpacontest.org/documentation.php'."
		# sys.exit(1)
	else: db_name= 'pgdb';  # Choosing "pgdb" as a back-up
else: db_name= 'pyPgSQL'; # Choosing "pyPgSQL.PgSQL"

class traces_database:
	""" Class providing database IOs """
	# Some class constants
	__host = "dpa.enst.fr"
	__user = "guest"
	__pass = "guest"
	__db   = "production_traces"

	# Private variables
	__conn = None
	__curs = None

	def __init__(self, table):
		""" No arguments needed """

		# Connexion to the DataBase
		try:
			self.__conn  = db.connect(
				user     = self.__user,
				password = self.__pass,
				host     = self.__host,
				database = self.__db
			)
		except db.DatabaseError, e:
			print e
			sys.exit(1)
		else:
			print "Connected to database"
			self.__table= table
			self.__curs  = self.__conn.cursor()

		# Building list of content
		try:
			print "Building files list, please wait..."
			cmd= "SELECT filename FROM "+self.__table
			# <DEBUG>
			#cmd= "SELECT filename FROM "+self.__table+" LIMIT 1000"
			#print "!! WARNING !! Access to database limited to first 1000 files"
			# <\DEBUG>
			self.__curs.execute( cmd )
			self.__dbd= [] # Empty list to store filenames
			while 1:
				one= self.__curs.fetchone()
				if one:
					self.__dbd.append( one[0] ) # @note fetchone has returned a list with a single element "filename"
				else: # End of DataBase reached
					break
		except db.DatabaseError, e:
			print e
			sys.exit(1)

		self.__dbd = sorting.order( self.__dbd )
		self.__i  = 0; # File index, in [0, len(dd)[

	def __del__(self):
		""" Automatically called method """
		if DEBUG:
			print "Destructor", self
		#self.__curs.close () # Useless, automatically done by db.__del__
		#self.__conn.close()  # Useless, automatically done by db.__del__

	def get_fileslist(self):
		return self.__dbd

	def get_trace(self):
		"""
		Do not take any argument.
		Returns the next triplet (message, cipher, trace), where:
		 - message is an ascii string containing a 64 bits clear message in hexadecimal,
		 - cipher is an ascii string containing a 64 bits ciphered message in hexadecimal,
		 - trace is a float vector containing a trace during the cipher operation.
		"""
		if self.__i == len( self.__dbd ):
			return None, None, None; # Error, since we have reached the last file

		trace_name= self.__dbd[self.__i]
		self.__i+= 1;

		try:
			cmd= "SELECT message,cryptogram,filecontent FROM "+self.__table+" WHERE filename = '"+trace_name+"'"
			self.__curs.execute( cmd )
			one= self.__curs.fetchone()
			msg, crypt, raw_data= one
			if db_name=='pgdb':
		 		raw_data= db.unescape_bytea( raw_data )
			return msg, crypt, parse_binary( str(raw_data) )
		except db.DatabaseError, e:
			print e
			sys.exit(1)

	def get_file(self, filename):
		"""
		Returns the raw trace (header plus float vector) of <filename> 
		"""
		try:
			cmd= "SELECT filecontent FROM "+self.__table+" WHERE filename = '"+filename+"'"
			self.__curs.execute( cmd )
			raw_data= self.__curs.fetchone()
			if db_name=='pgdb':
		 		raw_data= db.unescape_bytea( raw_data )
			return parse_binary( str(raw_data) )
		except db.DatabaseError, e:
			print e
			sys.exit(1)

	def rollback(self):
		self.__i= 0

if __name__ == "__main__":
	print "Using "+db_name+" DB API"
	tdb= traces_database("secmatv3_20080424"); # /!\ Not the table to use for the contest

	print "First pass:"
	for i in range(10):
		msg, crypt, data= tdb.get_trace()
		print msg, crypt

	tdb.rollback()
	print "Second pass:"
	for i in range(10):
		msg, crypt, data= tdb.get_trace()
		print msg, crypt
