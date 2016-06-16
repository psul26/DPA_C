# traces_database.py allow accessing a PostgreSQL database
# through the PgSQL library. And retrieve side-channel traces from
# such database.
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

# Class managing transaction with the database
from pyPgSQL import PgSQL

from NativeDes import Trace


class traces_database:
	""" Class providing database IOs """
	# Some constants
	__host = "dpa.enst.fr"
	__user = "guest"
	__pass = "guest"
	__db   = "production_traces"

	# Private variables
	__conn  = None
	__curs  = None

	def __init__(self, table):
		""" No arguments needed """
		self.__conn  = PgSQL.connect(
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
		Returns the next couple (message, trace),
		where
		message is an ascii string containing a 64 bits clear message in hexadecimal,
		trace is a float vector containing a trace during the cipher operation.
		"""
		msg, crypt, raw_data= self.__curs.fetchone()
		return msg, crypt, Trace(str(raw_data))

def test():
	tdb= traces_database("secmatv3_20080424")
	#msg, data= tdb.get_trace()
	#fd= open("data.csv", "w")
	#for f in data: fd.write(str(f)+"\n")
	#fd.close()
	for i in range(10):
		msg, crypt, data= tdb.get_trace()
		print msg, crypt

if __name__ == "__main__":
	test()
