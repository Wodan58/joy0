/*
    module  : utils.c
    version : 1.1.1.2
    date    : 10/02/23
*/
#include <stdio.h>
#include <time.h>
#include "globals.h"

#define CORRECT_GARBAGE_COLLECTOR

/* PUBLIC int clock(); */		/* file time.h		*/
PUBLIC void getsym();			/* file scan.c		*/
PUBLIC void error();
PUBLIC void execerror();		/* file main.c		*/
PUBLIC void lookup();
/* PUBLIC void exit(); */		/* file interp.c	*/

static Node
    memory[MEMORYMAX],
    *memoryindex = memory,
    *mem_low = memory,
    *mem_mid;
#define MEM_HIGH (MEMORYMAX-1)
static int direction = +1;
static int nodesinspected, nodescopied;
static int start_gc_clock;

PUBLIC void inimem1()
{
    stk = conts = dump = dump1 = dump2 = dump3 = dump4 = dump5 = NULL;
    direction = +1;
    memoryindex = mem_low;
}
PUBLIC void inimem2()
{
    mem_low = memoryindex;
#ifdef CORRECT_GARBAGE_COLLECTOR
    mem_mid = mem_low + (&memory[MEM_HIGH] - mem_low) / 2;
#else
    mem_mid = mem_low + (MEM_HIGH)/2;
#endif
    if (tracegc > 1)
      { printf("memory = %ld : %ld\n",
		(long)memory,MEM2INT(memory));
	printf("memoryindex = %ld : %ld\n",
		(long)memoryindex,MEM2INT(memoryindex));
	printf("mem_low = %ld : %ld\n",
		(long)mem_low,MEM2INT(mem_low));
	printf("top of mem = %ld : %ld\n",
		(long)(&memory[MEM_HIGH]),MEM2INT((&memory[MEM_HIGH])));
	printf("mem_mid = %ld : %ld\n",
		(long)mem_mid,MEM2INT(mem_mid)); }
}
PUBLIC void printnode(p)
    Node *p;
{
    printf("%10ld:	%-10s %10ld %10ld\n",
	MEM2INT(p),
	symtab[(short) p->op].name,
	p->op == LIST_ ? MEM2INT(p->u.lis) : p->u.num,
	MEM2INT(p->next));
}
PRIVATE Node *copy(n)
    Node *n;
{
    Node *temp;
    nodesinspected++;
    if (tracegc > 4) printf("copy ..\n");
    if (n == NULL) return NULL;
    if (n < mem_low) return n; /* later: combine with previous line */
    if (n->op == ILLEGAL_)
      { printf("copy: illegal node  "); printnode(n);  return(NULL); }
    if (n->op == COPIED_) return n->u.lis;
    temp = memoryindex; memoryindex += direction;
    temp->op = n->op;
    temp->u.num = n->op == LIST_ ? (long)copy(n->u.lis) : n->u.num;
    temp->next = copy(n->next);
    n->op = COPIED_;
    n->u.lis = temp;
    nodescopied++;
    if (tracegc > 3)
      { printf("%5d -    ",nodescopied); printnode(temp); }
    return temp;
}
PUBLIC void writeterm();

PUBLIC void gc1(mess)
    char * mess;
{
    start_gc_clock = clock();
    if (tracegc > 1)
	printf("begin %s garbage collection\n",mess);
    direction = - direction;
    memoryindex = (direction == 1) ? mem_low : &memory[MEM_HIGH];
/*
    if (tracegc > 1)
      { printf("direction = %d\n",direction);
	printf("memoryindex = %d : %d\n",
		(long)memoryindex,MEM2INT(memoryindex)); }
*/
    nodesinspected = nodescopied = 0;

#define COP(X,NAME)						\
    if (X != NULL)						\
      { if (tracegc > 2)					\
	  { printf("old %s = ",NAME);				\
	    writeterm(X); printf("\n"); }			\
	X = copy(X);						\
	if (tracegc > 2)					\
	  { printf("new %s = ",NAME);				\
	    writeterm(X); printf("\n"); } }

    COP(stk,"stk"); COP(prog,"prog"); COP(conts,"conts");
    COP(dump,"dump"); COP(dump1,"dump1"); COP(dump2,"dump2");
    COP(dump3,"dump3"); COP(dump4,"dump4"); COP(dump5,"dump5");
}
PRIVATE void gc2(mess)
    char * mess;
{
    int this_gc_clock;
    this_gc_clock = clock() - start_gc_clock;
    if (this_gc_clock == 0) this_gc_clock = 1; /* correction */
    if (tracegc > 0)
	printf("gc - %d nodes inspected, %d nodes copied, clock: %d\n",
	       nodesinspected,nodescopied,this_gc_clock);
    if (tracegc > 1)
	printf("end %s garbage collection\n",mess);
    gc_clock += this_gc_clock;
}
PUBLIC void gc_()
{
    gc1("user requested");
    gc2("user requested");
}
PUBLIC Node *newnode(o,l,r)
    Operator o;
    long l;
    Node *r;
{
    Node *p;
    if (memoryindex == mem_mid)
      { gc1("automatic");
	if (o == LIST_) l = (long)copy(l);
	r = copy(r);
#ifdef CORRECT_GARBAGE_COLLECTOR
	gc2("automatic");
	if ((direction == +1 && memoryindex >= mem_mid) ||
	    (direction == -1 && memoryindex <= mem_mid))
	    execerror("memory", "copying"); }
#else
	gc2("automatic"); }
#endif
    p = memoryindex;
    memoryindex += direction;
    p->op = o;
    p->u.num = l;
    p->next = r;
D(  printnode(p); )
    return p;
}
PUBLIC void memoryindex_()
{
    stk = newnode(INTEGER_,MEM2INT(memoryindex),stk);
}
PUBLIC void readfactor()	/* read a JOY factor		*/
{
    switch (sym)
      { case ATOM:
	    lookup();
D(	    printf("readfactor: location = %ld\n",(long) location); )
	    if (location < firstlibra)
		stk =  newnode(LOC2INT(location),location->u.proc,stk);
		else stk =  newnode(USR_,location,stk);
	    return;
	case INTEGER_: case CHAR_: case STRING_:
	    stk =  newnode(sym,num,stk);
	    return;
	case LBRACE:
	  { int set = 0; getsym();
	    while (sym != RBRACE)
	      { if (sym == CHAR_ || sym == INTEGER_)
		    set = set | (1 << num);
		  else error("numeric expected in set");
		getsym(); }
	    stk =  newnode(SET_,set,stk); }
	    return;
	case LBRACK:
	  { void readterm();
	    getsym();
	    readterm();
	    if (sym != RBRACK)
		error("']' expected");
	    return; }
/* MU  -     MU x IN --x--x-- .
	    stk = newnode(MU,
			   newnode(IN, entry to x, body), stk);
	    return;
*/
	default:
	    error("a factor cannot begin with this symbol");
	    return; }
}
PUBLIC void readterm()
{
    stk = newnode(LIST_,NULL,stk);
    if (sym <= ATOM)
      { readfactor();
	stk->next->u.lis = stk;
	stk = stk->next;
	stk->u.lis->next = NULL;
	dump = newnode(LIST_,stk->u.lis,dump);
	getsym();
	while (sym <= ATOM)
	  { readfactor();
	    dump->u.lis->next = stk;
	    stk = stk->next;
	    dump->u.lis->next->next = NULL;
	    dump->u.lis = dump->u.lis->next;
	    getsym(); }
	dump = dump->next; }
}

PUBLIC void writefactor(n)
    Node *n;
{
    if (n == NULL)
#ifdef DEBUG
	return;
#else
	execerror("non-empty stack","print");
#endif
    switch (n->op)
      { case BOOLEAN_:
	    printf("%s", n->u.num ? "true" : "false"); return;
	case INTEGER_:
	    printf("%ld",n->u.num); return;
	case SET_:
	  { int i; long set = n->u.set;
	    printf("{");
	    for (i = 0; i < SETSIZE; i++)
		if (set & (1 << i))
		  { printf("%d",i);
		    set = set & ~(1 << i);
		    if (set != 0)
			printf(" "); }
	    printf("}");
	    return; }
	case CHAR_:
	    printf("'%c", (char) n->u.num); return;
	case STRING_:
	    printf("\"%s\"",n->u.str); return;
	case LIST_:
	    printf("%s","[");
	    writeterm(n->u.lis);
	    printf("%s","]");
	    return;
	case USR_:
	    printf("%s", n->u.ent->name ); return;
	default:
	    printf("%s",symtab[(int) n->op].name); return; }
}
PUBLIC void writeterm(n)
    Node *n;
{
    while  (n != NULL)
	{
	writefactor(n);
	n = n->next;
	if (n != NULL)
	    printf(" ");
	}
}
