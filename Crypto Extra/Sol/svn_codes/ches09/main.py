# main.py is the top level of the side-channel analysis python application.
# It permits making an attack on side channel traces, in order to extract
# the key used to encipher some messages using the Data Encryption Standard.
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
from   os import path, mkdir
import sys # For exit

# DPACONTEST modules
from des_breaker       import des_breaker
from traces_database   import traces_database
from traces_filesystem import traces_filesystem
from dsp               import dsp
from constants         import *

def main():
	# Preparing output directory
	OUTDIR= outdir()
	if not path.isdir( OUTDIR ):
		print "WARNING: Output directory "+OUTDIR+ " does not exist: creating it !!"
		mkdir(OUTDIR)

	# Checking consistency of <N> and <ORDER>
	if N>1 and ORDER != "randomized":
		print "ERROR: You choose multiple analyses (N="+str(N)+") but ORDER is different of \"randomized\" !!"
		sys.exit(1)

	for n in range(N):
		# Choose whether or not we are local or distant
		if DB_or_FS:
			tdb= traces_database(TABLE)
		else:
			tdb= traces_filesystem(DIRECTORY)

		# Some variables
		fullkey= None
		subkeys= None
		stability= 0
		iteration= 0

		# Build name and opening file log
		logfile= OUTDIR+"/output_#"+str(n)+".txt"
		fd= open( logfile, 'w' )
		def out( s ): # Multiply outputs
			fd.write(s+"\n")
			fd.flush()
			print s

		out( "# Table: " + TABLE )
		out( "# Stability threshold: " + str(STABILITY_THRESHOLD)  )
		out( "# Iteration threshold: " + str(ITERATION_THRESHOLD)  )
		out( "#" )
		out( "# Columns: Iteration Stability Subkey0 ... Subkey7" )

		# Create DES breaker object
		brk= des_breaker()

		# Searchs for the key
		while fullkey == None:
			msg, crypt, trace= tdb.get_trace()
			if not msg: # At the end of the database
				print "WARNING: All files used, key undisclosed !!"
				sys.exit(1)
			wave= dsp( trace )
			brk.process(msg, wave)
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
				for sbox in range(8):
					if sbox in SB_LST:
						text+= " " + str( subkeys[ SB_LST.index(sbox) ] ).rjust(2)
					else:
						text+= " ".rjust(2) 

				# If subkeys have been stable for long enough, try to guess the full key
				if stability >= STABILITY_THRESHOLD:
					if len( SB_LST ) == 8: # Attack on all SBox
						fullkey= brk.get_key(msg, crypt)
					else:
						fullkey="All attacked subkeys disclosed" 
						for sbox in SB_LST:
							if subkeys[sbox] != SK_R1[sbox]:
								fullkey=None

			# Flushing output
			out( text )

		# End of Attack
		out( "# Key: " + str(fullkey) + "\n" )

		fd.close()

if __name__ == "__main__": main()
