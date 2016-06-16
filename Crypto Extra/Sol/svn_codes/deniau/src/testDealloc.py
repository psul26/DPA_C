from sbox_breaker import sbox_breaker
from constants import __TABLE__
from NativeDes import Trace

from key_estimator import key_estimator

# sb= sbox_breaker( 1 )

key_estimators= map( lambda x:key_estimator(1, x), range(2) )

for i in range(100):
	msg = "993fa9b70fe852af"
	trace = Trace(200)
        trace2 = Trace(200)
        trace3 = trace.getDifferentiel(trace2)
	# k.process(msg, trace)
	
	for i in range(len(key_estimators)):
		key_estimators[i].process( msg, trace )
		
	print "processed trace:", i
