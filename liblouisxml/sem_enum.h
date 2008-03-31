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

typedef enum
{
  no = 0,
  skip,
  cdata,
  linkto,
  changetable,
  start_text,
  document,
  blanklinebefore,
  para,
  pagenum,
  heading1,
  heading2,
  heading3,
  heading4,
  runninghead,
  footer,
  section,
  subsection,
  compbrl,
  uncontracted,
  contracted,
  table,
  tblhead,
  tblbody,
  tblrow,
  tblcol,
  volume,
  titlepage,
  contents,
  frontmatter,
  acknowledge,
  bodymatter,
  rearmatter,
  code,
  boxline,
  quotation,
  attribution,
  indexx,
  glossary,
  biblio,
  list,
  caption,
  exercise1,
  exercise2,
  exercise3,
  directions,
  stanza,
  line,
  italicx,
  boldx,
  allcaps,
  center,
  noindent,
  rightalign,
  blankline,
  softreturn,
  newpage,
  righthandpage,
  note,
  endnotes,
  preface,
  jacket,
  specsym,
  trnote,
  copyright,
  disptext,
  booktitle,
  author,
  transcriber,
  dedication,
  tnpage,
  style1,
  style2,
  style3,
  style4,
  style5,
  start_math,
  math,
  mi,
  mn,
  mo,
  mtext,
  mspace,
  ms,
  mglyph,
  mrow,
  mfrac,
  msqrt,
  mroot,
  mstyle,
  merror,
  mpadded,
  mphantom,
  mfenced,
  menclose,
  msub,
  msup,
  msubsup,
  munder,
  mover,
  munderover,
  mmultiscripts,
  none,
  semantics,
  mprescripts,
  mtable,
  mtr,
  mtd,
  maligngroup,
  malignmark,
  mlabeledtr,
  maction,
  spatial,
  matrix,
  row,
  arith,
  dispmath,
  start_chem,
  chemistry,
  start_graphic,
  graphic,
  graph,
  graphlabel,
  start_music,
  music,
  end_all
}
sem_act;
