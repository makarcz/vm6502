
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


unsigned char conv2bcd(unsigned short v)
{
   unsigned char arg8 = 0;
   arg8 = (unsigned char)((v/10) << 4);
   arg8 |= ((unsigned char)(v - (v/10)*10)) & 0x0F;
   return arg8;
}

char *conv24bitbin(unsigned char v)
{
   static char retbuf[5];
   int i=3;

   memset(retbuf, '0', 4);
   retbuf[4]=0;
   if (v == 0) return retbuf;
   while (v > 0 && i >= 0) {
      int r = v % 2;
      retbuf[i] = ((r==0) ? '0' : '1');
      v = v/2;
      i--;
   }

   return retbuf;
}

/* run this program using the console pauser or add your own getch, system("pause") or input loop */

int main(int argc, char *argv[])
{
   unsigned short v = 0;

   for (v = 0; v < 100; v++) {
      unsigned char cv = conv2bcd(v), hinyb, lonyb;
      hinyb = (cv & 0xF0) >> 4;
      lonyb = cv & 0x0F;
      char buf_hinyb[5], buf_lonyb[5];
      strcpy(buf_hinyb, conv24bitbin(hinyb));
      strcpy(buf_lonyb, conv24bitbin(lonyb));
      printf("%d (dec) \t= %4s(%d) %4s(%d) (BCD, dec:%d)\n", v,
                                 buf_hinyb,
                                 hinyb,
                                 buf_lonyb,
                                 lonyb,
                                 cv);
   }

   return 0;
}

