# dsp.py provides class to pre-process traces
# It permits making an attack on side channel traces, in order to extract
# the key used to encipher some messages using the Data Encryption Standard.
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
from math import sqrt

# DPACONTEST modules
from kurtosis  import k4
from constants import *

# Modules only for test
from traces_database import traces_database

def dsp( wave ):
	#Limiting number of samples
	trace= wave[BEGIN_T:END_T]

	# Pre-processing
	if   DSP == "kurtosis":
		trace= k4( 100, trace )
	elif DSP == "RMS":
		sum = 0.
		n = 0.
		for sample in trace:
			sum += sample ** 2
			n += 1
		trace= [ sqrt( sum / n ) ] # @note "trace" should remain a list for other classes

	return trace

if __name__ == "__main__":
	tdb= traces_database(TABLE)  # Open database
	msg, crypt, data= tdb.get_trace() # Reads one trace
	wave= dsp( data ) # Read one trace and apply pre-processing
                     # @note Change "constant.py" to modify DSP

	fd= open("trace.csv",'w')
	for sample in wave:
		fd.write( str(sample)+'\n' )
	fd.close()
