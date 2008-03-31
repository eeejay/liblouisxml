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

#ifndef __transcriber_h
#define __transcriber_h

typedef enum
{
  leftJustified = 0,
  rightJustified = 1,
  centered = 2,
  alignColumnsLeft = 3,
  alignColumnsRight = 4,
  listColumns = 5,
  listLines = 6,
  computerCoded = 7
} StyleFormat;

typedef struct
{				/*Paragraph formatting instructions */
  sem_act action;
  int lines_before;
  int lines_after;
  int left_margin;
  int first_line_indent;	/* At true margin if negative */
  sem_act translate;
  int skip_number_lines;	/*Don't write on lines with page numbers */
  StyleFormat format;
  int newpage_before;
  int newpage_after;
  int righthand_page;
} StyleType;

int insert_utf8 (unsigned char *intext);
int insert_utf16 (widechar * intext, int length);
int insert_translation (const char const *table);
int write_paragraph (sem_act action);
int start_document (void);
void end_document (void);
int transcribe_text_string (void);
int transcribe_text_file (void);
int back_translate_file (void);
StyleType *find_current_style (void);
void insert_text (xmlNode * node);
#endif /*__transcriber_h*/
