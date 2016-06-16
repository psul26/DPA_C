# sorting.py provides a method to sort the power traces
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
from random import shuffle

# DPACONTEST modules
from constants import *

# Modules only for test
from traces_database import *

#  --sorting unix|alphanumeric|giga|randomized|suffix
def order( trace_list ):
	# @note, if ORDER=="none", Nothing to do...
	if ORDER=="randomized":
		shuffle( trace_list )

	# Else, code the others...

	return trace_list[BEGIN_N:END_N] # After sorting, limits the number of traces

if __name__ == "__main__":
	tdb= traces_database(TABLE)
	print "List of files sorted by processing order:" #@note Change params such as "BEGIN_N" in "constant.py"
	for file in tdb.get_fileslist():
		print file
