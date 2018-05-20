#include "dt.h"

// If machine-specific addressing modes (see Section 14.8.2 [addressingmodes],
// page 127) are used, an appropriate structure can be specified here. 
// Otherwise, just enter the following code:
//  struct AddressingMode {
//  int never_used;
//  };

struct AddressingMode
  {
    int never_used;
  };

// Insert the number of available registers.

// (EAQ isn't really a register)
// A Q AQ E  
// X0-7
// PR0-7

#define MAXR 20



// Insert the number of command line flags that can be used to configure the
// behaviour of the code generator. This must be at least one even if you do not
// use any flags.

#define MAXGF 1 

// If this is set to zero, vbcc will not generate ICs with the target operand 
// being the same as the 2nd source operand. This can sometimes simplify the 
// codegenerator, but usually the code is better if the code-generator allows
// it.

#define USEQ2ASZ 0

// Insert the smallest integer type that can be added to a pointer. Smaller 
// types will be automatically converted to type MINADDI2P when they are to be
// added to a pointer. This may be subsumed by shortcut() in the future.

#define MINADDI2P INT

// Insert the largest integer type that can be added to a pointer. Larger types
// will be automatically converted to type MAXADDI2P when they are to be added
// to a pointer. This may be subsumed by shortcut() in the future.

#define MAXADDI2P INT

// Insert 1 if integers are represented in big endian, i.e. the most
// significant byte is at the lowest memory address, the least significant byte
// at the highest.

#define BIGENDIAN 1


// Insert 1 if integers are represented in little endian, i.e. the least
// significant byte is at the lowest memory address, the most significant byte
// at the highest.

#define LITTLEENDIAN 0

// Insert 1 if switch-statements should be compiled into a series of
// SUB/TEST/BEQ instructions rather than COMPARE/BEQ. This may be useful if the
// target has a more efficient SUB-instruction which sets condition codes (e.g.
// 68k).

#define SWITCHSUBS 1

// Insert the largest size in bytes allowed for inline memcpy. In optimizing
// compilation, certain library memcpy/strcpy-calls with length known at
// compile-time will be inlined using an ASSIGN IC if the size is less or equal
// to INLINEMEMCPY.  The type used for the ASSIGN IC will be UNSIGNED|CHAR.
// This may be replaced by a variable of type zmax in the future.

#define INLINEMEMCPY 1048575 // 2^18 * 4 - 1

//Insert 1 if PUSH ICs for function arguments shall be generated from left to
//right instead right to left.

//#define ORDERED_PUSH 1

// Insert this line if the backend supports register parameters (see Section
// 14.8.4 [regparm], page 131).

//#define HAVE_REGPARMS 1

// Insert this line if the backend supports register pairs (see Section 14.8.5
// [regpairs], page 131).

#define HAVE_REGPAIRS 1

// Insert this line if size_t shall be of type unsigned int rather than
// unsigned long.

#define HAVE_INT_SIZET 1

// Insert the maximum length of a line of code output.

#define EMIT_BUF_LEN 1024

// Insert the number of ouput lines that should be buffered. This can be useful
// for peephole-optimizing the assembly output (see below).

#define EMIT_BUF_DEPTH 1

// Insert 1 if the backend provides an asm_peephole() function (see Section
// 14.8.14 [asmpeephole], page 134).

#define HAVE_TARGET_PEEPHOLE 0

// Insert this line if the backend provides old target-specific
// variable-attributes (see Section 14.9.6 [targetattributes], page 138).

// #define HAVE_TARGET_ATTRIBUTES 1

// Insert this line if the backend provides target-specific #pragma-directives
// (see Section 14.9.7 [targetpragmas], page 139).

#define HAVE_TARGET_PRAGMAS 1

// Insert this line if the backend supports inter-procedural
// register-allocation (see Section 14.8.11 [regsmodified], page 134).

//#define HAVE_REGS_MODIFIED 1

// Insert this line if the backend supports context-sensitive
// register-allocation (see Section 14.8.10 [targetralloc], page 133).

// #define HAVE_TARGET_RALLOC 1

// Insert this line if the backend provides a mark_eff_ics() function (see
// Section 14.8.15 [markeffics], page 135).

//#define HAVE_TARGET_EFF_IC 1

// Insert this line if the backend provides a struct ext_ic (see Section
// 14.8.13 [extic], page 134).

//#define HAVE_EXT_IC 1

// Insert this line if the backend supports additional types (see Section
// 14.9.8 [exttypes], page 139).

#define HAVE_EXT_TYPES 1

/* We have extended types! What we have to do to support them:      */
/* - #define HAVE_EXT_TYPES
   - #undef all standard types
   - #define all standard types plus new types
   - write eval_const and insert_const
   - write typedefs for zmax and zumax
   - write typname[]
   - write conv_typ()
   - optionally #define ISPOINTER, ISARITH, ISINT etc.
   - optionally #define HAVE_TGT_PRINTVAL and write printval
   - optionally #define POINTER_TYPE
   - optionally #define HAVE_TGT_FALIGN and write falign
   - optionally #define HAVE_TGT_SZOF and write szof
   - optionally add functions for attribute-handling
*/

#undef CHAR
#undef SHORT
#undef INT
#undef LONG
#undef LLONG
#undef FLOAT
#undef DOUBLE
#undef LDOUBLE
#undef VOID
#undef POINTER
#undef ARRAY
#undef STRUCT
#undef UNION
#undef ENUM
#undef FUNKT
#undef BOOL
#undef MAXINT
#undef MAX_TYPE

#define CHAR 1
#define SHORT 2
#define INT 3
#define LONG 4
#define LLONG 5
#define FLOAT 6
#define DOUBLE 7
#define LDOUBLE 8
#define VOID 9
#define POINTER 10
// New
#define FPOINTER 11
#define ARRAY 12
#define STRUCT 13
#define UNION 14
#define ENUM 15
#define FUNKT 16
#define BOOL 17

#define MAXINT 18

#define MAX_TYPE MAXINT

#define POINTER_TYPE(x) pointer_type(x)
extern int pointer_type();
#define ISPOINTER(x) ((x&NQ)>=POINTER&&(x&NQ)<=FPOINTER)
#define ISSCALAR(x) ((x&NQ)>=CHAR&&(x&NQ)<=FPOINTER)
#define ISINT(x) ((x&NQ)>=CHAR&&(x&NQ)<=LLONG)
// Subtracting far pointers in only meaningful if both pointers
// are in the same segment, so the diff is at most 2^18
#define PTRDIFF_T(x) (SHORT)

typedef zllong zmax;
typedef zullong zumax;

union atyps{
  zchar vchar;
  zuchar vuchar;
  zshort vshort;
  zushort vushort;
  zint vint;
  zuint vuint;
  zlong vlong;
  zulong vulong;
  zllong vllong;
  zullong vullong;
  zmax vmax;
  zumax vumax;
  zfloat vfloat;
  zdouble vdouble;
  zldouble vldouble;
};


// Insert this line if the backend provides an own printval function see
// Section 14.9.9 [tgtprintval], page 139).

#define HAVE_TGT_PRINTVAL 1

// These values controls the creation of jump-tables (see Section 14.8.9
// [jumptables], page 132).

#define JUMP_TABLE_DENSITY 0.8
#define JUMP_TABLE_LENGTH 8

// Necessary defines for C99 variable-length-arrays (see Section 14.9.14
// [vlas], page 142).

//#define ALLOCVLA_REG <reg>
//#define ALLOCVLA_INLINEASM <inline-asm>
//#define FREEVLA_REG <reg>
//#define FREEVLA_INLINEASM <inline-asm>
//#define OLDSPVLA_INLINEASM <inline-asm>
//#define FPVLA_REG <reg>

// Insert this line if the backend wants certain ICs to be replaced with calls
// to library functions (see Section 14.9.15 [libcalls], page 142).

// #define HAVE_LIBCALLS 1

// Insert these lines to tell the frontend not to generate CONVERT ICs that
// convert between unsigned integers and floating point. In those cases,
// additional intermediate code will be generated that implements the
// conversion using only signed integers.

//#define AVOID_FLOAT_TO_UNSIGNED 1
//#define AVOID_UNSIGNED_TO_FLOAT 1
