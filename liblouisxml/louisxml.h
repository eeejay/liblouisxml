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

#ifndef __liblouisxml_h
#define __liblouisxml_h
#include <libxml/parser.h>
#include "liblouisxml.h"
#include "sem_enum.h"
#include "transcriber.h"

#ifdef WIN32
#define strcasecmp _strnicmp
#define snprintf _snprintf
#endif

#define CHARSIZE sizeof (widechar)
#define BUFSIZE 8192
#define MAXNAMELEN 256
#define MAXNUMLEN 32
#define STACKSIZE 100

typedef enum
{
  utf8 = 0,
  utf16,
  utf32,
  ascii8
} Encoding;

typedef enum
{
  plain = 0,
  html
} TextFormat;

typedef struct
{				/*user data */
  int text_length;
  int max_length;
  int translated_length;
  int max_trans_length;
  int paragraph_interrupted;
  int interline;
  int has_math;
  int has_comp_code;
  int has_chem;
  int has_graphics;
  int has_music;
  int has_cdata;
  Encoding input_encoding;
  Encoding output_encoding;
  TextFormat back_text;
  int back_line_length;
  int cells_per_line;
  int lines_per_page;
  int beginning_braille_page_number;
  int interpoint;
  int print_page_number_at;
  int braille_page_number_at;
  int hyphenate;
  int internet_access;
  int new_entries;
  FILE *inFile;
  FILE *outFile;
  char *mainBrailleTable;
  char *inbuf;
  int inlen;
  widechar *outbuf;
  int outlen;
  int outlen_so_far;
  int lines_on_page;
  int braille_page_number;
  int paragraphs;
  int braille_pages;
  int print_pages;
  char developer_path[MAXNAMELEN];
  char user_path[MAXNAMELEN];
  widechar running_head[MAXNAMELEN / 2];
  widechar footer[MAXNAMELEN / 2];
  int running_head_length;
  int footer_length;
  char contracted_table_name[MAXNAMELEN];
  char uncontracted_table_name[MAXNAMELEN];
  char compbrl_table_name[MAXNAMELEN];
  char mathtext_table_name[MAXNAMELEN];
  char mathexpr_table_name[MAXNAMELEN];
  char edit_table_name[MAXNAMELEN];
  char interline_back_table_name[MAXNAMELEN];
  char semantic_files[MAXNAMELEN];
  widechar print_page_number[MAXNUMLEN];
  char lineEnd[8];
  char pageEnd[8];
  char fileEnd[8];
  /*stylesheet */
  StyleType document_style;
  StyleType para_style;
  StyleType heading1_style;
  StyleType heading2_style;
  StyleType heading3_style;
  StyleType heading4_style;
  StyleType section_style;
  StyleType subsection_style;
  StyleType table_style;
  StyleType volume_style;
  StyleType titlepage_style;
  StyleType contents_style;
  StyleType code_style;
  StyleType quotation_style;
  StyleType attribution_style;
  StyleType indexx_style;
  StyleType glossary_style;
  StyleType biblio_style;
  StyleType list_style;
  StyleType caption_style;
  StyleType exercise1_style;
  StyleType exercise2_style;
  StyleType exercise3_style;
  StyleType directions_style;
  StyleType stanza_style;
  StyleType line_style;
  StyleType spatial_style;
  StyleType arith_style;
  StyleType note_style;
  StyleType trnote_style;
  StyleType dispmath_style;
  StyleType disptext_style;
  StyleType matrix_style;
  StyleType music_style;
  StyleType graph_style;
  StyleType graphlabel_style;
  StyleType dedication_style;
  StyleType blanklinebefore_style;
  StyleType style1_style;
  StyleType style2_style;
  StyleType style3_style;
  StyleType style4_style;
  StyleType style5_style;
  StyleType scratch_style;
/*end of stylesheet*/
  int top;
  sem_act stack[STACKSIZE];
  widechar text_buffer[2 * BUFSIZE];
  unsigned char typeform[BUFSIZE];
  widechar translated_buffer[2 * BUFSIZE];
  char xml_header[BUFSIZE];
} UserData;
extern UserData *ud;

StyleType *style_cases (sem_act action);
sem_act find_semantic_number (const char *semName);
int find_file (const char *fileName, char *filePath);
int read_configuration_file (const char *const configFileName,
			     const char const *logFileName);
int examine_document (xmlNode * node);
int transcribe_document (xmlNode * node);
int transcribe_math (xmlNode * node, int action);
int transcribe_computerCode (xmlNode * node, int action);
int transcribe_paragraph (xmlNode * node, int action);
int transcribe_chemistry (xmlNode * node, int action);
int transcribe_graphic (xmlNode * node, int action);
int transcribe_music (xmlNode * node, int action);
int compile_semantic_table (xmlNode * rootElement);
sem_act set_sem_attr (xmlNode * node);
sem_act get_sem_attr (xmlNode * node);
sem_act push_sem_stack (xmlNode * node);
sem_act pop_sem_stack ();
void destroy_semantic_table (void);
void append_new_entries (void);
int insert_code (xmlNode *node, int which);
xmlChar *get_attr_value (xmlNode * node);
int change_table (xmlNode * node);
int sem_examine (void);
int sem_rout (void);
#endif /*__louisxml_h*/
