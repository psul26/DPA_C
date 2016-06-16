from key_estimator2 import key_estimator2
from sbox_breaker import sbox_breaker

class sbox_breaker2 (sbox_breaker):
	"""
	Provides method to break a sbox, given some traces.
	"""
	__box_number= None
	__known_bits= None

	def __init__(self, sbox, known_bits):
		"Builds 2 key_estimator2, on for each possible subkey for the given sbox"
		self.__box_number= sbox
		self.__known_bits= known_bits
		print "Constructing sbox_breaker2 ", sbox
		if sbox == 0:
			self.key_estimators = map( lambda x:key_estimator2(sbox, x), [known_bits, known_bits+4, known_bits+16, known_bits+20] )
			if known_bits & 20: print("ERREUR DANS S0")
		if sbox == 1:
			self.key_estimators = map( lambda x:key_estimator2(sbox, x), [known_bits, known_bits+2] )
			if known_bits & 2: print("ERREUR DANS S0")
		if sbox == 2:
			self.key_estimators = map( lambda x:key_estimator2(sbox, x), [known_bits, known_bits+1] )
			if known_bits & 1: print("ERREUR DANS S0")
		if sbox == 4:
			self.key_estimators = map( lambda x:key_estimator2(sbox, x), [known_bits, known_bits+4] )
			if known_bits & 4: print("ERREUR DANS S0")
		if sbox == 6:
			self.key_estimators = map( lambda x:key_estimator2(sbox, x), [known_bits, known_bits+1, known_bits+2, known_bits+3] )
			if known_bits & 3: print("ERREUR DANS S0")
		if sbox == 7:
			self.key_estimators = map( lambda x:key_estimator2(sbox, x), [known_bits, known_bits+16] )
			if known_bits & 16: print("ERREUR DANS S0")
	
	def get_key(self):
		"Gives the current best key"
		if self.best_key == None:
			if self.__box_number == 0:
				marks= map( lambda i:self.key_estimators[i].get_mark(), range(4) )
			if self.__box_number == 1:
				marks= map( lambda i:self.key_estimators[i].get_mark(), range(2) )
			if self.__box_number == 2:
				marks= map( lambda i:self.key_estimators[i].get_mark(), range(2) )
			if self.__box_number == 4:
				marks= map( lambda i:self.key_estimators[i].get_mark(), range(2) )
			if self.__box_number == 6:
				marks= map( lambda i:self.key_estimators[i].get_mark(), range(4) )
			if self.__box_number == 7:
				marks= map( lambda i:self.key_estimators[i].get_mark(), range(2) )
			idx= marks.index( max(marks) )
			if self.__box_number == 0:
				self.best_key= 4*(idx % 2) + 16*(idx / 2)
			if self.__box_number == 1:
				self.best_key= 2*(idx % 2)
			if self.__box_number == 2:
				self.best_key= 1*(idx % 2)
			if self.__box_number == 4:
				self.best_key= 4*(idx % 2)
			if self.__box_number == 6:
				self.best_key= 1*(idx % 2) + 2*(idx / 2)
			if self.__box_number == 7:
				self.best_key= 16*(idx % 2)
			self.best_key+= self.__known_bits
		return self.best_key