#!/usr/bin/python
# @file   meanMTD.py 
# @brief  Given a directory <IDIR>, computes the number of Measurements To Disclose (MTD)
#         the key in average.
# @author Laurent SAUVAGE <laurent.sauvage@telecom-paristech.fr>

###########
# Imports #
###########

import sys
import  os

#############
# Constants #
#############

#############
# Fonctions #
#############

def usage():
	print 'Usage: meanMTD.py <IDIR>'
	print '          where IDIR is a directory containing results of analyses'
	exit(1)

def main():

	# Checks if enough argument in the command line
	if len( sys.argv ) < 2:
		print "!! ERROR !! Not enough arguments"
		usage()

	IDIR= sys.argv[1]

	# Checks if <IDIR> is a directory
	if not os.path.isdir( IDIR ):
		print "!! ERROR !! " + IDIR  +" is not a directory !!"
		exit(1)

	# Computes mean MTD
	sum= 0
	n= 0

	for file in os.listdir( IDIR ):
		right_fullkey= False
		MTDs= [] # List to store MTD of each SBox of current file
		fd= open( IDIR+'/'+file, 'r' )
		while 1:
			line= fd.readline()
			if line == "": # At the end of the file
				break
			if line[:4] == "SBox": # Found MTD of one SBox
				MTDs.append( int( line.split("MTD= ")[1] ) )
			if line[:6] == "# Key:": # Found full key
				if line.split("# Key: ")[1] == "0x6a64786a64786a64\n":
					right_fullkey= True

		if not right_fullkey:
			print "!!ERROR!! Right key not found", file
			continue

		if len( MTDs ) != 8:
			print "!!ERROR!! All MTD not found"
			exit(1)

		print "File: " ,file, " - MTDmax=", max(MTDs)
		sum+= max( MTDs ) 
		n+= 1.
		fd.close()
	print "Mean MTD for ", IDIR, " is ", sum/n

if __name__ == "__main__":
   main()
