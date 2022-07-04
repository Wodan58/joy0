/*
    module  : globals.h
    version : 1.1
    date    : 12/12/12
*/
/* FILE : globals.h */

				/* configure			*/
#define SHELLESCAPE	'$'
#define INPSTACKMAX	10
#define INPLINEMAX	80
#define ALEN		20
#define HASHSIZE	9
#define SYMTABMAX	2000
#define MEMORYMAX	100000
#define INIECHOFLAG	0
#define INIAUTOPUT	1
#define INITRACEGC	1
				/* installation dependent	*/
#define SETSIZE		32
#define MAXINT		2147483647
				/* symbols from getsym		*/
#define ILLEGAL_	 0
#define COPIED_		 1
#define USR_		 2
#define ANON_FUNCT_	 3
#define BOOLEAN_	 4
#define CHAR_		 5
#define INTEGER_	 6
#define SET_		 7
#define STRING_	 	 8
#define LIST_		 9
#define LBRACK		900
#define LBRACE		901
#define LPAREN		902
#define ATOM		999	/* last legal factor begin */
#define RBRACK		1001
#define RPAREN		1003
#define RBRACE		1005
#define PERIOD		1006
#define SEMICOL		1007
#define LIBRA		1100
#define EQDEF		1101
#define HIDE		1102
#define IN		1103
#define END		1104

#ifdef DEBUG
#    define D(x) x
#else
#    define D(x)
#endif

#define PRIVATE static
#define PUBLIC

				/* types			*/
typedef int Symbol;
typedef short Operator;

typedef struct Node
  { Operator op;
    union
      { long num;
	long set;
	char *str;
	struct Node *lis;
	struct Entry *ent;
	void (*proc)(); } u;
    struct Node *next; } Node;
typedef struct Entry
  { char *name;
    union 
      { Node *body;
	void  (*proc) (); } u;
    struct Entry *next; } Entry;

#ifdef ALLOC
#    define CLASS
#else
#    define CLASS extern
#endif

CLASS int echoflag;
CLASS int autoput;
CLASS int tracegc;
CLASS int startclock,gc_clock;			/* main		*/
CLASS char ch;					/* scanner	*/
CLASS Symbol sym;
CLASS long num;
CLASS char id[ALEN];
CLASS int hashvalue;

CLASS Entry					/* symbol table	*/
    symtab[SYMTABMAX],
    *hashentry[HASHSIZE],
    *localentry,
    *symtabindex,
    *firstlibra,				/* inioptable	*/
    *location;					/* getsym	*/

#define LOC2INT(e) (((long)e - (long)symtab) / sizeof(Entry))
#define INT2LOC(x) ((Entry*) ((x + (long)symtab)) * sizeof(Entry))

CLASS Node			/* dynamic memory	*/
/*
    memory[MEMORYMAX],
    *memoryindex,
*/
    *prog, *stk, *conts,
    *dump, *dump1, *dump2, *dump3, *dump4, *dump5;

#define MEM2INT(n) (((long)n - (long)memory) / sizeof(Node))
#define INT2MEM(x) ((Node*) ((x + (long)&memory) * sizeof(Node)))

/* GOOD REFS:
	005.133l H4732		A LISP interpreter in C
	Manna p139  recursive Ackermann SCHEMA

   OTHER DATA TYPES
	WORD = "ABCD" - up to four chars
	LIST of SETs of char [S0 S1 S2 S3]
	        LISTS - binary tree [left right]
			" with info [info left right]
	STRING of 32 chars = 32 * 8 bits = 256 bits = bigset
	CHAR = 2 HEX
	32 SET = 2 * 16SET
*/
