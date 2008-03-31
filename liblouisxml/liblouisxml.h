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

#ifndef __LIBLOUISXML_H_
#define __LIBLOUISXML_H_
#include <liblouis.h>
#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */


  void * lbx_initialize (const char *const configFileName, const char 
const *logFileName);


/* This function initializes the libxml2 library and processes the 
* configuration file given in configFileName. It returns a pointer to 
* the UserData structure. This pointer is void and must be cast to 
* (UserData *) in the calling program. To access the information in 
* this structure you must include louisxml.h */

  int lbx_translateString
    (const char *const configFileName,
     char * inbuf, widechar *outbuf, 
int *outlen);

/* This function takes a well-formed xml expression in inbuf and
translates it into a string of 16-bit braille characters in outbuf.  The
xml expression must be immediately followed by a zero or null byte. If
it does not begin with an xul header, one is added. The header is
specified by the xmlHeader line in the configuration file. If no such
line is present, a default header ssecifying UTF-8 encoding is used.
Which 16-bit character in outbuf represents which dot pattern is
indicated in the liblouis translation tables. The
configFileName parameter points to a configuration file. Among other
things, this file specifies a translation table. It is this table which
controls just how the translation is made, whether in Grade 2, Grade 1,
or something else. Note that the *outlen parameter is a pointer to an
integer. When the function is called, this integer contains the maximum
output length. When it returns, it is set to the actual length used. The
function returns 1 if no errors were encountered and a negative number
if a conplete translation could not be done.  */

int lbx_translateFile (char *configFileName, char *inputFileName, 
char *outputFileName);

int lbx_translateTextFile (char *configFileName, char *inputFileName, 
char *outputFileName);
int lbx_backTranslateFile (char *configFileName, char 
*inputFileName, 
char *outputFileName);

  void lbx_free (void);

/* This function should be called at the end of * the application to
free all memory allocated by LIBLOUISXML. If you wish to change
configuration files during your application, you must call lbx_free
first. The configuration file and any tables needed are compiled the
first time they are used. */


#ifdef __cplusplus
}
#endif				/* __cplusplus */


#endif				/*LIBLOUISXML_H_ */
