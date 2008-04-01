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
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "louisxml.h"

typedef struct
{
  const char *fileName;
  FILE *in;
  int lineNumber;
  char line[1024];
  char *action;
  int actionLength;
  char *value;
  int valueLength;
  char *value2;
  int value2Length;
}
FileInfo;

static int errorCount = 0;

static void
configureError (FileInfo * nested, char *format, ...)
{
  char buffer[1024];
  va_list arguments;
  va_start (arguments, format);
#ifdef WIN32
  _vsnprintf (buffer, sizeof (buffer), format, arguments);
#else
  vsnprintf (buffer, sizeof (buffer), format, arguments);
#endif
  va_end (arguments);
  if (nested)
    lou_logPrint ("File %s line %d: %s",
		  nested->fileName, nested->lineNumber, buffer);
  else
    lou_logPrint ("%s", buffer);
  errorCount++;
}

int
find_file (const char *fileList, char *filePath)
{
  struct stat statInfo;
  char trialPath[MAXNAMELEN];
  int commaPos;			/*for file lists */
  printf("Enter %s %s\n", fileList, filePath);
  filePath[0] = 0;
  for (commaPos = 0; fileList[commaPos] && fileList[commaPos] != ',';
       commaPos++);
  strcpy (trialPath, ud->user_path);
  strncat (trialPath, (char *) fileList, commaPos);
  if (stat (trialPath, &statInfo) != -1)
    {
      strcpy (filePath, ud->user_path);
      strcat (filePath, (char *) fileList);
      return 1;
    }
  if (strcmp (ud->developer_path, ud->user_path) == 0)
    return 0;
  strcpy (trialPath, ud->developer_path);
  strncat (trialPath, (char *) fileList, commaPos);
  if (stat (trialPath, &statInfo) != -1)
    {
      strcpy (filePath, ud->developer_path);
      strcat (filePath, (char *) fileList);
      return 1;
    }
  strcpy (trialPath, LIBLOUIS_TABLES_PATH);
  strncat (trialPath, (char *) fileList, commaPos);
  printf("liblouis table: %s\n", trialPath);
  if (stat (trialPath, &statInfo) != -1)
    {
      strcpy (filePath, LIBLOUIS_TABLES_PATH);
      strcat (filePath, (char *) fileList);
      return 1;
    }
  return 0;
}

int
findTable (FileInfo * nested, const char *tableName, char *tablePath)
{
  if (!find_file (tableName, tablePath))
    {
      configureError (nested, "Table %s cannot be found.", tableName);
      return 0;
    }
  return 1;
}

static int
controlCharValue (FileInfo * nested)
{
/*Decode centrol characters*/
  int k = 0;
  char decoded[100];
  int decodedLength = 0;
  while (k < nested->valueLength)
    {
      if (nested->value[k] == '~' || nested->value[k] == '^')
	{
	  decoded[decodedLength++] = (nested->value[k + 1] | 32) - 96;
	  k += 2;
	}
      else if (nested->value[k] == '\\')
	{
	  k++;
	  switch (nested->value[k] | 32)
	    {
	    case 'f':
	      decoded[decodedLength++] = '\f';
	      break;
	    case 'n':
	      decoded[decodedLength++] = '\n';
	      break;
	    case 'r':
	      decoded[decodedLength++] = '\r';
	      break;
	    default:
	      configureError (nested, "invalid value %s", nested->value);
	      return 0;
	    }
	  k++;
	}
      else
	decoded[decodedLength++] = nested->value[k++];
    }
  decoded[decodedLength] = 0;
  strcpy (nested->value, decoded);
  nested->valueLength = decodedLength;
  return 1;
}

static int compileConfig (FileInfo * nested);

static int
config_compileFile (const char *fileName)
{
/*Compile an input file */
  FileInfo nested;
  char completePath[MAXNAMELEN];
  if (!*fileName)
    return 1;			/*Probably run with defaults */
  if (!find_file (fileName, completePath))
    {
      configureError (NULL, "Can't find configuration file %s", fileName);
      return 0;
    }
  nested.fileName = fileName;
  nested.lineNumber = 0;
  if ((nested.in = fopen ((char *) completePath, "r")))
    {
      compileConfig (&nested);
      fclose (nested.in);
    }
  else
    {
      configureError (NULL, "Can't open configuration file %s", fileName);
      return 0;
    }
  return 1;
}

static int
parseLine (FileInfo * nested)
{
  char *curchar = NULL;
  int ch = 0;
  while (fgets (nested->line, sizeof (nested->line), nested->in))
    {
      nested->lineNumber++;
      curchar = nested->line;
      while ((ch = *curchar++) <= 32 && ch != 0);
      if (ch == 0 || ch == '#' || ch == '<')
	continue;
      nested->action = curchar - 1;
      while ((ch = *curchar++) > 32);
      nested->actionLength = curchar - nested->action - 1;
      nested->action[nested->actionLength] = 0;
      while ((ch = *curchar++) <= 32 && ch != 0);
      if (ch == 0)
	{
	  nested->value = NULL;
	  return 1;
	}
      else
	{
	  nested->value = curchar - 1;
	  if (*nested->value == 34)	/*quote */
	    {
	      nested->value++;
	      while (*curchar && *curchar != 34)
		curchar++;
	      nested->valueLength = curchar - nested->value;
	    }
	  else
	    {
	      while (*curchar++ > 32);
	      nested->valueLength = curchar - nested->value - 1;
	    }
	  nested->value[nested->valueLength] = 0;
	}
      while ((ch = *curchar++) <= 32 && ch != 0);
      if (ch != 0)
	{
	  nested->value2 = curchar - 1;
	  if (*nested->value2 == 34)	/*quote */
	    {
	      nested->value2++;
	      while (*curchar && *curchar != 34)
		curchar++;
	      nested->value2Length = curchar - nested->value2;
	    }
	  else
	    {
	      while (*curchar++ > 32);
	      nested->value2Length = curchar - nested->value2 - 1;
	    }
	  nested->value2[nested->value2Length] = 0;
	}
      else
	nested->value2 = NULL;
      return 1;
    }
  return 0;
}

#define NOTFOUND 1000
static int mainActionNumber = NOTFOUND;
static int subActionNumber;
static int entities = 0;

static int
ignoreCaseComp (const char *str1, const char *str2, int length)
{
/* Replaces strncasecmp, which some compilers don't support */
  int k;
  for (k = 0; k < length; k++)
    if ((str1[k] | 32) != (str2[k] | 32))
      break;
  if (k != length)
    return 1;
  return 0;
}

static int
checkActions (FileInfo * nested, const char **actions)
{
  int k;
  for (k = 0; actions[k]; k += 2)
    if (nested->actionLength == strlen (actions[k]) &&
	ignoreCaseComp
	(actions[k], nested->action, nested->actionLength) == 0)
      break;
  if (actions[k] == NULL)
    return NOTFOUND;
  return atoi (actions[k + 1]);
}

static int
checkValues (FileInfo * nested, const char **values)
{
  int k;
  for (k = 0; values[k]; k += 2)
    if (nested->valueLength == strlen (values[k]) &&
	ignoreCaseComp (values[k], nested->value, nested->valueLength) == 0)
      break;
  if (values[k] == NULL)
    {
      configureError (nested, "word %s in column 2 not recognized",
		      nested->value);
      return NOTFOUND;
    }
  return atoi (values[k + 1]);
}

static int
checkSubActions (FileInfo * nested, const char **mainActions, const char
		 **subActions)
{
  int subAction;
  mainActionNumber = NOTFOUND;
  subAction = checkActions (nested, subActions);
  if (subAction != NOTFOUND && nested->value == NULL)
    {
      configureError (nested, "column 2 is required");
      return NOTFOUND;
    }
  if (subAction == NOTFOUND)
    {
      mainActionNumber = checkActions (nested, mainActions);
      if (mainActionNumber == NOTFOUND)
	configureError (nested, "word %s in first column not recognized",
			nested->action);
      return NOTFOUND;
    }
  return (subActionNumber = subAction);
}

static int
compileConfig (FileInfo * nested)
{
  static const char *mainActions[] = {
    "outputFormat",
    "0",
    "style",
    "1",
    "translation",
    "2",
    "xml",
    "3",
    "include",
    "5",
    NULL
  };
  static const char *yesNo[] = {
    "no",
    "0",
    "yes",
    "1",
    NULL
  };
  int k;
  if (!parseLine (nested))
    return 1;			/*No configuration, run with defaults */
  mainActionNumber = checkActions (nested, mainActions);
  if (mainActionNumber == NOTFOUND)
    {
      configureError (nested,
		      "word %s in first column not recognized",
		      nested->action);
      return 0;
    }
choseMainAction:
  switch (mainActionNumber)
    {
    case 0:			/*outputFormat */
      {
	static const char *actions[] = {
	  "cellsPerLine",
	  "0",
	  "linesPerPage",
	  "1",
	  "interpoint",
	  "2",
	  "lineEnd",
	  "3",
	  "pageEnd",
	  "4",
	  "beginningPageNumber",
	  "5",
	  "braillePages",
	  "6",
	  "paragraphs",
	  "7",
	  "fileEnd",
	  "8",
	  "printPages",
	  "9",
	  "printPageNumberAt",
	  "10",
	  "braillePageNumberAt",
	  "11",
	  "hyphenate",
	  "12",
	  "encoding",
	  "13",
	  "backFormat",
	  "14",
	  "backLineLength",
	  "15",
	  "interline",
	  "16",
	  NULL
	};
	static const char *topBottom[] = {
	  "bottom",
	  "0",
	  "top",
	  "1",
	  NULL
	};
	static const char *encodings[] = {
	  "utf8",
	  "0",
	  "utf16",
	  "1",
	  "utf32",
	  "2",
	  "ascii8",
	  "3",
	  NULL
	};
	static const char *backFormats[] = {
	  "plain",
	  "0",
	  "html",
	  "1",
	  NULL
	};

	while (parseLine (nested))
	  {
	    checkSubActions (nested, mainActions, actions);
	    if (mainActionNumber != NOTFOUND)
	      goto choseMainAction;
	    switch (subActionNumber)
	      {
	      case NOTFOUND:
		break;
	      case 0:
		ud->cells_per_line = atoi (nested->value);
		break;
	      case 1:
		ud->lines_per_page = atoi (nested->value);
		break;
	      case 2:
		if ((k = checkValues (nested, yesNo)) != NOTFOUND)
		  ud->interpoint = k;
		break;
	      case 3:
		if (controlCharValue (nested))
		  memcpy (ud->lineEnd, nested->value,
			  nested->valueLength + 1);
		break;
	      case 4:
		if (controlCharValue (nested))

		  memcpy (ud->pageEnd, nested->value,
			  nested->valueLength + 1);
		break;
	      case 5:
		ud->beginning_braille_page_number = atoi (nested->value);
		break;
	      case 6:
		if ((k = checkValues (nested, yesNo)) != NOTFOUND)
		  ud->braille_pages = k;
		break;
	      case 7:
		if ((k = checkValues (nested, yesNo)) != NOTFOUND)
		  ud->paragraphs = k;
		break;
	      case 8:
		if (controlCharValue (nested))
		  memcpy (ud->fileEnd, nested->value,
			  nested->valueLength + 1);
		break;
	      case 9:
		if ((k = checkValues (nested, yesNo)) != NOTFOUND)
		  ud->print_pages = k;
		break;
	      case 10:
		if ((k = checkValues (nested, topBottom)) != NOTFOUND)
		  ud->print_page_number_at = ud->braille_page_number_at = k;
		break;
	      case 11:
		if ((k = checkValues (nested, topBottom)) != NOTFOUND)
		  {
		    if (k)
		      k = 0;
		    else
		      k = 1;
		    ud->print_page_number_at = ud->braille_page_number_at = k;
		  }
		break;
	      case 12:
		if ((k = checkValues (nested, yesNo)) != NOTFOUND)
		  ud->hyphenate = k;
		break;
	      case 13:
		if ((k = checkValues (nested, encodings)) != NOTFOUND)
		  ud->output_encoding = k;
		break;
	      case 14:
		if ((k = checkValues (nested, backFormats)) != NOTFOUND)
		  ud->back_text = k;
		break;
	      case 15:
		ud->back_line_length = atoi (nested->value);
		break;
	      case 16:
		if ((k = checkValues (nested, yesNo)) != NOTFOUND)
		  ud->interline = k;
		break;
	      default:
		configureError (nested, "Program error in readconfig.c");
		continue;
	      }
	  }
	if (!((ud->print_page_number_at && ud->braille_page_number_at) ||
	      (!ud->print_page_number_at && !ud->braille_page_number_at)))
	  configureError (nested,
			  "invalid combination of braille and print page number placements");
	break;

    case 1:			/*style */
	{
	  static const char *actions[] = {
	    "linesBefore",
	    "0",
	    "linesAfter",
	    "1",
	    "leftMargin",
	    "2",
	    "firstLineIndent",
	    "3",
	    "translate",
	    "6",
	    "skipNumberLines",
	    "7",
	    "format",
	    "8",
	    "newPageBefore",
	    "9",
	    "newPageAfter",
	    "10",
	    "rightHandPage",
	    "11",
	    NULL
	  };
	  static const char *formats[] = {
	    "leftJustified",
	    "0",
	    "rightJustified",
	    "1",
	    "centered",
	    "2",
	    "alignColumnsLeft",
	    "3",
	    "alignColumnsRight",
	    "4",
	    "listColumns",
	    "5",
	    "listLines",
	    "6",
	    "computerCoded",
	    "7",
	    NULL
	  };
	  static int styleCount = 0;
	  StyleType *style;
	  sem_act styleAction;
	  if (nested->value == NULL)
	    {
	      configureError (nested, "no style name given in second column");
	      break;
	    }
	  styleCount++;
	  styleAction = find_semantic_number (nested->value);
	  style = style_cases (styleAction);
	  if (style == NULL)
	    {
	      configureError (nested,
			      "invalid style name %s in column two",
			      nested->value);
	      break;
	    }
	  if (style->action == document)
	    {
	      if (styleCount != 1)
		{
		  configureError (nested,
				  "docunent style must be the first one specified");
		  break;
		}
	      else
		memcpy (&ud->para_style, &ud->document_style,
			(&ud->scratch_style - &ud->para_style));
	    }
	  style->action = styleAction;
	  while (parseLine (nested))
	    {
	      checkSubActions (nested, mainActions, actions);
	      if (mainActionNumber != NOTFOUND)
		goto choseMainAction;
	      switch (subActionNumber)
		{
		case NOTFOUND:
		  break;
		case 0:
		  style->lines_before = atoi (nested->value);
		  break;
		case 1:
		  style->lines_after = atoi (nested->value);
		  break;
		case 2:
		  style->left_margin = atoi (nested->value);
		  break;
		case 3:
		  style->first_line_indent = atoi (nested->value);
		  break;
		case 6:
		  switch ((k = find_semantic_number (nested->value)))
		    {
		    case contracted:
		    case uncontracted:
		    case compbrl:
		      style->translate = k;
		      break;
		    default:
		      configureError (nested, "no such translation");
		      break;
		    }
		  break;
		case 7:
		  if ((k = checkValues (nested, yesNo)) != NOTFOUND)
		    style->skip_number_lines = k;
		  break;
		case 8:
		  if ((k = checkValues (nested, formats)) != NOTFOUND)
		    style->format = k;
		  break;
		case 9:
		  if ((k = checkValues (nested, yesNo)) != NOTFOUND)
		    style->newpage_before = k;
		  break;
		case 10:
		  if ((k = checkValues (nested, yesNo)) != NOTFOUND)
		    style->newpage_after = k;
		  break;
		case 11:
		  if ((k = checkValues (nested, yesNo)) != NOTFOUND)
		    style->righthand_page = k;
		  break;
		default:
		  configureError (nested, "Program error in readconfig.c");
		  continue;
		}
	    }
	  break;

    case 2:			/*translation */
	  {
	    static const char *actions[] = {
	      "contractedTable",
	      "0",
	      "literarytextTable",
	      "0",
	      "editTable",
	      "1",
	      "uncontractedTable",
	      "2",
	      "compbrlTable",
	      "3",
	      "mathtextTable",
	      "4",
	      "mathexprTable",
	      "5",
	      "interlineBackTable",
	      "6",
	      NULL
	    };
	    while (parseLine (nested))
	      {
		checkSubActions (nested, mainActions, actions);
		if (mainActionNumber != NOTFOUND)
		  goto choseMainAction;
		switch (subActionNumber)
		  {
		  case NOTFOUND:
		    break;
		  case 0:
		    findTable (nested, nested->value,
			       ud->contracted_table_name);
		    break;
		  case 1:
		    findTable (nested, nested->value, ud->edit_table_name);
		    break;
		  case 2:
		    findTable (nested, nested->value,
			       ud->uncontracted_table_name);
		    break;
		  case 3:
		    findTable (nested, nested->value, ud->compbrl_table_name);
		    break;
		  case 4:
		    findTable (nested, nested->value,
			       ud->mathtext_table_name);
		    break;
		  case 5:
		    findTable (nested, nested->value,
			       ud->mathexpr_table_name);
		    break;
		  case 6:
		    findTable (nested, nested->value,
			       ud->interline_back_table_name);
		    break;
		  default:
		    configureError (nested, "Program error in readconfig.c");
		    continue;
		  }
	      }
	    break;

    case 3:			/*xml */
	    {
	      static const char *actions[] = {
		"xmlHeader",
		"0",
		"entity",
		"1",
		"internetAccess",
		"2",
		"semanticFiles",
		"3",
		"newEntries",
		"4",
		NULL
	      };
	      while (parseLine (nested))
		{
		  checkSubActions (nested, mainActions, actions);
		  if (mainActionNumber != NOTFOUND)
		    goto choseMainAction;
		  switch (subActionNumber)
		    {
		    case NOTFOUND:
		      break;
		    case 0:
		      if (entities)
			{
			  configureError
			    (nested,
			     "The header definition must precede all entity definitions.");
			  break;
			}
		      strncpy (ud->xml_header, nested->value,
			       nested->valueLength);
		      break;
		    case 1:
		      if (!entities)
			strcat (ud->xml_header, "<!DOCTYPE entities [\n");
		      entities = 1;
		      strcat (ud->xml_header, "<!ENTITY ");
		      strcat (ud->xml_header, nested->value);
		      strcat (ud->xml_header, " \"");
		      strcat (ud->xml_header, nested->value2);
		      strcat (ud->xml_header, "\">\n");
		      break;
		    case 2:
		      if ((k = checkValues (nested, yesNo)) != NOTFOUND)
			ud->internet_access = k;
		      break;
		    case 3:
		      strcpy (ud->semantic_files, nested->value);
		      break;
		    case 4:
		      if ((k = checkValues (nested, yesNo)) != NOTFOUND)
			ud->new_entries = k;
		      break;
		    default:
		      configureError (nested,
				      "Program error in readconfig.c");
		      continue;
		    }
		}
	      break;

    case 4:			/*reserved */
	      {
		static const char *actions[] = {
		  NULL
		};
		while (parseLine (nested))
		  {
		    checkSubActions (nested, mainActions, actions);
		    if (mainActionNumber != NOTFOUND)
		      goto choseMainAction;
		    switch (subActionNumber)
		      {
		      case NOTFOUND:
			break;
		      default:
			configureError (nested,
					"Program error in readconfig.c");
			continue;
		      }
		  }
		break;

    case 5:			/*include */
		{
		  static const char *actions[] = { NULL };
		  if (nested->value == NULL)
		    configureError (nested,
				    "a file name in column 2 is required");
		  else
		    config_compileFile (nested->value);
		  parseLine (nested);
		  checkSubActions (nested, mainActions, actions);
		  if (mainActionNumber != NOTFOUND)
		    goto choseMainAction;
		}
		break;

    default:
		configureError (nested, "Program error in readconfig.c");
		break;
	      }
	    }
	  }
	}
      }
    }
  return 1;
}

static char oldFileList[MAXNAMELEN];

static const char const *logFileNamex = NULL;

static char configPath[MAXNAMELEN];

static int
initConfigFiles (const char *fileList, char *fileName)
{
  int k;
#ifndef DEVELOPERPATH
  strcpy (ud->developer_path, getenv ("HOME"));
  strcat (ud->developer_path, "/lbx_files/");
#else
  strcpy (ud->developer_path, DEVELOPERPATH);
#endif
  strcpy (configPath, fileList);
  for (k = strlen (configPath); k >= 0; k--)
    if (configPath[k] == '/' || configPath[k] == '\\')
      break;
  strcpy (fileName, &configPath[k + 1]);
  configPath[++k] = 0;
#ifdef USERISCONFIG
  strcpy (ud->user_path, configPath);
#elif defined (USERPATH)
  strcpy (ud->user_path, USERPATH);
#else
  if (!*configPath)
    {
      strcpy (configPath, getenv ("HOME"));
      strcat (configPath, "/lbx_files/");
    }
  strcpy (ud->user_path, configPath);
#endif
  if (logFileNamex)
    {
      strcpy (ud->contracted_table_name, ud->user_path);
      strcat (ud->contracted_table_name, logFileNamex);
      lou_logFile (ud->contracted_table_name);
    }
  if (!config_compileFile ("canonical.cfg"))
    return 0;
  return 1;
}

int
read_configuration_file (const char *const configFileList, const char
			 const *logFileName)
{
/* read the configuration file and perform other initialization*/
  int k;
  char mainFile[MAXNAMELEN];
  char subFile[MAXNAMELEN];
  int listLength;
  int currentListPos = 0;
  errorCount = 0;
  logFileNamex = logFileName;
  if (strcmp (oldFileList, configFileList) == 0)
    {
      ud->has_comp_code = 0;
      ud->has_math = 0;
      ud->has_chem = 0;
      ud->has_graphics = 0;
      ud->has_music = 0;
      ud->has_cdata = 0;
      ud->print_page_number[0] = '_';
      ud->inFile = NULL;
      ud->outFile = NULL;
      ud->mainBrailleTable = ud->contracted_table_name;
      ud->outlen_so_far = 0;
      ud->lines_on_page = 0;
      ud->braille_page_number = ud->beginning_braille_page_number;
      return 1;
    }
  strcpy (oldFileList, configFileList);
  if (ud != NULL)
    free (ud);
  if (!(ud = malloc (sizeof (UserData))))
    return 0;
  memset (ud, 0, sizeof (UserData));
  errorCount = 0;
  entities = 0;
  ud->max_length = BUFSIZE - 4;
  ud->max_trans_length = 2 * BUFSIZE - 4;
  ud->top = -1;
  ud->input_encoding = utf8;
  ud->output_encoding = ascii8;
  *ud->print_page_number = '_';
/*Process file list*/
  listLength = strlen (configFileList);
  for (k = 0; k < listLength; k++)
    if (configFileList[k] == ',')
      break;
  if (k == listLength)
    {				/* Only one file */
      initConfigFiles (configFileList, mainFile);
      config_compileFile (mainFile);
    }
  else
    {				/* Compile a list of files */
      strncpy (subFile, configFileList, k);
      subFile[k] = 0;
      initConfigFiles (subFile, mainFile);
      currentListPos = k + 1;
      config_compileFile (mainFile);
      while (currentListPos < listLength)
	{
	  for (k = currentListPos; k < listLength; k++)
	    if (configFileList[k] == ',')
	      break;
	  strncpy (subFile, &configFileList[currentListPos],
		   k - currentListPos);
	  subFile[k - currentListPos] = 0;
	  config_compileFile (subFile);
	  currentListPos = k + 1;
	}
    }
  if (errorCount)
    {
      lou_logPrint ("%d errors found", errorCount);
      free (ud);
      ud = NULL;
      return 0;
    }
  ud->braille_page_number = ud->beginning_braille_page_number;
  if (entities)
    strcat (ud->xml_header, "]>\n");
  return 1;
}
