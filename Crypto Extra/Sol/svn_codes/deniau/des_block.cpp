#include "des_block.h"
#include "des_sboxes.h"

#include <iostream>

const Word DesBlock::wMask[] = { 1, 1, 3, 3, 3, 3, 3, 3,
					 1, 3, 3, 3, 3, 3, 3, 1 };

const Byte DesBlock::bShift[] = { 1, 1, 2, 2, 2, 2, 2, 2,
					  1, 2, 2, 2, 2, 2, 2, 1 };

// Table for key permutation
const Byte DesBlock::bPC1[] = { 56, 48, 40, 32, 24, 16,  8,
					 0, 57, 49, 41, 33, 25, 17,
					 9,  1, 58, 50, 42, 34, 26,
					18, 10,  2, 59, 51, 43, 35,
					62, 54, 46, 38, 30, 22, 14,
					 6, 61, 53, 45, 37, 29, 21,
					13,  5, 60, 52, 44, 36, 28,
					20, 12,  4, 27, 19, 11,  3 };

	// Table for compression permutation
const Byte DesBlock::bPC2[] = { 13, 16, 10, 23,  0,  4,
					 2, 27, 14,  5, 20,  9,
					22, 18, 11,  3, 25,  7,
					15,  6, 26, 19, 12,  1,
					40, 51, 30, 36, 46, 54,
					29, 39, 50, 44, 32, 47,
					43, 48, 38, 55, 33, 52,
					45, 41, 49, 35, 28, 31 };

	// Table for initial permutation IP
const Byte DesBlock::bIP1[] = { 57, 49, 41, 33, 25, 17,  9,  1,
					59, 51, 43, 35, 27, 19, 11,  3,
					61, 53, 45, 37, 29, 21, 13,  5,
					63, 55, 47, 39, 31, 23, 15,  7,
					56, 48, 40, 32, 24, 16,  8,  0,
					58, 50, 42, 34, 26, 18, 10,  2,
					60, 52, 44, 36, 28, 20, 12,  4,
					62, 54, 46, 38, 30, 22, 14,  6 };

	// Table for expansion permutation
const Byte DesBlock::bE[] = { 31,  0,  1,  2,  3,  4,
				   3,  4,  5,  6,  7,  8,
				   7,  8,  9, 10, 11, 12,
				  11, 12, 13, 14, 15, 16,
				  15, 16, 17, 18, 19, 20,
				  19, 20, 21, 22, 23, 24,
				  23, 24, 25, 26, 27, 28,
				  27, 28, 29, 30, 31,  0 };

	// Table for P-Box permutation
const Byte DesBlock::bP[] = { 15,  6, 19, 20, 28, 11, 27, 16,
				   0, 14, 22, 25,  4, 17, 30,  9,
				   1,  7, 23, 13, 31, 26,  2,  8,
				  18, 12, 29,  5, 21, 10,  3, 24 };
				
Dword DesBlock::applyTable(Dword input, int inputBits, Byte table[], int length, int direc) {
		Dword res = 0;
//		std::cerr << "input is " << std::hex << input << " length " << std::dec << length << " direc " << direc << std::endl;
		if (direc == -1) {
			for (int i=0; i<length; i++)
				PUTBIT(res, length-table[i]-1, GETBIT(input, inputBits-i-1));
			return res;
		}
		else { 
			for (int i=0; i<length; i++)
				PUTBIT(res, length-i-1, GETBIT(input, inputBits-table[i]-1));
		//	std::cerr << "res is " << std::hex << res << std::endl;
			return res;
		}
	}
	
Byte DesBlock::applyS(int iBox, Byte input) {
	return bS[iBox][input];
}
