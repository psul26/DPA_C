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
import time
from operator import itemgetter # For sorting list

# DPACONTEST modules
from des_breaker12     import des_breaker12
from traces_database   import traces_database
from traces_filesystem import traces_filesystem
from dsp               import dsp
from constants         import *

def main():
	# Checks consistency of <N> and <ORDER>
	if N>1 and ORDER != "randomized":
		print "ERROR: You choose multiple analyses (N="+str(N)+") but ORDER is different of \"randomized\" !!"
		sys.exit(1)

	# Prepares output directory
	OUTDIR= outdir()
	if not path.isdir( OUTDIR ):
		print "WARNING: Output directory "+OUTDIR+ " does not exist: creating it !!"
		mkdir(OUTDIR)

	# Runs <N> analyses
	for n in range(N):

		# Chooses whether or not we are local or distant
		if DB_or_FS:
			tdb= traces_database(TABLE)
		else:
			tdb= traces_filesystem(DIRECTORY)

		# Builds name and opens file log
		logfile= OUTDIR + '/' + str( id(tdb) ) + time.strftime("_%Y_%b_%d_%H:%M:%S") + "_key_info_#" + str(n) + ".txt"
		fd= open( logfile, 'w' )
		def out( s ): # Multiple outputs
			fd.write(s+"\n")
			fd.flush()
			print s

		# Displays some informations
		out( "# Table: " + TABLE )
		out( "# Stability threshold: " + str(STABILITY_THRESHOLD)  )
		out( "# Iteration threshold: " + str(ITERATION_THRESHOLD)  )
		out( "#" )
		out( "# Columns: Iteration Stability Subkey0 ... Subkey7" )

		# Searches for the key
		fullkey= None
		sk_lst= [] # List of disclosed key_sbox couples

		while fullkey == None:

			# Searches for one subkey
			iteration= 0
			prev_subkeys= {} # Dictionnary to store subkeys of previous step
			stability= {}    # Dictionnary to store stability and each undisclosed sboxes
			brk= des_breaker12( list(sk_lst) ) # Creates DES breaker object

			new_sb= False # No new subkey disclosed
			while not new_sb:
				msg, crypt, trace= tdb.get_trace()
				if not msg: # At the end of the database
					print "WARNING: All files used, key undisclosed !!"
					sys.exit(1)
				wave= dsp( trace ) # Passes trace to DSP
				brk.process(msg, wave) # Passes trace to DES breaker
				iteration+= 1
				text= str(iteration).rjust(4) + " "
	
				# When enough iterations, tries to guess subkeys
				if iteration >= ITERATION_THRESHOLD:
					# Computes new subkeys
					cur_subkeys= brk.get_subkeys() # Returns (sbox, key)
	
					# Computes stability mark...
					for s,k in cur_subkeys:
						if s not in map( lambda sk: sk[0], sk_lst ): # ...for undisclosed subkeys
							if k == prev_subkeys.get( s, None ):
								stability[s]= ( k, stability.get( s, [0,0] )[1] + 1 ) # stability= { sbox:( key, stability ) }
							else:
								stability[s]= ( k, 0 )
								prev_subkeys[s]= k

					# Gathers some informations
					max_stab= max( map( lambda k_s: k_s[1], stability.values() ) )
					text+= str( max_stab ).rjust(3)
					for sbox in range(8):
						if sbox in SB_LST:
							text+= " " + str( cur_subkeys[ SB_LST.index(sbox) ][1] ).rjust(2)
						else:
							text+= " ".rjust(2) 
					out( text )
	
					# Checks if one subkey has been stable for long enough
					for sbox, key_stab in stability.items():
						if key_stab[1] == STABILITY_THRESHOLD:
							new_sb= True
							sk_lst.append( [ sbox, key_stab[0] ] ) # Saves info on new disclosed subkey
							out( "SBox #" + str(sbox) + " disclosed (" + str( key_stab[0] ) + ") - MTD= " + str(iteration) )

					# If all subkey disclosed, tries to guess the full key
					if len( sk_lst ) == 8: # Attack on all SBox
						full_sk= list (sk_lst ) # Hard copy of sk_lst
						full_sk.sort( reverse=False, key=itemgetter(0) ) # Sorts subkeys by increasing sbox
						full_subkeys= map( lambda s_k: s_k[1], full_sk)
						fullkey= brk.get_key(msg, crypt, full_subkeys )
					elif len( sk_lst ) == len( SB_LST ):
						fullkey="All attacked sbox stable" 
						# @note we do no longer check the full key as error may have occur later
						#for sbox in SB_LST:
						#	if subkeys[sbox] != SK_R1[sbox]:
						#		fullkey=None

			# Prepares next run
			tdb.rollback() # Rolls back database
			del brk

		# Outputs value of full key
		out( "# Key: " + str(fullkey) + "\n" )

		# Prepares next analysis (if any)
		del tdb
		fd.close()

if __name__ == "__main__": main()
