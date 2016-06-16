# main.py is the top level of the side-channel analysis python application.
# It permits making an attack on side channel traces, in order to extract
# the key used to encipher some messages using the Data Encryption Standard.
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

from des_breaker import des_breaker
from traces_database import traces_database
from struct import *
from constants import *

def main():
	tdb= traces_database( TABLE )
	brk= des_breaker()
	
	# Some variables
	fullkey= None
	subkeys= None
	stability= 0
	iteration= 0
	
	fd= open(OUTPUT_FILE, "w")
	# Multiply outputs
	def out( s ):
		fd.write(s+"\n")
		print s
	
	out( "# Table: " + TABLE )
	out( "# Stability threshold: " + str(STABILITY_THRESHOLD)  )
	out( "# Iteration threshold: " + str(ITERATION_THRESHOLD)  )
	out( "#" )
	out( "# Columns: Iteration Stability Subkey0 ... Subkey7" )
	
	# Searching for the key
	while fullkey == None:
		msg, crypt, trace= tdb.get_trace()
		brk.process(msg, trace[5095:5800])
		iteration+= 1
		text= str(iteration).rjust(4) + " "
		# When enough iterations, trying to guess subkeys
		if iteration >= ITERATION_THRESHOLD:
			# Computing new subkeys and stability mark
			loc_subkeys= brk.get_subkeys()
			if loc_subkeys == subkeys:
				stability+= 1
			else:
				subkeys= loc_subkeys
				stability= 0
			# Gathering some informations
			text+= str(stability).rjust(3)
			for sk in subkeys:
				text+= " " + str(sk).rjust(2)
			# If subkeys have been stable for long enough, try to guess the full key
			if stability >= STABILITY_THRESHOLD:
				fullkey= brk.get_key(msg, crypt)
		# Flushing output
		out( text )

	# End of Attack
	out( "# Key: " + str(fullkey) + "\n" )
	
	fd.close()

if __name__ == "__main__": main()
