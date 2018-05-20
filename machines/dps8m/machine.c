#define TRACE

#include "supp.h"

#include <stdio.h>

// For ierror()

static char FILE_[]=__FILE__;
/*  Public data that MUST be there.                             */

/* Name and copyright. */
char cg_copyright[]="vbcc code-generator for DPS8/M V0.0";

/*  Commandline-flags the code-generator accepts                */

int g_flags[MAXGF]={0};
char *g_flags_name[MAXGF]={"NULL"};
union ppi g_flags_val[MAXGF];


// The number of bits in a char on the target (usually 8).

zmax char_bit;

// This array must contain the necessary alignments for every type in bytes.
// Some of the entries in this array are not actually used, but align[type&NQ]
// must yield the correct alignment for every type. align[CHAR] must be 1.
//
// The alignment of a structure depends not only on sizetab[STRUCT] but also
// on the alignment of the members. The maximum of the alignments of all
// members and sizetab[STRUCT] is the alignment of any particular structure,
// i.e.  sizetab[STRUCT] is only a minimum alignment. 
//
// The same applies to unions and arrays.

// Typenames (needed because of HAVE_EXT_TYPES

char * typname[MAX_TYPE+1] =
  {
    "strange",
    "char",
    "short",
    "int",
    "long",
    "long long",
    "float",
    "double",
    "long double",
    "void",
    "pointer",
    "far pointer",
    "array",
    "struct",
    "union",
    "enum",
    "function",
    "bool",
    "<maxint>"
  };

zmax align[MAX_TYPE+1];

// This variable must be set to an alignment in bytes that is used when pushing
// arguments on the stack. (FIXME: describe stackalign)

zmax maxalign;

// This array must contain the sizes of every type in bytes.

zmax sizetab[MAX_TYPE+1];

// This array must contain the smallest representable number for every signed
// integer type.

zmax t_min[MAX_TYPE+1];

// This array must contain the largest representable number for every signed
// integer type.

zumax t_max[MAX_TYPE+1];

// This array must contain the largest representable number for every unsigned
// integer type.

zumax tu_max[MAX_TYPE+1];

// This array must contain the names for every register. They do not
// necessarily have to be used in the assembly output but are used for explicit
// register arguments.

char *regnames[MAXR+1] =
  {
    /*  0 */ "no reg",
    /*  1 */ "A", "Q", "AQ", "E",
    /*  5 */ "X0", "X1", "X2", "X3", "X4", "X5", "X6", "X7",
    /* 13 */ "PR0", "PR1", "PR2", "PR3", "PR4", "PR5", "PR6", "PR7"
  };
#define R_A 1
#define R_Q 2
#define R_AQ 3
#define R_X0 5
#define R_PR0 13

#define R_IS_PR(r) ((r) >= R_PR0 && (r) < R_PR0 + 8)


// This array must contain the size of each register in bytes. It is used to
// create storage if registers have to be saved.

zmax regsize[MAXR+1];

// Not documented in vbcc.pdf

struct Typ * regtype[MAXR+1];


// This array must contain information whether a register is a scratchregister,
// i.e. may be destroyed during a function call (1 or 0). vbcc will generate
// code 120 vbcc manual to save/restore all scratch-registers which are
// assigned a value when calling a function (unless it knows the register will
// not be modified). However, if the code generator uses additional
// scratch-registers it has to take care to save/restore them.
//
// Also, the code generator must take care that used non-scratch-registers are
// saved/restored on function entry/exit.

// XXX this is wrong
int regscratch[MAXR+1] =
  { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

// This array must contain information whether a register is in use or not at
// the beginning of a function (1 or 0). The compiler will not use any of those
// registers for register variables or temporaries, therefore this can be used
// to mark special registers like a stack- or frame-pointer and to reserve
// registers to the code-generator. The latter may be reasonable if for many
// ICs code cannot be generated without using additional registers.
//
// You must set regsratch[i] = 0 if regsa[i] == 1. If you want it to be saved
// across function calls the code generator has to take care of this.

// XXX this is wrong
int regsa[MAXR+1] =
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

// This array must contain a priority (>=0) for every register. When the
// register allocator has to choose between several registers which seem to be
// equal, it will choose the one with the highest priority (if several
// registers have the same priority it is undefined which one will be taken).
//
// Note that this priority is only the last decision factor if everything else
// seems to be equal. If one register seems to give a higher cost saving
// (according to the estimation of the register allocator) but has a lower
// priority, it will nevertheless be used. The priority can be used to
// fine-tune the register selection. Some guidelines:
//
// − Scratch registers might have a higher priority than non-scratch registers
// (although the register-allocator will usually handle this anyway).
//
// − Registers which are more restricted should have a higher priority (if they
// seem to give the same saving it is usually better to use the restricted
// registers and try to keep the more versatile ones for situation in which
// they can give better savings).
//
// − Registers which are used for argument-passing should have lower priority
// than registers not used for arguments. The priority within the
// argumentregisters should decrease as the frequency of usage as argument
// increases (typically the register for the first argument ist used most
// frequently, etc.).  Note that for the array zmax regsize[] the same comments
// mentioned in the section on data types regarding initialization apply.

// XXX this is wrong
int reg_prio[MAXR+1] =
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };


////////
//
// Private data and functions
//

#define STR_FAR "far"

static long malign[MAX_TYPE+1] =
  {
    1,  // ?     0
    1,  // CHAR 1
    2,  // SHORT 2
    4,  // INT 3
    8,  // LONG 4
    8,  // LLONG 5
    4,  // FLOAT 6
    8,  // DOUBLE 7
    8,  // LDOUBLE 8
    4,  // VOID 9
    4,  // POINTER 10
    8,  // FPOINTER 11
    4,  // ARRAY 12
    4,  // STRUCT 13
    4,  // UNION 14
    4,  // ENUM 15
    4,  // FUNKT 16
    4,  // BOOL 17
    8   // MAXINT 18
  };

static long msizetab[MAX_TYPE+1] =
  {
    1,  // ?     0
    1,  // CHAR 1
    2,  // SHORT 2
    4,  // INT 3
    8,  // LONG 4
    8,  // LLONG 5
    4,  // FLOAT 6
    8,  // DOUBLE 7
    8,  // LDOUBLE 8
    4,  // VOID 9
    4,  // POINTER 10
    8,  // FPOINTER 11
    4,  // ARRAY 12
    4,  // STRUCT 13
    4,  // UNION 14
    4,  // ENUM 15
    4,  // FUNKT 16
    4,  // BOOL 17
    8   // MAXINT 18
  };

/* used to initialize regtyp[] */
static struct Typ ityp = {INT,NULL,NULL,0,NULL, NULL, 0};
static struct Typ ltyp = {LONG,NULL,NULL,0,NULL, NULL, 0};
static struct Typ ultyp = {UNSIGNED|LONG,NULL,NULL,0,NULL, NULL, 0};
static struct Typ uctyp = {UNSIGNED|CHAR,NULL,NULL,0,NULL, NULL, 0};
static struct Typ ustyp = {UNSIGNED|SHORT,NULL,NULL,0,NULL, NULL, 0};
static struct Typ fptyp = {FPOINTER,NULL,NULL,0,NULL, NULL, 0};


/////////
//
// Backend
//


// This function is called after the commandline arguments are parsed. It can
// set up certain internal data, etc. The arrays regarding the data types and
// the register set can be set up at this point rather than with a static
// initialization, however the arrays regarding the commandline options have to
// be static initialized. The results of the commandline options are available
// at this point.
//
// If something goes wrong, 0 has to be returned, otherwise 1.

int init_cg(void)
  {
    // Initialize some values which cannot be statically initialized
    //  because they are stored in the target's arithmetic. 
    char_bit = l2zm (9L);
    maxalign = l2zm (4L);

    for(int i=0; i <= MAX_TYPE; i ++)
      {
        sizetab[i] = l2zm (msizetab[i]);
        align[i] = l2zm (malign[i]);
      }

    regsize[1] = 4; // A
    regtype[1] = &ityp;

    regsize[2] = 4; // Q
    regtype[2] = &ityp;

    regsize[3] = 8; // AQ
    regtype[3] = &ltyp;

    regsize[4] = 1; // E
    regtype[4] = &uctyp;

    regsize[5] = 2; // X0
    regtype[5] = &ustyp;

    regsize[6] = 2; // X1
    regtype[6] = &ustyp;

    regsize[7] = 2; // X2
    regtype[7] = &ustyp;

    regsize[8] = 2; // X3
    regtype[8] = &ustyp;

    regsize[9] = 2; // X4
    regtype[9] = &ustyp;

    regsize[10] = 2; // X5
    regtype[10] = &ustyp;

    regsize[11] = 2; // X6
    regtype[11] = &ustyp;

    regsize[12] = 2; // X7
    regtype[12] = &ustyp;

    regsize[13] = 8; // PR0
    regtype[13] = &fptyp;

    regsize[14] = 8; // PR1
    regtype[14] = &fptyp;

    regsize[15] = 8; // PR2
    regtype[15] = &fptyp;

    regsize[16] = 8; // PR3
    regtype[16] = &fptyp;

    regsize[17] = 8; // PR4
    regtype[17] = &fptyp;

    regsize[18] = 8; // PR5
    regtype[18] = &fptyp;

    regsize[19] = 8; // PR6
    regtype[19] = &fptyp;

    regsize[20] = 8; // PR7
    regtype[20] = &fptyp;



    // Initialize the min/max-settings. Note that the types of the
    // host system may be different from the target system and you may
    // only use the smallest maximum values ANSI guarantees if you
    // want to be portable.
    // That's the reason for the subtraction in t_min[INT]. Long could
    // be unable to represent -2147483648 on the host system.


// XXX Not portable
    t_min[CHAR]   = -(256LL);
    t_min[SHORT]  = -(1LL<<17);
    t_min[INT]    = -(1LL<<35);
    t_min[LONG]   = -(1LL<<35);
    t_min[LLONG]  = -(((__int128)1LL)<<71);
    t_min[MAXINT] = t_min (LLONG);

    t_max[CHAR]  = 1LL<<8-1;
    t_max[SHORT] = 1LL<<17-1;
    t_max[INT]   = 1LL<<35-1;
    t_max[LONG]  = 1LL<<35-1;
    t_max[LLONG] = ((__int128)1LL)<<71-1;
    t_max[MAXINT] = t_max(LLONG);

    tu_max[CHAR]  = 1LL<<9-1;
    tu_max[SHORT] = 1LL<<18-1;
    tu_max[INT]   = 1LL<<36-1;
    tu_max[LONG]  = 1LL<<36-1;
    tu_max[LLONG] = ((__int128)1LL)<<72-1;
    tu_max[MAXINT] = t_max(UNSIGNED|LLONG);

    return 1;
  }




// This function is called before the compiler exits. f is the output file
// which must be checked against 0 before using.

void cleanup_cg (FILE *f)
  {
    // XXX
    fprintf (stderr, "XXX missing %s\n", __func__);
  }

//  Returns the register in which variables of type t are returned.  If the
//  value cannot be returned in a register returns 0.       A pointer MUST be
//  returned in a register. The code-generator  has to simulate a pseudo
//  register if necessary. 




int freturn (struct Typ *t)
  {
    int tu = t->flags & NQ;
    if (! ISSCALAR (t->flags))
      return 0;
    if (zmleq(szof(t),l2zm(4L)))
      return R_A;
    if (zmeqto(szof(t),l2zm(8L)))
      return R_AQ;
    return 0;
  }




// Check whether the type t can be stored in register r and whether the usual
// operations (for this type) can be generated. Return 0, if not.
//
// If t is a pointer and mode==0 the register only has to be able to store the
// pointer and do arithmetic, but if mode!=0 it has to be able to dereference
// the pointer.
//
// mode==-1 is used with context-sensitive register-allocation (see Section
// 14.8.10 [targetralloc], page 133). If the backend does not support it, this
// case can be handled equivalent to mode==0.
//
// If t==0 return whether the register can be used to store condition codes.
// This is only relevant if multiple_ccs is set to 1.

// cac: "able to dereference a pointer" I am deciding that a pointer in A/Q can
// be dereferenced, as it has to be unpacked into a PR in any case. This
// will change if I can get smarter about pointer optimization.

int regok(int r,int t,int mode)
  {
    if (r == 0)
      return 0;
    t &= NQ;

    if (R_IS_PR(r))
      return 0;
    if (sizetab[t] <= regsize[r])
      return 1;
    return 0;
  }



// Check if this IC can raise exceptions or is otherwise dangerous. Movement of
// ICs which are dangerous is restricted to preserve the semantics of the
// program.  Typical dangerous ICs are divisions or pointer dereferencing. On
// certain targets floating point or even signed integer arithmetic can raise
// exceptions, too.  

int dangerous_IC (struct IC *p)
  {
    // XXX
    fprintf (stderr, "XXX missing %s\n", __func__);
    return 0;
  }



// Check if code must be generated to convert from type from to type to. E.g.
// on many machines certain types have identical representations (integers of
// the same size or pointers and integers of the same size).
//
// If const_expr != 0 return if a conversion was necessary in a constant
// expression.
//
// For example, a machine may have identical pointers and integers, but
// different sets of registers (one set supports integer operations and the
// other pointer operations). Therefore, must_convert() would return 1 (we need
// a CONVERT IC to move the value from one register set to the other).
//
// This would imply that vbcc would not allow a cast from a pointer to an
// integer or vice-versa in constant expressions (as it will not generate code
// for static initializations). However, in this case, a static initialization
// would be ok as the representation is identical and registers are not
// involved. Therefore, the backend can return 1 if const_expr == 0 and 0
// otherwise.

int must_convert (int o,int t,int const_expr)
  { 
    // XXX
    fprintf (stderr, "XXX missing %s\n", __func__);
    return 0;
/*
    int op = o & NQ;
    int tp = t & NQ;
    
    if(tp==POINTER&&op==POINTER) return 0;
  if((t&UNSIGNED)&&(o&UNSIGNED)&&zmeqto(sizetab[tp],sizetab[op])) return 0;
  if((tp==INT&&op==LONG)||(tp==LONG&&op==INT)) return 0;
  if(op==tp) return 0;
  return 1;
*/
} 


// In C no operations are done with chars and shorts because of integral
// promotion.  However sometimes vbcc might see that an operation could be
// performed with the short types yielding the same result.
//
// Before generating such an instruction with short types vbcc will ask the
// code generator by calling shortcut() to find out whether it should do so.
// Return true iff it is a win to perform the operation code with type t rather
// than promoting the operands and using e.g. int.

int shortcut (int code, int t)
  {
// XXX Change to 1 when the code generator can do X reg ops?
    return 0;
  }


// gen_code support routines

// assembly-prefixes for labels and external identifiers 

static char * labprefix = "l", * idprefix = "_";

static void title (FILE *f)
  {
    static int done = 0;
    extern char *inname; /*grmpf*/
    if(! done && f)
      {
        done = 1;
        emit (f, "\" file '%s'\n", inname);
      }
  }

static char * code_coding [] =
  {
    "1701",
    "KOMMA", // KOMMA 1
    "ASSIGN", // ASSIGN 2
    "ASSIGNOP", // ASSIGNOP 3

    //"obsolete ASSIGNADD", // ASSIGNADD 3
    "obsolete ASSIGNSUB", // ASSIGNSUB 4
    "obsolete ASSIGNMULT", // ASSIGNMULT 5
    "obsolete ASSIGNDIV", // ASSIGNDIV 6
    "obsolete ASSIGNMOD", // ASSIGNMOD 7
    "obsolete ASSIGNAND", // ASSIGNAND 8
    "obsolete ASSIGNXOR", // ASSIGNXOR 9
    "obsolete ASSIGNOR", // ASSIGNOR 10
    "obsolete ASSIGNLSHIFT", // ASSIGNLSHIFT 11
    "obsolete ASSIGNRSHIFT", // ASSIGNRSHIFT 12

    "COND", // COND 13
    "LOR", // LOR 14
    "LAND", // LAND 15
    "OR", // OR 16
    "XOR", // XOR 17
    "AND", // AND 18
    "EQUAL", // EQUAL 19
    "INEQUAL", // INEQUAL 20
    "LESS", // LESS 21
    "LESSEQ", // LESSEQ 22
    "GREATER", // GREATER 23
    "GREATEREQ", // GREATEREQ 24
    "LSHIFT", // LSHIFT 25
    "RSHIFT", // RSHIFT 26
    "ADD", // ADD 27
    "SUB", // SUB 28
    "MULT", // MULT 29
    "DIV", // DIV 30
    "MOD", // MOD 31
    "NEGATION", // NEGATION 32
    "KOMPLEMENT", // KOMPLEMENT 33
    "PREINC", // PREINC 34
    "POSTINC", // POSTINC 35
    "PREDEC", // PREDEC 36
    "POSTDEC", // POSTDEC 37
    "MINUS", // MINUS 38
    "CONTENT", // CONTENT 39
    "ADDRESS", // ADDRESS 40
    "CAST", // CAST 41
    "CALL", // CALL 42
    "INDEX", // INDEX 43
    "DPSTRUCT", // DPSTRUCT 44
    "DSTRUCT", // DSTRUCT 45
    "IDENTIFIER", // IDENTIFIER 46
    "CEXPR", // CEXPR 47
    "STRING", // STRING 48
    "MEMBER", // MEMBER 49
    "CONVERT", // CONVERT 50
    //"obsolete CONVERT", // CONVCHAR 50
    "obsolete CONVSHORT", // CONVSHORT 51
    "obsolete CONVINT", // CONVINT 52
    "obsolete CONVLONG", // CONVLONG 53
    "obsolete CONVFLOAT", // CONVFLOAT 54
    "obsolete CONVDOUBLE", // CONVDOUBLE 55
    "obsolete CONVVOID", // CONVVOID 56
    "obsolete CONVPOINTER", // CONVPOINTER 57
    "obsolete CONVUCHAR", // CONVUCHAR 58
    "obsolete CONVUSHORT", // CONVUSHORT 59
    "obsolete CONVUINT", // CONVUINT 60
    "obsolete CONVULONG", // CONVULONG 61
    "ADDRESSA", // ADDRESSA 62
    "FIRSTELEMENT", // FIRSTELEMENT 63
    "PMULT", // PMULT 64
    "ALLOCREG", // ALLOCREG 65
    "FREEREG", // FREEREG 66
    "PCEXPR", // PCEXPR 67
    "TEST", // TEST 68
    "LABEL", // LABEL 69
    "BEQ", // BEQ 70
    "BNE", // BNE 71
    "BLT", // BLT 72
    "BGE", // BGE 73
    "BLE", // BLE 74
    "BGT", // BGT 75
    "BRA", // BRA 76
    "COMPARE", // COMPARE 77
    "PUSH", // PUSH 78
    "POP", // POP 79
    "ADDRESSS", // ADDRESSS 80
    "ADDI2P", // ADDI2P 81
    "SUBIFP", // SUBIFP 82
    "SUBPFP", // SUBPFP 83
    "PUSHREG", // PUSHREG 84
    "POPREG", // POPREG 85
    "POPARGS", // POPARGS 86
    "SAVEREGS", // SAVEREGS 87
    "RESTOREREGS", // RESTOREREGS 88
    "ILABEL", // ILABEL 89
    "DC", // DC 90
    "ALIGN", // ALIGN 91
    "COLON", // COLON 92
    "GETRETURN", // GETRETURN 93
    "SETRETURN", // SETRETURN 94
    "MOVEFROMREG", // MOVEFROMREG 95
    "MOVETOREG", // MOVETOREG 96
    "NOP", // NOP 97
    "BITFIELD", // BITFIELD 98
    "LITERAL", // LITERAL 99
    "REINTERPRET", // REINTERPRET 100
  };

#if 0
// ALM storage types 
static char *i dct[MAX_TYPE+1] =
  {
    "",
    "byte",    // CHAR
    "short",   // SHORT
    "long",    // INT
    "long",    // LONG
    "long",    // LLONG
    "long",    // FLOAT
    "long",    // DOUBLE
    "long",    // LDOUBLE
// VOID
// POINTER
// FPOINTER
// ARRAY
// STRUCT
// UNION
// ENUM
// FUNKT
// BOOL
  };   
#endif


// This function has to emit code for a function to stream f. v is the function
// being generated, p is a pointer to the list of ICs, that has to be
// converted.  offset is the space needed for local variables in bytes.
// 
// This function has to take care that only scratchregisters are destroyed by
// this function. The array regused contains information about the registers
// that have been used by vbcc in this function. However if the code generator
// uses additional registers it has to take care of them, too.
// 
// The regs[] and regused[] arrays may be overwritten by gen_code() as well as
// parts of the list of ICs. However the list of ICs must still be a valid list
// of ICs after gen_code() returns.
// 
// All assembly output should be generated using the available emit functions.
// These functions are able to keep several lines of assembly output buffered
// and allow peephole optimizations on assembly output (see Section 14.8.14
// [asmpeep- hole], page 134).

void gen_code (FILE *f, struct IC *p, struct Var *v, zmax offset)
  { 
    // XXX
    //fprintf (stderr, "XXX missing %s\n", __func__);

    title (f);

    for (; p; p = p -> next)
      {
        int c = p->code;
        int t = p->typf & NU;
        fprintf (stderr, "XXX gen_code failing on (%d %d) %s %s\n", c, t, code_coding[c], typname [t]);
        emit (f, "\" XXX gen_code failing on (%d %d) %s %s\n", c, t, code_coding[c], typname [t]);
      }
  }



// Has to emit output that generates size bytes of type t initialized with
// proper 0.
// 
// t is a pointer to a struct Typ which contains the precise type of the
// variable.  On machines where every type can be initialized to 0 by setting
// all bits to zero, the type does not matter. Otherwise see Section 14.3.4
// [compositetypes], page 108.
// 
// All assembly output should be generated using the available emit functions.


// For every target data type there is a corresponding global variable of that
// type, e.g. zchar vchar, zuchar vuchar, zmax vmax etc. These two functions
// simplify handling of target data types by transferring between a union atyps
// and these variables.
// 
// eval_const() reads the member of the union corresponding to the type t and
// converts it into all the global variables while insert_const() takes the
// global variable according to t and puts it into the appropriate member of
// the union atyps.
// 
// The global variable gval may be used as a temporary union atyps by the
// backend.

void gen_ds(FILE *f, zmax size, struct Typ *t)
  { 
    // XXX
    fprintf (stderr, "XXX missing %s\n", __func__);
  }



// Has to emit output that ensures the following data to be aligned to align
// bytes.  All assembly output should be generated using the available emit
// functions.
void gen_align(FILE *f, zmax align)
  { 
    // XXX
    fprintf (stderr, "XXX missing %s\n", __func__);
  }


// constants come at us in byte (or short) sized chunks, but ALM requires
// words. We will keep a buffer holding the 'vfd' being built.

static char vfdbuf[128];
static int vfdcnt;


static void vfdinit (void)
  {
    vfdbuf[0]=0;
    vfdcnt = 0;
  }

static void vfdemit (FILE *f)
  {
    strcat (vfdbuf, "\n");
    emit (f, vfdbuf);
    vfdinit ();
  }

static void vfdadd (FILE *f, unsigned int val)
  {
    if (vfdcnt)
      {
        char * p = vfdbuf + strlen (vfdbuf);
        sprintf (p, ",9/%o", val);
        vfdcnt ++;
        if (vfdcnt == 4)
          vfdemit (f);
      }
    else
      {
        sprintf (vfdbuf, "\tvfd\t9/%o", val);
        vfdcnt = 1;
      }
  }



// Has to print the head of a static or external variable v. This includes the
// label and necessary informations for external linkage etc.
//
// Typically variables will be generated by a call to gen_align() followed by
// gen_ var_head() and (a series of) calls to gen_dc() and/or gen_ds(). It may
// be necessary to keep track of the information passed to gen_var_head().  All
// assembly output should be generated using the available emit functions.

void gen_var_head(FILE *f, struct Var *v)
  { 
    // XXX
    //fprintf (stderr, "XXX missing %s\n", __func__);
    int constflag = 0;

    title (f);

    if (v -> clist)
      constflag = is_const (v->vtyp);

    vfdinit ();

    if (v->storage_class == STATIC)
      {
        if (ISFUNC (v->vtyp->flags))
          return;
        fprintf (stderr, "\" XXX gen_var_head failing static %s%ld size %ld\n", labprefix, zm2l (v->offset), zm2l (szof (v->vtyp)));
        emit (f, "\" XXX gen_var_head failing static %s%ld size %ld\n", labprefix, zm2l (v->offset), zm2l (szof (v->vtyp)));
#if 0
        if (v->clist &&
            (! constflag || (g_flags[2] & USEDFLAG))&&
            section!=DATA)
          {
            emit (f,dataname);
            if(f)
               section=DATA;
          }
#endif

#if 0
        if (v->clist &&
            constflag && 
            !(g_flags[2] & USEDFLAG) &&
            section != CODE)
          {
            emit (f, codename);
            if (f)
              section = CODE;
          }
#endif

#if 0
        if (! v->clist &&
            section != BSS)
          {
            emit (f, bssname);
            if (f)
              section = BSS;
          }
#endif

#if 0
        emit (f, "\t.type\t%s%ld,@object\n", labprefix, zm2l (v->offset));
        emit (f, "\t.size\t%s%ld,%ld\n", labprefix, zm2l (v->offset), zm2l (szof (v->vtyp)));
#endif

#if 0
        if (section != BSS)
          emit (f,"\t.align\t3\n%s%ld:\n",labprefix,zm2l(v->offset));
        else
          {
            emit (f,"\t.lcomm\t%s%ld,",labprefix,zm2l(v->offset));

            newobj = 1;
          }
#endif
      } // if static


    else if (v->storage_class == EXTERN)
      {
#if 0
        emit (f,"\t.global\t%s%s\n", idprefix, v->identifier);
#endif
        if (v->flags & (DEFINED | TENTATIVE))
          {
            fprintf (stderr, "XXX gen_var_head failing extern %s\n", v->identifier);
            emit (f, "XXX gen_var_head failing extern %s\n", v->identifier);
            
#if 0
            if (v->clist &&
                (! constflag || (g_flags[2] & USEDFLAG)) &&
                section != DATA)
              {
                emit (f,dataname);
                if (f)
                  section = DATA;
              }
            if (v->clist &&
                constflag &&
                !(g_flags[2] & USEDFLAG) &&
                section != CODE)
              {
                emit (f,codename);
                if (f)
                  section = CODE;
              }
            if (! v->clist &&
                section != BSS)
              {
                emit (f, bssname);
                if (f)
                 section = BSS;
              }
#endif

#if 0
            emit (f, "\t.type\t%s%s,@object\n", idprefix, v->identifier);
            emit (f, "\t.size\t%s%s,%ld\n", idprefix, v->identifier, zm2l(szof(v->vtyp)));
#endif

#if 0
            if (section != BSS)
              emit (f, "\t.align\t3\n%s%s:\n", idprefix, v->identifier);
            else
              {
                emit (f, "\t.comm\t%s%s,", idprefix, v->identifier);
                newobj = 1;
              }
#endif
          } // if defined | tenative
      } // if extern
  }


// Emit initialized data. t is the basic type that has to be emitted. p points
// to a struct const_list.
//
// If p->tree != 0 then p->tree->o is a struct obj which has to be emitted.
// This will usually be the address of a variable of storage class static or
// unsigned, possibly with an offset added (see Section 14.3.2 [operands], page
// 105 for further details).
//
// if p->tree == 0 then p->val is a union atyps which contains (in the member
// corresponding to t) the constant value to be emitted.
//
// All assembly output should be generated using the available emit functions.

void gen_dc (FILE *f, int t, struct const_list *p)
  { 
    // XXX
    //fprintf (stderr, "XXX missing %s\n", __func__);
    title (f);

    if (p->tree)
      {
        fprintf (stderr, "XXX gen_dc failing tree\n");
        emit (f, "XXX gen_dc failing tree\n");
        return;
      }

    t = t & NU;

    if (ISFLOAT (t))
      {
        fprintf (stderr, "XXX gen_dc failing float\n");
        emit (f, "XXX gen_dc failing float\n");
        return;
      }

    if (t == CHAR)
      {
        vfdadd (f, (unsigned int) p->val.vchar);
        return;
      }

    fprintf (stderr, "XXX gen_dc failing on (%d) %s, tree %p, next %p, val %ld\n", t & NQ, typname [t & NQ], p->tree, p->next, p->val.vint);
    emit (f, "\" XXX gen_dc failing on (%d) %s, tree %p, next %p, val %ld\n", t & NQ, typname [t & NQ], p->tree, p->next, p->val.vint);
  }


void gen_close_list (FILE *f)
  {
     if (vfdcnt)
       vfdemit (f);
  }


// If debug-information is requested, this functions is called after init_cg(),
// but before any code is generated. See also Section 14.9.10 [debuginfo], page
// 139.

void init_db (FILE *f)
  { 
    // XXX
    fprintf (stderr, "XXX missing %s\n", __func__);
  }



// If debug-information is requested, this functions is called prior to
// cleanup_ cg(). See also Section 14.9.10 [debuginfo], page 139.

void cleanup_db (FILE *f)
  { 
    // XXX
    fprintf (stderr, "XXX missing %s\n", __func__);
  }


int handle_pragma(const char *p)

  { 
    // XXX
    fprintf (stderr, "XXX missing %s\n", __func__);
    return 0;
  }




///////////////////////////////////
//
// Needed to support HAVE_EXT_TYPES
//

// For every target data type there is a corresponding global variable of that
// type, e.g. zchar vchar, zuchar vuchar, zmax vmax etc. These two functions
// simplify handling of target data types by transferring between a union atyps
// and these variables.
//
// eval_const() reads the member of the union corresponding to the type t and
// converts it into all the global variables while insert_const() takes the
// global variable according to t and puts it into the appropriate member of
// the union atyps.

// The global variable gval may be used as a temporary union atyps by the
// backend.

union atyps gval;

// Places constant in corresponding field.

void insert_const(union atyps *p,int t)
  {                         
    if(!p)
      ierror (0);       
    t &= NU;                
    if(t == CHAR)
      {
        p->vchar = vchar;
        return;
      }
    if(t == SHORT)
      {
        p->vshort = vshort;
        return;
      }
    if(t == INT)
      {
        p->vint = vint;
        return;
      }
    if(t == LONG)
      {
        p->vlong = vlong;
        return;
      }
    if(t == LLONG)
      {
        p->vllong = vllong;
        return;
      }
    if(t == MAXINT)
      {
        p->vmax = vmax;
        return;
      }
    if(t == (UNSIGNED|CHAR))
      {
        p->vuchar = vuchar;
        return;
      }
    if(t == (UNSIGNED|SHORT))
      {
        p->vushort = vushort;
        return;
      }
    if(t == (UNSIGNED|INT))
      {
        p->vuint = vuint;
        return;
      }
    if(t == (UNSIGNED|LONG))
      {
        p->vulong = vulong;
        return;
      }
    if(t == (UNSIGNED|LLONG))
      {
        p->vullong = vullong;
        return;
      }
    if(t == (UNSIGNED|MAXINT))
      {
        p->vumax = vumax;
        return;
      }
    if(t == FLOAT) 
      {
        p->vfloat = vfloat;
        return;
      }
    if(t == DOUBLE)
      {
        p->vdouble = vdouble;
        return;
      }
    if(t == LDOUBLE)
      {
        p->vldouble = vldouble;
        return;
      }
    if(t == POINTER)
      {
        p->vuint = vuint;
        return;
      }
    if(t == FPOINTER)
      {
        p->vulong = vulong;
        return;
      }
  }


void eval_const(union atyps *p,int t)
  {
    /* Assigns value of a CEXPR to certain global variables.*/
    int f = t & NQ; // mask out type-qualifers
    if (! p)
      ierror (p);
    if (f == MAXINT ||(f >= CHAR && f <= LLONG))
      { // Scaler
        if (! (t & UNSIGNED))
          { // Signed scaler
            if (f == CHAR)
              vmax = zc2zm (p -> vchar);
           else if (f == SHORT)
              vmax = zs2zm (p->vshort);
           else if (f == INT)
              vmax = zi2zm (p->vint);
           else if (f == LONG)
              vmax = zl2zm (p->vlong);
           else if (f == LLONG)
              vmax = zll2zm (p->vllong);
           else if (f == MAXINT)
              vmax = p->vmax;
           else
              ierror (0);
           vumax = zm2zum (vmax);
           vldouble = zm2zld (vmax);
          }
        else
          { // Unsigned scaler
           if (f == CHAR)
             vumax = zuc2zum (p->vuchar);
           else if (f == SHORT)
             vumax = zus2zum (p->vushort);
           else if (f == INT)
             vumax = zui2zum (p->vuint);
           else if (f == LONG)
             vumax = zul2zum (p->vulong);
           else if (f == LLONG)
             vumax = zull2zum (p->vullong);
           else if (f == MAXINT)
             vumax = p->vumax;
           else
             ierror (0);
           vmax = zum2zm (vumax);
           vldouble = zum2zld (vumax);
         }
      }
    else // not scaler
      {
        if (ISPOINTER (f))
          {
            if (f == POINTER)
              {
                vumax = zui2zum (p->vuint);
                vmax = zum2zm (vumax);
                vldouble = zum2zld (vumax);
              }
            else if (f == FPOINTER)
              {
                vumax = zul2zum (p->vulong);
                vmax = zum2zm (vumax);
                vldouble = zum2zld (vumax);
              }
          }
        else
          {
            if (f == FLOAT)
              vldouble = zf2zld (p->vfloat);
            else if(f == DOUBLE)
              vldouble = zd2zld (p->vdouble);
            else
              vldouble = p->vldouble;
            vmax = zld2zm (vldouble);
            vumax = zld2zum (vldouble);
          }
      }
    vfloat = zld2zf(vldouble);
    vdouble = zld2zd(vldouble);
    vuchar = zum2zuc(vumax);
    vushort = zum2zus(vumax);
    vuint = zum2zui(vumax);
    vulong = zum2zul(vumax);
    vullong = zum2zull(vumax);
    vchar = zm2zc(vmax);
    vshort = zm2zs(vmax);
    vint = zm2zi(vmax);
    vlong = zm2zl(vmax);
    vllong = zm2zll(vmax);
  }




void conv_typ(struct Typ *p)
  { 
/* Erzeugt extended types in einem Typ. */
    char * attr;
    while (p)
      {
        if (ISPOINTER (p->flags))
          {
            p -> flags = ((p->flags & ~NU) | POINTER_TYPE (p->next));
            if( attr = p->next->attr)
              {
                if (strstr (attr,STR_FAR))
                  p->flags = ((p->flags & ~NU) | FPOINTER);
              }
          }
        p = p->next;
     }
  }





int pointer_type (struct Typ * p)
  {
    if (!p)
      ierror (0);
    while ((p->flags & NQ) == ARRAY) 
      p = p->next;
    if (p->attr)
      if(strstr (p->attr, STR_FAR))
          return FPOINTER;
    return POINTER;
  }



//////////////////////////////
//
// Needed for HAVE_TGT_PRINTVAL
//

void printval (FILE *f,union atyps *p,int t)
  {
    t&= NU;
    if (t == CHAR)
      {
        fprintf (f,"C");
        vmax = zc2zm (p->vchar);
        printzm (f,vmax);
      }
    if (t == (UNSIGNED|CHAR))
      {
        fprintf (f,"UC");
        vumax = zuc2zum (p->vuchar);
        printzum (f,vumax);
      }
    if (t == SHORT)
      {
        fprintf (f,"S");
        vmax = zs2zm (p->vshort);
        printzm (f,vmax);
      }
    if (t == (UNSIGNED|SHORT))
      {
        fprintf (f,"US");
        vumax = zus2zum (p->vushort);
        printzum (f,vumax);
      }
    if (t == FLOAT)
      {
        fprintf (f,"F");
        vldouble = zf2zld(p->vfloat);
        printzld(f,vldouble);
      }
    if (t == DOUBLE)
      {
        fprintf (f,"D");
        vldouble = zd2zld(p->vdouble);
        printzld(f,vldouble);
      }
    if (t == LDOUBLE)
      {
        fprintf (f,"LD");
        printzld(f,p->vldouble);
      }
    if (t == INT)
      {
        fprintf (f,"I");
        vmax = zi2zm (p->vint);
        printzm (f,vmax);
      }
    if (t == (UNSIGNED|INT))
      {
        fprintf (f,"UI");
        vumax = zui2zum (p->vuint);
        printzum (f,vumax);
      }
    if (t == LONG)
      {
        fprintf (f,"L");
        vmax = zl2zm (p->vlong);
        printzm (f,vmax);
      }
    if (t == (UNSIGNED|LONG))
      {
        fprintf (f,"UL");
        vumax = zul2zum (p->vulong);
        printzum (f,vumax);
      }
    if (t == MAXINT)
      {
        fprintf (f,"M");
        printzm (f,p->vmax);
      }
    if (t == (UNSIGNED|MAXINT))
      {
        fprintf (f,"UM");
        printzum (f,p->vumax);
      }
    /*FIXME*/
    if (t == POINTER)
      {
        fprintf (f,"P");
        vumax = zui2zum (p->vuint);
        printzum (f,vumax);
      }
    /*FIXME*/
    if (t == FPOINTER)
      {
        fprintf (f,"FP");
        vumax = zul2zum (p->vulong);
        printzum (f,vumax);
      }
  }


///////////////////////////
//
// Needed for HAVE_REGPAIRS
//


// If register r is a register pair, the function has to set p->r1 and p->r2 to
// the first and second register which comprise the pair and return 1.
// Otherwise, zero has to be returned.

int reg_pair(int r,struct rpair *p)
  {
    if (r != R_AQ)
      return 0;
    p->r1 = R_A;
    p->r2 = R_Q;
    return 1;
  }

