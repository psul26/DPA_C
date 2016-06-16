// (C) COPYRIGHT 2009 Korea University C.I.S.T (Center For Information Security Technologies)
// File Name : make_target.c
// Type : C-file
// Author : Hae-Il Jung.

/************************************************************************************************************

Explan Function :

A) int make_target(UC* Plan, UC Target_Subkey, UI Target_Sbox, UC* Target_Bit, UI *Hw) 

: The function generates target-bit and target-hamming weight of target-sbox output for 16 round. 

B) int Hex_to_Bit(UC A, UC *B, UI Num, UI Bit_Num)

: Hex code transform to bit code.
 
C) int F_FT(UC *Input, UI Select_Sbox)

: F function of Des.

************************************************************************************************************/


#include <stdio.h>
#include "Ref.h"
#include "make_target.h"


UC IP[64] ={ 58, 50, 42, 34, 26, 18, 10, 2, 
			 60, 52, 44, 36, 28, 20, 12, 4,
			 62, 54, 46, 38, 30, 22, 14, 6, 
			 64, 56, 48, 40, 32, 24, 16, 8,
			 57, 49, 41, 33, 25, 17, 9, 1, 
			 59, 51, 43, 35, 27, 19, 11, 3,
			 61, 53, 45, 37, 29, 21, 13, 5, 
			 63, 55, 47, 39, 31, 23, 15, 7  };

UC Expan[48] = { 32, 1, 2, 3, 4, 5, 4, 5, 6, 7, 8, 9, 8, 
				  9, 10, 11, 12, 13, 12, 13, 14, 15, 16, 17,
			     16, 17, 18, 19, 20, 21, 20, 21, 22, 23, 24, 25,
	             24, 25, 26, 27, 28, 29, 28, 29, 30, 31, 32, 1 };

UC S_box[8][4][16] = { { {14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7},
						{ 0, 15, 7, 4, 14, 2, 13, 1, 10, 6, 12, 11, 9, 5, 3, 8},
						{ 4, 1, 14, 8, 13, 6, 2, 11, 15, 12, 9, 7, 3, 10, 5, 0},
						{15, 12, 8, 2, 4, 9, 1, 7, 5, 11, 3, 14, 10, 0, 6, 13} },
						{ {15, 1, 8, 14, 6, 11, 3, 4, 9, 7, 2, 13, 12, 0, 5, 10},
						{ 3, 13, 4, 7, 15, 2, 8, 14, 12, 0, 1, 10, 6, 9, 11, 5},
						{ 0, 14, 7, 11, 10, 4, 13, 1, 5, 8, 12, 6, 9, 3, 2, 15},
						{13, 8, 10, 1, 3, 15, 4, 2, 11, 6, 7, 12, 0, 5, 14, 9} },
						{ {10, 0, 9, 14, 6, 3, 15, 5, 1, 13, 12, 7, 11, 4, 2, 8},
						{13, 7, 0, 9, 3, 4, 6, 10, 2, 8, 5, 14, 12, 11, 15, 1},
						{13, 6, 4, 9, 8, 15, 3, 0, 11, 1, 2, 12, 5, 10, 14, 7},
						{ 1, 10, 13, 0, 6, 9, 8, 7, 4, 15, 14, 3, 11, 5, 2, 12} },
						{ { 7, 13, 14, 3, 0, 6, 9, 10, 1, 2, 8, 5, 11, 12, 4, 15},
						{13, 8, 11, 5, 6, 15, 0, 3, 4, 7, 2, 12, 1, 10, 14, 9},
						{10, 6, 9, 0, 12, 11, 7, 13, 15, 1, 3, 14, 5, 2, 8, 4},
						{ 3, 15, 0, 6, 10, 1, 13, 8, 9, 4, 5, 11, 12, 7, 2, 14 } },
						{ { 2, 12, 4, 1, 7, 10, 11, 6, 8, 5, 3, 15, 13, 0, 14, 9},
						{14, 11, 2, 12, 4, 7, 13, 1, 5, 0, 15, 10, 3, 9, 8, 6},
						{ 4, 2, 1, 11, 10, 13, 7, 8, 15, 9, 12, 5, 6, 3, 0, 14},
						{11, 8, 12, 7, 1, 14, 2, 13, 6, 15, 0, 9, 10, 4, 5, 3 } },
						{ {12, 1, 10, 15, 9, 2, 6, 8, 0, 13, 3, 4, 14, 7, 5, 11},
						{10, 15, 4, 2, 7, 12, 9, 5, 6, 1, 13, 14, 0, 11, 3, 8},
						{ 9, 14, 15, 5, 2, 8, 12, 3, 7, 0, 4, 10, 1, 13, 11, 6},
						{ 4, 3, 2, 12, 9, 5, 15, 10, 11, 14, 1, 7, 6, 0, 8, 13 } },
						{ { 4, 11, 2, 14, 15, 0, 8, 13, 3, 12, 9, 7, 5, 10, 6, 1},
						{13, 0, 11, 7, 4, 9, 1, 10, 14, 3, 5, 12, 2, 15, 8, 6},
						{ 1, 4, 11, 13, 12, 3, 7, 14, 10, 15, 6, 8, 0, 5, 9, 2},
						{ 6, 11, 13, 8, 1, 4, 10, 7, 9, 5, 0, 15, 14, 2, 3, 12 } },
						{ {13, 2, 8, 4, 6, 15, 11, 1, 10, 9, 3, 14, 5, 0, 12, 7}, 
						{ 1, 15, 13, 8, 10, 3, 7, 4, 12, 5, 6, 11, 0, 14, 9, 2},
						{ 7, 11, 4, 1, 9, 12, 14, 2, 0, 6, 10, 13, 15, 3, 5, 8},
						{ 2, 1, 14, 7, 4, 10, 8, 13, 15, 12, 9, 0, 3, 5, 6, 11 } } };

InverP[32]={ 9, 17, 23, 31, 13, 28,  2, 18, 24, 16, 30,  6,
			26, 20, 10,  1,  8, 14, 25,  3,  4, 29, 11, 19,
			32, 12, 22,  7,  5, 27, 15, 21 };


UI make_target(UC* Plan, UC Target_Subkey, UI Target_Sbox, UC* Target_Bit, UI *Hw)
{
	
	// Define temp variables 
	UI i=0;
	UC Bit_Plan[64]={0,}, IP_Plan[64]={0,}, Lc[32]={0,}, Rc[32]={0,}, Expan_Plan[48]={0,}, Target_Bit_Subkey[6]={0,};
	UC In_Sbox[6]={0,}, Out_Sbox=0, Pinv[32]={0,};
	
	// Hex code of text transform to bit(8) code
	for(i=0; i<8; i++)
		Hex_to_Bit( Plan[i], Bit_Plan, 8*i, 8);
	
	// Initial  Permutation 
	for(i=0; i<64; i++)
	{
		IP_Plan[i]=Bit_Plan[IP[i]-1];
		
		if(i<32)
			Lc[i]=IP_Plan[i];
		else
			Rc[i-32]=IP_Plan[i];
	}
	
	// Expansion of text
	for(i=0; i<48; i++)
		Expan_Plan[i]=Rc[Expan[i]-1];

	// Hex code of guess-key transform to bit(6) code
	Hex_to_Bit(Target_Subkey, Target_Bit_Subkey, 0, 6);
	
	// Text Xor guessed sub-key 
	for(i=0; i<6; i++)
		In_Sbox[i]=Expan_Plan[i+(6*Target_Sbox)]^Target_Bit_Subkey[i];
	
	// F function
	Out_Sbox=F_FT(In_Sbox, Target_Sbox);

	// Hex code of sbox-output transform to bit(4) code
	Hex_to_Bit(Out_Sbox, Target_Bit, 0, 4);
	
	// Inver P-Permutation, generated target-bit and target-hamming weight by Lc Xor Rc Xor target-sbox output
	for(i=0; i<32; i++)
		Pinv[i]=Lc[InverP[i]-1]^Rc[InverP[i]-1];
	
	for(i=0; i<4; i++)
		Target_Bit[i]=Pinv[i+(4*Target_Sbox)]^Target_Bit[i]; // Target-bit

	*Hw = Target_Bit[0]+Target_Bit[1]+Target_Bit[2]+Target_Bit[3]; // Target-hamming weight
	
	return 0;
	
}


int Hex_to_Bit(UC A, UC *B, UI Num, UI Bit_Num)
{
	UI i=0,j=0;
	
	for(j=0; j<Bit_Num; j++)
		B[Bit_Num-(j+1)+Num]=(A>>(j))&0x1;
	
	return 0;

}


int F_FT(UC *Input, UI Select_Sbox)
{
	UI Sout=0, Sin=0;
	UI i=0;
	
	Sout=(Input[0]<<1)^(Input[5]);
	Sin=(Input[1]<<3)^(Input[2]<<2)^(Input[3]<<1)^(Input[4]);
	
	return S_box[Select_Sbox][Sout][Sin];

}
