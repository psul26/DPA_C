typedef unsigned int                    Word;
typedef unsigned long long              Dword;
typedef unsigned char                   Byte;
typedef unsigned short                  Short;

#define DWORDCONST(x)                           (x##LL)
#define WORDCONST(x)                            (x##L)
#define BASE                                    DWORDCONST(0x100000000)
#define ONE                                     DWORDCONST(0x1)
#define MaxWORD                                 WORDCONST(0xFFFFFFFF)

#define GETBIT(a, b)(((a) >> (b)) & ONE)
#define SETBIT(a, b)                            ((a) = (a) | (ONE << (b)))
#define CLRBIT(a, b)                            ((a) = (a) & (~(ONE << (b))))
#define PUTBIT(a, b, c)                         (((c) == 1) ? SETBIT((a), (b)) : CLRBIT((a), (b)))

#define STIRBITS(a, b, c, d)            { d = 0; for (int ii = 0; ii < (b); ii++) PUTBIT((d), ii, GETBIT((a), (c)[ii])); }


class DesBlock 
{
public:
  static const Word wMask[];

  static const Byte bShift[];

	// Table for key permutation
  static const Byte bPC1[];

	// Table for compression permutation
  static const Byte bPC2[];

	// Table for initial permutation IP
  static const Byte bIP1[];

	// Table for expansion permutation
  static const Byte bE[];

	// Table for P-Box permutation
  static const Byte bP[];
				
  Dword applyTable(Dword input, int nbits, Byte table[], int length, int direc = 1);
	Byte applyS(int iBox, Byte input);
	
};

