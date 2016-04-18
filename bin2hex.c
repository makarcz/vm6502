/*
 *----------------------------------------------------------------------------
 * File:  bin2hex.c
 *
 * Author: Marek Karcz
 *
 * Date created: 3/8/2016
 *
 * Purpose: Convert binary file to memory image definition (plain text) file.
 *----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const int DEBUG = 0;
const char *hdr1 = "; Created with BIN2HEX (C) Marek Karcz 2016. All rights reserved.\n";

char g_szInputFileName[256] = {0};
char g_szHexFileName[256] = {0};
int g_nStartAddr = 2048; /* $0800 */
int g_nExecAddr = 2048;	/* $0800 */
int g_nSuppressAutoExec = 1;
int g_nSuppressAllZeroRows = 0;
int g_nConvert2IntelHex = 0;

void ScanArgs(int argc, char *argv[]);
void ConvertFile(void);
void Convert2IntelHex(void);

/*
 *--------------------------------------------------------------------
 * Method:    Usage()
 * Purpose:   Print usage information/help.
 * Arguments: char * - program name.
 * Returns:   n/a
 *--------------------------------------------------------------------
 */
void Usage(char *prgn)
{
  printf("\nProgram: %s\n  Convert binary file to Intel HEX format.\nOR\n", prgn);
	printf("  Convert binary file to memory image definition for MKBASIC (VM65) emulator.\n\n");
	printf("Copyright: Marek Karcz 2016. All rights reserved.\n");
	printf("Free for personal and educational use.\n\n");
	printf("Usage:\n\n");
	printf("  %s -f input -o output [-w addr] [-x exec] [[-s] [-z] | -i]\n\n", prgn);
	printf("Where:\n\n");
	printf("  input  - binary file name\n");
	printf("  output - output file name\n");
	printf("  addr   - starting address to load data (default: %d)\n", g_nStartAddr);
	printf("  exec   - address to auto-execute code from (default: %d)\n", g_nExecAddr);
	printf("  -s     - suppress auto-execute statement in output\n");
	printf("  -z     - suppress data blocks with 0-s only\n");
  printf("  -i     - convert to Intel HEX format\n");
  printf("           NOTE: When this switch is used, addr, exec, -s, -z are ignored,\n");
  printf("                 addr = 0, exec is not set and data blocks with 0-s only\n");
  printf("                 are always suppressed.\n");
	printf("\n");
}

/*
 *--------------------------------------------------------------------
 * Method:    ScanArgs()
 * Purpose:   Scan/parse command line arguments and set internal
 *            flags and parameters.
 * Arguments: int argc - # of command line arguments,
 *            char *argv[] - array of command line arguments.
 * Returns:   n/a
 *--------------------------------------------------------------------
 */
void ScanArgs(int argc, char *argv[])
{
   int n = 1;

   while (n < argc)
   {
      if (strcmp(argv[n], "-f") == 0)
      {
         n++;
         strcpy(g_szInputFileName,argv[n]);
      }
      else if (strcmp(argv[n],"-o") == 0)
      {
         n++;
         strcpy(g_szHexFileName,argv[n]);
      }
      else if (strcmp(argv[n],"-w") == 0)
      {
         n++;
         g_nStartAddr = atoi(argv[n]);
		 		 g_nExecAddr = g_nStartAddr;
		 		 g_nSuppressAutoExec = 0;
      }
      else if (strcmp(argv[n],"-x") == 0)
      {
         n++;
         g_nExecAddr = atoi(argv[n]);
		 		 g_nSuppressAutoExec = 0;
      }
      else if (strcmp(argv[n],"-s") == 0)
      {
		 		 g_nSuppressAutoExec = 1;
      }
      else if (strcmp(argv[n],"-z") == 0)
      {
		 		 g_nSuppressAllZeroRows = 1;
      }
      else if (strcmp(argv[n],"-i") == 0)
      {
         g_nConvert2IntelHex = 1;
      }      

      n++;
   }
}

/*
 *--------------------------------------------------------------------
 * Method:    ConvertFile()
 * Purpose:   Convert binary file to plain text memory definition
 *            file for MKBASIC (VM65) emulator.
 * Arguments: n/a
 * Returns:   n/a
 *--------------------------------------------------------------------
 */
void ConvertFile(void)
{
   FILE *fpi = NULL;
   FILE *fpo = NULL;
   unsigned char bt[17];
   char hex[80];
   int i, addr, allzero, prev_allzero;

   addr = g_nStartAddr;
   printf("Processing...\n");
   printf("Start address: $%04x\n", addr);
   if (NULL != (fpi = fopen(g_szInputFileName,"rb")))
   {
      if (NULL != (fpo = fopen(g_szHexFileName,"w")))
      {
      	time_t t = time(NULL);
				struct tm *tm = localtime(&t);
				char s[64] = {0};
				strftime(s, sizeof(s), "; %c\n", tm);

      	 fputs(hdr1, fpo);
      	 fputs(s, fpo);
      	 sprintf(hex, "ADDR\n$%04x\nORG\n$%04x\n", g_nExecAddr, addr);
      	 if (DEBUG) printf("Adding line:\n%s\n", hex);
      	 fputs(hex,fpo);
      	 prev_allzero = 1;
         while(0 == feof(fpi) && addr <= 0xFFFF)
         {
					 memset(bt, 0, 17);
           memset(hex, 0, 80);
					 if (DEBUG) printf("Reading input file...");
           fread(bt, sizeof(char), 16, fpi);
					 if (DEBUG) printf("done.\n");
					 if (DEBUG) printf("Preparing hex string...\n");
					 allzero = 1;
           for(i=0; i<16; i++)
           {
			   		 if (DEBUG) printf("Code: %d\n", bt[i]);
			   		 if (*hex == 0) sprintf(hex, "$%02x", bt[i]);
			   		 else sprintf(hex, "%s $%02x", hex, bt[i]);
			   		 if (allzero && bt[i] > 0)
			      		allzero = 0;
           }
					 if (g_nSuppressAllZeroRows && prev_allzero && 0 == allzero) {
					 		char buff[20];
					 		sprintf (buff, "ORG\n$%04x\n", addr);
					 		fputs(buff, fpo);
					 }           
					 if (0 == g_nSuppressAllZeroRows
							||
							(g_nSuppressAllZeroRows && 0 == allzero)
			   			)
					 {
          	 sprintf(hex, "%s\n", hex);
						 if (DEBUG) printf("Adding line: %s", hex);
             fputs(hex, fpo);
					 }
           addr += 16;
           prev_allzero = allzero;
         }
		 		 if (0 == g_nSuppressAutoExec)
		 		 {
           memset(hex, 80, sizeof(char));
           sprintf(hex, "EXEC\n$%04x\n", g_nExecAddr);
		    	 if (DEBUG) printf("Adding line: %s", hex);
           fputs(hex, fpo);
		 		 }
         fclose(fpi);
         fclose(fpo);
		 		 printf("Done.\n");
		 		 printf("End address: $%04x\n", (addr <= 0xFFFF) ? addr : 0xFFFF);
		 		 printf("Run address: $%04x\n", g_nExecAddr);
      }
      else
         printf("ERROR: Unable to create output file.\n");
   }
   else
      printf("ERROR: Unable to open input file.\n");
}

/*
 *--------------------------------------------------------------------
 * Method:    Convert2IntelHex()
 * Purpose:   Convert binary file to Intel HEX format.
 * Arguments: n/a
 * Returns:   n/a
 *--------------------------------------------------------------------
 */
void Convert2IntelHex(void)
{
   FILE *fpi = NULL;
   FILE *fpo = NULL;
   unsigned char bt[17], cksum;
   char hex[80];
   int i, allzero;
   unsigned int addr;

   addr = 0;
   g_nSuppressAllZeroRows = 1;
   g_nExecAddr = 0;
   printf("Processing...\n");
   printf("Start address: $%04x\n", addr);
   if (NULL != (fpi = fopen(g_szInputFileName,"rb")))
   {
      if (NULL != (fpo = fopen(g_szHexFileName,"w")))
      {
         while(0 == feof(fpi) && addr <= 0xFFFF)
         {
           memset(bt, 0, 17);
           memset(hex, 0, 80);
           if (DEBUG) printf("Reading input file...");
           fread(bt, sizeof(char), 16, fpi);
           if (DEBUG) printf("done.\n");
           if (DEBUG) printf("Preparing hex string...\n");
           /* start the Intel HEX data record, all data blocks
              generated by this program are 16-bytes long
           */
           sprintf(hex, ":10%04x00", addr);
           cksum = 0;
           allzero = 1;
           /* append data to record */
           for(i=0; i<16; i++)
           {
             cksum += bt[i];
             if (DEBUG) printf("Code: %d\n", bt[i]);
             sprintf(hex, "%s%02x", hex, bt[i]);
             if (allzero && bt[i]) allzero = 0;
           }
           cksum = ~cksum + 1;  /* Two's complement of modulo 256 sum */
           /* end record with check sum value */
           sprintf(hex, "%s%02x", hex, cksum);
           /* output only if non-zero data present in 16-byte block */
           if (0 == g_nSuppressAllZeroRows
              ||
              (g_nSuppressAllZeroRows && 0 == allzero)
              )
           {
             sprintf(hex, "%s\n", hex);
             if (DEBUG) printf("Adding line: %s", hex);
             fputs(hex, fpo);
           }
           addr += 16;
         }
         /* add EOF */
         hex[0] = 0;
         strcpy(hex, ":00000001FF");
         sprintf(hex, "%s\n", hex);
         fputs(hex, fpo);
         fclose(fpi);
         fclose(fpo);
         printf("Done.\n");
         printf("End address: $%04x\n", (addr <= 0xFFFF) ? addr : 0xFFFF);
         printf("Run address: $%04x\n", g_nExecAddr);
      }
      else
         printf("ERROR: Unable to create output file.\n");
   }
   else
      printf("ERROR: Unable to open input file.\n");
}

/*
 *--------------------------------------------------------------------
 * Method:    main()
 * Purpose:   Main program loop/routine.
 * Arguments: int argc - # of provided in command line arguments.
 *            char *argv[] - array of command line arguments.
 * Returns:   int - always 0.
 *--------------------------------------------------------------------
 */
int main(int argc, char *argv[])
{
	 if (argc == 1) 
	 	Usage(argv[0]);
	 else {
   	ScanArgs(argc, argv);
   	if (*g_szInputFileName == 0 || *g_szHexFileName == 0)
   		Usage(argv[0]);
   	else {
      if (0 == g_nConvert2IntelHex)
        ConvertFile();
      else
        Convert2IntelHex();
    }
	 }
   return 0;
}


