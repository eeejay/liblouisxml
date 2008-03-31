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
// #include <libxml2/HTMLparser.h>
UserData *ud = NULL;

static void
liblouisxmlErrors (void *ctx, xmlChar * msg, ...)
{
  va_list args;
  char buffer[MAXNAMELEN];
  va_start (args, msg);
  vsnprintf ((char *) buffer, sizeof (buffer), (char *) msg, args);
  va_end (args);
  lou_logPrint ("%s", buffer);
}

static void
initLibxml2 (void)
{
  static int initialized = 0;
  if (initialized)
    return;
  initialized = 1;
  LIBXML_TEST_VERSION xmlKeepBlanksDefault (0);
  xmlSubstituteEntitiesDefault (1);
//  initGenericErrorDefaultFunc ((xmlGenericErrorFunc *) 
//  liblouisxmlErrors);
}

static int
processXmlDocument (xmlDoc * doc)
{
  xmlNode *rootElement = NULL;
  int haveSemanticFile;
  if (doc == NULL)
    return -1;
  rootElement = xmlDocGetRootElement (doc);
  if (rootElement == NULL)
    return -1;
  haveSemanticFile = compile_semantic_table (rootElement);
  examine_document (rootElement);
  append_new_entries ();
  if (!haveSemanticFile)
    return -2;
  transcribe_document (rootElement);
  return 1;
}

void *
lbx_initialize (const char *const configFileName, const char const 
*logFileName)
{
  initLibxml2 ();
  if (!read_configuration_file (configFileName, logFileName))
    return NULL;
  return (void *) ud;
}

int
lbx_translateString (const char *const configFileName, char *inbuf,
		     widechar * outbuf, int *outlen)
{
/* Translate the well-formed xml expression in inbuf into braille 
* according to the specifications in configFileName. If the expression 
* is not well-formed or there are oteer errors, return a negative 
* number indicating the error.*/
  int k;
  char *xmlInbuf;
  int inlen = strlen (inbuf);
  xmlDoc *doc;
  initLibxml2 ();
  if (!read_configuration_file (configFileName, NULL))
    return -3;
  ud->inbuf = inbuf;
  ud->inlen = inlen;
  ud->outbuf = outbuf;
  ud->outlen = *outlen;
  for (k = 0; k < inlen; k++)
    if (inbuf[k] > ' ')
      break;
  if (inbuf[k] != '<')
    {
      transcribe_text_string ();
      *outlen = ud->outlen_so_far;
      return 1;
    }
  if (inbuf[k + 1] == '?')
    xmlInbuf = inbuf;
  else
    {
      inlen += strlen (ud->xml_header);
      if (!(xmlInbuf = malloc (inlen + 4)))
	return -2;
      strcpy (xmlInbuf, ud->xml_header);
      strcat (xmlInbuf, "\n");
      strcat (xmlInbuf, inbuf);
    }
  doc = xmlParseMemory ((char *) xmlInbuf, inlen);
  processXmlDocument (doc);
  xmlFreeDoc (doc);
  xmlCleanupParser ();
  *outlen = ud->outlen_so_far;
  if (xmlInbuf != inbuf)
    free (xmlInbuf);
  return 1;
}

int
  lbx_translateFile
  (char *configFileName, char *inFileName, char *outFileName)
{
/* Translate the well-formed xml expression in inFileName into 
* braille according to the specifications in configFileName. If the 
* expression is not well-formed or there are oteer errors, return a negative 
* number indicating the error.*/
  widechar outbuf[2 * BUFSIZE];
  xmlParserCtxtPtr ctxt;
  xmlDoc *doc;
  if (!read_configuration_file (configFileName, NULL))
    return -3;
  ud->outbuf = outbuf;
  ud->outlen = (sizeof (outbuf) / CHARSIZE) - 4;
  if (strcmp (outFileName, "stdout"))
    {
      if (!(ud->outFile = fopen (outFileName, "w")))
	{
	  lou_logPrint ("Can't open file %s.", outFileName);
	  return -3;
	}
    }
  else
    ud->outFile = stdout;
  initLibxml2 ();
  if (ud->internet_access)
    {
      ctxt = xmlNewParserCtxt ();
      doc = xmlCtxtReadFile (ctxt, inFileName, NULL, XML_PARSE_DTDVALID |
			     XML_PARSE_NOENT);
    }
  else
    doc = xmlParseFile (inFileName);
// if (doc == NULL)
// doc = htmlParseFile (inFileName, NULL);
  processXmlDocument (doc);
  xmlFreeDoc (doc);
  if (ud->internet_access)
    xmlFreeParserCtxt (ctxt);
  else
    xmlCleanupParser ();
  xmlCleanupParser ();
  if (ud->outFile != stdout)
    fclose (ud->outFile);
  return 1;
}

int
  lbx_translateTextFile
  (char *configFileName, char *inFileName, char *outFileName)
{
/* Translate the text file in inFileName into braille according to
* the specifications in configFileName. If there are errors, return a negative
* number indicating the error.*/
  if (!read_configuration_file (configFileName, NULL))
    return -3;
  if (strcmp (inFileName, "stdin"))
    {
      if (!(ud->inFile = fopen (inFileName, "r")))
	{
	  lou_logPrint ("Can't open file %s.\n", 
inFileName);
	  return -3;
	}
    }
  else
    ud->inFile = stdin;
  if (strcmp (outFileName, "stdout"))
    {
      if (!(ud->outFile = fopen (outFileName, "w")))
	{
	  lou_logPrint ("Can't open file %s.\n", outFileName);
	  return -3;
	}
    }
  else
    ud->outFile = stdout;
  transcribe_text_file ();
  if (ud->inFile != stdin)
    fclose (ud->inFile);
  if (ud->outFile != stdout)
    fclose (ud->outFile);
  return 1;
}

int
  lbx_backTranslateFile
  (char *configFileName, char *inFileName, char *outFileName)
{
/* Back translate the braille file in inFileName into either an 
* xml file or a text file according to
* the specifications in configFileName. If there are errors, return a negative
* number indicating the error.*/
  if (!read_configuration_file (configFileName, NULL))
    return -3;
  if (strcmp (inFileName, "stdin"))
    {
      if (!(ud->inFile = fopen (inFileName, "r")))
	{
	  lou_logPrint ("Can't open file %s.\n", 
inFileName);
	  return -3;
	}
    }
  else
    ud->inFile = stdin;
  if (strcmp (outFileName, "stdout"))
    {
      if (!(ud->outFile = fopen (outFileName, "w")))
	{
	  lou_logPrint ("Can't open file %s.\n", outFileName);
	  return -3;
	}
    }
  else
    ud->outFile = stdout;
  back_translate_file ();
  if (ud->inFile != stdin)
    fclose (ud->inFile);
  if (ud->outFile != stdout)
    fclose (ud->outFile);
  return 1;
}

void
lbx_free (void)
{
/* Free all memory used by liblouisxml. You MUST call this function at 
* the END of your application.*/
  lou_free ();
  destroy_semantic_table ();
  if (ud != NULL)
    free (ud);
  ud = NULL;
}
