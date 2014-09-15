/*  File: CIND.C   Updated: 25-Feb-1997 14:06
Copyright (c) Fabrizio Aversa
===========================================================*/
/****************************************************************/
/**                                                            **/
/**                                                            **/
/**          JAVA, C, C++ indention program                    **/
/**                                                            **/
/**                        Fabrizio Aversa                     **/
/**                                                            **/
/**          First release :   30/08/89                        **/
/**          Latest Update :   26/02/97                        **/
/**                                                            **/
/**               Version 1.2                                  **/
/**                                                            **/
/****************************************************************/

#define INCL_DOS
#include <os2.h>

#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>
#include<string.h>
#include<time.h>

#define MAX_LINE_LEN 200

void TimeString1(PSZ);
INT getNextFile(HDIR*, PSZ, PSZ, INT*);

void main(int argc, char * argv[])
{
   int  indent= 0 ;
   int i, n, sw= 0;
   long iStatements= 0 ;
   char filename[90], source_name[90], out_name[90], path[90], ch ;
   char ext[90], *pp, act[MAX_LINE_LEN], temps[90] ;
   FILE *source, *out ;
   INT iFileCount = 0;
   HDIR FindHandle;

   printf("\nC Indent Utility CIND  ver. 1.2\n") ;
   printf("Copyright (c)  Fabrizio Aversa  1989-97\n") ;

   if (argc != 2) {
      printf ("file mask (e.g. *.java) : ");
      fflush(stdout);
      scanf("%s",source_name);
   }
   else
   {
      strcpy(source_name,argv[1]);
   }

   printf ("\n");

   /* see if source name contains path info */
   *path = 0;
   for (i = strlen(source_name); i >= 0 ; i--) {
      if(*(source_name + i) == '\\') {
         strncpy(path, source_name, i+1);
         break;
      }
   }

   while(getNextFile(&FindHandle, source_name, filename, &iFileCount))  {

      if ( (pp= strchr(filename,'.')) == NULL) {
         strcpy(ext,".cpp");
      }
      else
      {
         strcpy(ext,pp);
         *pp = '\0' ;
      }

      strcpy(out_name,path);
      strcpy(source_name,path);

      strcat(out_name,filename);
      strcat(source_name,filename);

      strcat(source_name,ext);
      strcat(out_name,".$$$");

      printf("%s%s: ", filename, ext);

      if ( (source = fopen(source_name,"rb")) == NULL )
      {
         strcpy(filename,"File open error : ");
         strcat(filename,source_name);
         printf ("%s\n\n",filename);
         exit(1);
      }

      if ( (out = fopen(out_name,"wb")) == NULL )
      {
         strcpy(filename,"File open error : ");
         strcat(filename,out_name);
         printf ("%s\n\n",filename);
         exit(1);
      }

      /* look for header line: get 1 line */
      i= 0;
      while ( (*(act+ i++) = getc(source)) != 10
      && (! feof(source)) && i <= MAX_LINE_LEN ) ;

      strcpy (temps, "/*  File:" ) ;

      if( act != strstr(act, temps) ) {
         fseek(source, (long)0, SEEK_SET);
      } else {
         /* ignore 2 lines */
         i= 0;
         while ( (*(act+ i++) = getc(source)) != 10
         && (! feof(source)) && i <= MAX_LINE_LEN ) ;
         i= 0;
         while ( (*(act+ i++) = getc(source)) != 10
         && (! feof(source)) && i <= MAX_LINE_LEN ) ;
      }
      /* print header lines */
      TimeString1(act);
      fprintf(out,
      "/*  File: %s%s   Updated: %s\r\nCopyright (c) Fabrizio Aversa\r\n===========================================================*/\r\n"
      ,filename, ext, act);

      do { /* scan file until end-of-it */

         /* get 1 line */
         i= 0;
         while ( (*(act+ i++) = getc(source)) != 10
         && (! feof(source)) && i <= MAX_LINE_LEN ) ;

         if ( i >= MAX_LINE_LEN ) {
            printf ("Line termination not found \n\n");
            exit(1);
         }

         /* scan line to update indent value */
         i= 0;
         while ( (ch= *(act+ i++)) != 10) {

            /* count semicolons */
         if(ch ==';' || ch =='{') iStatements++;    /* } only for indent !! */

            switch (ch) {
               case '{' : sw++ ;
                  continue;
               case '}' : indent -= 3;
            }
         }

         /* ignore leading blanks */
         i= 0;
         while ( (*(act+ i)) == ' ' ||  (*(act+ i)) == '\t') i++;

         /* indent */
         if ( act[i] != '#' ) {
            n= indent;
            while ( (n--) > 0) putc(' ',out);
         }
         if (sw) {
            indent += sw*3;
            sw= 0 ;
         }

         /* print line */
         if ( ! feof(source)) {
            do {
               ch=  *(act+ i++);
               putc(ch, out);
            } while ( ch != 10 );
         }
      } while ( ! feof(source)) ;

      fclose(out);
      fclose(source);

      remove (source_name) ;
      rename (out_name,source_name) ;

      printf("approx. %ld statements found.\n", iStatements) ;

   }

   printf ("\n");

}

/*===================================================
Time String
===================================================  */
void TimeString1(PSZ str)
{
   int i;
   char str1[25];

   time_t lt;
   lt=time(NULL);
   for (i=0; i< 24; i++) str1[i] = *(asctime(localtime(&lt))+i);

   memcpy(str, str1+8, 2); /* day */
   str[2]='-';
   memcpy(str+3, str1+4, 3); /* month */
   str[6]='-';
   memcpy(str+7, str1+20, 4); /* year */
   str[11]=' ';
   memcpy(str+12, str1+11, 5); /* time */
   str[17]=0;

}

/*===================================================
Directory
===================================================  */
INT getNextFile(HDIR *FindHandle, PSZ pszPath, PSZ pszRet, INT * iCount)
/* returns 0 if no more files */
{
   ULONG FindCount= 1;
   FILEFINDBUF3  FindBuffer;
   APIRET rc;

   if((*iCount) == 0) {

      *FindHandle= 0x0001;

      rc = DosFindFirst(pszPath,     /* File pattern */
      FindHandle, /* Directory search handle */
      0,     /* Search attribute */
      (PVOID) &FindBuffer,   /* Result buffer */
      sizeof(FindBuffer),  /* Result buffer length */
      &FindCount,  /* # of entries to find */
      FIL_STANDARD); /* Return level 1 file info */

   } else {

      rc = DosFindNext(*FindHandle, /* Directory handle */
      (PVOID) &FindBuffer,  /* Result buffer */
      sizeof(FindBuffer), /* Result buffer length */
      &FindCount);        /* Number of entries to find */

   }

   if(rc) {
      *pszRet= 0;
      DosFindClose(*FindHandle);
      return 0;
   }

   (*iCount)++;
   strcpy(pszRet, FindBuffer.achName);

   return 1;
}


