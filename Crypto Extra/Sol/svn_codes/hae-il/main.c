// (C) COPYRIGHT 2009 Korea University C.I.S.T (Center For Information Security Technologies)
// File Name : main.c
// Type : C-file
// Author : Hae-Il Jung.

/************************************************************************************************************

Explan Function :

A) UI Classification_Hw(DB** Wave, DB** Dif_Wave_0, DB** Dif_Wave_1, UI* Target_Hw, UI Count_Sbox, DB* Hw_Mean, UI Count_Num ) 

: The function is a classification function of trace by target-hamming weight.

B) UI Classification_Bit(DB** Wave, DB** Dif_Wave_0, DB** Dif_Wave_1, UC Target_Bit[][4], DB* Bit_Mean, UI Count_Num )

: The function is a classification function of trace by target-bit.

C) int Dif_Value(DB ** Dif_Wave_0, DB ** Dif_Wave_1, UI Length_0, UI Length_1, DB* Mean_Result)

: The function generates differential trace of classification trace.

D) UC Read(UC Plan[][8])

: The function read Cipher-text of database-order

E) UI Write_output(UC* DATA, UI Num)

: Write output.txt (result)

*************************************************************************************************************

Some explan variables :

Count_Sbox = sbox order.

Target_Sbox[8] = Target sbox order.

Guess_Key = Guess sub-key.

Whole_Sub_Key[8] = Correct sub-key by max peak.

Target_Hw[Tr_Num] = Hamming weight for output of target sbox.

Sum_Target_Hw[Tr_Num] = Sum of Hamming weight for correct sbox output (By builed-in determined sub-key)

Target_Bit[Tr_Num][4] = Target 4-bit for output of target sbox. 

Hw_Mean[Point] =  Differential trace (By Hamming Weight).

Bit_Mean[Point] = Sum of differential trace (By each of bit).

Result_Mean[Point] = Decision trace (From sum of each for Hw_Mean[Point] and Bit_Mean[Point].)

************************************************************************************************************/

#include <stdio.h>
#include "Ref.h"
#include "make_target.h"
#include "preprocess.h"


UI Classification_Hw(DB** Wave, DB** Dif_Wave_0, DB** Dif_Wave_1, UI* Target_Hw, UI Count_Sbox, DB* Hw_Mean, UI Count_Num );
UI Classification_Bit(DB** Wave, DB** Dif_Wave_0, DB** Dif_Wave_1, UC Target_Bit[][4], DB* Bit_Mean, UI Count_Num );
UI Dif_Value(DB ** Dif_Wave_0, DB ** Dif_Wave_1, UI Length_0, UI Length_1, DB* Mean_Result);
UC Read(UC Plan[][8]);
UI Write_output(UC* DATA, UI Num);

int main()
{
	// Define temp variables 
	UI i=0, j=0, Count_Sbox=0,Count_Num=0, Stability=0, Temp=0;
	UC Guess_Key=0, Target_Sbox[8]={0,7,6,3,4,5,1,2}, Whole_Sub_Key[8]={0,};
	UI Target_Hw[Tr_Num]={0,},Target_Info_Hw[Tr_Num]={0,}, Sum_Target_Hw[Tr_Num]={0,};
	UC Plan[Tr_Num][8]={0,}, Target_Bit[Tr_Num][4]={0,},Key_Check[8]={0x3c,0xb, 0x38, 0x2e, 0x16, 0x32, 0x10, 0x2c};
	DB Peak=0, Max_Peak=0;
	DB Hw_Mean[Point]={0,}, Bit_Mean[Point]={0,}, Result_Mean[Point]={0,};
	DB ** Wave, **Dif_Wave_0, **Dif_Wave_1;
	
	Wave=(double *)malloc(sizeof(double)*Tr_Num);
	Dif_Wave_0=(double *)malloc(sizeof(double)*Tr_Num);
	Dif_Wave_1=(double *)malloc(sizeof(double)*Tr_Num);
	
	for(i=0; i<Tr_Num; i++)
	{
		Wave[i]=(double*)malloc(sizeof(double)*Point);
		Dif_Wave_0[i]=(double*)malloc(sizeof(double)*Point);
		Dif_Wave_1[i]=(double*)malloc(sizeof(double)*Point);

		if(Wave[i] == NULL || Dif_Wave_0[i]==NULL || Dif_Wave_1[i]==NULL )
		{
			fprintf(stderr, "Memory allocation error [%d]\n", i);
			getchar();
		}
	}

	// Select Point and then compress 3 points to one
	Preprocess(Wave);
	
	// Read cipher-text
	Read(Plan);

	
for(Count_Num=0; Count_Num<109+Iteration; Count_Num++)
{
	for(Count_Sbox=0; Count_Sbox<8; Count_Sbox++)// Sbox order 
	{
		for(Guess_Key=0; Guess_Key<64; Guess_Key++)// Guess Key 0~63
		{

			for(i=0; i<Tr_Num; i++)
			{	// Generates target bit and target hamming weight of target sbox output
				make_target(Plan[i], Guess_Key, Target_Sbox[Count_Sbox], Target_Bit[i], &Target_Hw[i]);

				// Accumulate hamming weight for correct sbox output (By builed-in determined sub-key)
				Target_Hw[i]=Target_Hw[i]+Sum_Target_Hw[i];
			}

			// Classification
			Classification_Hw(Wave, Dif_Wave_0, Dif_Wave_1, Target_Hw, Count_Sbox, Hw_Mean, Count_Num);
			Classification_Bit(Wave, Dif_Wave_0, Dif_Wave_1, Target_Bit, Bit_Mean, Count_Num);
			
			// Generates decision trace Find peak and determine sub-key (from Decision trace )
			for(i=0; i<Point; i++)
				Result_Mean[i]=Bit_Mean[i]+Hw_Mean[i]*0.5;// Weight 0.5 for differential trace of hamming weight

			// Find peak and determine sub-key (from Decision trace )			
			for(i=0; i<Point; i++)
				Peak=Peak+Result_Mean[i];
						
			if(Peak>Max_Peak)
			{
				Max_Peak=Peak;
				Whole_Sub_Key[Target_Sbox[Count_Sbox]]=Guess_Key;

				// Save hamming weight for sbox output
				memcpy(Target_Info_Hw, Target_Hw, Tr_Num*sizeof(UI));
			}

			Peak=0;
		
		}

		// Update hamming weight for determined sbox output (By builed-in determined sub-key)
		memcpy(Sum_Target_Hw, Target_Info_Hw, Tr_Num*sizeof(UI));
		
		memset(Target_Hw, 0, Tr_Num*sizeof(UI));
		Max_Peak=0;
		
	}
	// Check correct key
	for(i=0; i<8; i++)
	{
		if(Whole_Sub_Key[i]==Key_Check[i])
		{
			Temp++;
		}
	}
	
	if(Temp==8)
	  Stability++;
	else
	  Stability=0;

	Temp=0;

    // Write output 
	Write_output( Whole_Sub_Key, Count_Num, Stability);

	memset(Sum_Target_Hw, 0, Tr_Num*4);
		
}	
    
	free(Wave);
	free(Dif_Wave_0);
	free(Dif_Wave_1);

	return 0;
}

UI Classification_Hw(DB** Wave, DB** Dif_Wave_0, DB** Dif_Wave_1, UI* Target_Hw, UI Count_Sbox, DB* Hw_Mean, UI Count_Num )
{
	UI i=0, j=0;
	UI Length_0=0, Length_1=0;

	// Classification of trace (By sum of target hamming weight/2)
	for(i=0; i<Count_Num; i++)
	{
		if(Target_Hw[i] < 2+2*Count_Sbox)
		{
			for(j=0; j<Point; j++)
				Dif_Wave_0[Length_0][j]=Wave[i][j];
			
			Length_0++;
			
		}
		else
		{
			for(j=0; j<Point; j++)
				Dif_Wave_1[Length_1][j]=Wave[i][j];
			
			Length_1++;
		}
	}
	
	Dif_Value(Dif_Wave_0, Dif_Wave_1, Length_0, Length_1, Hw_Mean);
	
	return 0;
}

UI Classification_Bit(DB** Wave, DB** Dif_Wave_0, DB** Dif_Wave_1, UC Target_Bit[][4], DB* Bit_Mean, UI Count_Num )
{
	UI i=0, j=0,k=0;
	UI Length_0=0, Length_1=0;
	DB Bit_Mean_Sum[Point]={0,};

	for(k=0; k<4; k++)
	{
		//Classification of trace (By each of bit)
		for(i=0; i<Count_Num; i++)
		{
			if(Target_Bit[i][k]==0)
			{
				for(j=0; j<Point; j++)
					Dif_Wave_0[Length_0][j]=Wave[i][j];
				
				Length_0++;
			}
			else
			{
				for(j=0; j<Point; j++)
					Dif_Wave_1[Length_1][j]=Wave[i][j];
				
				Length_1++;
			}
		}

		Dif_Value(Dif_Wave_0, Dif_Wave_1, Length_0, Length_1, Bit_Mean);

		// Sum each of differental trace 
		for(i=0; i<Point; i++)
			Bit_Mean_Sum[i]=Bit_Mean_Sum[i]+Bit_Mean[i]*0.125;// Weight 0.125 for differential trace of mono bit
				
		Length_0=0;
		Length_1=0;
	}
	
	memcpy(Bit_Mean, Bit_Mean_Sum, Point*sizeof(DB));

	return 0;
}

UI Dif_Value(DB ** Dif_Wave_0, DB ** Dif_Wave_1, UI Length_0, UI Length_1, DB* Mean_Result)
{	
	UI i=0,j=0;
	DB Mean_0[Point]={0,}, Mean_1[Point]={0,};
	
	// Mean of classfication trace
	for(i=0; i<Point; i++)
	{
		for(j=0; j<Length_0; j++)
		{
			Mean_0[i]=Mean_0[i]+Dif_Wave_0[j][i];
			
		}
		if(Length_0==0)
			Mean_0[i]=0;
		else
			Mean_0[i]=Mean_0[i]/Length_0;
		
	}
	
	// Mean of other classficaiton trace
	for(i=0; i<Point; i++)
		{
			for(j=0; j<Length_1; j++)
			{
				Mean_1[i]=Mean_1[i]+Dif_Wave_1[j][i];
				
			}
			
			if(Length_1==0)
				Mean_1[i]=0;
			else
				Mean_1[i]=Mean_1[i]/Length_1;
	
		}
	
	// Make Differential trace
	for(i=0; i<Point; i++)
	{
		Mean_Result[i]=Mean_0[i]-Mean_1[i];
		
		if(Mean_Result[i]<0)
			Mean_Result[i]=-Mean_Result[i];

	}

	return 0;
}

UC Read(UC Plan[][8])
{	
	UC Temp[100]={0,};
	UC x=0;
	UI j=0,i=0,k=0;
	FILE* fp;
	
	fp=fopen("database_order.txt", "r");
	
	if(fp  == NULL)
	{
		fprintf(stderr, "Error at opening %s\n", "database_order.txt");
		exit(0);
	}
	
	for(i=0; i<Tr_Num; i++)	
	{	
		fscanf(fp, "%s", &Temp);
		k=0;
		for(j=0; j<8; j++)
		{
			Plan[i][j]=Temp[j+73+k]-0x30;
			
			if(Plan[i][j]>=0x30)
				Plan[i][j]=Plan[i][j]-0x30+0x09;
			
			Plan[i][j]=Plan[i][j]<<4;
		
			x=Temp[j+74+k]-0x30;

			if(x>=0x30)
				x=x-0x30+0x09;
			
			Plan[i][j]=Plan[i][j]^x;
			k+=1;
		}
	}
	
	fclose(fp);

	return 0;

}

UI Write_output(UC* DATA, UI Num, UI Stability)
{
	UI i=0,j=0;
	FILE * fp;

	fp=fopen("output.txt", "r+");

	if(Num==0)
	{	
		fp=fopen("output.txt", "w");
		fseek(fp, 0, SEEK_SET);
		fprintf(fp, "# Table: secmatv1_2006_04_0809\n");
		fprintf(fp, "# Stability threshold: 100\n");
		fprintf(fp, "# Iteration threshold: 109\n");
		fprintf(fp, "# \n");
		fprintf(fp, "# Columns: Iteration Stability Subkey0 ... Subkey7\n");
	}
	else
	 fseek(fp, 0, SEEK_END);

	fprintf(fp,"\n%d\t%d\t",Num,Stability);

	for(i=0; i<8; i++)
	{
		fprintf(fp, "%d\t", DATA[i]);
	}

	fclose(fp);
	
	return 0;
}