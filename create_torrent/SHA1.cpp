#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

//#include <conio.h>
//#include <wtypes.h>

typedef /** [string] */ char *LPSTR;

void creat_w(unsigned char input[64],uint32_t w[80])
{
   int i,j;uint32_t temp,temp1;
   for(i=0;i<16;i++)
          {
             j=4*i;
             w[i]=((uint32_t)input[j])<<24 |((uint32_t)input[1+j])<<16|((uint32_t)input[2+j])<<8|((uint32_t)input[3+j])<<0;

          }
   for(i=16;i<80;i++)
         {
             w[i]=w[i-16]^w[i-14]^w[i-8]^w[i-3];
             temp=w[i]<<1;
             temp1=w[i]>>31;
             w[i]=temp|temp1;

         }
}
char ms_len(long a,char intput[64])
{
    unsigned long temp3,p1;  int i,j;
    temp3=0;
    p1=~(~temp3<<8);
    for(i=0;i<4;i++)
       {
          j=8*i;
          intput[63-i]=(char)((a&(p1<<j))>>j);

       }
	return '0';
}

void gethash(unsigned char* input, char* hash)
{
   uint32_t H0=0x67452301,H1=0xefcdab89,H2=0x98badcfe,H3=0x10325476,H4=0xc3d2e1f0;
   uint32_t A,B,C,D,E,temp,temp1,temp2,temp3,k,f;
   int i,flag;
   uint32_t w[80];
   long x;
   int n;

   n=strlen((LPSTR)input);
   if(n<57)
          {
                 x=n*8;
                 ms_len(x,(char*)input);
                 if(n==56)
                     for(i=n;i<60;i++)
                     input[i]=0;
                 else
                    {
                     input[n]=128;
                     for(i=n+1;i<60;i++)
                     input[i]=0;
                    }

          }

   creat_w(input,w);
   A=H0;B=H1;C=H2;D=H3;E=H4;
   for(i=0;i<80;i++)
         {
               flag=i/20;
               switch(flag)
                  {
                   case 0: k=0x5a827999;f=(B&C)|(~B&D);break;
                   case 1: k=0x6ed9eba1;f=B^C^D;break;
                   case 2: k=0x8f1bbcdc;f=(B&C)|(B&D)|(C&D);break;
                   case 3: k=0xca62c1d6;f=B^C^D;break;
                  }
               temp1=A<<5;
               temp2=A>>27;
               temp3=temp1|temp2;
               temp=temp3+f+E+w[i]+k;
               E=D;
               D=C;

               temp1=B<<30;
               temp2=B>>2;
               C=temp1|temp2;
               B=A;
               A=temp;
         }
   H0=H0+A;
   H1=H1+B;
   H2=H2+C;
   H3=H3+D;
   H4=H4+E;
   sprintf(hash,"%08X%08X%08X%08X%08X",H0,H1,H2,H3,H4);
}
