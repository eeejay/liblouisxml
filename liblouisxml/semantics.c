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
#include "sem_names.h"

typedef struct
{
  const char *fileName;
  FILE *in;
  int lineNumber;
  int numEntries;
  int unedited;
  char line[MAXNAMELEN];
}
FileInfo;

#define HASHSIZE 1123
#define NOTFOUND -1

#define MAXINSERTS 256
typedef struct
{
  widechar numInserts;
  widechar lastInsert;
  widechar numChars;
  widechar charInserts[MAXINSERTS];
} InsertsType;

typedef struct
{
  void *next;
  unsigned char *key;
  int value;
  InsertsType *inserts;
} hashEntry;

typedef struct
{
  int curBucket;
  hashEntry *curEntry;
  hashEntry *entries[HASHSIZE];
} hashTable;

static int errorCount = 0;

static void
semanticError (FileInfo * nested, char *format, ...)
{
  char buffer[MAXNAMELEN];
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

static unsigned int
stringHash (const unsigned char *s)
{
  int k;
  unsigned int h = 0, g;
  for (k = 0; s[k]; k++)
    {
      h = (h << 4) + s[k];
      if ((g = h & 0xf0000000))
	{
	  h = h ^ (g >> 24);
	  h = h ^ g;
	}
    }
  return h;
}

static hashTable *
hashNew (void)
{
  hashTable *table;
  table = malloc (sizeof (hashTable));
  memset (table, 0, sizeof (hashTable));
  table->curBucket = -1;
  return table;
}

static void
hashFree (hashTable * table)
{
  int i;
  hashEntry *e, *next;
  if (table == NULL)
    return;
  for (i = 0; i < HASHSIZE; i++)
    for (e = table->entries[i]; e; e = next)
      {
	next = e->next;
	free (e->key);
	if (e->inserts != NULL)
	  free (e->inserts);
	free (e);
      }
  free (table);
}

static hashEntry *
hashScan (hashTable * table)
{
  hashEntry *e;
  if (table == NULL)
    return NULL;
  if (table->curBucket == -1)
    table->curEntry = NULL;
  while (table->curBucket < HASHSIZE)
    {
      if (table->curEntry != NULL)
	{
	  e = table->curEntry;
	  table->curEntry = e->next;
	  return e;
	}
      else
	table->curBucket++;
      while (table->curBucket < HASHSIZE && table->entries[table->curBucket]
	     == NULL)
	table->curBucket++;
      if (table->curBucket < HASHSIZE)
	table->curEntry = table->entries[table->curBucket];
    }
  table->curBucket = -1;
  table->curEntry = NULL;
  return NULL;
}

static hashEntry *latestEntry;

/* assumes that key is not already present! */
static void
hashInsert (hashTable * table, const unsigned char *key, int value,
	    InsertsType * inserts)
{
  int i;
  if (table == NULL || key == NULL || value < 0)
    return;
  i = stringHash (key) % HASHSIZE;
  latestEntry = malloc (sizeof (hashEntry));
  latestEntry->next = table->entries[i];
  latestEntry->key = malloc (strlen ((char *) key) + 1);
  strcpy ((char *) latestEntry->key, (char *) key);
  latestEntry->value = value;
  latestEntry->inserts = inserts;
  table->entries[i] = latestEntry;
}

static int
hashLookup (hashTable * table, const unsigned char *key)
{
  int i;
  int keyLength;
  int entryKeyLength;
  int k;
  if (table == NULL || key == NULL)
    return NOTFOUND;
  keyLength = strlen ((char *) key);
  i = stringHash (key) % HASHSIZE;
  for (latestEntry = table->entries[i]; latestEntry; latestEntry =
       latestEntry->next)
    {
      entryKeyLength = strlen ((char *) latestEntry->key);
      if (entryKeyLength != keyLength)
	continue;
      for (k = 0; k < keyLength; k++)
	if (key[k] != latestEntry->key[k])
	  break;
      if (k == keyLength)
	return latestEntry->value;
    }
  return NOTFOUND;
}

static hashTable *actionTable = NULL;
static hashTable *namesAndActions = NULL;
static hashTable *newEntriesTable;
static char oldFileList[MAXNAMELEN];
static char firstFileName[MAXNAMELEN];
static int haveSemanticFile = 1;

sem_act
find_semantic_number (const char *name)
{
  static const char *pseudoActions[] = { "include", NULL };
  int k;
  xmlChar lowerName[MAXNAMELEN];
  if (actionTable == NULL)
    {
      actionTable = hashNew ();
      for (k = 0; k < end_all; k++)
	hashInsert (actionTable, (xmlChar *) semNames[k], k, NULL);
      k = 0;
      while (pseudoActions[k] != NULL)
	{
	  hashInsert (actionTable, (xmlChar *) pseudoActions[k],
		      k + end_all + 1, NULL);
	  k++;
	}
    }
  for (k = 0; name[k]; k++)
    lowerName[k] = name[k] | 32;
  lowerName[k] = 0;
  return (hashLookup (actionTable, lowerName));
}

void
destroy_semantic_table (void)
{
  hashFree (namesAndActions);
  namesAndActions = NULL;
  hashFree (newEntriesTable);
  newEntriesTable = NULL;
  hashFree (actionTable);
  actionTable = NULL;
}

static widechar
hexValue (FileInfo * nested, const xmlChar const *digits, int length)
{
  int k;
  unsigned int binaryValue = 0;
  for (k = 0; k < length; k++)
    {
      unsigned int hexDigit = 0;
      if (digits[k] >= '0' && digits[k] <= '9')
	hexDigit = digits[k] - '0';
      else if (digits[k] >= 'a' && digits[k] <= 'f')
	hexDigit = digits[k] - 'a' + 10;
      else if (digits[k] >= 'A' && digits[k] <= 'F')
	hexDigit = digits[k] - 'A' + 10;
      else
	{
	  semanticError (nested, "invalid %d-digit hexadecimal number",
			 length);
	  return (widechar) 0xffffffff;
	}
      binaryValue |= hexDigit << (4 * (length - 1 - k));
    }
  return (widechar) binaryValue;
}

static InsertsType *
encodeInsertions (FileInfo * nested, xmlChar * insertions, int length)
{
  int k = 0;
  int prevk = 0;
  xmlChar *oneIns = insertions;
  int oneLength = 0;
  InsertsType inserts;
  int insertsSize = sizeof (InsertsType) - (MAXINSERTS * CHARSIZE);
  InsertsType *insertsPtr;
  int kk;
  int startInsert;
  int sizeInsert;
  widechar ch;
  inserts.numInserts = 0;
  inserts.numChars = 1;
  inserts.lastInsert = 0;
  k = 0;
  prevk = 0;
/*Inserjions are placed in asserts.charInserts and consist of a length 
* followed by the characters to be inserted. The length is one more than 
* the numbr of characters to make it simpler to step through the 
* insertions in the next function.*/
  while (k < length)
    {
      if (insertions[k] == '\\' && insertions[k + 1] == ',')
	{
	  k += 2;
	  continue;
	}
      if (insertions[k] == ',' || k == (length - 1))
	{
	  oneLength = k - prevk;
	  if (k == (length - 1))
	    oneLength++;
	  if (insertions[prevk] == '\\' && insertions[prevk + 1] == '*')
	    {
	      inserts.lastInsert = inserts.numChars;
	      if ((prevk + oneLength) < length)
		semanticError (nested,
			       "an insertion with \\* must be the last.");
	    }
	  else
	    inserts.numInserts++;
	  startInsert = inserts.numChars++;
	  sizeInsert = 0;
	  kk = 0;
	  while (kk < oneLength)
	    {
	      ch = (widechar) oneIns[kk++];
	      if (ch == '\\')
		switch (oneIns[kk])
		  {
		  case 's':
		    inserts.charInserts[inserts.numChars++] = ' ';
		    kk++;
		    break;
		  case ',':
		    inserts.charInserts[inserts.numChars++] = ',';
		    kk++;
		    break;
		  case 'x':
		  case 'X':
		    inserts.charInserts[inserts.numChars++] = hexValue
		      (nested, &oneIns[kk + 1], 4);
		    kk += 5;
		    break;
		  case 'y':
		  case 'Y':
		    if (CHARSIZE == 2)
		      {
		      not32:
			semanticError (nested,
				       "liblouisxml has not been compiled for 32-bit Unicode");
			break;
		      }
		    if (oneLength - k > 5)
		      {
			inserts.charInserts[inserts.numChars++] =
			  hexValue (nested, &oneIns[k + 1], 5);
			k += 6;
		      }
		    break;
		  case 'z':
		  case 'Z':
		    if (CHARSIZE == 2)
		      goto not32;
		    if (oneLength - k > 8)
		      {
			inserts.charInserts[inserts.numChars++] =
			  hexValue (nested, &oneIns[k + 1], 8);
			k += 9;
		      }
		    break;
		  case '*':
		    kk++;
		    sizeInsert--;
		    break;
		  default:
		    kk++;
		    semanticError (nested, "invalid escape sequence.");
		    break;
		  }
	      else
		inserts.charInserts[inserts.numChars++] = ch;
	      sizeInsert++;
	    }
	  inserts.charInserts[startInsert] = sizeInsert + 1;
	  oneIns = &insertions[k + 1];
	  prevk = k + 1;
	}
      k++;
    }
  insertsSize += inserts.numChars * CHARSIZE;
  insertsPtr = malloc (insertsSize);
  memcpy (insertsPtr, &inserts, insertsSize);
  return insertsPtr;
}

int
insert_code (xmlNode * node, int which)
{
  int entryStart;
  widechar *insertStart;
  int insertLength = 0;
  int k;
  int sumLength;
  InsertsType *inserts;
  hashEntry *nodeEntry;
  if (node == NULL)
    return 0;
  nodeEntry = (hashEntry *) node->_private;
  if (nodeEntry == NULL)
    return 0;
  if (nodeEntry->inserts == NULL)
    return 1;
  inserts = nodeEntry->inserts;
  if (which == -1)
    {
      if ((entryStart = inserts->lastInsert) == 0)
	return 1;
      insertStart = &inserts->charInserts[entryStart + 1];
      insertLength = inserts->charInserts[entryStart] - 1;
    }
  else
    {
      if (inserts->numInserts == 0 || inserts->numInserts < which
	  || (which == 1 && inserts->numInserts == 1))
	return 1;
      sumLength = 1;
      for (k = 0; k < which; k++)
	sumLength += inserts->charInserts[sumLength];
      insertStart = &inserts->charInserts[sumLength + 1];
      insertLength = inserts->charInserts[sumLength] - 1;
      if (insertLength <= 0)
	return 0;
    }
  if ((ud->text_length + insertLength) > ud->max_length)
    return 0;
  memcpy (&ud->text_buffer[ud->text_length], insertStart, CHARSIZE *
	  insertLength);
  ud->text_length += insertLength;
  return 1;
}

#define NUMCOUNTS 1024
#define MAXNUMVAL 5
static int *attrValueCounts = NULL;
static hashTable *attrValueCountsTable;

static int
countAttrValues (xmlChar * key)
{
  int k;
  int numItems = 1;
  int lastComma = 0;
  static int curCount = 0;
  int thisCount = NOTFOUND;
  if (!ud->new_entries)
    return 0;
  if (attrValueCounts == NULL)
    {
      attrValueCounts = malloc (NUMCOUNTS * sizeof (int));
      attrValueCountsTable = hashNew ();
      curCount = 0;
    }
  for (k = 0; key[k]; k++)
    if (key[k] == ',')
      {
	lastComma = k;
	numItems++;
      }
  switch (numItems)
    {
    case 1:
      return 1;
    case 2:
      if (hashLookup (attrValueCountsTable, key) != NOTFOUND)
	return 1;
      if (curCount >= NUMCOUNTS)
	return 0;
      hashInsert (attrValueCountsTable, key, curCount, NULL);
      curCount++;
      return 1;
    case 3:
      if (curCount >= NUMCOUNTS)
	return 0;
      key[lastComma] = 0;
      thisCount = hashLookup (attrValueCountsTable, key);
      if (thisCount == NOTFOUND)
	{
	  attrValueCounts[curCount]++;
	  hashInsert (attrValueCountsTable, key, curCount, NULL);
	  curCount++;
	}
      key[lastComma] = ',';
      if (thisCount == NOTFOUND)
	return 1;
      if (attrValueCounts[thisCount] >= MAXNUMVAL)
	return 0;
      attrValueCounts[thisCount]++;
      return 1;
    default:
      return 0;
    }
  return 0;
}

static void
destroyattrValueCountsTable (void)
{
  if (attrValueCounts == NULL)
    return;
  hashFree (attrValueCountsTable);
  free (attrValueCounts);
  attrValueCounts = NULL;
}
static int sem_compileFile (const char *fileName);

static int
compileLine (FileInfo * nested)
{
  char *curchar = NULL;
  int ch = 0;
  char *action = NULL;
  int actionLength = 0;
  char *lookFor;
  int lookForLength;
  char *insertions;
  int insertionsLength;
  InsertsType *inserts;
  sem_act actionNum;
  if (namesAndActions == NULL)
    namesAndActions = hashNew ();
  curchar = nested->line;
  while ((ch = *curchar++) <= 32 && ch != 0);
  if (ch == 0 || ch == '#' || ch == '<')
    return 1;
  action = curchar - 1;
  while ((ch = *curchar++) > 32);
  actionLength = curchar - action - 1;
  action[actionLength] = 0;
  if (actionLength != 2 && *action != 'n' && action[1] != 'o')
    nested->unedited = 0;
  while ((ch = *curchar++) <= 32 && ch != 0);
  if (ch == 0)
    {
      semanticError (nested, "Nothing to look for");
      return 0;
    }
  lookFor = curchar - 1;
  while (*curchar++ > 32);
  lookForLength = curchar - lookFor - 1;
  lookFor[lookForLength] = 0;
  actionNum = find_semantic_number (action);
  if (actionNum == NOTFOUND)
    {
      semanticError (nested, "Action %s not recognized", action);
      return 0;
    }
/*Handle pseudo actions*/
  switch (actionNum)
    {
    case end_all + 1:		/*include */
      if (!sem_compileFile (lookFor))
	return 0;
      break;
    default:
      break;
    }
  if (hashLookup (namesAndActions, (xmlChar *) lookFor) != NOTFOUND)
    return 1;
  countAttrValues ((xmlChar *) lookFor);
  inserts = NULL;
  while ((ch = *curchar++) <= 32 && ch != 0);
  if (ch != 0)
    {
      insertions = curchar - 1;
      while (*curchar++ > 32);
      insertionsLength = curchar - insertions - 1;
      insertions[insertionsLength] = 0;
      inserts = encodeInsertions (nested, (xmlChar *) insertions,
				  insertionsLength);
    }
  hashInsert (namesAndActions, (xmlChar *) lookFor, actionNum, inserts);
  nested->numEntries++;
  return 1;
}

static int
getALine (FileInfo * nested)
{
/*Read a line of char's from an input file */
  int ch;
  int numchars = 0;
  memset (nested->line, 0, sizeof (nested->line));
  while ((ch = fgetc (nested->in)) != EOF)
    {
      if (ch == 13)
	continue;
      if (ch == 10 || numchars >= sizeof (nested->line))
	break;
      nested->line[numchars++] = ch;
    }
  if (ch == EOF)
    return 0;
  return 1;
}

static int numEntries = 0;

static int
sem_compileFile (const char *fileName)
{
/*Compile an input file */
  FileInfo nested;
  char completePath[MAXNAMELEN];
  if (!*fileName)
    return 1;			/*Probably run with defaults */
  if (!find_file (fileName, completePath))
    {
      if (strncmp (fileName, "appended_", 9) != 0)
	{
	  semanticError (NULL, "Can't find semantic-action file %s",
			 fileName);
	  return 0;
	}
      return 1;
    }
  nested.fileName = fileName;
  nested.lineNumber = 0;
  nested.numEntries = 0;
  nested.unedited = 1;
  if ((nested.in = fopen ((char *) completePath, "r")))
    {
      while (getALine (&nested))
	{
	  nested.lineNumber++;
	  compileLine (&nested);
	}
      fclose (nested.in);
      if (nested.unedited)
	semanticError (&nested,
		       "File %s needs editing to produce good results.",
		       nested.fileName);
    }
  else
    {
      semanticError (NULL, "Can't open semantic-action file %s", fileName);
      return 0;
    }
  numEntries += nested.numEntries;
  return 1;
}

int
compile_semantic_table (xmlNode * rootElement)
{
  char fileName[MAXNAMELEN];
  attrValueCounts = NULL;
  if (*ud->semantic_files)
    {
/*Process file list*/
      int listLength;
      int currentListPos;
      int k;
      listLength = strlen (ud->semantic_files);
      if (strcmp (ud->semantic_files, oldFileList) == 0)
	return 1;
      strcpy (oldFileList, ud->semantic_files);
      firstFileName[0] = 0;
      for (k = 0; k < listLength; k++)
	if (ud->semantic_files[k] == ',')
	  break;
      if (k == listLength)
	{			/* Only one file */
	  if (!sem_compileFile (ud->semantic_files))
	    return (haveSemanticFile = 0);
	  strcpy (firstFileName, ud->semantic_files);
	}
      else
	{			/* Compile a list of files */
	  strncpy (fileName, ud->semantic_files, k);
	  fileName[k] = 0;
	  if (!sem_compileFile (fileName))
	    return (haveSemanticFile = 0);
	  strcpy (firstFileName, fileName);
	  currentListPos = k + 1;
	  while (currentListPos < listLength)
	    {
	      for (k = currentListPos; k < listLength; k++)
		if (ud->semantic_files[k] == ',')
		  break;
	      strncpy (fileName, &ud->semantic_files[currentListPos],
		       k - currentListPos);
	      fileName[k - currentListPos] = 0;
	      if (!sem_compileFile (fileName))
		return (haveSemanticFile = 0);
	      currentListPos = k + 1;
	    }
	}
    }
  else
    {
      const xmlChar *rootName;
      char *curchar = NULL;
      rootName = rootElement->name;
      strcpy (fileName, (char *) rootName);
      curchar = fileName;
      while (*curchar)
	{
	  if (*curchar == ':' || *curchar == '/'
	      || *curchar == '\\' || *curchar == 34 || *curchar == 39
	      || *curchar == '(' || *curchar == ')' || *curchar < 33 ||
	      *curchar > 126)
	    *curchar = '_';
	  curchar++;
	}
      strcat (fileName, ".sem");
      if (strcmp (fileName, oldFileList) == 0)
	return 1;
      strcpy (oldFileList, fileName);
      strcpy (firstFileName, fileName);
      if (!sem_compileFile (fileName))
	return (haveSemanticFile = 0);
    }
  strcpy (fileName, "appended_");
  strcat (fileName, firstFileName);
  sem_compileFile (fileName);
  if (numEntries == 0)
    {
      destroy_semantic_table ();
      return 0;
    }
  return 1;
}

#define VALLEN 50

static void addNewEntries (const xmlChar * key);

sem_act
set_sem_attr (xmlNode * node)
{
  sem_act action = no;
  sem_act actionNum = NOTFOUND;
  xmlChar key[MAXNAMELEN];
  int k;
  int oldKeyLength = 0;
  const xmlChar *name = node->name;
  if (namesAndActions == NULL)
    namesAndActions = hashNew ();
  if (node->properties)
    {
      xmlAttr *attributes = node->properties;
      while (attributes)
	{
	  const xmlChar *attrName = attributes->name;
	  xmlChar *attrValue = xmlGetProp (node, attrName);
	  strcpy ((char *) key, (char *) name);
	  strcat ((char *) key, ",");
	  strcat ((char *) key, (char *) attrName);
	  strcat ((char *) key, ",");
	  oldKeyLength = strlen ((char *) key);
	  strncat ((char *) key, (char *) attrValue, VALLEN);
	  for (k = 0; key[k]; k++)
	    if ((key[k] <= 32 || key[k] > 126)
		|| (k >= oldKeyLength && key[k] == ','))
	      key[k] = '_';
	  actionNum = NOTFOUND;
	  if (((actionNum = hashLookup (namesAndActions, key)) != NOTFOUND) ||
	      countAttrValues (key))
	    {
	      if (actionNum == NOTFOUND)
		addNewEntries (key);
	    }
	  if (actionNum == NOTFOUND || actionNum == no)
	    {
	      key[oldKeyLength - 1] = 0;
	      actionNum = hashLookup (namesAndActions, key);
	      if (actionNum == NOTFOUND)
		addNewEntries (key);
	      else if (actionNum == no)
		actionNum = NOTFOUND;
	    }
	  if (actionNum != NOTFOUND && actionNum != no)
	    break;
	  attributes = attributes->next;
	}
    }
  if (actionNum == NOTFOUND)
    {
      strcpy ((char *) key, (char *) name);
      actionNum = hashLookup (namesAndActions, key);
    }
  if (actionNum == NOTFOUND)
    addNewEntries (name);
  else
    action = actionNum;
  node->_private = latestEntry;
  return action;
}

sem_act
get_sem_attr (xmlNode * node)
{
  hashEntry *nodeEntry = (hashEntry *) node->_private;
  if (nodeEntry != NULL)
    return nodeEntry->value;
  else
    return no;
}

xmlChar *
get_attr_value (xmlNode * node)
{
  int firstComma = 0, secondComma = 0;
  int k;
  xmlChar attrName[MAXNAMELEN];
  hashEntry *nodeEntry = (hashEntry *) node->_private;
  if (nodeEntry == NULL || !node->properties)
    return (xmlChar *) "";
  for (k = 0; nodeEntry->key[k]; k++)
    if (!firstComma && nodeEntry->key[k] == ',')
      firstComma = k;
    else if (firstComma && nodeEntry->key[k] == ',')
      secondComma = k;
  if (firstComma == 0)
    return (xmlChar *) "";
  if (secondComma == 0)
    secondComma = strlen ((char *) nodeEntry->key);
  k = 0;
  firstComma++;
  while (firstComma < secondComma)
    attrName[k++] = nodeEntry->key[firstComma++];
  attrName[k] = 0;
  return xmlGetProp (node, attrName);
}

sem_act
push_sem_stack (xmlNode * node)
{
  if (ud->top > (STACKSIZE - 2) || ud->top < -1)
    return no;
  return (ud->stack[++ud->top] = get_sem_attr (node));
}

sem_act
pop_sem_stack ()
{
  if (ud->top < 0)
    {
      ud->top = -1;
      return no;
    }
  ud->top--;
  if (ud->top > -1)
    return ud->stack[ud->top];
  return no;
}


static int moreEntries = 0;

static void
addNewEntries (const xmlChar * newEntry)
{
  if (newEntry == NULL || *newEntry == 0 || !ud->new_entries)
    return;
  if (moreEntries == 0)
    {
      moreEntries = 1;
      newEntriesTable = hashNew ();
    }
  if (hashLookup (newEntriesTable, newEntry) != NOTFOUND)
    return;
  hashInsert (newEntriesTable, newEntry, 0, NULL);
}

void
append_new_entries (void)
{
  int items;
  char filePrefix[20];
  char outFileName[MAXNAMELEN];
  FILE *semOut;
  int numEntries = 0;
  destroyattrValueCountsTable ();
  if (!moreEntries || !*firstFileName)
    return;
  if (haveSemanticFile)
    strcpy (filePrefix, "appended_");
  else
    strcpy (filePrefix, "new_");
  strcpy (outFileName, ud->user_path);
  strcat (outFileName, filePrefix);
  strcat (outFileName, firstFileName);
  semOut = fopen ((char *) outFileName, "a");
  if (!haveSemanticFile)
    {
      fprintf (semOut,
	       "# This file was produced by liblouisxml and is considered paot of\n");
      fprintf (semOut,
	       "# the code. See the file copyright-notice for permissions and\n");
      fprintf (semOut, "# restrictions.\n\n");
      fprintf (semOut,
	       "# You must edit this file as explained in the documentation to get\n");
      fprintf (semOut, "# proper output.\n\n");
    }
  for (items = 1; items < 4; items++)
    {
      hashEntry *curEntry;
      int k;
      while ((curEntry = hashScan (newEntriesTable)))
	{
	  int numItems = 1;
	  for (k = 0; curEntry->key[k]; k++)
	    if (curEntry->key[k] == ',')
	      numItems++;
	  if (numItems != items)
	    continue;
	  fprintf (semOut, "no %s\n", curEntry->key);
	  numEntries++;
	}
    }
  fclose (semOut);
  if (haveSemanticFile)
    lou_logPrint ("%d new entries appended to %s%s.", numEntries,
		  filePrefix, firstFileName);
  else
    lou_logPrint ("%d entries written to new semantic-action file %s%s.",
		  numEntries, filePrefix, firstFileName);
  moreEntries = 0;
}

StyleType *
style_cases (sem_act st)
{
  StyleType *style;
  switch (st)
    {
    case document:
      style = &ud->document_style;
      break;
    case code:
      style = &ud->code_style;
      break;
    case contents:
      style = &ud->contents_style;
      break;
    case dedication:
      style = &ud->dedication_style;
      break;
    case dispmath:
      style = &ud->dispmath_style;
      break;
    case disptext:
      style = &ud->disptext_style;
      break;
    case glossary:
      style = &ud->glossary_style;
      break;
    case graph:
      style = &ud->graph_style;
      break;
    case graphlabel:
      style = &ud->graphlabel_style;
      break;
    case indexx:
      style = &ud->indexx_style;
      break;
    case matrix:
      style = &ud->matrix_style;
      break;
    case music:
      style = &ud->music_style;
      break;
    case note:
      style = &ud->note_style;
      break;
    case spatial:
      style = &ud->spatial_style;
      break;
    case titlepage:
      style = &ud->titlepage_style;
      break;
    case trnote:
      style = &ud->trnote_style;
      break;
    case volume:
      style = &ud->volume_style;
      break;
    case arith:
      style = &ud->arith_style;
      break;
    case biblio:
      style = &ud->biblio_style;
      break;
    case heading4:
      style = &ud->heading4_style;
      break;
    case heading3:
      style = &ud->heading3_style;
      break;
    case heading2:
      style = &ud->heading2_style;
      break;
    case heading1:
      style = &ud->heading1_style;
      break;
    case list:
      style = &ud->list_style;
      break;
    case table:
      style = &ud->table_style;
      break;
    case caption:
      style = &ud->caption_style;
      break;
    case exercise1:
      style = &ud->exercise1_style;
      break;
    case exercise2:
      style = &ud->exercise2_style;
      break;
    case exercise3:
      style = &ud->exercise3_style;
      break;
    case directions:
      style = &ud->directions_style;
      break;
    case stanza:
      style = &ud->stanza_style;
      break;
    case line:
      style = &ud->line_style;
      break;
    case quotation:
      style = &ud->quotation_style;
      break;
    case attribution:
      style = &ud->attribution_style;
      break;
    case section:
      style = &ud->section_style;
      break;
    case subsection:
      style = &ud->subsection_style;
      break;
    case para:
      style = &ud->para_style;
      break;
    case blanklinebefore:
      style = &ud->blanklinebefore_style;
      break;
    case style1:
      style = &ud->style1_style;
      break;
    case style2:
      style = &ud->style2_style;
      break;
    case style3:
      style = &ud->style3_style;
      break;
    case style4:
      style = &ud->style4_style;
      break;
    case style5:
      style = &ud->style5_style;
      break;
    default:
      style = NULL;
      break;
    }
  return style;
}
