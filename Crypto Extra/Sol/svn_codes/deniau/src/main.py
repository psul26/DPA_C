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
from des_block import des_block

from constants import __TABLE__
from constants import __OUTPUT_FILE__
from constants import __STABILITY_THRESHOLD__
from constants import __ITERATION_THRESHOLD__

def main():
	tdb= traces_database( __TABLE__ )
	brk= des_breaker()
	
	# Some variables
	fullkey= None
	subkeys= None
	stability= 0
	iteration= 0
	
	fd= open(__OUTPUT_FILE__(), "w")
	# Multiply outputs
	def out( s ):
		fd.write(s+"\n")
		print s
	
	out( "# Table: " + __TABLE__ )
	out( "# Stability threshold: " + str(__STABILITY_THRESHOLD__)  )
	out( "# Iteration threshold: " + str(__ITERATION_THRESHOLD__)  )
	out( "#" )
	out( "# Columns: Iteration Stability Subkey0 ... Subkey7" )
	# Searching for the key
	out( "Recherche de 48 bits de cle par analyse du premier tour" )
	while fullkey == None:
		msg, crypt, trace= tdb.get_trace()
		brk.process(msg, trace)
		iteration+= 1
		text= str(iteration).rjust(4) + " "
		# When enough iterations, trying to guess subkeys
		if iteration >= __ITERATION_THRESHOLD__:
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
			if stability >= __STABILITY_THRESHOLD__:
#				fullkey= brk.get_key(msg, crypt)
				break
		# Flushing output
		out( text )
	second_round_begin= iteration
	subkeys2= None
  	out( "Recherche des 8 derniers bits par analyse du second tour" )
	brk.__begin_second_round__()
	while fullkey == None:
		msg, crypt, trace= tdb.get_trace()
  		brk.process2(msg, trace)
  		iteration+= 1
		text= str(iteration).rjust(4) + " "
		if iteration >= second_round_begin + __ITERATION_THRESHOLD__:
			# When enough iterations, trying to guess subkeys
			# Computing new subkeys and stability mark
			loc_subkeys= brk.get_subkeys2()
			if loc_subkeys == subkeys2:
				stability+= 1
			else:
				subkeys2= loc_subkeys
				stability= 0
			# Gathering some informations
			text+= str(stability).rjust(3)
			text+= " " + str(subkeys2[0]).rjust(2)
			text+= " " + str(subkeys2[1]).rjust(2)
			text+= " " + str(subkeys2[2]).rjust(2)
			text+= " " + str(0).rjust(2)
			text+= " " + str(subkeys2[3]).rjust(2)
			text+= " " + str(0).rjust(2)
			text+= " " + str(subkeys2[4]).rjust(2)
			text+= " " + str(subkeys2[5]).rjust(2)
			# If subkeys have been stable for long enough, try to guess the full key
			if stability >= __STABILITY_THRESHOLD__:
				break
		# Flushing output
		out( text )
	# End of Attack
	fullkey= brk.recover_full_key(subkeys, subkeys2)
	out( "# Key: " + str(fullkey) + "\n" )
	
	fd.close()

if __name__ == "__main__": main()
