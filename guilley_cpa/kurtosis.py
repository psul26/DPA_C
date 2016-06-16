# vim: set fileencoding=iso-8859-15 :

# kurtosis.py implements a function that is able to compute the fourth-order
# cumulant of each trace.
# Copyright (C) 2008 Sylvain Guilley (sylvain.guilley@telecom-paristech.fr)
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

"""
For the theory on the fourth-order cumulant, refer to this paper:

@article{
  thanhhale-tifs08,
  author  = "Thanh-Ha Le and Jessy Cledière and Christine Servière and Jean-Louis Lacoume",
  title   = "{Noise Reduction in Side Channel Attack using Fourth-order Cumulant}",
  journal = "IEEE Transaction on Information Forensics and Security",
  volume  = "2",
  number  = "4",
  year    = "2007",
  month   = "December",
  pages   = "710--720",
  note    = "DOI: 10.1109/TIFS.2007.910252"
}
"""

class fifo_sum:
	"A class optimizing a FIFO (First In First Out) sum computation"
	# The FIFO size
	L= None
	def __init__( self ):
		# Index in the FIFO
		self.__l = 0
		# Empty list, to accomodate the current window of the trace
		self.__samples = []
		# The sum of all samples
		self.__sum = 0
		for i in range( fifo_sum.L ):
			self.__samples.append( 0. )
	def replace( self, l, s ): 
		self.__sum += s - self.__samples[l]
		self.__samples[l] = s
	def get_sum( self ):
		return self.__sum

# Global square function
def sq( x ): return x*x

class kurtosis:
	"Evaluation of the kurtosis excess"
	def __init__( self ):
		# The polynomial (X[i] <=> X^{i+1})
		self.__X = []
		# The position, in [0,L[
		self.__l = 0
		for i in range( 4 ):
			self.__X.append( fifo_sum() )
	def add_point( self, s ):
		y = s
		for i in range( 4 ):
			self.__X[i].replace( self.__l, y )
			y *= s
		self.__l = ( self.__l+1 ) % fifo_sum.L;
	def get_kurtosis( self ):
		#return self.__X[0].get_sum() # For DEBUG only
		return \
		(
			 (fifo_sum.L+2) * ( self.__X[3].get_sum() - self.__X[0].get_sum()/fifo_sum.L * ( 4*self.__X[2].get_sum() - self.__X[0].get_sum()/fifo_sum.L * ( 6*self.__X[1].get_sum() - 3/sq(fifo_sum.L)*sq(self.__X[0].get_sum()) ) ) ) -
			 3 * sq( self.__X[1].get_sum() - sq(self.__X[0].get_sum())/fifo_sum.L )
		) \
		/ ( fifo_sum.L*(fifo_sum.L-1) );

# Applies the k4 transformation on the trace (data) with a given window (L)
def k4( L, data ):
	# Computing the k4 trace
	fifo_sum.L = 10 # The window size
	k4 = kurtosis()
	data_k4 = []
	for i in range( len( data )):
		# Shifting one point away
		k4.add_point( data[i] );
		# Saving the current estimation of k4
		data_k4.append( k4.get_kurtosis() )
	return data_k4

#-------#
# DEBUG #
#-------#

def main():
	# The window size
	fifo_sum.L = 10 ##argv( 0 );
	k4= kurtosis()
	for i in range( 2*fifo_sum.L ):
		print "i=", i, "\t", k4.get_kurtosis()
		k4.add_point( i )

from trace_loader import traces_database

def test():
	tdb= traces_database("secmatv1_2006_04_0809");
	msg, crypt, data= tdb.get_trace()
	# Saving the plain trace
	fd = open("data.csv", "w")
	for f in data: fd.write(str(f)+"\n")
	fd.close()
	# Computing the k4 trace
	data_k4 = k4( 10, data )
	# Saving the k4 trace
	fd_k4 = open("data_k4.csv", "w")
	for f in data_k4: fd_k4.write(str(f)+"\n")
	fd_k4.close()
	# Viewing the results:
	# In gnuplot, type "plot 'data.csv' w l, 'data_k4.csv' u 0:($1*5e2) w l"

if __name__ == "__main__":
	#main()
	test()