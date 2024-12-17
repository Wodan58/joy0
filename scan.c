/*
    module  : scan.c
    version : 1.1.1.2
    date    : 12/16/24
*/
/* FILE : scan.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "globals.h"

#define EOLN '\n'

PUBLIC void quit_();			/* file main.c	*/
PUBLIC void execerror(char *message, char *op);

static FILE  *infile[INPSTACKMAX];
static int ilevel;
static int linenumber = 0;
static char linbuf[INPLINEMAX];
static int linelength, currentcolumn = 0;
static int errorcount = 0;

PUBLIC void inilinebuffer()
{
    ilevel = 0; infile[ilevel] = stdin;
}

PUBLIC void putline()
{
    if (echoflag > 2) printf("%4d",linenumber);
    if (echoflag > 1) printf("\t");
    printf("%s\n",linbuf);
}
PRIVATE void getch()
{
    char c;
    if (currentcolumn == linelength)
      { Again:
	currentcolumn = 0; linelength = 0;
	linenumber++;
	while ((c = getc(infile[ilevel])) != EOLN)
	  { linbuf[linelength++] = c;
	    if (feof(infile[ilevel]))
	      { ilevel--;
D(		printf("reset to level %d\n",ilevel); )
		if (ilevel < 0) quit_(); } }
	linbuf[linelength++] = '\0';
	if (echoflag) putline();
	if (linbuf[0] == SHELLESCAPE)
	    { system(&linbuf[1 ]); goto Again; } }
    ch = linbuf[currentcolumn++];
}
PUBLIC int endofbuffer()
{
    return (currentcolumn == linelength);
}
PUBLIC void error(message)
    char *message;
{
    int i;
    putline();
    if (echoflag > 1) putchar('\t');
    for (i = 0; i < currentcolumn-2; i++)
	if (linbuf[i] <= ' ') putchar(linbuf[i]); else putchar(' ');
    printf("^\n\t%s\n",message);
    errorcount++;
}
PUBLIC void doinclude(filnam)
    char *filnam;
{
    if (ilevel+1 == INPSTACKMAX)
	execerror("fewer include files","include");
    if ((infile[ilevel+1] = fopen(filnam,"r")) != NULL)
	{ ilevel++; return; }
    execerror("valid file name","include");
}
PRIVATE char specialchar()
{
    getch();
    switch (ch)
      { case 'n' : return '\n';
	case 't' : return '\t';
	case 'b' : return '\b';
	case 'r' : return '\r';
	case 'f' : return '\f';
	case '\'': return '\'';
	case '\"': return '\"';
	default :
	    if (ch >= '0' && ch <= '9')
	      { int i;
		num = ch - '0';
		for (i = 0; i < 2; i++)
		  { getch();
		    if (ch < '0' || ch > '9')
		      { currentcolumn++; /* to get pointer OK */
			error("digit expected");
			currentcolumn--; }
		    num = 10 * num + ch - '0'; }
		return num; }
	    else return ch; }
}
PUBLIC void getsym()
{
Start:
    while (ch <= ' ') getch();
    switch (ch)
      { case '(':
	    getch();
	    if (ch == '*')
		{ getch();
		  do {while (ch != '*') getch(); getch();}
		    while (ch != ')');
		  getch(); goto Start; }
	    else {sym =  LPAREN; return;}
	case ')':
	    sym = RPAREN; getch(); return;
	case '[':
	    sym = LBRACK; getch(); return;
	case ']':
	    sym = RBRACK; getch(); return;
	case '{':
	    sym = LBRACE; getch(); return;
	case '}':
	    sym = RBRACE; getch(); return;
	case '.':
	    sym = PERIOD; getch(); return;
	case ';':
	    sym = SEMICOL; getch(); return;
	case '\'':
	    getch();
	    if (ch == '\\') ch = specialchar();
	    num = ch;
	    sym = CHAR_; getch(); return;
	case '"':
	  { char string[INPLINEMAX];
	    register int i = 0;
	    getch();
	    while (ch != '"' && !endofbuffer())
	      { if (ch == '\\') ch = specialchar();
		string[i++] = ch; getch();}
	    string[i] = '\0'; getch();
D(	    printf("getsym: string = %s\n",string); )
	    num = (long) malloc(strlen(string) + 1);
	    strcpy((char *) num, string);
	    sym = STRING_; return; }
	case '-': /* PERHAPS unary minus */
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	  { int sign = 1;
	    if ( isdigit(ch) ||
		 ( currentcolumn < linelength &&
		   isdigit((int)linbuf[currentcolumn]) ) )
	      { if (! isdigit(ch)) {sign = -1; getch();}
		num = 0;
		do {num = 10 * num + ch - '0'; getch();}
		    while (isdigit(ch));
		num *= sign; sym = INTEGER_; return; } }
	    /* ELSE '-' is not unary minus, fall through */
	    goto Next;
Next:
	default:
	  { int i = 0;
	    hashvalue = 0; /* ensure same algorithm in inisymtab */
	    do { if (i < ALEN-1) {id[i++] = ch; hashvalue += ch;}
		 getch(); }
	      while (isalpha(ch) || isdigit(ch) ||
		       ch == '=' || ch == '_' || ch == '-');
	    id[i] = '\0'; hashvalue %= HASHSIZE;
	    if (isupper((int)id[1]))
	      { if (strcmp(id,"LIBRA") == 0 || strcmp(id,"DEFINE") == 0)
		   { sym = LIBRA; return; }
		if (strcmp(id,"HIDE") == 0)
		  { sym = HIDE; return; }
		if (strcmp(id,"IN") == 0)
		  { sym = IN; return; }
		if (strcmp(id,"END") == 0)
		  { sym = END; return; }
		/* possibly other uppers here */
		}
	    if (strcmp(id,"==") == 0)
		{sym = EQDEF; return;}
	    sym = ATOM; return; } }
}
