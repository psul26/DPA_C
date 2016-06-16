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

# ----------------
# Files management
# ----------------

# Traces access 
#DB_or_FS  = True  # From the DataBase
DB_or_FS  = False # From the filesystem (False)

# Traces on which we'll lead our DPA
TABLE     = "secmatv1_2006_04_0809" # For traces from the DataBase
#DIRECTORY = "/home/traces/secmatv1_2006_04_0809" # For traces from the filesystem on attack
DIRECTORY = "/traces/secmatv1_2006_04_0809" # For traces from the filesystem on attack2
DIRECTORY = "../../secmatv1_2006_04_0809" # For traces from the filesystem on attack2
# DIRECTORY = "../../secmatv3_20070924_des" # For traces from the filesystem on attack2

# Files order
#ORDER = "none"
ORDER = "randomized"

# Number of traces to consider
BEGIN_N, END_N = None, None # All files
#BEGIN_N, END_N = 1000, 2000 # 1000 traces, from the 1000th to the 2000th (after sort)

# ---------------------
# Traces pre-processing
# ---------------------

# Trace Zoom in
# BEGIN_T, END_T = None, None # Full trce

BEGIN_T, END_T = 5700, 5900 # Zoom in on power consumption of first round
                            # @note Entire round1 clokc cycle is in [5500:6200]

# DSP
#DSP="none"
#DSP="kurtosis"
DSP="RMS"

# --------------------
# Algorithm parameters
# --------------------

#Type of analysis
PPA="dpa"
#PPA="cpa"

# List of SBoxes to break
SB_LST = [0,1,2,3,4,5,6,7] # All SBoxes 
#SB_LST = [0] 

# The DES substitution box output targeted bit
#BITS = [0] # Monobit with bit 0 of each SBox
BITS = [0,1,2,3] # All bits of each SBox
#BITS = [0,2] # You can test some strange combinaisons

# ----------------
# Results analysis
# ----------------

# Number of analyses to perform:
#    - =1: single analysis
#    - >1: multiple analyses (representative order)
N = 1
# N = 1000

# The number of iterations during which the key must have been stable
STABILITY_THRESHOLD = 100

# The number of traces we have to compute before doing any test
ITERATION_THRESHOLD = 1

# Subkeys of round1 when master key is "keykeyke" 
SK_R1 = (56,11,59,38,0,13,25,55)

# Output directory where to store results files
def outdir():
	OUTDIR= PPA
	if len(SB_LST) == 1:
		OUTDIR += "_sbox" + str( SB_LST[0] )
	if len(BITS) == 1:
		OUTDIR += "_bit" + str( BITS[0] )
	if DSP != "none":
		OUTDIR += "_" + DSP
	if BEGIN_T:
		OUTDIR += "_" + str(BEGIN_T) + '-' + str(END_T)
	if ORDER != "none":
		OUTDIR += "_" + ORDER
	return OUTDIR
