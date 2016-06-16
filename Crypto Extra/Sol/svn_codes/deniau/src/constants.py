# constants.py customizes the side-channel analysis python application.
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

# The traces on which we'll lead our DPA
__TABLE__ = "secmatv1_2006_04_0809"

# The DES substitution box output targeted bit (in {0, 1, 2, 3}),
# used for traces partitioning in the mono-bit DPA
__BIT__ = 0

# Write some log to the following output file
def __OUTPUT_FILE__(): return "output"+str(__BIT__)+".txt"

# The number of iterations during which the key must have been stable
__STABILITY_THRESHOLD__ = 100

# The  number of traces we have to compute before doing any test
__ITERATION_THRESHOLD__ = 1800
