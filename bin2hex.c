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

void ScanArgs(int argc, char *argv[]);
void ConvertFile(void);

void Usage(char *prgn)
{
	printf("\nProgram: %s\n  Convert binary file to memory image definition for MKBASIC (VM65) emulator.\n\n", prgn);
	printf("Copyright: Marek Karcz 2016. All rights reserved.\n");
	printf("Free for personal and educational use.\n\n");
	printf("Usage:\n\n");
	printf("  %s -f input_fname -o output_fname [-w load_addr] [-x exec_addr] [-s] [-z]\n\n", prgn);
	printf("Where:\n\n");
	printf("  input_fname  - binary file name\n");
	printf("  output_fname - output file name\n");
	printf("  load_addr    - starting address to load data (default: %d)\n", g_nStartAddr);
	printf("  exec_addr    - address to auto-execute code from (default: %d)\n", g_nExecAddr);
	printf("  -s           - suppress auto-execute statement in output\n");
	printf("  -z           - suppress data blocks with 0-s only\n");
	printf("\n");
}

/*
 * bin2hex -f InputFile -o OutputFile -w StartAddr
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

      n++;
   }
}

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

int main(int argc, char *argv[])
{
	 if (argc == 1) 
	 	Usage(argv[0]);
	 else {
   	ScanArgs(argc, argv);
   	if (*g_szInputFileName == 0 || *g_szHexFileName == 0)
   		Usage(argv[0]);
   	else
   		ConvertFile();	 	
	 }
   return 0;
}


