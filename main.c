/* file: main.c */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include <time.h>
#define ALLOC
#include "globals.h"

PUBLIC void inilinebuffer();		/* file scan.c		*/
PUBLIC  int endofbuffer();
PUBLIC void doinclude();
PUBLIC void getsym();
PUBLIC void inimem1();			/* file utils.c		*/
PUBLIC void inimem2();
PUBLIC void inisymboltable();
PUBLIC void readterm();
PUBLIC void writeterm();
PUBLIC void writefactor();
PUBLIC void error();
PUBLIC void exeterm();			/* file interp.c	*/
PUBLIC void abortexecution_();		/* forward */

PUBLIC void enterglobal()
{
    location = symtabindex++;
D(  printf("getsym, new: '%s'\n",id); )
    location->name = (char *) malloc(strlen(id) + 1);
    strcpy(location->name,id);
    location->u.body = NULL; /* may be assigned in definition */
    location->next = hashentry[hashvalue];
D(  printf("entered %s at %d\n",id,LOC2INT(location)); )
    hashentry[hashvalue] = location;
}
PUBLIC void lookup()
{
D(  printf("%s  hashes to %d\n",id,hashvalue); )
    location = localentry;
    while (location != symtab &&
	   strcmp(id,location->name) != 0)
	location = location->next;
    if (location != symtab) /* found in local table */
      {
D(	printf("found %s in local table\n",id); )
	return; }
    location = hashentry[hashvalue];
    while (location != symtab &&
	   strcmp(id,location->name) != 0)
	location = location->next;
    if (location == symtab) /* not found, enter in global */
	enterglobal();
}
PRIVATE void defsequence();		/* forward */

PRIVATE void definition(hidden)
int hidden;
{
    Entry *here = NULL;
    Entry *savelocalentry;
    if (sym == HIDE)
      { getsym();
	savelocalentry = localentry;
	defsequence(1);
	if (sym == IN) getsym();
	    else error(" IN expected in HIDE-declaration");
	defsequence(hidden);
	localentry = savelocalentry;
	if (sym == END) getsym();
	    else error(" END expected in HIDE-declaration");
	return; }
    if (sym != ATOM)
      { error("atom expected at start of definition");
	abortexecution_(); }
    else if (hidden)
      { location = symtabindex++;
D(	printf("hidden definition '%s' at %ld \n",id,LOC2INT(location)); )
	location->name = (char *) malloc(strlen(id) + 1);
	strcpy(location->name, id);
	location->u.body = NULL; /* may be assigned later */
	location->next = localentry;
	localentry = location; }
    else lookup();
    if (location < firstlibra)
      { printf("warning: overwriting inbuilt '%s'\n",location->name);
	enterglobal(); }
    here = location; getsym();
    if (sym == EQDEF) getsym();
	else error(" == expected in definition");
    readterm();
D(  printf("assigned this body: "); )
D(  writeterm(stk->u.lis); )
D(  printf("\n"); )
    if (here != NULL) here->u.body = stk->u.lis;
    stk = stk->next;
}

PRIVATE void defsequence(hidden)
int hidden;
{
    definition(hidden);
    while (sym == SEMICOL)
      { getsym(); definition(hidden); }
}

jmp_buf begin;

PUBLIC void abortexecution_()
{
    conts = dump = dump1 = dump2 = dump3 = dump4 = dump5 = NULL;
    longjmp(begin,0);
}
PUBLIC void execerror(message,op)
    char *message, *op;
{
    printf("run time error: %s needed for %s\n",message,op);
    abortexecution_();
}
PUBLIC void quit_()
{
    long totaltime;
    totaltime = clock() - startclock;
    printf("time:  %ld CPU,  %d gc (= %ld%%)\n",
	totaltime, gc_clock,
	totaltime ? (1004*gc_clock)/(10*totaltime) : 0);
    exit(0);
}
static int mustinclude = 1;

#define CHECK(D,NAME)						\
    if (D)							\
      { printf("->  %s is not empty:\n",NAME);			\
	writeterm(D); printf("\n"); }

int main()
{
    printf("JOY  -  compiled at %s on %s \n",__TIME__,__DATE__);
    startclock = clock();
    gc_clock = 0;
    echoflag = INIECHOFLAG;
    tracegc = INITRACEGC;
    autoput = INIAUTOPUT;
    ch = ' ';
    inilinebuffer();
    inisymboltable();
    inimem1(); inimem2();
    setjmp(begin);
D(  printf("starting main loop\n"); )
    while (1)
     { if (mustinclude)
	  { mustinclude = 0;
	    if (fopen("usrlib.joy","r"))
	        doinclude("usrlib.joy"); }
	getsym();
	if (sym == LIBRA)
	  { inimem1();
	    getsym(); defsequence(0);
	    inimem2(); }
	else
	  { readterm();
D(	    printf("program is: "); writeterm(stk->u.lis); printf("\n"); )
	    prog = stk->u.lis;
	    stk = stk->next;
	    conts = NULL;
	    exeterm(prog);
	    if (conts || dump || dump1 || dump2 || dump3 || dump4 || dump5)
	      { printf("the dumps are not empty\n");
		CHECK(conts,"conts");
		CHECK(dump,"dump"); CHECK(dump1,"dump1");
		CHECK(dump2,"dump2"); CHECK(dump3,"dump3");
		CHECK(dump4,"dump4"); CHECK(dump5,"dump5"); }
	    if (autoput && stk != NULL)
	      { writefactor(stk); printf("\n"); stk = stk->next; } } }
}
