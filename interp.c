/*
    module  : interp.c
    version : 1.1.1.2
    date    : 10/02/23
*/
/* FILE: interp.c */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "globals.h"

/* #define TRACING */

PUBLIC void printnode();		/* file utils.c		*/

PRIVATE void helpdetail_();		/* this file		*/
PRIVATE void make_manual(int latex);
PUBLIC char * opername();

#define ONEPARAM(NAME)						\
    if (stk == NULL)						\
	execerror("one parameter",NAME)
#define TWOPARAMS(NAME)						\
    if (stk == NULL || stk->next == NULL)			\
	execerror("two parameters",NAME)
#define THREEPARAMS(NAME)					\
    if (stk == NULL || stk->next == NULL			\
	    || stk->next->next == NULL)				\
	execerror("three parameters",NAME)
#define FOURPARAMS(NAME)					\
    if (stk == NULL || stk->next == NULL			\
	    || stk->next->next == NULL				\
	    || stk->next->next->next == NULL)			\
	execerror("four parameters",NAME)
#define FIVEPARAMS(NAME)					\
    if (stk == NULL || stk->next == NULL			\
	    || stk->next->next == NULL				\
	    || stk->next->next->next == NULL			\
	    || stk->next->next->next->next == NULL)		\
	execerror("five parameters",NAME)
#define ONEQUOTE(NAME)						\
    if (stk->op != LIST_)					\
	execerror("quotation as top parameter",NAME)		
#define TWOQUOTES(NAME)						\
    ONEQUOTE(NAME);						\
    if (stk->next->op != LIST_)					\
	execerror("quotation as second parameter",NAME)
#define THREEQUOTES(NAME)					\
    TWOQUOTES(NAME);						\
    if (stk->next->next->op != LIST_)				\
	execerror("quotation as third parameter",NAME)
#define FOURQUOTES(NAME)					\
    THREEQUOTES(NAME);						\
    if (stk->next->next->next->op != LIST_)			\
	execerror("quotation as fourth parameter",NAME)
#define FIVEQUOTES(NAME)					\
    FOURQUOTES(NAME);						\
    if (stk->next->next->next->next->op != LIST_)		\
	execerror("quotation as fifth parameter",NAME)
#define SAME2TYPES(NAME)					\
    if (stk->op != stk->next->op)				\
	execerror("two parameters of the same type",NAME)
#define STRING(NAME)						\
    if (stk->op != STRING_)					\
	execerror("string",NAME)
#define INTEGER(NAME)						\
    if (stk->op != INTEGER_)					\
	execerror("integer",NAME)
#define INTEGERS2(NAME)						\
    if (stk->op != INTEGER_ || stk->next->op != INTEGER_)	\
	execerror("two integers",NAME)
#define NUMERICTYPE(NAME)					\
    if (stk->op != INTEGER_ && stk->op !=  CHAR_)		\
	execerror("numeric",NAME)
#define NUMERIC2(NAME)						\
    if (stk->next->op != INTEGER_ && stk->next->op != CHAR_)	\
	execerror("numeric second parameter",NAME)
#define CHECKZERO(NAME)						\
    if (stk->u.num == 0)					\
	execerror("non-zero operand",NAME)
#define LIST(NAME)						\
    if (stk->op != LIST_)					\
	execerror("list",NAME)
#define LIST2(NAME)						\
    if (stk->next->op != LIST_)					\
	execerror("list as second parameter",NAME)
#define USERDEF(NAME)						\
    if (stk->op != USR_)					\
	execerror("user defined symbol",NAME)
#define CHECKLIST(OPR,NAME)					\
    if (OPR != LIST_)						\
	execerror("internal list",NAME)
#define CHECKSETMEMBER(NODE,NAME)				\
    if ((NODE->op != INTEGER_ && NODE->op != CHAR_) || 		\
	NODE->u.num >= SETSIZE)					\
	execerror("small numeric",NAME)
#define CHECKEMPTYSET(SET,NAME)					\
    if (SET == 0)						\
	execerror("non-empty set",NAME)
#define CHECKEMPTYSTRING(STRING,NAME)				\
    if (*STRING == '\0')					\
	execerror("non-empty string",NAME)
#define CHECKEMPTYLIST(LIST,NAME)				\
    if (LIST == NULL)						\
	execerror("non-empty list",NAME)
#define INDEXTOOLARGE(NAME)					\
    execerror("smaller index",NAME)
#define BADAGGREGATE(NAME)					\
    execerror("aggregate parameter",NAME)
#define BADDATA(NAME)						\
    do { execerror("different type",NAME); break; } while (0)

#define DMP dump->u.lis
#define DMP1 dump1->u.lis
#define DMP2 dump2->u.lis
#define DMP3 dump3->u.lis
#define DMP4 dump4->u.lis
#define DMP5 dump5->u.lis
#define SAVESTACK  dump = newnode(LIST_,stk,dump)
#define SAVED1 DMP
#define SAVED2 DMP->next
#define SAVED3 DMP->next->next
#define SAVED4 DMP->next->next->next
#define SAVED5 DMP->next->next->next->next
#define SAVED6 DMP->next->next->next->next->next

#define POP(X) X = X->next

#define NULLARY(TYPE,VALUE)					\
    stk = newnode(TYPE,VALUE,stk)
#define UNARY(TYPE,VALUE)					\
    stk = newnode(TYPE,VALUE,stk->next)
#define BINARY(TYPE,VALUE)					\
    stk = newnode(TYPE,VALUE,stk->next->next)
#define TERNARY(TYPE,VALUE)					\
    stk = newnode(TYPE,VALUE,stk->next->next->next)

#define GETSTRING(NODE)						\
  ( NODE->op == STRING_  ?  NODE->u.str :			\
   (NODE->op == USR_  ?  NODE->u.ent->name :			\
    opername(NODE->op) ) )

/* PUBLIC int clock(); */		/* file time.h		*/
PUBLIC Node *newnode();			/* file utils.c 	*/
PUBLIC void memoryindex_();
PUBLIC void execerror();		/* file main.c		*/
PUBLIC void abortexecution_();
PUBLIC void getsym();			/* file scan.c		*/
PUBLIC void doinclude();
PUBLIC void readfactor();		/* file utils.c		*/
PUBLIC void writefactor();
PUBLIC void writeterm();
PUBLIC void quit_();
PUBLIC void gc_();
/* PUBLIC  int malloc(); */		/* in the library	*/
/* PUBLIC void system(); */

/* - - - -  O P E R A N D S   - - - - */

#define PUSH(PROCEDURE,TYPE,VALUE)				\
PRIVATE void PROCEDURE()					\
{   NULLARY(TYPE,VALUE); }
PUSH(true_,BOOLEAN_,1)				/* constants	*/
PUSH(false_,BOOLEAN_,0)
PUSH(setsize_,INTEGER_,SETSIZE)
PUSH(maxint_,INTEGER_,MAXINT)
PUSH(symtabmax_,INTEGER_,SYMTABMAX)
PUSH(memorymax_,INTEGER_,MEMORYMAX)
PUSH(stack_,LIST_,stk)				/* variables	*/
PUSH(dump_,LIST_,dump)
PUSH(conts_,LIST_,newnode(LIST_,conts->u.lis->next,conts->next))
PUSH(symtabindex_,INTEGER_,LOC2INT(symtabindex))
/* this is now in utils.c
PUSH(memoryindex_,INTEGER_,MEM2INT(memoryindex))
*/
PUSH(echo_,INTEGER_,echoflag)
PUSH(autoput_,INTEGER_,autoput)
PUSH(clock_,INTEGER_,clock() - startclock)

/* - - - - -   O P E R A T O R S   - - - - - */

PRIVATE void id_()
{
    /* do nothing */
}
PRIVATE void unstack_()
{
    ONEPARAM("unstack");
    LIST("unstack");
    stk = stk->u.lis;
}
/*
PRIVATE void newstack_()
{
    stk = NULL;
}
*/

/* - - -   STACK   - - - */

PRIVATE void name_()
{
    ONEPARAM("name");
    UNARY(STRING_, stk->op == USR_ ?
		   stk->u.ent->name : opername(stk->op));
}
PRIVATE void body_()
{
    ONEPARAM("body");
    USERDEF("body");
    UNARY(LIST_,stk->u.ent->u.body);
}
PRIVATE void pop_()
{
    ONEPARAM("pop");
    POP(stk);
}
PRIVATE void swap_()
{
    TWOPARAMS("swap");
    SAVESTACK;
    BINARY(SAVED1->op,SAVED1->u.num);
    NULLARY(SAVED2->op,SAVED2->u.num);
    POP(dump);
}
PRIVATE void rollup_()
{
    THREEPARAMS("rollup");
    SAVESTACK;
    TERNARY(SAVED1->op,SAVED1->u.num);
    NULLARY(SAVED3->op,SAVED3->u.num);
    NULLARY(SAVED2->op,SAVED2->u.num);
    POP(dump);
}
PRIVATE void rolldown_()
{
    THREEPARAMS("rolldown");
    SAVESTACK;
    TERNARY(SAVED2->op,SAVED2->u.num);
    NULLARY(SAVED1->op,SAVED1->u.num);
    NULLARY(SAVED3->op,SAVED3->u.num);
    POP(dump);
}
PRIVATE void rotate_()
{
    THREEPARAMS("rotate");
    SAVESTACK;
    TERNARY(SAVED1->op,SAVED1->u.num);
    NULLARY(SAVED2->op,SAVED2->u.num);
    NULLARY(SAVED3->op,SAVED3->u.num);
    POP(dump);
}
PRIVATE void dup_()
{
    ONEPARAM("dup");
    NULLARY(stk->op,stk->u.num);
}

/* - - -   BOOLEAN   - - - */

#define ANDORXOR(PROCEDURE,NAME,OPER1,OPER2)			\
PRIVATE void PROCEDURE()					\
{   TWOPARAMS(NAME);						\
    SAME2TYPES(NAME);						\
    switch (stk->next->op)					\
      { case SET_:						\
	    BINARY(SET_,stk->next->u.set OPER1 stk->u.set);	\
	    return;						\
	case BOOLEAN_: case CHAR_: case INTEGER_: case LIST_:	\
	    BINARY(BOOLEAN_,stk->next->u.num OPER2 stk->u.num);	\
	    return;						\
	default:						\
	    BADDATA(NAME); } }
ANDORXOR(and_,"and",&,&&)
ANDORXOR(or_,"or",|,||)
ANDORXOR(xor_,"xor",^,!=)


/* - - -   INTEGER   - - - */

PRIVATE void abs_()
{
    ONEPARAM("abs");
    INTEGER("abs");
    if (stk->u.num < 0) UNARY(INTEGER_, - stk->u.num);
}
PRIVATE void sign_()
{
    ONEPARAM("sign");
    INTEGER("sign");
    if (stk->u.num < 0) UNARY(INTEGER_,-1);
    else if (stk->u.num > 0) UNARY(INTEGER_,1);
}
#define MULDIVREM(PROCEDURE,NAME,OPER,CHECK)			\
PRIVATE void PROCEDURE()					\
{   TWOPARAMS(NAME);						\
    INTEGERS2(NAME);						\
    CHECK;							\
    BINARY(INTEGER_,stk->next->u.num OPER stk->u.num); }
MULDIVREM(mul_,"*",*,)
MULDIVREM(rem_,"rem",%,CHECKZERO("rem"))
MULDIVREM(divide_,"/",/,CHECKZERO("/"))


/* - - -   NUMERIC   - - - */

#define PREDSUCC(PROCEDURE,NAME,OPER)				\
PRIVATE void PROCEDURE()					\
{   ONEPARAM(NAME);						\
    NUMERICTYPE(NAME);						\
    UNARY(stk->op,stk->u.num OPER 1); }
PREDSUCC(pred_,"pred",-)
PREDSUCC(succ_,"succ",+)

#define PLUSMINUS(PROCEDURE,NAME,OPER)				\
PRIVATE void PROCEDURE()					\
{   TWOPARAMS(NAME);						\
    INTEGER(NAME);						\
    NUMERIC2(NAME);						\
    BINARY(stk->next->op,stk->next->u.num OPER stk->u.num); }
PLUSMINUS(plus_,"+",+)
PLUSMINUS(minus_,"-",-)

#define MAXMIN(PROCEDURE,NAME,OPER)				\
PRIVATE void PROCEDURE()					\
{   TWOPARAMS(NAME);						\
    SAME2TYPES(NAME);						\
    NUMERICTYPE(NAME);						\
    BINARY(stk->op,						\
	stk->u.num OPER stk->next->u.num ?			\
	stk->next->u.num : stk->u.num); }
MAXMIN(max_,"max",<)
MAXMIN(min_,"min",>)

#define COMPREL(PROCEDURE,NAME,TYPE,OPR)			\
PRIVATE void PROCEDURE()					\
  { int comp = 0;						\
    TWOPARAMS(NAME);						\
    switch (stk->op)						\
      { case BOOLEAN_: case CHAR_: case INTEGER_:		\
	    comp = stk->next->u.num - stk->u.num OPR 0;		\
	    break;						\
	case SET_:						\
	  { int i = 0;						\
	    while ( i < SETSIZE &&				\
		    ( (stk->next->u.set & 1 << i) ==		\
		      (stk->u.set & 1 << i) )  )		\
		++i; 						\
	    if (i == SETSIZE) i = 0; else ++i;			\
	    if (!(stk->u.set & 1 << i)) i = -i;			\
	    comp = i OPR 0;					\
	    break; }						\
	case LIST_:						\
	    BADDATA(NAME);					\
	default:						\
	    comp = strcmp(GETSTRING(stk->next), GETSTRING(stk))	\
		   OPR 0;					\
	    break; }						\
    BINARY(TYPE,comp); }
COMPREL(eql_,"=",BOOLEAN_,==)
COMPREL(neql_,"!=",BOOLEAN_,!=)
COMPREL(less_,"<",BOOLEAN_,<)
COMPREL(leql_,"<=",BOOLEAN_,<=)
COMPREL(greater_,">",BOOLEAN_,>)
COMPREL(geql_,">=",BOOLEAN_,>=)
COMPREL(compare_,"compare",INTEGER_,+)

/* - - -   AGGREGATES   - - - */

PRIVATE void first_()
{
    ONEPARAM("first");
    switch (stk->op)
      { case LIST_:
	    CHECKEMPTYLIST(stk->u.lis,"first");
	    UNARY(stk->u.lis->op,stk->u.lis->u.num);
	    return;
	case STRING_:
	    CHECKEMPTYSTRING(stk->u.str,"first");
	    UNARY(CHAR_,*(stk->u.str));
	    return;
	case SET_:
	  { int i = 0;
	    CHECKEMPTYSET(stk->u.set,"first");
	    while (!(stk->u.set & (1 << i))) i++;
	    UNARY(INTEGER_,i);
	    return; }
	default:
	    BADAGGREGATE("first"); }
}
PRIVATE void rest_()
{
    ONEPARAM("rest");
    switch (stk->op)
      { case SET_:
	  { int i = 0;
	    CHECKEMPTYSET(stk->u.set,"rest");
	    while (!(stk->u.set & (1 << i))) i++;
	    UNARY(SET_,stk->u.set & ~(1 << i));
	    break; }
	case STRING_:
	  { char *s = stk->u.str;
	    CHECKEMPTYSTRING(s,"rest");
	    UNARY(STRING_, ++s);
	    break; }
	case LIST_:
	    CHECKEMPTYLIST(stk->u.lis,"rest");
	    UNARY(LIST_,stk->u.lis->next);
	    return;
	default:
	    BADAGGREGATE("rest"); }
}
PRIVATE void uncons_()
{
    ONEPARAM("uncons");
    switch (stk->op)
      { case SET_:
	  { int i = 0; long set = stk->u.set;
	    CHECKEMPTYSET(set,"uncons");
	    while (!(set & (1 << i))) i++;
	    UNARY(INTEGER_,i);
	    NULLARY(SET_,set & ~(1 << i));
	    break; }
	case STRING_:
	  { char *s = stk->u.str;
	    CHECKEMPTYSTRING(s,"uncons");
	    UNARY(CHAR_,*s);
	    NULLARY(STRING_,++s);
	    break; }
	case LIST_:
	    SAVESTACK;
	    CHECKEMPTYLIST(SAVED1->u.lis,"uncons");
	    UNARY(SAVED1->u.lis->op,SAVED1->u.lis->u.num);
	    NULLARY(LIST_,SAVED1->u.lis->next);
	    POP(dump);
	    return;
	default:
	    BADAGGREGATE("uncons"); }
}
PRIVATE void unswons_()
{
    ONEPARAM("unswons");
    switch (stk->op)
      { case SET_:
	  { int i = 0; long set = stk->u.set;
	    CHECKEMPTYSET(set,"unswons");
	    while (!(set & (1 << i))) i++;
	    UNARY(SET_,set & ~(1 << i));
	    NULLARY(INTEGER_,i);
	    break; }
	case STRING_:
	  { char *s = stk->u.str;
	    CHECKEMPTYSTRING(s,"unswons");
	    UNARY(STRING_,++s);
	    NULLARY(CHAR_,*(--s));
	    break; }
	case LIST_:
	    SAVESTACK;
	    CHECKEMPTYLIST(SAVED1->u.lis,"unswons");
	    UNARY(LIST_,SAVED1->u.lis->next);
	    NULLARY(SAVED1->u.lis->op,SAVED1->u.lis->u.num);
	    POP(dump);
	    return;
	default:
	    BADAGGREGATE("unswons"); }
}
PRIVATE int equal_aux(); /* forward */

PRIVATE int equal_list_aux(n1,n2)
Node *n1, *n2;
{
    if (n1 == NULL && n2 == NULL) return 1;
    if (n1 == NULL || n2 == NULL) return 0;
    if (equal_aux(n1,n2))
	return equal_list_aux(n1->next,n2->next);
    else return 0;
}
PRIVATE int equal_aux(n1,n2)
Node *n1, *n2;
{
    if (n1 == NULL && n2 == NULL) return 1;
    if (n1 == NULL || n2 == NULL) return 0;
    switch (n1->op)
      { case BOOLEAN_: case CHAR_: case INTEGER_:
	    if (n2->op != BOOLEAN_ && n2->op != CHAR_ &&
		n2->op != INTEGER_)
		return 0;
	    return n1->u.num == n2->u.num;
	case SET_ :
	    if (n2->op != SET_) return 0;
	    return n1->u.num == n2->u.num;
	case LIST_ :
	    if (n2->op != LIST_) return 0;
	    return equal_list_aux(n1->u.lis,n2->u.lis);
	default:
	    return strcmp(GETSTRING(n1),GETSTRING(n2)) == 0; }
}
PRIVATE void equal_()
{
    TWOPARAMS("equal");
    BINARY(BOOLEAN_,equal_aux(stk,stk->next));
}
#define INHAS(PROCEDURE,NAME,AGGR,ELEM)				\
PRIVATE void PROCEDURE()					\
{   int found = 0;						\
    TWOPARAMS(NAME);						\
    switch (AGGR->op)						\
      { case SET_:						\
	    found = ((AGGR->u.set) & (1 << ELEM->u.num)) > 0;	\
	    break;						\
	case STRING_:						\
	  { char *s;						\
	    for (s = AGGR->u.str;				\
		 *s != '\0' && *s != ELEM->u.num;		\
		 s++);						\
	    found = *s != '\0';					\
	    break; }						\
	case LIST_:						\
	  { Node *n = AGGR->u.lis;				\
	    while (n != NULL && n->u.num != ELEM->u.num)	\
		n = n->next;					\
	    found = n != NULL;					\
	    break; }						\
	default:						\
	    BADAGGREGATE(NAME); }				\
    BINARY(BOOLEAN_,found);					\
}
INHAS(in_,"in",stk,stk->next)
INHAS(has_,"has",stk->next,stk)

#define OF_AT(PROCEDURE,NAME,AGGR,INDEX)			\
PRIVATE void PROCEDURE()					\
{   TWOPARAMS(NAME);						\
    switch (AGGR->op)						\
      { case SET_:						\
	  { int i; int indx = INDEX->u.num;			\
	    CHECKEMPTYSET(AGGR->u.set,NAME);			\
	    for (i = 0; i < SETSIZE; i++)			\
	      { if (AGGR->u.set & (1 << i))			\
		  { if (indx == 0)				\
			{BINARY(INTEGER_,i); return;}		\
		    indx--; } }					\
	    INDEXTOOLARGE(NAME);				\
	    return; }						\
	case STRING_:						\
	    if (strlen(AGGR->u.str) < (size_t)INDEX->u.num)	\
		INDEXTOOLARGE(NAME);				\
	    BINARY(CHAR_,AGGR->u.str[INDEX->u.num]);		\
	    return;						\
	case LIST_:						\
	  { Node *n = AGGR->u.lis;  int i  = INDEX->u.num;	\
	    CHECKEMPTYLIST(n,NAME);				\
	    while (i > 0)					\
	      { if (n->next == NULL)				\
		    INDEXTOOLARGE(NAME);			\
		n = n->next; i--; }				\
	    BINARY(n->op,n->u.num);				\
	    return; }						\
	default:						\
	    BADAGGREGATE(NAME); }				\
}
OF_AT(of_,"of",stk,stk->next)
OF_AT(at_,"at",stk->next,stk)

PRIVATE void choice_()
{
    THREEPARAMS("choice");
    if (stk->next->next->u.num)
	 stk = newnode(stk->next->op,stk->next->u.num,
		       stk->next->next->next);
    else stk = newnode(stk->op,stk->u.num,
		       stk->next->next->next);
}
PRIVATE void opcase_()
{
    Node *n;
    ONEPARAM("opcase");
    LIST("opcase");
    n = stk->u.lis;
    CHECKEMPTYLIST(n,"opcase");
    while ( n->next != NULL &&
	    n->op == LIST_ &&
	    n->u.lis->op != stk->next->op )
	n = n->next;
    CHECKLIST(n->op,"opcase");
    UNARY(LIST_,
	n->next != NULL ? n->u.lis->next : n->u.lis);
}
#define CONS_SWONS(PROCEDURE,NAME,AGGR,ELEM)			\
PRIVATE void PROCEDURE()					\
{   TWOPARAMS(NAME);						\
    Node *temp;							\
    switch (AGGR->op)						\
      { case LIST_:						\
	    temp = newnode(ELEM->op, ELEM->u.num, AGGR->u.lis);	\
	    BINARY(LIST_,temp);					\
	    break;						\
	case SET_:						\
	    CHECKSETMEMBER(ELEM,NAME);				\
	    BINARY(SET_,AGGR->u.set | (1 << ELEM->u.num));	\
	    break;						\
	case STRING_:						\
	  { char *s;						\
	    s = (char *) malloc(strlen(AGGR->u.str) + 2);	\
	    s[0] = ELEM->u.num;					\
	    strcpy(++s,AGGR->u.str);				\
	    BINARY(STRING_,--s);				\
	    break; }						\
	default:						\
	    BADAGGREGATE(NAME); }				\
}
CONS_SWONS(cons_,"cons",stk,stk->next)
CONS_SWONS(swons_,"swons",stk->next,stk)

PRIVATE void drop_()
{   int n = stk->u.num;
    TWOPARAMS("drop");
    switch (stk->next->op)
      { case SET_:
	  { int i; int result = 0;
	    for (i = 0; i < SETSIZE; i++)
		if (stk->next->u.set & (1 << i))
		  { if (n < 1) result = result | (1 << i);
		    else n--; }
	    BINARY(SET_,result);
	    return; }
	case STRING_:
	  { char *result = stk->next->u.str;
	    while (n-- > 0  &&  *result != '\0') ++result;
	    BINARY(STRING_,result);
	    return; }
	case LIST_:
	  { Node *result = stk->next->u.lis;
	    while (n-- > 0 && result != NULL) result = result->next;
	    BINARY(LIST_,result);
	    return; }
	default:
	    BADAGGREGATE("drop"); }
}

PRIVATE void take_()
{   int n = stk->u.num;
    TWOPARAMS("take");
    switch (stk->next->op)
      { case SET_:
	  { int i; int result = 0;
	    for (i = 0; i < SETSIZE; i++)
		if (stk->next->u.set & (1 << i))
		  { if (n > 0)
		      { --n;  result = result | (1 << i); }
		    else break; }
	    BINARY(SET_,result);
	    return; }
	case STRING_:
	  { int i; char *old, *p, *result;
	    i = stk->u.num;
	    old = stk->next->u.str;
	    p = result = (char *) malloc(strlen(old) - i + 1);
	    while (i-- > 0)  *p++ = *old++;
	    BINARY(STRING_,result);
	    return; }
	case LIST_:
	  { int i = stk->u.num;
	    if (i < 1) { BINARY(LIST_,NULL); return; }
	    dump1 = newnode(LIST_,stk->next->u.lis,dump1);/* old  */
	    dump2 = newnode(LIST_,NULL, dump2);		  /* head */
	    dump3 = newnode(LIST_,NULL, dump3);		  /* last */
	    while (DMP1 != NULL && i-- > 0)
	      { if (DMP2 == NULL)			/* first */
		  { DMP2 = newnode(DMP1->op,DMP1->u.num,NULL);
		    DMP3 = DMP2; }
		else					/* further */
		  { Node *temp = newnode(DMP1->op,DMP1->u.num,NULL);
		    DMP3->next = temp;
		    DMP3 = DMP3->next; }
		DMP1 = DMP1->next; }
	    DMP3->next = NULL;
	    BINARY(LIST_,DMP2);
	    POP(dump1); POP(dump2); POP(dump3);
	    return; }
	default:
	    BADAGGREGATE("take"); }
}
PRIVATE void concat_()
{
    TWOPARAMS("concat");
    switch (stk->op)
      { case SET_:
	    BINARY(SET_,stk->next->u.set | stk->u.set);
	    return;
	case STRING_:
	  { char *s, *p;
	    s = p = (char *)malloc(strlen(stk->next->u.str) +
				   strlen(stk->u.str) + 1);
	    while ((*p++ = *(stk->next->u.str)++) != '\0');
	    --p; /* don't want terminating null */
	    while ((*p++ = *(stk->u.str)++) != '\0');
	    BINARY(STRING_,s);
	    return; }
	case LIST_:
	    if (stk->next->u.lis == NULL)
	      { BINARY(LIST_,stk->u.lis); return; }
	    dump1 = newnode(LIST_,stk->next->u.lis,dump1);/* old  */
	    dump2 = newnode(LIST_,NULL,dump2);		 /* head */
	    dump3 = newnode(LIST_,NULL,dump3);		 /* last */
	    while (DMP1 != NULL)
	      { if (DMP2 == NULL)			/* first */
		  { DMP2 =
			newnode(DMP1->op,
			    DMP1->u.num,NULL);
		    DMP3 = DMP2; }
		else					/* further */
		  { Node *temp =
			newnode(DMP1->op,
			    DMP1->u.num,NULL);
		    DMP3->next = temp;
		    DMP3 = DMP3->next; };
		DMP1 = DMP1->next; }
	    DMP3->next = stk->u.lis;
	    BINARY(LIST_,DMP2);
	    POP(dump1);
	    POP(dump2);
	    POP(dump3);
	    return;
	default:
	    BADAGGREGATE("concat"); };
}
PRIVATE void null_()
{
    ONEPARAM("null");
    UNARY(BOOLEAN_,
	stk->op == STRING_ ? *(stk->u.str) == '\0' : ! stk->u.num);
/*
    switch (stk->op)
      { case STRING_:
	    UNARY(BOOLEAN_, *(stk->u.str) == '\0');
	    break;
	default:
	    UNARY(BOOLEAN_, ! stk->u.num); }
*/
}
PRIVATE void nullval_()
{
    ONEPARAM("nullval");
    if (stk->op == STRING_)
	UNARY(STRING_,""); else
	UNARY(stk->op,0);
}
PRIVATE void not_()
{
    ONEPARAM("not");
    switch (stk->op)
      { case SET_:
	    UNARY(SET_,~ stk->u.set);
	    break;
	case STRING_:
	    UNARY(BOOLEAN_, *(stk->u.str) != '\0');
	    break;
	case BOOLEAN_: case CHAR_: case INTEGER_: case LIST_:
	    UNARY(BOOLEAN_, ! stk->u.num);
	    break;
	default:
	    BADDATA("not"); }
}
PRIVATE void size_()
{
    int siz = 0;
    ONEPARAM("size");
    switch (stk->op)
      { case SET_:
	  { int i;
	    for (i = 0; i < SETSIZE; i++)
		if (stk->u.set & (1 << i)) siz++;
	    break; }
	case STRING_:
	    siz = strlen(stk->u.str);
	    break;
	case LIST_:
	  { Node *e = stk->u.lis;
	    while (e != NULL) {e = e->next; siz++;};
	    break; }
	default :
	    BADDATA("size"); }
    UNARY(INTEGER_,siz);
}
PRIVATE void small_()
{
    int sml = 0;
    ONEPARAM("small");
    switch (stk->op)
      { case BOOLEAN_: case INTEGER_:
	    sml = stk->u.num < 2;
	    break;
	case SET_:
	    if (stk->u.set == 0) sml = 1; else
	      { int i = 0;
		while  (!(stk->u.set & (1 << i))) i++;
D(		printf("small: first member found is %d\n",i); )
		sml = (stk->u.set & ~(1 << i)) == 0; }
	    break;
	case STRING_:
	    sml = stk->u.str[0] == '\0' || stk->u.str[1] == '\0';
	    break;
	case LIST_:
	    sml = stk->u.lis == NULL || stk->u.lis->next == NULL;
	    break;
	default:
	    BADDATA("small"); }
    UNARY(BOOLEAN_,sml);
}
#define TYPE(PROCEDURE,NAME,REL,TYP)				\
    PRIVATE void PROCEDURE()					\
    {   ONEPARAM(NAME);						\
	UNARY(BOOLEAN_,stk->op REL TYP); }
TYPE(integer_,"integer",==,INTEGER_)
TYPE(char_,"char",==,CHAR_)
TYPE(logical_,"logical",==,BOOLEAN_)
TYPE(string_,"string",==,STRING_)
TYPE(set_,"set",==,SET_)
TYPE(list_,"list",==,LIST_)
TYPE(leaf_,"leaf",!=,LIST_)
TYPE(user_,"user",==,USR_)

#define USETOP(PROCEDURE,NAME,TYPE,BODY)			\
    PRIVATE void PROCEDURE()					\
    { ONEPARAM(NAME); TYPE(NAME); BODY; POP(stk); }
USETOP( put_,"put",ONEPARAM, writefactor(stk);printf(" "))
USETOP( putch_,"putch",NUMERICTYPE, printf("%c", (char) stk->u.num) )
USETOP( setecho_,"setecho",NUMERICTYPE, echoflag = stk->u.num )
USETOP( setautoput_,"setautoput",NUMERICTYPE, autoput = stk->u.num )
USETOP( settracegc_,"settracegc",NUMERICTYPE, tracegc = stk->u.num )
USETOP( include_,"include",STRING, doinclude(stk->u.str) )
USETOP( system_,"system",STRING, system(stk->u.str) )

PRIVATE void get_()
{
    getsym();
    readfactor();
}

PUBLIC void dummy_()
{
    /* never called */
}
#define HELP(PROCEDURE,REL)					\
PRIVATE void PROCEDURE()					\
{   Entry *i = symtabindex;					\
    while (i != symtab)						\
	if ((--i)->name[0] REL '_')				\
	    printf("%s ",i->name);				\
    printf("\n"); }
HELP(help1_,!=)
HELP(h_help1_,==)

/* - - - - -   C O M B I N A T O R S   - - - - - */

#ifdef TRACING
PRIVATE void writestack(Node *n)
{
    if (n) {
	writestack(n->next);
	if (n->next)
	    putchar(' ');
	writefactor(n);
    }
}
#endif

PUBLIC void exeterm(n)
    Node *n;
{
    Node *stepper;
start:
    if (n == NULL) return;
    conts = newnode(LIST_,n,conts);
    while (conts->u.lis != NULL)
      {
	if (tracegc > 5)
	  { printf("exeterm1: %ld ",(long)conts->u.lis);
	    printnode(conts->u.lis); }
	stepper = conts->u.lis;
#ifdef TRACING
	writestack(stk);
	printf(" : ");
	writeterm(stepper);
	putchar('\n');
#endif
	conts->u.lis = conts->u.lis->next;
	switch (stepper->op)
	  { case BOOLEAN_: case CHAR_: case INTEGER_:
	    case SET_: case STRING_: case LIST_:
		NULLARY(stepper->op,stepper->u.num); break;
	    case USR_:
		if (stepper->u.ent->u.body == NULL)
		    execerror("definition", stepper->u.ent->name);
		if (stepper->next == NULL)
		  { POP(conts);
		    n = stepper->u.ent->u.body;
		    goto start; }
		  else exeterm(stepper->u.ent->u.body );
		break;
	    case COPIED_: case ILLEGAL_:
		printf("exeterm: attempting to execute bad node\n");
		printnode(stepper);
		break;
	    default:
D(		printf("trying to do "); )
D(		writefactor(dump1); )
		(*(stepper->u.proc))(); break; }
	if (tracegc > 5)
	  { printf("exeterm2: %ld ",(long)stepper);
	    printnode(stepper); }
/*
	stepper = stepper->next; }
*/
		}
    POP(conts);
D(  printf("after execution, stk is:\n"); )
D(  writeterm(stk); )
D(  printf("\n"); )
}
PRIVATE void x_()
{
    ONEPARAM("x");
    ONEQUOTE("x");
    exeterm(stk->u.lis);
}
PRIVATE void i_()
{
    ONEPARAM("i");
    ONEQUOTE("i");
    SAVESTACK;
    POP(stk);
    exeterm(SAVED1->u.lis);
    POP(dump);
}
PRIVATE void dip_()
{
    TWOPARAMS("dip");
    ONEQUOTE("dip");
    SAVESTACK;
    stk = stk->next->next;
    exeterm(SAVED1->u.lis);
    NULLARY(SAVED2->op,SAVED2->u.num);
    POP(dump);
}
#define DIPPED(PROCEDURE,NAME,PARAMCOUNT,ARGUMENT)		\
PRIVATE void PROCEDURE()					\
{   PARAMCOUNT(NAME);						\
    SAVESTACK;							\
    POP(stk);							\
    ARGUMENT();							\
    NULLARY(SAVED1->op,SAVED1->u.num);				\
    POP(dump);							\
}
DIPPED(popd_,"popd",TWOPARAMS,pop_)
DIPPED(dupd_,"dupd",TWOPARAMS,dup_)
DIPPED(swapd_,"swapd",THREEPARAMS,swap_)
DIPPED(rolldownd_,"rolldownd",FOURPARAMS,rolldown_)
DIPPED(rollupd_,"rollupd",FOURPARAMS,rollup_)
DIPPED(rotated_,"rotated",FOURPARAMS,rotate_)

#define N_ARY(PROCEDURE,NAME,PARAMCOUNT,TOP)			\
PRIVATE void PROCEDURE()					\
{   PARAMCOUNT(NAME);						\
    ONEQUOTE(NAME);						\
    SAVESTACK;							\
    POP(stk);							\
    exeterm(SAVED1->u.lis);					\
    stk = newnode(stk->op,stk->u.num,TOP);			\
    POP(dump);							\
}
N_ARY(nullary_,"nullary",ONEPARAM,SAVED2)
N_ARY(unary_,"unary",TWOPARAMS,SAVED3)
N_ARY(binary_,"binary",THREEPARAMS,SAVED4)
N_ARY(ternary_,"ternary",FOURPARAMS,SAVED5)
/*
PRIVATE void nullary_()
{
    ONEPARAM("nullary");
    SAVESTACK;
    POP(stk);
    exeterm(SAVED1->u.lis);
    stk->next = SAVED2;
    POP(dump);
}
*/
PRIVATE void times_()
{
    int i,n;
    TWOPARAMS("times");
    ONEQUOTE("times");
    SAVESTACK;
    stk = stk->next->next;
    n = SAVED2->u.num;
    for (i = 1; i <= n; i++)
	exeterm(SAVED1->u.lis);
    POP(dump);
}
PRIVATE void infra_()
{
    TWOPARAMS("infra");
    ONEQUOTE("infra");
    LIST2("infra");
    SAVESTACK;
    stk = SAVED2->u.lis;
    exeterm(SAVED1->u.lis);
    stk = newnode(LIST_,stk,SAVED3);
    POP(dump);
}
PRIVATE void app1_()
{
    TWOPARAMS("app1");
    ONEQUOTE("app1");
    SAVESTACK;
    POP(stk);
    exeterm(SAVED1->u.lis);
    POP(dump);
}
PRIVATE void cleave_()
{			/*  X [P1] [P2] cleave ==>  X1 X2	*/
    THREEPARAMS("cleave");
    TWOQUOTES("cleave");
    SAVESTACK;
    stk = SAVED3;
    exeterm(SAVED2->u.lis);			/* [P1]		*/
    dump1 = newnode(stk->op,stk->u.num,dump1);	/*  X1		*/
    stk = SAVED3;
    exeterm(SAVED1->u.lis);			/* [P2]		*/
    dump1 = newnode(stk->op,stk->u.num,dump1);	/*  X2		*/
    stk = dump1; dump1 = dump1->next->next; stk->next->next = SAVED4;
    POP(dump);
}
PRIVATE void app11_()
{
    THREEPARAMS("app11");
    ONEQUOTE("app11");
    app1_();
    stk->next = stk->next->next;
}
PRIVATE void app2_()
{			/*   Y  Z  [P]  app2     ==>  Y'  Z'  */
    THREEPARAMS("app2");
    ONEQUOTE("app2");
    SAVESTACK;
    stk = SAVED2->next;				/* just Y on top */
    exeterm(SAVED1->u.lis);			/* execute P */
    dump1 = newnode(stk->op,stk->u.num,dump1);	/* save P(Y) */
    stk = newnode(SAVED2->op,SAVED2->u.num,
	  SAVED3->next);			/* just Z on top */
    exeterm(SAVED1->u.lis);			/* execute P */
    dump1 = newnode(stk->op,stk->u.num,dump1);	/* save P(Z) */
    stk = dump1; dump1 = dump1->next->next; stk->next->next = SAVED4;
    POP(dump);
}
PRIVATE void app3_()
{			/*  X Y Z [P]  app3    ==>  X' Y' Z'	*/
    FOURPARAMS("app3");
    ONEQUOTE("app3");
    SAVESTACK;
    stk = SAVED3->next;				/* just X on top */
    exeterm(SAVED1->u.lis);			/* execute P */
    dump1 = newnode(stk->op,stk->u.num,dump1);	/* save p(X) */
    stk = newnode(SAVED3->op,SAVED3->u.num,
	  SAVED4->next);			/* just Y on top */
    exeterm(SAVED1->u.lis);			/* execute P */
    dump1 = newnode(stk->op,stk->u.num,dump1);	/* save P(Y) */
    stk = newnode(SAVED2->op,SAVED2->u.num,
	  SAVED4->next);			/* just Z on top */
    exeterm(SAVED1->u.lis);			/* execute P */
    dump1 = newnode(stk->op,stk->u.num,dump1);	/* save P(Z) */
    stk = dump1; dump1 = dump1->next->next->next;
    stk->next->next->next = SAVED5;
    POP(dump);
}
PRIVATE void app4_()
{			/*  X Y Z W [P]  app4    ==>  X' Y' Z' W'	*/
    FIVEPARAMS("app4");
    ONEQUOTE("app4");
    SAVESTACK;
    stk = SAVED4->next;				/* just X on top */
    exeterm(SAVED1->u.lis);			/* execute P */
    dump1 = newnode(stk->op,stk->u.num,dump1);	/* save p(X) */
    stk = newnode(SAVED4->op,SAVED4->u.num,
	  SAVED5->next);			/* just Y on top */
    exeterm(SAVED1->u.lis);			/* execute P */
    dump1 = newnode(stk->op,stk->u.num,dump1);	/* save P(Y) */
    stk = newnode(SAVED3->op,SAVED3->u.num,
	  SAVED5->next);			/* just Z on top */
    exeterm(SAVED1->u.lis);			/* execute P */
    dump1 = newnode(stk->op,stk->u.num,dump1);	/* save P(Z) */
    stk = newnode(SAVED2->op,SAVED2->u.num,
	  SAVED4->next);			/* just W on top */
    exeterm(SAVED1->u.lis);			/* execute P */
    dump1 = newnode(stk->op,stk->u.num,dump1);	/* save P(W) */
    stk = dump1; dump1 = dump1->next->next->next->next;
    stk->next->next->next->next = SAVED6;
    POP(dump);
}
PRIVATE void app12_()
{
    /*   X  Y  Z  [P]  app12  */
    THREEPARAMS("app12");
    app2_();
    stk->next->next = stk->next->next->next;	/* delete X */
}
PRIVATE void map_()
{
    TWOPARAMS("map");
    ONEQUOTE("map");
    SAVESTACK;
    switch(SAVED2->op)
      { case LIST_:
	  { dump1 = newnode(LIST_,SAVED2->u.lis,dump1);	/* step old */
	    dump2 = newnode(LIST_,NULL,dump2);		/* head new */
	    dump3 = newnode(LIST_,NULL,dump3);		/* last new */
	    while (DMP1 != NULL)
	      { stk = newnode(DMP1->op,
			      DMP1->u.num,SAVED3);
		exeterm(SAVED1->u.lis);
D(		printf("map: "); writefactor(stk); printf("\n"); )
		if (DMP2 == NULL)			/* first */
		  { DMP2 =
			newnode(stk->op,stk->u.num,NULL);
		    DMP3 = DMP2; }
		else					/* further */
		  { Node *temp =
			newnode(stk->op,stk->u.num,NULL);
		    DMP3->next = temp;
		    DMP3 = DMP3->next; }
		DMP1 = DMP1->next; }
	    stk = newnode(LIST_,DMP2,SAVED3);
	    POP(dump3);
	    POP(dump2);
	    POP(dump1);
	    break; }
	case STRING_:
	  { char *s, *resultstring; int j = 0;
	    resultstring =
		(char *) malloc(strlen(SAVED2->u.str) + 1);
	    for (s = SAVED2->u.str; *s != '\0'; s++)
	      { stk = newnode(CHAR_,*s,SAVED3);
		exeterm(SAVED1->u.lis);
		resultstring[j++] = stk->u.num; }
	    stk = newnode(STRING_,resultstring,SAVED3);
	    break; }
	case SET_:
	  { int i; long resultset = 0;
	    for (i = 0; i < SETSIZE; i++)
		if (SAVED2->u.set & (1 << i))
		  { stk = newnode(INTEGER_,i,SAVED3);
		    exeterm(SAVED1->u.lis);
		    resultset = resultset | (1 << stk->u.num); }
	    stk = newnode(SET_,resultset,SAVED3);
	    break; }
	default:
	    BADAGGREGATE("map"); }
    POP(dump);
}
PRIVATE void step_()
{
    TWOPARAMS("step");
    ONEQUOTE("step");
    SAVESTACK;
    stk = stk->next->next;
    switch(SAVED2->op)
      { case LIST_:
	  { dump1 = newnode(LIST_,SAVED2->u.lis,dump1);
	    while (DMP1 != NULL)
	      { NULLARY(DMP1->op,DMP1->u.num);
		exeterm(SAVED1->u.lis);
		DMP1 = DMP1->next; }
	    POP(dump1);
	    break; }
	case STRING_:
	  { char *s;
	    for (s = SAVED2->u.str; *s != '\0'; s++)
	      { stk = newnode(CHAR_, *s,stk);
		exeterm(SAVED1->u.lis); }
	    break; }
	case SET_:
	  { int i;
	    for (i = 0; i < SETSIZE; i++)
		if (SAVED2->u.set & (1 << i))
		  { stk = newnode(INTEGER_,i,stk);
		    exeterm(SAVED1->u.lis); }
	    break; }
	default:
	    BADAGGREGATE("step"); }
    POP(dump);
}
PRIVATE void fold_()
{
    swapd_(); step_();
}
PRIVATE void cond_()
{
    int result = 0;
    ONEPARAM("cond");
				/* must check for QUOTES in list */
    LIST("cond");
    CHECKEMPTYLIST(stk->u.lis,"cond");
    SAVESTACK;
    dump1 = newnode(LIST_,stk->u.lis,dump1);
    while ( result == 0 &&
	    DMP1 != NULL &&
	    DMP1->next != NULL )
      { stk = SAVED2;
	exeterm(DMP1->u.lis->u.lis);
	result = stk->u.num;
	if (!result) DMP1 = DMP1->next; }
    stk = SAVED2;
    if (result) exeterm(DMP1->u.lis->next);
	else exeterm(DMP1->u.lis); /* default */
    POP(dump1);
    POP(dump);
}
#define IF_TYPE(PROCEDURE,NAME,TYP)				\
    PRIVATE void PROCEDURE()					\
    {   TWOPARAMS(NAME);					\
	TWOQUOTES(NAME);					\
	SAVESTACK;						\
	stk = SAVED3;						\
	exeterm(stk->op == TYP ? SAVED2->u.lis : SAVED1->u.lis);\
	POP(dump); }
IF_TYPE(ifinteger_,"ifinteger",INTEGER_)
IF_TYPE(ifchar_,"ifchar",CHAR_)
IF_TYPE(iflogical_,"iflogical",BOOLEAN_)
IF_TYPE(ifstring_,"ifstring",STRING_)
IF_TYPE(ifset_,"ifset",SET_)
IF_TYPE(iflist_,"iflist",LIST_)
PRIVATE void filter_()
{
    TWOPARAMS("filter");
    ONEQUOTE("filter");
    SAVESTACK;
    switch (SAVED2->op)
      { case SET_ :
	  { int j; long resultset = 0;
	    for (j = 0; j < SETSIZE; j++)
	      { if (SAVED2->u.set & (1 << j))
		  { stk = newnode(INTEGER_,j,SAVED3);
		    exeterm(SAVED1->u.lis);
		    if (stk->u.num)
			resultset = resultset | (1 << j); } }
	    stk = newnode(SET_,resultset,SAVED3);
	    break; }
	case STRING_ :
	  { char *s, *resultstring; int j = 0;
	    resultstring =
		(char *) malloc(strlen(SAVED2->u.str) + 1);
	    for (s = SAVED2->u.str; *s != '\0'; s++)
	      { stk = newnode(CHAR_, *s, SAVED3);
		exeterm(SAVED1->u.lis);
		if (stk->u.num) resultstring[j++] = *s; }
	    resultstring[j] = '\0';
	    stk = newnode(STRING_,resultstring,SAVED3);
	    break; }
	case LIST_:
	  { dump1 = newnode(LIST_,SAVED2->u.lis,dump1);	/* step old */
	    dump2 = newnode(LIST_,NULL,dump2);		/* head new */
	    dump3 = newnode(LIST_,NULL,dump3);		/* last new */
	    while (DMP1 != NULL)
	      { stk = newnode(DMP1->op,DMP1->u.num,SAVED3);
		exeterm(SAVED1->u.lis);
D(		printf("filter: "); writefactor(stk); printf("\n"); )
		if (stk->u.num)	{			/* test */
		    if (DMP2 == NULL)		/* first */
		      { DMP2 =
			    newnode(DMP1->op,
				DMP1->u.num,NULL);
			DMP3 = DMP2; }
		    else {				/* further */
		      { Node *temp =
			    newnode(DMP1->op,
				DMP1->u.num,NULL);
			DMP3->next = temp;
			DMP3 = DMP3->next; } } }
		DMP1 = DMP1->next; }
	    stk = newnode(LIST_,DMP2,SAVED3);
	    POP(dump3);
	    POP(dump2);
	    POP(dump1);
	    break; }
	default :
	    BADAGGREGATE("filter"); }
    POP(dump);
}
PRIVATE void split_()
{
    TWOPARAMS("split");
    SAVESTACK;
    switch (SAVED2->op)
      { case SET_ :
	  { int j; long yes_set = 0, no_set = 0;
	    for (j = 0; j < SETSIZE; j++)
	      { if (SAVED2->u.set & (1 << j))
		  { stk = newnode(INTEGER_,j,SAVED3);
		    exeterm(SAVED1->u.lis);
		    if (stk->u.num)
			  yes_set = yes_set | (1 << j);
		    else  no_set = no_set | (1 << j); } }
	    stk = newnode(SET_,yes_set,SAVED3);
	    NULLARY(SET_,no_set);
	    break; }
	case STRING_ :
	  { char *s, *yesstring, *nostring; int yesptr = 0, noptr = 0;
	    yesstring =
		(char *) malloc(strlen(SAVED2->u.str) + 1);
	    nostring =
		(char *) malloc(strlen(SAVED2->u.str) + 1);
	    for (s = SAVED2->u.str; *s != '\0'; s++)
	      { stk = newnode(CHAR_, *s, SAVED3);
		exeterm(SAVED1->u.lis);
		if (stk->u.num) yesstring[yesptr++] = *s;
			   else nostring[noptr++] = *s; }
	    yesstring[yesptr] = '\0'; nostring[noptr] = '\0';
	    stk = newnode(STRING_,yesstring,SAVED3);
	    NULLARY(STRING_,nostring);
	    break; }
	case LIST_:
	  { dump1 = newnode(LIST_,SAVED2->u.lis,dump1);	/* step old */
	    dump2 = newnode(LIST_,NULL,dump2);		/* head true */
	    dump3 = newnode(LIST_,NULL,dump3);		/* last true */
	    dump4 = newnode(LIST_,NULL,dump4);		/* head false */
	    dump5 = newnode(LIST_,NULL,dump5);		/* last false */
	    while (DMP1 != NULL)
	      { stk = newnode(DMP1->op,DMP1->u.num,SAVED3);
		exeterm(SAVED1->u.lis);
D(		printf("split: "); writefactor(stk); printf("\n"); )
		if (stk->u.num)				/* pass */
		    if (DMP2 == NULL)		/* first */
		      { DMP2 =
			    newnode(DMP1->op,
				DMP1->u.num,NULL);
			DMP3 = DMP2; }
		    else				/* further */
		      { Node *temp =
			    newnode(DMP1->op,
				DMP1->u.num,NULL);
			DMP3->next = temp;
			DMP3 = DMP3->next; }
		else					/* fail */
		    if (DMP4 == NULL)		/* first */
		      { DMP4 =
			    newnode(DMP1->op,
				DMP1->u.num,NULL);
			DMP5 = DMP4; }
		    else				/* further */
		      { Node *temp =
			    newnode(DMP1->op,
				DMP1->u.num,NULL);
			DMP5->next = temp;
			DMP5 = DMP5->next; }
		DMP1 = DMP1->next; }
	    stk = newnode(LIST_,DMP2,SAVED3);
	    NULLARY(LIST_,DMP4);
	    POP(dump5);
	    POP(dump4);
	    POP(dump3);
	    POP(dump2);
	    POP(dump1);
	    break; }
	default :
	    BADAGGREGATE("split"); }
    POP(dump);
}
#define SOMEALL(PROCEDURE,NAME,INITIAL)				\
PRIVATE void PROCEDURE()					\
{   int result = INITIAL;					\
    TWOPARAMS(NAME);						\
    SAVESTACK;							\
    switch (SAVED2->op)						\
      { case SET_ :						\
	  { int j;						\
	    for (j = 0; j < SETSIZE && result == INITIAL; j++)	\
	      { if (SAVED2->u.set & (1 << j))			\
		  { stk = newnode(INTEGER_,j,SAVED3);		\
		    exeterm(SAVED1->u.lis);			\
		    if (stk->u.num != INITIAL)			\
			result = 1 - INITIAL; } }		\
	    break; }						\
	case STRING_ :						\
	  { char *s;						\
	    for (s = SAVED2->u.str;				\
		 *s != '\0' && result == INITIAL; s++)		\
	      { stk = newnode(CHAR_,*s,SAVED3);			\
		exeterm(SAVED1->u.lis);				\
		if (stk->u.num != INITIAL)			\
		    result = 1 - INITIAL; }			\
	    break; }						\
	case LIST_ :						\
	  { dump1 = newnode(LIST_,SAVED2->u.lis,dump1);		\
	    while (DMP1 != NULL && result == INITIAL)		\
	      { stk = newnode(DMP1->op,				\
			DMP1->u.num,SAVED3);			\
		exeterm(SAVED1->u.lis);				\
		if (stk->u.num != INITIAL)			\
		     result = 1 - INITIAL; 			\
		DMP1 = DMP1->next; }				\
	    POP(dump1);						\
	    break; }						\
	default :						\
	    BADAGGREGATE(NAME); }				\
    stk = newnode(BOOLEAN_,result,SAVED3);			\
    POP(dump);							\
}
SOMEALL(some_,"some",0)
SOMEALL(all_,"all",1)

PRIVATE void primrec_()
{
    int n = 0; int i;
    THREEPARAMS("primrec");
    SAVESTACK;
    stk = stk->next->next->next;
    switch (SAVED3->op)
      { case LIST_:
	  { Node *current = SAVED3->u.lis;
	    while (current != NULL)
	      { stk = newnode(current->op,current->u.num,stk);
		current = current->next;
		n++; }
	    break; }
	case STRING_:
	  { char *s;
	    for (s = SAVED3->u.str; *s != '\0'; s++)
	      { stk = newnode(CHAR_, *s, stk);
		n++; }
	    break; }
	case SET_:
	  { int j; long set = SAVED3->u.set;
	    for (j = 0; j < SETSIZE; j++)
		if (set & (1 << j))
		  { stk = newnode(INTEGER_,j,stk);
		    n++; }
	    break; }
	case INTEGER_:
	  { int j;
	    for (j = SAVED3->u.num; j > 0; j--)
	      { stk = newnode(INTEGER_,j, stk);
		n++; }
	    break; }
	default:
	    BADDATA("primrec"); }
    exeterm(SAVED2->u.lis);
    for (i = 1; i <= n; i++)
	exeterm(SAVED1->u.lis);
    POP(dump);
}
PRIVATE void tailrecaux()
{
    int result;
  tailrec:
    dump1 = newnode(LIST_,stk,dump1);
    exeterm(SAVED3->u.lis);
    result = stk->u.num;
    stk = DMP1; POP(dump1);
    if (result) exeterm(SAVED2->u.lis); else
      { exeterm(SAVED1->u.lis);
	goto tailrec; }  /* tail recursion optimisation */
}
PRIVATE void tailrec_()
{
    THREEPARAMS("tailrec");
    SAVESTACK;
    stk = SAVED4;
    tailrecaux();
    POP(dump);
}
PRIVATE void construct_()
{			/* [P] [[P1] [P2] ..] -> X1 X2 ..	*/
    TWOPARAMS("construct");
    TWOQUOTES("construct");
    SAVESTACK;
    stk = SAVED3;			/* pop progs		*/
    dump1 = newnode(LIST_,dump2,dump1);	/* save dump2		*/
    dump2 = stk;			/* save old stack	*/
    exeterm(SAVED2->u.lis);		/* [P]			*/
    dump3 = newnode(LIST_,stk,dump3);	/* save current stack	*/
    dump4 = newnode(LIST_,SAVED1->u.lis,dump4);	/* step [..]	*/
    while (DMP4 != NULL)
      { stk = DMP3;			/* restore new stack	*/
	exeterm(DMP4->u.lis);
	dump2 = newnode(stk->op,stk->u.num,dump2); /* result	*/
	DMP4 = DMP4->next; }
    POP(dump4);
    POP(dump3);
    stk = dump2; dump2 = dump1->u.lis;	/* restore dump2	*/
    POP(dump1);
    POP(dump);
}
PRIVATE void branch_()
{
    THREEPARAMS("branch");
    TWOQUOTES("branch");
    SAVESTACK;
    stk = SAVED4;
    exeterm(SAVED3->u.num ? SAVED2->u.lis : SAVED1->u.lis);
    POP(dump);
}
PRIVATE void while_()
{
    TWOPARAMS("while");
    TWOQUOTES("while");
    SAVESTACK;
    do
      { stk = SAVED3;
	exeterm(SAVED2->u.lis);	/* TEST */
	if (! stk->u.num) break;
	stk = SAVED3;
	exeterm(SAVED1->u.lis);		/* DO */
	SAVED3 = stk; }
	while (1);
    stk = SAVED3;
    POP(dump);
}
PRIVATE void ifte_()
{
    int result;
    THREEPARAMS("ifte");
    THREEQUOTES("ifte");
    SAVESTACK;
    stk = SAVED4;
    exeterm(SAVED3->u.lis);
    result = stk->u.num;
    stk = SAVED4;
    exeterm(result ? SAVED2->u.lis : SAVED1->u.lis);
    POP(dump);
}
PRIVATE void condlinrecaux()
{
    int result = 0;
    dump1 = newnode(LIST_,SAVED1->u.lis,dump1);
    dump2 = newnode(LIST_,stk,dump2);
    while ( result == 0 &&
	    DMP1 != NULL && DMP1->next != NULL )
      { stk = DMP2;
	exeterm(DMP1->u.lis->u.lis);
	result = stk->u.num;
	if (!result) DMP1 = DMP1->next; }
    stk = DMP2;
    if (result)
      { exeterm(DMP1->u.lis->next->u.lis);
	if (DMP1->u.lis->next->next != NULL)
	  { condlinrecaux();
	    exeterm(DMP1->u.lis->next->next->u.lis); } }
    else
      { exeterm(DMP1->u.lis->u.lis);
	if (DMP1->u.lis->next != NULL)
	  { condlinrecaux();
	    exeterm(DMP1->u.lis->next->u.lis); } }
    POP(dump2);
    POP(dump1);
}
PRIVATE void condlinrec_()
{
    ONEPARAM("condlinrec");
    LIST("condlinrec");
    CHECKEMPTYLIST(stk->u.lis,"condlinrec");
    SAVESTACK;
    stk = SAVED2;
    condlinrecaux();
    POP(dump);
}
PRIVATE void linrecaux()
{
    int result;
    dump1 = newnode(LIST_,stk,dump1);
    exeterm(SAVED4->u.lis);
    result = stk->u.num;
    stk = DMP1; POP(dump1);
    if (result) exeterm(SAVED3->u.lis); else
      { exeterm(SAVED2->u.lis);
	linrecaux();
	exeterm(SAVED1->u.lis); }
}
PRIVATE void linrec_()
{
    FOURPARAMS("linrec");
    FOURQUOTES("linrec");
    SAVESTACK;
    stk = SAVED5;
    linrecaux();
    POP(dump);
}
PRIVATE void reclinaux()
{
    int result;
    dump1 = newnode(LIST_,stk,dump1);
    exeterm(SAVED4->u.lis);
    result = stk->u.num;
    stk = DMP1; POP(dump1);
    if (result) exeterm(SAVED2->u.lis); else
      { exeterm(SAVED3->u.lis);
	reclinaux();
	exeterm(SAVED1->u.lis); }
}
PRIVATE void reclin_()
{
    FOURPARAMS("reclin");
    FOURQUOTES("reclin");
    SAVESTACK;
    stk = SAVED5;
    reclinaux();
    POP(dump);
}
PRIVATE void binrecaux()
{
    int result;
    dump1 = newnode(LIST_,stk,dump1);
    exeterm(SAVED4->u.lis);
    result = stk->u.num;
    stk = DMP1; POP(dump1);
    if (result) exeterm(SAVED3->u.lis); else
      { exeterm(SAVED2->u.lis);		/* split */
	dump2 = newnode(stk->op,stk->u.num,dump2);
	POP(stk);
	binrecaux();			/* first */
	NULLARY(dump2->op,dump2->u.num);
	POP(dump2);
	binrecaux();			/* second */
	exeterm(SAVED1->u.lis); }	/* combine */
}
PRIVATE void binrec_()
{
    FOURPARAMS("binrec");
    FOURQUOTES("binrec");
    SAVESTACK;
    stk = SAVED5;
    binrecaux();
    POP(dump);
}
PRIVATE void treestepaux(item)
    Node *item;
{
    if (item->op != LIST_)
      { NULLARY(item->op,item->u.num);
	exeterm(SAVED1->u.lis); }
    else
      { dump1 = newnode(LIST_,item->u.lis,dump1);
	while (DMP1 != NULL)
	  { treestepaux(DMP1);
	    DMP1 = DMP1->next; }
	POP(dump1); }
}
PRIVATE void treestep_()
{
    TWOPARAMS("treestep");
    ONEQUOTE("treestep");
    SAVESTACK;
    stk = SAVED3;
    treestepaux(SAVED2);
    POP(dump);
}
PRIVATE void treerecaux()
{
    Node *temp;
    if (stk->next->op == LIST_)
      { temp = newnode(ANON_FUNCT_,treerecaux,NULL);
	NULLARY(LIST_,temp);
	cons_();		/*  D  [[[O] C] ANON_FUNCT_]	*/
D(	printf("treerecaux: stack = "); )
D(	writeterm(stk); printf("\n"); )
	exeterm(stk->u.lis->u.lis->next); }
    else
      { temp = stk;
	POP(stk);
	exeterm(temp->u.lis->u.lis); }
}
PRIVATE void treerec_()
{
    THREEPARAMS("treerec");
    cons_();
D(  printf("deep: stack = "); writeterm(stk); printf("\n"); )
    treerecaux();
}
PRIVATE void genrecaux()
{
    Node *temp;
    int result;
D(  printf("genrecaux: stack = "); )
D(  writeterm(stk); printf("\n"); )
    SAVESTACK;
    POP(stk);
    exeterm(SAVED1->u.lis->u.lis);		/*	[I]	*/
    result = stk->u.num;
    stk = SAVED2;
    if (result)
	exeterm(SAVED1->u.lis->next->u.lis);	/*	[T]	*/
    else
      { exeterm(SAVED1->u.lis->next->next->u.lis); /*	[R1]	*/
	NULLARY(SAVED1->op,SAVED1->u.lis);
	temp = newnode(ANON_FUNCT_,genrecaux,NULL);
	NULLARY(LIST_,temp);
	cons_();
	exeterm(SAVED1->u.lis->next->next->next); } /*   [R2]	*/
    POP(dump);
}
PRIVATE void genrec_()
{
    FOURPARAMS("genrec");
    FOURQUOTES("genrec");
    cons_(); cons_(); cons_();
    genrecaux();
}
PRIVATE void treegenrecaux()
{
    Node *temp;
D(  printf("treegenrecaux: stack = "); )
D(  writeterm(stk); printf("\n"); )
    if (stk->next->op == LIST_)
      { SAVESTACK;				/* begin DIP	*/
	POP(stk);
	exeterm(SAVED1->u.lis->next->u.lis);	/*	[O2]	*/
	NULLARY(SAVED1->op,SAVED1->u.num);
	POP(dump);				/*   end DIP	*/
	temp = newnode(ANON_FUNCT_,treegenrecaux,NULL);
	NULLARY(LIST_,temp);
	cons_();
	exeterm(stk->u.lis->u.lis->next->next); } /*	[C]	*/
    else
      { temp = stk;
	POP(stk);
	exeterm(temp->u.lis->u.lis); }		/*	[O1]	*/
}
PRIVATE void treegenrec_()
{					/* T [O1] [O2] [C]	*/
    FOURPARAMS("treegenrec");
    cons_(); cons_();
D(  printf("treegenrec: stack = "); writeterm(stk); printf("\n"); )
    treegenrecaux();
}

PRIVATE void o_online_manual_()
{
    make_manual(0);
}
PRIVATE void l_latex_manual_()
{
    make_manual(1);
}
/* - - - - -   I N I T I A L I S A T I O N   - - - - - */

static struct {char *name; void (*proc) (); char *messg1, *messg2;}
    optable[] =
	/* THESE MUST BE DEFINED IN THE ORDER OF THEIR VALUES */
{

{"__ILLEGAL",		dummy_,		"->",
"internal error, cannot happen - supposedly."},

{"__COPIED",		dummy_,		"->",
"no message ever, used for gc."},

{"__USR",		dummy_,		"usg",
"user node."},

{"__ANON_FUNCT",	dummy_,		"->",
"op for anonymous function call."},

/* LITERALS */

{" truth value type",	dummy_,		"->  B",
"The logical type, or the type of truth values. It has just two literals: true and false."},

{" character type",	dummy_,		"->  C",
"The type of characters. Literals are written with a single quote. Examples:  'A  '7  ';  and so on. Unix style escapes are allowed."},

{" integer type",	dummy_,		"->  I",
"The type of negative, zero or positive integers. Literals are written in decimal notation. Examples:  -123   0   42."},

{" set type",		dummy_,		"->  {...}",
"The type of sets of small non-negative integers. The maximum is platform dependent, typically the range is 0..31. Literals are written inside curly braces. Examples:  {}  {0}  {1 3 5}  {19 18 17}."},

{" string type",	dummy_,		"->  \"...\" ",
"The type of strings of characters. Literals are written inside double quotes. Examples: \"\"  \"A\"  \"hello world\" \"123\". Unix style escapes are accepted."},

{" list type",		dummy_,		"->  [...]",
"The type of lists of values of any type (including lists), or the type of quoted programs which may contain operators or combinators. Literals of this type are written inside square brackets. Examples: []  [3 512 -7]  [john mary]  ['A 'C ['B]]  [dup *]."},

/* OPERANDS */

{"false",		false_,		"->  false",
"Pushes the value false."},

{"true",		true_,		"->  true",
"Pushes the value true."},

{"maxint",		maxint_,	"->  maxint",
"Pushes largest integer (platform dependent). Typically it is 32 bits."},

{"setsize",		setsize_,	"->  setsize",
"Pushes the maximum number of elements in a set (platform dependent). Typically it is 32, and set members are in the range 0..31."},

{"stack",		stack_,		".. X Y Z  ->  .. X Y Z [Z Y X ..]",
"Pushes the stack as a list."},

{"__symtabmax",		symtabmax_,	"->",
"Pushes value of maximum size of the symbol table."},

{"__symtabindex",	symtabindex_,	"->",
"Pushes current size of the symbol table."},

{"__dump",		dump_,		"->",
"debugging only: pushes the dump as a list."},

{"conts",		conts_,		"->  [[P] [Q] ..]",
"Pushes current continuations."},

{"autoput",		autoput_,	"->  I",
"Pushes current value of flag  for automatic output, I = 0..2."},

{"echo",		echo_,		"->  I",
"Pushes value of echo flag, I = 0..3."},

{"clock",		clock_,		"->  I",
"Pushes the integer value of current CPU usage in hundreds of a second."},

{"__memorymax",		memorymax_,	"->",
"Pushes value of total size of memory."},

/* OPERATORS */

{"id",			id_,		"->",
"Identity function, does nothing. Any program of the form  P id Q  is equivalent to just  P Q."},

{"dup",			dup_,		" X  ->   X X",
"Pushes an extra copy of X onto stack."},

{"swap",		swap_,		" X Y  ->   Y X",
"Interchanges X and Y on top of the stack."},

{"rollup",		rollup_,	"X Y Z  ->  Z X Y",
"Moves X and Y up, moves Z down"},

{"rolldown",		rolldown_,	"X Y Z  ->  Y Z X",
"Moves Y and Z down, moves X up"},

{"rotate",		rotate_,	"X Y Z  ->  Z Y X",
"Interchanges X and Z"},

{"pop",			pop_,		" X  ->",
"Removes X from top of the stack."},

{"choice",		choice_,	"B T F  ->  X",
"If B is true, then X = T else X = F."},

{"or",			or_,		"X Y  ->  Z",
"Z is the union of sets X and Y, logical disjunction for truth values."},

{"xor",			xor_,		"X Y  ->  Z",
"Z is the symmetric difference of sets X and Y, logical exclusive disjunction for truth values."},

{"and",			and_,		"X Y  ->  Z",
"Z is the intersection of sets X and Y, logical conjunction for truth values."},

{"not",			not_,		"X  ->  Y",
"Y is the complement of set X, logical negation for truth values."},

{"+",			plus_,		"M I  ->  N",
"Numeric N is the result of adding integer I to numeric M."},

{"-",			minus_,		"M I  ->  N",
"Numeric N is the result of subtracting integer I from numeric M."},

{"*",			mul_,		"I J  ->  K",
"Integer K is the product of integers I and J."},

{"/",			divide_,	"I J  ->  K",
"Integer K is the (rounded) ratio of integers I and J."},

{"rem",			rem_,		"I J  ->  K",
"Integer K is the remainder of dividing I by J."},

{"sign",		sign_,		"I  ->  J",
"Integer J is the sign (-1 or 0 or +1) of integer I."},

{"abs",			abs_,		"I  ->  J",
"Integer J is the absolute value (0,1,2..) of integer I."},

{"pred",		pred_,		"M  ->  N",
"Numeric N is the predecessor of numeric M."},

{"succ",		succ_,		"M  ->  N",
"Numeric N is the successor of numeric M."},

{"max",			max_,		"N1 N2  ->  N",
"N is the maximum of numeric values N1 and N2."},

{"min",			min_,		"N1 N2  ->  N",
"N is the minimum of numeric values N1 and N2."},

{"unstack",		unstack_,	"[X Y ..]  ->  ..Y X",
"The list [X Y ..] becomes the new stack."},

{"cons",		cons_,		"X A  ->  B",
"Aggregate B is A with a new member X (first member for sequences)."},

{"swons",		swons_,		"A X  ->  B",
"Aggregate B is A with a new member X (first member for sequences)."},

{"first",		first_,		"A  ->  F",
"F is the first member of the non-empty aggregate A."},

{"rest",		rest_,		"A  ->  R",
"R is the non-empty aggregate A with its first member removed."},

{"compare",		compare_,	"A B  ->  I",
"I (=-1,0,+1) is the comparison of aggregates A and B.  The values correspond to the predicates <=, =, >=."},

{"at",			at_,		"A I  ->  X",
"X (= A[I]) is the member of A at position I."},

{"of",			of_,		"I A  ->  X",
"X (= A[I]) is the I-th member of aggregate A."},

{"size",		size_,		"A  ->  I",
"Integer I is the number of elements of aggregate A."},

{"opcase",		opcase_,	"X [..[X Xs]..]  ->  [Xs]",
"Indexing on type of X, returns the list [Xs]."},

{"uncons",		uncons_,	"A  ->  F R",
"F and R are the first and the rest of non-empty aggregate A."},

{"unswons",		unswons_,	"A  ->  R F",
"R and F are the rest and the first of non-empty aggregate A."},

{"drop",		drop_,		"A N  ->  B",
"Aggregate B is the result of deleting the first N elements of A."},

{"take",		take_,		"A N  ->  B",
"Aggregate B is the result of retaining just the first N elements of A."},

{"concat",		concat_,	"S T  ->  U",
"Sequence U is the concatenation of sequences S and T."},

{"name",		name_,		"sym  ->  \"sym\"",
"For operators and combinators, the string \"sym\" is the name of item sym, for literals sym the result string is its type."},

{"body",		body_,		"U  ->  [P]",
"Quotation [P] is the body of user-defined symbol U."},

{"nullval",		nullval_,	"A  ->  A0",
"A0 is the empty aggregate of the type of A, or the 0-value of numerics."},

/* PREDICATES */

{"null",		null_,		"X  ->  B",
"Tests for empty aggregate X or zero numeric."},

{"small",		small_,		"X  ->  B",
"Tests whether aggregate X has 0 or 1 members, or numeric 0 or 1."},

{">=",			geql_,		"X Y  ->  B",
"Either both X and Y are numeric or both are strings or symbols. Tests whether X greater than or equal to Y."},

{">",			greater_,	"X Y  ->  B",
"Either both X and Y are numeric or both are strings or symbols. Tests whether X greater than Y."},

{"<=",			leql_,		"X Y  ->  B",
"Either both X and Y are numeric or both are strings or symbols. Tests whether X less than or equal to Y."},

{"<",			less_,		"X Y  ->  B",
"Either both X and Y are numeric or both are strings or symbols. Tests whether X less than Y."},

{"!=",			neql_,		"X Y  ->  B",
"Either both X and Y are numeric or both are strings or symbols. Tests whether X not equal to Y."},

{"=",			eql_,		"X Y  ->  B",
"Either both X and Y are numeric or both are strings or symbols. Tests whether X equal to Y."},

{"equal",		equal_,		"T U  ->  B",
"(Recursively) tests whether trees T and U are identical."},

{"has",			has_,		"A X  ->  B",
"Tests whether aggregate A has X as a member."},

{"in",			in_,		"X A  ->  B",
"Tests whether X is a member of aggregate A."},

{"integer",		integer_,	"X  ->  B",
"Tests whether X is an integer."},

{"char",		char_,		"X  ->  B",
"Tests whether X is a character."},

{"logical",		logical_,	"X  ->  B",
"Tests whether X is a logical."},

{"set",			set_,		"X  ->  B",
"Tests whether X is a set."},

{"string",		string_,	"X  ->  B",
"Tests whether X is a string."},

{"list",		list_,		"X  ->  B",
"Tests whether X is a list."},

{"leaf",		leaf_,		"X  ->  B",
"Tests whether X is not a list."},

{"user",		user_,		"X  ->  B",
"Tests whether X is a user-defined symbol."},

/* COMBINATORS */

{"i",			i_,		"[P]  ->  ...",
"Executes P. So, [P] i  ==  P."},

{"x",			x_,		"[P]i  ->  ...",
"Executes P without popping [P]. So, [P] x  ==  [P] P."},

{"dip", 		dip_,		"X [P]  ->  ... X",
"Saves X, executes P, pushes X back."},

{"popd",		popd_,		"Y Z  ->  Z",
"As if defined by:   popd  ==  [pop] dip "},

{"dupd",		dupd_,		"Y Z  ->  Y Y Z",
"As if defined by:   dupd  ==  [dup] dip"},

{"swapd",		swapd_,		"X Y Z  ->  Y X Z",
"As if defined by:   swapd  ==  [swap] dip"},

{"rollupd",		rollupd_,	"X Y Z W  ->  Z X Y W",
"As if defined by:   rollupd  ==  [rollup] dip"},

{"rolldownd",		rolldownd_,	"X Y Z W  ->  Y Z X W",
"As if defined by:   rolldownd  ==  [rolldown] dip "},

{"rotated",		rotated_,	"X Y Z W  ->  Z Y X W",
"As if defined by:   rotated  ==  [rotate] dip"},

{"app1",		app1_,		"X [P]  ->  R",
"Executes P, pushes result R on stack without X."},

{"app2",		app2_,		"X1 X2 [P]  ->  R1 R2",
"Executes P twice, with X1 and X2 on top of the stack. Returns the two values R1 and R2."},

{"app3",		app3_,		"X1 X2 X3 [P]  ->  R1 R2 R3",
"Executes P three times, with Xi, returns Ri (i = 1..3)."},

{"app4",		app4_,		"X1 X2 X3 X4 [P]  ->  R1 R2 R3 R4",
"Executes P four times, with Xi, returns Ri (i = 1..4)."},

{"app11",		app11_,		"X Y [P]  ->  R",
"Executes P, pushes result R on stack."},

{"app12",		app12_,		"X Y1 Y2 [P]  ->  R1 R2",
"Executes P twice, with Y1 and Y2, returns R1 and R2."},

{"construct",		construct_,	"[P] [[P1] [P2] ..]  ->  R1 R2 ..",
"Saves state of stack and then executes [P]. Then executes each [Pi] to give Ri pushed onto saved stack."},

{"nullary",		nullary_,	"[P]  ->  R",
"Executes P, which leaves R on top of the stack. No matter how many parameters this consumes, none are removed from the stack."},

{"unary",		unary_,		"X [P]  ->  R",
"Executes P, which leaves R on top of the stack. No matter how many parameters this consumes, exactly one is removed from the stack."},

{"binary",		binary_,	"X Y [P]  ->  R",
"Executes P, which leaves R on top of the stack. No matter how many parameters this consumes, exactly two removed from the stack."},

{"ternary",		ternary_,	"X Y Z [P]  ->  R",
"Executes P, which leaves R on top of the stack. No matter how many parameters this consumes, exactly three are removed from the stack."},

{"cleave",		cleave_,	"X [P1] [P2]  ->  R1 R2",
"Executes P1 and P2, each with X on top, producing two results."},

{"branch",		branch_,	"B [T] [F]  ->  ...",
"If B is true, then executes T else executes F."},

{"ifte",		ifte_,		"[B] [T] [F]  ->  ...",
"Executes B. If that yields true, then executes T else executes F."},

{"ifinteger",		ifinteger_,	"X [T] [E]  ->  ...",
"If X is an integer, executes T else executes E."},

{"ifchar",		ifchar_,	"X [T] [E]  ->  ...",
"If X is a character, executes T else executes E."},

{"iflogical",		iflogical_,	"X [T] [E]  ->  ...",
"If X is a logical or truth value, executes T else executes E."},

{"ifset",		ifset_,		"X [T] [E]  ->  ...",
"If X is a set, executes T else executes E."},

{"ifstring",		ifstring_,	"X [T] [E]  ->  ...",
"If X is a string, executes T else executes E."},

{"iflist",		iflist_,	"X [T] [E]  ->  ...",
"If X is a list, executes T else executes E."},

{"cond",		cond_,		"[..[[Bi] Ti]..[D]]  ->  ...",
"Tries each Bi. If that yields true, then executes Ti and exits. If no Bi yields true, executes default D."},

{"while",		while_,		"[B] [D]  ->  ...",
"While executing B yields true executes D."},

{"linrec",		linrec_,	"[I] [T] [R1] [R2]  ->  ...",
"Executes I. If that yields true, executes T. Else executes R1, recurses, executes R2."},

{"reclin",		reclin_,	"[I] [R1] [T] [R2]  ->  ...",
"Executes I. If that yields true, executes T. Else executes R1, recurses, executes R2. (= linrec with 2nd and 3rd parameter interchanged.)"},

{"tailrec",		tailrec_,	"[P] [T] [R1]  ->  ...",
"Executes P. If that yields true, executes T. Else executes R1, recurses."},

{"binrec",		binrec_,	"[B] [T] [R1] [R2]  ->  ...",
"Executes P. If that yields true, executes T. Else uses R1 to produce two intermediates, recurses on both, then executes R2 to combines their results."},

{"genrec",		genrec_,	"[B] [T] [R1] [R2]  ->  ...",
"Executes B, if that yields true executes T. Else executes R1 and then [[B] [T] [R1] [R2] genrec] R2."},

{"condlinrec",		condlinrec_,	"[ [C1] [C2] .. [D] ]  ->  ...",
"Each [Ci] is of the forms [[B] [T]] or [[B] [R1] [R2]]. Tries each B. If that yields true and there is just a [T], executes T and exit.  If there are [R1] and [R2], executes R1, recurses, executes R2. Subsequent case are ignored. If no B yields true, then [D] is used. It is of the forms [[T]] or [[R1] [R2]]. For the former, executes T. For the latter executes R1, recurses, executes R2."},

{"step",		step_,		"A  [P]  ->  ...",
"Sequentially putting members of aggregate A onto stack, executes P for each member of A."},

{"fold",		fold_,		"A  V0  BIN  ->  V1",
"Starting with value V0, sequentially puts members of aggregate A onto stack andcombines with binary operator BIN to finally prodice value V1"},

{"map",			map_,		"A [P]  ->  B",
"Executes P on each member of aggregate A, collects results in sametype aggregate B."},

{"times",		times_,		"[P] N  ->  ...",
"Executes P  N times."},

{"infra",		infra_,		"L1 [P]  ->  L2",
"Using list L1 as stack, executes P and returns a new list L2."},

{"primrec",		primrec_,	"X [I] [C]  ->  R",
"Executes I to obtain an initial value R0. For integer X uses increasing positive integers up to X and combines by C for new R. For aggregate X uses successive members and combines by C for new R."},

{"filter",		filter_,	"A [B]  ->  A1",
"Uses test B to filter aggregate A producing sametype aggregate A1."},

{"split",		split_,		"A [B]  ->  A1 A2",
"Uses test B to split aggregate A into sametype aggregates A1 and A2 ."},

{"some",		some_,		"A  [B]  ->  X",
"Applies test B to members of aggregate A, X = true if some pass."},

{"all",			all_,		"A [B]  ->  X",
"Applies test B to members of aggregate A, X = true if all pass."},

{"treestep",		treestep_,	"T [P]  ->  ...",
"Recursivly traverses leaves of tree T, executes P for each leaf."},

{"treerec",		treerec_,	"T [O] [C]  ->  ...",
"T is a tree. If T is a leaf, executes O. Else executes [[O] [C] treerec] C."},

{"treegenrec",		treegenrec_,	"T [O1] [O2] [C]  ->  ...",
"T is a tree. If T is a leaf, executes O1. Else executes O2 and then [[O1] [O2] [C] treegenrec] C."},

/* MISCELLANEOUS */

{"help",		help1_,		"->",
"Lists all defined symmbols, including those from library files. Then lists all primitives of raw Joy."},

{"_help",		h_help1_,	"->",
"Lists all hidden symbols in library and then all hidden inbuilt symbols."},

{"helpdetail",		helpdetail_,	"[ S1  S2  .. ]",
"Gives brief help on each symbol S in the list."},

{"manual",		o_online_manual_,	"->",
"Writes this manual of all Joy primitives to output file."},

{"__latex_manual",	l_latex_manual_,	"->",
"Writes this manual of all Joy primitives in Latex to output file."},

{"__settracegc",	settracegc_,	"I  ->",
"Sets value of flag for tracing garbage collection to I (= 0..5)."},

{"setautoput",		setautoput_,	"I  ->",
"Sets value of flag for automatic put to I (= 0 or 1)."},

{"setecho",		setecho_,	"I ->",
"Sets value of echo flag for listing. I = 0: no echo, 1: echo, 2: with tab, 3: and linenumber."},

{"gc",			gc_,		"->",
"Initiates garbage collection."},

{"system",		system_,	"\"command\"  ->",
"Escapes to shell, executes string \"command\". The string may cause execution of another program. When that has finished, the process returns to Joy."},

{"__memoryindex",	memoryindex_,	"->",
"Pushes current value of memory."},

{"get",			get_,		"->  F",
"Reads a factor from input and pushes it onto stack."},

{"put",			put_,		"X  ->",
"Writes X to output, pops X off stack."},

{"putch",		putch_,		"N  ->",
"N : numeric, writes character whose ASCII is N."},

{"include",		include_,	"\"filnam.ext\"  ->",
"Transfers input to file whose name is \"filnam.ext\". On end-of-file returns to previous input file."},

{"abort",		abortexecution_,"->",
"Aborts execution of current Joy program, returnsto Joy main cycle."},

{"quit",		quit_,		"->",
"Exit from Joy."},
{0, dummy_, "->","->"}
};

PUBLIC void inisymboltable()		/* initialise			*/
{
    int i; char *s;
    symtabindex = symtab;
    for (i = 0; i < HASHSIZE; hashentry[i++] = symtab) ;
    localentry = symtab;
    for (i = 0; optable[i].name; i++)
      { s = optable[i].name;
	/* ensure same algorithm in getsym */
	for (hashvalue = 0; *s != '\0';) hashvalue += *s++;
	hashvalue %= HASHSIZE;
	symtabindex->name = optable[i].name;
	symtabindex->u.proc = optable[i].proc;
	symtabindex->next = hashentry[hashvalue];
	hashentry[hashvalue] = symtabindex;
D(	printf("entered %s in symbol table at %ld = %ld\n", \
	    symtabindex->name, (long)symtabindex, \
	    LOC2INT(symtabindex)); )
	symtabindex++; }
    firstlibra = symtabindex;
}
PRIVATE void helpdetail_()
{
    Node *n;
    ONEPARAM("HELP");
    LIST("HELP");
    printf("\n");
    n = stk->u.lis;
    while (n != NULL)
      { if (n->op == USR_)
	  { printf("%s  ==\n    ",n->u.ent->name);
	    writeterm(n->u.ent->u.body);
	    printf("\n"); break; }
	else
	    printf("%s	:   %s.\n%s\n",
		optable[ (int) n->op].name,
		optable[ (int) n->op].messg1,
		optable[ (int) n->op].messg2);
	printf("\n");
	n = n->next; }
    POP(stk);
}
#define HEADER(N,NAME,HEAD)					\
    if (strcmp(N,NAME) == 0)					\
      { printf("\n\n");						\
	if (latex) printf("\\item[--- \\BX{");			\
	printf("%s",HEAD);					\
	if (latex) printf("} ---] \\verb# #");			\
	printf("\n\n"); }
PRIVATE void make_manual(int latex)
{
    int i; char * n;
    for (i = BOOLEAN_; optable[i].name != 0; i++)
      { n = optable[i].name;
	HEADER(n," truth value type","literal") else
	HEADER(n,"false","operand") else
	HEADER(n,"id","operator") else
	HEADER(n,"null","predicate") else
	HEADER(n,"i","combinator") else
	HEADER(n,"help","miscellaneous commands")
	if (n[0] != '_')
	  { if (latex)
	      { if (n[0] == ' ')
		  { n++;  printf("\\item[\\BX{"); }
		    else printf("\\item[\\JX{"); }
	    printf("%s",n);
	    if (latex) printf("}]  \\verb#");
	    printf("   :  %s", optable[i].messg1);
	    if (latex) printf("# \\\\ \n {\\small\\verb#");
		else printf("\n");
	    printf("%s", optable[i].messg2);
	    if (latex) printf("#}");
	    printf("\n\n"); } }
}
PUBLIC char *opername(o)
    int o;
{
    return optable[(short)o].name;
}
/* END of INTERP.C */
