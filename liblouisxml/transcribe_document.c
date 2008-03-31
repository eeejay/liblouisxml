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
#include <string.h>
#include "louisxml.h"

static void docuCdataSection (xmlNode * node);
static int docuEmptyElement (xmlNode * node);


int
transcribe_document (xmlNode * node)
{
  xmlNode *child;
  ud->top = -1;
  start_document ();
  push_sem_stack (node);
  switch (ud->stack[ud->top])
    {
    case no:
      if (ud->text_length > 0 && ud->text_length < ud->max_length &&
	  ud->text_buffer[ud->text_length - 1] > 32)
	ud->text_buffer[ud->text_length++] = 32;
      break;
    case skip:
      pop_sem_stack ();
      return 0;
    case code:
      transcribe_computerCode (node, 0);
      write_paragraph (para);
      pop_sem_stack ();
      return 1;
    case changetable:
      change_table (node);
      write_paragraph (para);
      return 1;
    case math:
      transcribe_math (node, 0);
      write_paragraph (para);
      pop_sem_stack ();
      return 1;
    case graphic:
      transcribe_graphic (node, 0);
      write_paragraph (para);
      pop_sem_stack ();
      return 1;
    case chemistry:
      transcribe_chemistry (node, 0);
      write_paragraph (para);
      pop_sem_stack ();
      return 1;
    case music:
      transcribe_music (node, 0);
      write_paragraph (para);
      pop_sem_stack ();
      return 1;
    case table:
    case caption:
    case exercise1:
    case exercise2:
    case exercise3:
    case directions:
    case stanza:
    case quotation:
    case attribution:
    case section:
    case subsection:
    case list:
    case center:
    case heading4:
    case heading3:
    case heading2:
    case heading1:
    case para:
      transcribe_paragraph (node, 0);
      pop_sem_stack ();
      return 1;
    default:
      break;
    }
  child = node->children;
  while (child)
    {
      switch (child->type)
	{
	case XML_ELEMENT_NODE:
	  if (child->children)
	    transcribe_paragraph (child, 0);
	  else
	    docuEmptyElement (child);
	  break;
	case XML_TEXT_NODE:
	  insert_text (child);
	  break;
	case XML_CDATA_SECTION_NODE:
	  docuCdataSection (child);
	  break;
	default:
	  break;
	}
      child = child->next;
    }
  end_document ();
  pop_sem_stack ();
  return 1;
}

static int
docuEmptyElement (xmlNode * node)
{
  push_sem_stack (node);
  switch (ud->stack[ud->top])
    {
    case para:
      break;
    case softreturn:
      insert_code (node, 0);
      break;
    case boxline:
      break;
    case blankline:
      break;
    case newpage:
      break;
    case righthandpage:
      break;
    default:
      break;
    }
  pop_sem_stack ();
  return 1;
}

static void
docuCdataSection (xmlNode * node)
{
  insert_utf8 (node->content);
}
