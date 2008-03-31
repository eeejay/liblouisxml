#include <stdio.h>
int
main (int argc, char **argv)
{
  int skip = 0;
  int ch = 0;
  int pch = 0;
  int ppch = 0;
  while ((ch = getchar ()) != EOF)
    {
      if (ch == 12 || ch == 13)
	continue;
      if (ch == '<')
	{
	  skip = 1;
	  continue;
	}
      if (skip)
	{
	  if (ch == '>')
	    skip = 0;
	  continue;
	}
      if (ch <= 32 && pch <= 32)
	continue;
      if (!(pch == 10 && ((ppch >= 97 && ppch <= 122) || ppch == ',')))
	{
	  if (pch == 10 && ch < 97)
	    putchar (10);
	}
      ppch = pch;
      pch = ch;
      putchar (ch);
    }
  return 0;
}
