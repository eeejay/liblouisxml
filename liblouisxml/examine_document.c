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

static void examText (xmlNode * node);
static void examCdataa (xmlNode * node);
static int examEmptyElement (xmlNode * node);

int
examine_document (xmlNode * node)
{
/*Examine the DOM tree, add semantic attributes and set indicators.*/
  xmlNode *child;
  ud->stack[++ud->top] = set_sem_attr (node);
  switch (ud->stack[ud->top])
    {
case code:
ud->has_comp_code = 1;
break;
    case math:
      ud->has_math = 1;
      break;
    case chemistry:
      ud->has_chem = 1;
      break;
    case graphic:
      ud->has_graphics = 1;
      break;
    case music:
      ud->has_music = 1;
      break;
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
	    examine_document (child);
	  else
	    examEmptyElement (child);
	  break;
	case XML_TEXT_NODE:
	  examText (child);
	  break;
	case XML_CDATA_SECTION_NODE:
	  examCdataa (child);
	  break;
	default:
	  break;
	}
      child = child->next;
    }
  ud->top--;
  return 1;
}

static int
examEmptyElement (xmlNode * node)
{
  ud->stack[++ud->top] = set_sem_attr (node);
  switch (ud->stack[ud->top])
    {
    case newpage:
      break;
    case righthandpage:
      break;
    default:
      break;
    }
  ud->top--;
  return 1;
}

static void
examText (xmlNode * node)
{
  switch (ud->stack[ud->top])
    {
    case pagenum:
      break;
    default:
      break;
    }
}

static void
examCdataa (xmlNode * node)
{
  ud->has_cdata = 1;
}
