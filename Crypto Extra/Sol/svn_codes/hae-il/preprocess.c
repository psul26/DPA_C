// (C) COPYRIGHT 2009 Korea University C.I.S.T (Center For Information Security Technologies)
// File Name : preprocess.c
// Type : C-file
// Author : Hae-Il Jung.

/************************************************************************************************************

Explan Function :

A) DB Preprocess(DB **Data)

: The function is implementation of select points and compress points. 

************************************************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include "Ref.h"
#include "preprocess.h"

DB Preprocess(DB **Data)
{
	UC str[256];
	DB *Wave,temp=0;
	UI i=0, j=0,k=0,Count=0,FileSize=0;
	FILE *fp, *fp1;

	Wave=(float *)malloc(sizeof(float)*Main_Point);
	
	fp=fopen("database_order.txt", "r");
	
	for(Count=0; Count<Tr_Num; Count++ )
	{	
		
		fscanf(fp, "%s", &str);
		

		if(fp  == NULL)
		{
			fprintf(stderr, "Error at opening %s\n", str);
			exit(0);
		}
		
		

		fp1=fopen(str, "r");

		fseek(fp1, 0, SEEK_END);
		FileSize = ftell(fp1);

		if(FileSize>0x05000000)  
			return -1;
	 
		fseek(fp1, 164, SEEK_SET);
		fread(Wave, 1, FileSize-164, fp1);
		fclose(fp1);

		// Compress Points 3 points to one.
		for(i=0; i<2 ; i++)
		{	

			j=3*i;
			for(k=j; k<j+3; k++)
			temp=temp+(Wave[14486+k]);
			
			Data[Count][i]=temp/3;
			temp=0;
		}	
	}

	free(Wave);
	fclose(fp);
	return 0;
}


