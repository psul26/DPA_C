# parse_binary.py interpretes the content of the binary agilent acquisition format.
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


from struct import unpack # Function allowing conversion from C binary data

def parse_binary( raw_data ):
	"""
	Takes a raw binary string containing data from our oscilloscope.
	Returns the corresponding float vector.
	"""
	ins =  4   # Size of int stored in the raw binary string
	cur =  0   # Cursor walking in the string and getting data
	cur += 12  # Skipping the global raw binary string header
	whs =  unpack("i", raw_data[cur:cur+ins])[0] # Storing size of the waveform header
	cur += whs # Skipping the waveform header
	dhs =  unpack("i", raw_data[cur:cur+ins])[0] # Storing size of the data header
	cur += dhs # Skipping the data header
	bfs =  unpack("i", raw_data[cur-ins:cur])[0] # Storing the data size
	sc  =  bfs/ins # Samples Count - How much samples compose the wave
	dat =  unpack("f"*sc, raw_data[cur:cur+bfs])
	return dat
