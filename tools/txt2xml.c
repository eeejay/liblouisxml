#include <stdio.h>
int
main (int argc, char **argv)
{
  int ch = 0, pch = 0;
  int firstchar = 1;
  while ((ch = getchar ()) != EOF)
    {
      if (ch != '\n' && ch < 32 && ch > 126)
	ch = ' ';
      if (pch == ' ' && ch == '\n')
	{
	  pch = '\n';
	  continue;
	}
      if ((pch == '\n' || pch == ' ') && ch == ' ')
	continue;
      if (firstchar)
	printf ("<p>");
      if (pch == '\n' && ch == '\n')
	{
	  printf ("</p>\n\n<p>");
	  do
	    {
	    }
	  while ((ch = getchar ()) == '\n');
	}
      else if (!firstchar)
	putchar (pch);
      firstchar = 0;
      pch = ch;
    }
  if (!firstchar)
    {
      putchar (ch);
      printf ("</p>\n");
    }
  return 0;
}
