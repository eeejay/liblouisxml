/* liblouisxml xml to Braille Translation Library

   This file may contain code borrowed from the Linux screenreader
   BRLTTY, copyright (C) 1999-2006 by
   the BRLTTY Team

   Copyright (C) 2004, 2005, 2006
   ViewPlus Technologies, Inc. www.viewplus.com
   and
   JJB Software, Inc. www.jjb-software.com
   All rigets reserved

   This file is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

In addition to the permissions and restrictions contained in the GNU
General Public License (GPL), the copyright holders grant two explicit
permissions and impose one explicit restriction. The permissions are:

1) Using, copying, merging, publishing, distributing, sublicensing, 
   and/or selling copies of this software that are either compiled or loaded 
   as part of and/or linked into other code is not bound by the GPL.

2) Modifying copies of this software as needed in order to facilitate 
   compiling and/or linking with other code is not bound by the GPL.

The restriction is:

3. The translation, semantic-action and configuration tables that are 
   read at run-time are considered part of this code and are under the terms 
   of the GPL. Any changes to these tables and any additional tables that are 
   created for use by this code must be made publicly available.

All other uses, including modifications not required for compiling or linking 
and distribution of code which is not linked into a combined executable, are 
bound by the terms of the GPL.

   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

   Maintained by John J. Boyer john.boyer@jjb-software.cem

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "louisxml.h"

UserData *ud;

int
main (int argc, char **argv)
{
/* translate braille files into text or xml using liblouisxml*/
  int curarg = 1;
  char *configFileName;
  char *inputFileName;
  char *outputFileName;
  char tempFileName[MAXNAMELEN];
  char *logFileName = NULL;
  FILE *inputFile = NULL;
  FILE *tempFile;
  int ch = 0;
  int pch = 0;
  int ppch = 0;
  int charsRead = 0;
  configFileName = "default.cfg";
  inputFileName = "stdin";
  outputFileName = "stdout";
  UserData *ud;

  if (argc > curarg)
    while (curarg < argc && argv[curarg][0] == '-')
      {
	switch (argv[curarg][1] | 32)
	  {
	  case 'h':
	    printf
	      ("Usage: xml2brl [-l] [-f configFile] [inputFile] [outputFile]\n");
	    printf ("[-l]: write errors to log file instead of stderr.");
	    printf
	      ("ConfigFile: configuration file name, default: default.cfg.\n");
	    printf ("inputFile : input file, '-' means stdin\n");
	    printf ("outputFile : output file\n");
	    printf ("xml2brl with no argumenst takes input on stdin\n");
	    printf ("and gives output on stdout.\n");
	    exit (0);
	  case 'l':
	    logFileName = "brl2xml.log";
	    break;
	  case 'f':
	    if (argc < (curarg + 1))
	      {
		fprintf (stderr, "No configuration file!");
		exit (1);
	      }
	    configFileName = argv[curarg + 1];
	    curarg++;
	    break;
	  default:
	    fprintf (stderr, "Invalid option %s.\n", argv[curarg]);
	    exit (1);
	  }
	curarg++;
      }
  if (argc > curarg)
    {
      if (argv[curarg][0] != '-')
	inputFileName = argv[curarg];
      curarg++;
    }
  if (argc > curarg)
    outputFileName = argv[curarg];
  if ((ud = lbx_initialize (configFileName, logFileName)) == NULL)
    exit (1);
  if (strcmp (inputFileName, "stdin") != 0)
    {
      if (!(inputFile = fopen (inputFileName, "r")))
	{
	  lou_logPrint ("Can't open file %s.\n", inputFileName);
	  exit (1);
	}
    }
  else
    inputFile = stdin;
  /*Create somewhat edited temporary file to facilitate use of stdin. */
  strcpy (tempFileName, ud->user_path);
  strcat (tempFileName, "brl2xml.temp");
  if (!(tempFile = fopen (tempFileName, "w")))
    {
      lou_logPrint ("Can't open temporary file.\n");
      exit (1);
    }
while ((ch = fgetc (inputFile)) != EOF)
    {
      if (ch == 13)
	continue;
      fputc (ch, tempFile);
ppch = pch;
      pch = ch;
      charsRead++;
    }
  fclose (tempFile);
  if (inputFile != stdin)
    fclose (inputFile);
  if (charsRead > 2)
	lbx_backTranslateFile (configFileName, tempFileName, 
outputFileName);
      lbx_free ();
  return 0;
}
