# traces_database.py allows to access a PostGreSQL database through either the
# PgSQL or pgdb libraries and retrieve side-channel traces from such database.
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

# Function allowing conversion from C binary data
from struct import unpack

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
		import sys
		sys.exit( 1 )
	else: db_name= 'pgdb';  # Choosing "pgdb" as a back-up
else: db_name= 'pyPgSQL'; # Choosing "pyPgSQL.PgSQL"

def parse_binary( raw_data ):
	"""
	Takes a raw binary string containing data from our oscilloscope.
	Returns the corresponding float vector.
	"""
	ins =  4   # Size of int stored if the raw binary string
	cur =  0   # Cursor walking in the string and getting data
	cur += 12  # Skipping the global raw binary string header
	whs =  unpack("i", raw_data[cur:cur+ins])[0] # Storing size of the waveform header
	cur += whs # Skipping the waveform header
	dhs =  unpack("i", raw_data[cur:cur+ins])[0] # Storing size of the data header
	cur += dhs # Skipping the data header
	bfs =  unpack("i", raw_data[cur-ins:cur])[0] # Storing the data size
	sc  =  bfs/ins # Samples Count - How much samples compose the wave
	dat =  unpack("f"*sc, raw_data[cur:cur+bfs])
	return dat

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
		self.__conn  = db.connect(
			user     = self.__user,
			password = self.__pass,
			host     = self.__host,
			database = self.__db
		)
		self.__curs  = self.__conn.cursor()
		self.__curs.execute("SELECT message,cryptogram,filecontent FROM \""+table+"\"")

	def __del__(self):
		""" Automatically called method """
		self.__conn.close()

	def get_trace(self):
		"""
		Do not take any argument.
		Returns the next triplet (message, cipher, trace), where:
		 - message is an ascii string containing a 64 bits clear message in hexadecimal,
		 - cipher is an ascii string containing a 64 bits ciphered message in hexadecimal,
		 - trace is a float vector containing a trace during the cipher operation.
		"""
		msg, crypt, raw_data= self.__curs.fetchone()
		if db_name=='pgdb': raw_data= db.unescape_bytea( raw_data )
		return msg, crypt, parse_binary( str(raw_data) )

def test():
	tdb= traces_database("secmatv3_20080424"); # /!\ Not the table to use for the contest
	for i in range(10):
		msg, crypt, data= tdb.get_trace()
		print msg, crypt

if __name__ == "__main__":
	print "Using "+db_name+" DB API"
	test()
