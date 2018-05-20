

/* Machine generated file. DON'T TOUCH ME! */


#ifndef DT_H
#define DT_H 1
typedef signed short zchar;
typedef unsigned short zuchar;
typedef signed long zshort;
typedef unsigned long zushort;
typedef signed long zint;
typedef unsigned long zuint;
typedef signed __int128 zlong;
typedef unsigned __int128 zulong;
typedef signed __int128 zllong;
typedef unsigned __int128 zullong;
typedef double zfloat;
typedef long double zdouble;
typedef long double zldouble;
typedef unsigned long zpointer;
#define zc2zm(x) ((signed __int128)(x))
#define zs2zm(x) ((signed __int128)(x))
#define zi2zm(x) ((signed __int128)(x))
#define zl2zm(x) ((signed __int128)(x))
#define zll2zm(x) ((signed __int128)(x))
#define zm2zc(x) ((signed short)(x))
#define zm2zs(x) ((signed long)(x))
#define zm2zi(x) ((signed long)(x))
#define zm2zl(x) ((signed __int128)(x))
#define zm2zll(x) ((signed __int128)(x))
#define zuc2zum(x) ((unsigned __int128)(x))
#define zus2zum(x) ((unsigned __int128)(x))
#define zui2zum(x) ((unsigned __int128)(x))
#define zul2zum(x) ((unsigned __int128)(x))
#define zull2zum(x) ((unsigned __int128)(x))
#define zum2zuc(x) ((unsigned short)(x))
#define zum2zus(x) ((unsigned long)(x))
#define zum2zui(x) ((unsigned long)(x))
#define zum2zul(x) ((unsigned __int128)(x))
#define zum2zull(x) ((unsigned __int128)(x))
#define zum2zm(x) ((signed __int128)(x))
#define zm2zum(x) ((unsigned __int128)(x))
#define zf2zld(x) ((long double)(x))
#define zd2zld(x) ((long double)(x))
#define zld2zf(x) ((double)(x))
#define zld2zd(x) ((long double)(x))
#define zld2zm(x) ((signed __int128)(x))
#define zm2zld(x) ((long double)(x))
#define zld2zum(x) ((unsigned __int128)(x))
#define zum2zld(x) ((long double)(x))
#define zp2zum(x) ((unsigned __int128)(x))
#define zum2zp(x) ((unsigned long)(x))
#define l2zm(x) ((signed __int128)(x))
#define ul2zum(x) ((unsigned __int128)(x))
#define d2zld(x) ((long double)(x))
#define zm2l(x) ((long)(x))
#define zum2ul(x) ((unsigned long)(x))
#define zld2d(x) ((double)(x))
#define zmadd(a,b) ((a)+(b))
#define zumadd(a,b) ((a)+(b))
#define zldadd(a,b) ((a)+(b))
#define zmsub(a,b) ((a)-(b))
#define zumsub(a,b) ((a)-(b))
#define zldsub(a,b) ((a)-(b))
#define zmmult(a,b) ((a)*(b))
#define zummult(a,b) ((a)*(b))
#define zldmult(a,b) ((a)*(b))
#define zmdiv(a,b) ((a)/(b))
#define zumdiv(a,b) ((a)/(b))
#define zlddiv(a,b) ((a)/(b))
#define zmmod(a,b) ((a)%(b))
#define zummod(a,b) ((a)%(b))
#define zmlshift(a,b) ((a)<<(b))
#define zumlshift(a,b) ((a)<<(b))
#define zmrshift(a,b) ((a)>>(b))
#define zumrshift(a,b) ((a)>>(b))
#define zmand(a,b) ((a)&(b))
#define zumand(a,b) ((a)&(b))
#define zmor(a,b) ((a)|(b))
#define zumor(a,b) ((a)|(b))
#define zmxor(a,b) ((a)^(b))
#define zumxor(a,b) ((a)^(b))
#define zmmod(a,b) ((a)%(b))
#define zummod(a,b) ((a)%(b))
#define zmkompl(a) (~(a))
#define zumkompl(a) (~(a))
#define zmleq(a,b) ((a)<=(b))
#define zumleq(a,b) ((a)<=(b))
#define zldleq(a,b) ((a)<=(b))
#define zmeqto(a,b) ((a)==(b))
#define zumeqto(a,b) ((a)==(b))
#define zldeqto(a,b) ((a)==(b))
#endif
