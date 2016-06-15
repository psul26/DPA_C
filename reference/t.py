dict = []
dict.append([])
dict.append([])
dict[0].append(0)
dict[0].append(1)
dict[0].append(2)
a_list = []
newFiles =[]
listOfLists =[]


def hamming_weight_fcn(str_message):
	hamming_weight = 0
	bin_rep = str(bin(str_message))
	bin_rep = bin_rep[2:]
	for i in range(len(bin_rep)):
		if bin_rep[i] == '1':
			hamming_weight += 1
	return hamming_weight


d = ((0^1)^0)^1
print d
class hat():
	
	def __init__(self,val):
		self.val = val

	def sq(self):
		self.val = self.val*self.val

	def add2(self):
		self.val +=2

cat = hat(3)
(cat.sq()).add2()
print cat.val
