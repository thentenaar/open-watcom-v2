/****************************************************************************
*
*                            Open Watcom Project
*
*    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
*
*  ========================================================================
*
*    This file contains Original Code and/or Modifications of Original
*    Code as defined in and that are subject to the Sybase Open Watcom
*    Public License version 1.0 (the 'License'). You may not use this file
*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
*    provided with the Original Code and Modifications, and is also
*    available at www.sybase.com/developer/opensource.
*
*    The Original Code and all software distributed under the License are
*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
*    NON-INFRINGEMENT. Please see the License for the specific language
*    governing rights and limitations under the License.
*
*  ========================================================================
*
* Description:  C compiler global variables.
*
****************************************************************************/


#define BY_C_FRONT_END

#include <stdio.h>
#include <setjmp.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "target.h"

#ifndef _CPU
    #error _CPU macro not defined
#endif

#if _CPU != 8086
    /* enable Structured Exception Handling for all 32-bit targets */
    #define __SEH__
#endif

#ifdef __UNIX__
    #define FNAMECMPSTR     strcmp      /* for case sensitive file systems */
#else
    #define FNAMECMPSTR     stricmp     /* for case insensitive file systems */
#endif

#define CMPLIT(s,c) memcmp( s, c, sizeof( c ) )
#define CPYLIT(s,c) memcpy( s, c, sizeof( c ) )

typedef char        *MACADDR_T; /* contains actual pointer to block of memory */
typedef char        *SEGADDR_T; /* contains actual pointer to block of memory */

#include "dw.h"
#include "ctypes.h"
#include "macro.h"
#include "cfcb.h"
#include "csegid.h"
#include "ctokens.h"
#include "cerrs.h"
#include "toggle.h"
#include "cmsg.h"
#include "strsave.h"
#include "cmemmgr.h"

#ifndef local
    #define local
#endif

#ifndef global
    #define global  extern
#endif

global char         *PCH_Start;         // start of precompiled memory block
global char         *PCH_End;           // end of precompiled memory block
global char         *PCH_Macros;        // macros loaded from pre-compiled header
global char         *PCH_FileName;      // name to use for pre-compiled header
global INCFILE      *IncFileList;       // list of primary include files for PCH
global SYMPTR       *PCH_SymArray;      // array of symbol table pointers from PCH
global unsigned     PCH_MaxSymHandle;   // number of symbols in PCH_SymArray
global int          DebugFlag;
global TOKEN        CurToken;
global int          BadTokenInfo;
global size_t       TokenLen;
global source_loc   TokenLoc;
global source_loc   SrcFileLoc;         /* duplicate of SrcFile->src_line */
global source_loc   CommentLoc;
global int          CurrChar;
global DATA_TYPE    ConstType;
global unsigned     Constant;
global uint64       Const64;
global FCB          *MainSrcFile;       /* primary source file being compiled */
global FCB          *SrcFile;
global char         *SrcFName;          /* source file name without suffix */
global char         *DefFName;          /* .def file name (prototypes) */
global char         *WholeFName;        /* whole file name with suffix */
global char         *ForceInclude;
#if _CPU == 370
global char         *AuxName;
#endif
global fname_list   *FNames;            /* list of file names processed */
global rdir_list    *RDirNames;         /* list of read only directorys */
global ialias_list  *IAliasNames;       /* list of include aliases */
global FILE         *ErrFile;           /* error file */
global FILE         *DefFile;           /* output for func prototypes */
global FILE         *CppFile;           /* output for preprocessor */
global FILE         *DepFile;           /* make style auto depend file */
global struct cpp_info *CppStack;       /* #if structure control stack */
global char         *IncPathList;       /* list of path names to try for include files */
global source_loc   SrcLoc;
global unsigned     SrcLineCount;       /* # of lines in primary source file */
global unsigned     IncLineCount;       /* # of lines in all included files  */
global unsigned     ErrCount;           /* total # of errors encountered     */
global unsigned     WngCount;           /* total # of warnings encountered   */
global int          WngLevel;           /* warning severity level */
global int          TypeCount;          /* total # of type nodes allocated   */
global int          GblSymCount;        /* total # of global symbols */
global int          LclSymCount;        /* total # of local and temporary symbols */
global int          FuncCount;          /* total # of functions defined in module */
global int          MacroCount;         /* total # of macros defined */
global int          TagCount;           /* total # of tag entries */
global int          FieldCount;         /* total # of struct field names */
global int          EnumCount;          /* total # of enumerated symbols */
global int          SizeOfCount;        /* # of nested sizeof() expressions  */
global int          NestLevel;          /* pre-processing level of #if */
global int          SkipLevel;          /* pre-processing level of #if to skip to */
global id_level_stype SymLevel;         /* current lex level (# of nested {) */
#define ChkEqSymLevel(p)  ((p)->level == (id_level_type)SymLevel)
#define ChkLtSymLevel(p)  ((p)->level < (id_level_type)SymLevel)
global id_hash_idx  HashValue;          /* hash value for identifier */
global mac_hash_idx MacHashValue;       /* hash value for macro name */
global char         *SavedId;           /* saved id when doing look ahead */
global id_hash_idx  SavedHash;          /* hash value for saved id */
global source_loc   SavedTokenLoc;      /* value of TokenLine when id saved */
global TOKEN        LAToken;            /* look ahead token */
global LABELPTR     LabelHead;          /* list of all labels defined in function */
global TAGPTR       TagHead;            /* list of all struct, union, enum tags */
global TAGPTR       DeadTags;           /* list of all tags that are out of scope */
global TYPEPTR      TypeHead;           /* list of all type nodes allocated */
global TYPEPTR      FuncTypeHead[MAX_PARM_LIST_HASH_SIZE + 1];
global TYPEPTR      VoidParmList[2];    /* function with void parm */
global PARMPTR      ParmList;           /* list of parms for function */
global SYM_HANDLE   GlobalSym;          /* global symbol table list head */
#if _CPU == 386
global SYM_HANDLE   SymSTOD;            /* builtin symbol for 'rep stosd' */
global SYM_HANDLE   SymSTOSB;           /* builtin symbol for '__STOSB' */
global SYM_HANDLE   SymSTOSD;           /* builtin symbol for '__STOSD' */
#endif
#ifdef __SEH__
global SYM_HANDLE   SymTryInit;         /* builtin symbol for '__TryInit' */
global SYM_HANDLE   SymTryFini;         /* builtin symbol for '__TryFini' */
global SYM_HANDLE   SymExcept;          /* builtin symbol for '__Except' */
global SYM_HANDLE   SymFinally;         /* builtin symbol for '__Finally' */
global SYM_HANDLE   SymTryUnwind;       /* builtin symbol for '__TryUnwind' */
global SYM_HANDLE   TrySymHandle;       /* builtin symbol for local try block */
global tryindex_t   TryScope;           /* current scope of _try blocks */
global tryindex_t   TryCount;           /* current number of _try blocks */
#endif
global SYM_HANDLE   SymSTOW;            /* builtin symbol for 'rep stosw' */
global SYM_HANDLE   SymSTOWB;           /* builtin symbol for 'rep stosw, stosb' */
global SYM_HANDLE   SymMIN;             /* builtin symbol for 'min(a,b)' */
global SYM_HANDLE   SymMAX;             /* builtin symbol for 'max(a,b)' */
global SYM_HANDLE   SymMEMCMP;          /* builtin symbol for 'memcmp' func */
global SYM_HANDLE   SpecialSyms;        /* builtin symbols (thread linked) */
global SYM_HANDLE   CharSymHandle;      /* sym handle for "char" typedef */
global SYM_HANDLE   Sym_CS;             /* sym handle for __segname("_CODE") ie. CS */
global SYM_HANDLE   Sym_SS;             /* sym handle for __segname("_STACK")ie. SS */
global SYM_HANDLE   SymCover;           /* sym handle for '__COVERAGE' */
global SYM_HANDLE   SymDFAbbr;          /* sym handle for '__DFABBREV' */
global SYM_HANDLE   SymChipBug;         /* sym handle for '__chipbug' */
global FIELDPTR     ErrSym;

#if _CPU == 386
global BACK_HANDLE  FunctionProfileBlock;   /* handle for profiling data block */
global segment_id   FunctionProfileSegment; /* segment for profiling data block */
#endif

global int          MacroDepth;
global char         *MacroPtr;
global MEPTR        NextMacro;
global MEPTR        UndefMacroList;
global MEPTR        *MacHash;           /* [MACRO_HASH_SIZE] */
global ENUMPTR      EnumTable[ID_HASH_SIZE];
global SYM_HASHPTR  *HashTab;
global TYPEPTR      BaseTypes[TYPE_LAST_ENTRY];
global unsigned     CTypeCounts[TYPE_LAST_ENTRY];

#define BUF_SIZE    512
global size_t       BufSize;
global char         *Buffer;
global char         *TokenBuf;

global unsigned     GenSwitches;        /* target independant switches for code generator */
global unsigned     TargetSwitches;     /* target specific code generator switches */

global unsigned     ProcRevision;       /* processor revision for c.g. */
global char         *GenCodeGroup;      /* pointer to code group name */
global unsigned     ProEpiDataSize;     /* data to be alloc'd for pro/epi hook */
global int          Toggles;            /* global toggle flags */
global unsigned     ErrLimit;

global target_size  DataThreshold;      /* sizeof(obj) > this ==> separate segment */
global unsigned     Inline_Threshold;   /* -oe=num for function inlining */

global unsigned     DataPtrSize;
global unsigned     CodePtrSize;
global int          DeadCode;           /* non-zero => next stmt is unreachable */
global int          TmpSymCount;
global int          LitCount;
global target_size  LitPoolSize;
global size_t       MacroSize;
global struct comp_flags CompFlags;
global struct global_comp_flags GlobalCompFlags;
global segment_id   SegmentNum;         /* next PRIVATE segment number to use */
global segment_id   FarStringSegment;

global jmp_buf      *Environment;       /* var for Suicide() */

#define MAX_LEVEL   1024

/* The ValueStack array is also used by CGEN for saving _try block info */
global TREEPTR      ValueStack[MAX_LEVEL];
global char         Token[MAX_LEVEL];
global token_class  Class[MAX_LEVEL];
global expr_level_type ExprLevel;

global segment_list *SegListHead;
global segment_id   SegImport;          /* next segment # for import sym */
global segment_id   SegData;            /* data segment # for -nd option */

global TREEPTR      FirstStmt;          /* root of expression tree */

global dbug_type    ScopeStruct;
global dbug_type    ScopeUnion;
global dbug_type    ScopeEnum;

global dbug_type    B_Int_1;
global dbug_type    B_UInt_1;
global dbug_type    B_Short;
global dbug_type    B_UShort;
global dbug_type    B_Int;
global dbug_type    B_UInt;
global dbug_type    B_Int32;
global dbug_type    B_UInt32;
global dbug_type    B_Int64;
global dbug_type    B_UInt64;
global dbug_type    B_Bool;

global int          OptSize;            /* 100 => make pgm small as possible */
global char         __Time[10];         /* "HH:MM:SS" for __TIME__ macro */
global char         __Date[12];         /* "MMM DD YYYY" for __DATE__ macro */

global unsigned char *MsgFlags;         /* Bit mask of disabled messages */

global struct macro_seg_list {
    struct macro_seg_list *next;
    MACADDR_T             macro_seg;
} *MacSegList;                          /* pointer to list of macro segments */

global MACADDR_T    MacroSegment;       /* segment for macro definitions */
global MACADDR_T    MacroOffset;        /* first free byte in MacroSegment */
global MACADDR_T    MacroLimit;         /* last  free byte in MacroSegment */

global int          SwitchChar;         /* DOS switch character */
global int          LoopDepth;          /* current nesting of loop constructs */
global char         CLIB_Name[10];      /* "1CLIBMT3x" */
global char         MATHLIB_Name[10];   /* "5MATHx" or "8MATH87x" */
global char         *EmuLib_Name;       /* "9emu87" for -fpi, "9noemu87" for -fpi */

#define USER_LIB_PRIO '9'

typedef struct library_list {
    struct  library_list    *next;      // used by precompiled header
    char                    libname[2]; // first char is priority '1'-'9' followed by library name
} library_list;

global library_list *HeadLibs;

typedef struct alias_list {
    struct  alias_list      *next;
    const char              *name;      /* one of 'name', 'a_sym' is valid */
    SYM_HANDLE              a_sym;
    const char              *subst;
    SYM_HANDLE              s_sym;      /* one of 'subst', 's_sym' is valid */
    char                    names[1];
} alias_list;

global alias_list   *AliasHead;

global SYMPTR       CurFunc;            /* current function being worked on */
global SYM_ENTRY    CurFuncSym;         /* for contents of current function symbol */
global SYM_HANDLE   CurFuncHandle;      /* sym_handle for current function */
global SYM_HANDLE   LastFuncOutOfMem;   /* cinfo: */
global SYM_HASHPTR  HashFreeList;       /* list of available hash entries */

global unsigned     SymBufNum;          /* current buffer in memory */
global unsigned     SymSegNum;          /* segment # containing buffer */
global unsigned     LastSymBuf;         /* # of last symbol table buffer */
global unsigned     SymBufDirty;        /* 1 => buffer has been changed */
global SEGADDR_T    SymSegment;         /* segment # for symbol table buffers */

global TYPEPTR      StringType;         /* "unsigned char *" for use by literals */
global TYPEPTR      ConstCharType;      /* "const char" type */
global TYPEPTR      StringArrayType;    /* "unsigned char []" used by literals */

typedef struct nested_parm_lists {
    struct nested_parm_lists    *prev_list;
    TYPEPTR                     *next_parm_type;
} nested_parm_lists;

global nested_parm_lists    *NestedParms;

#ifndef LARGEST_QUAD_INDEX
    #define LARGEST_QUAD_INDEX      0xFFFF
    #define LARGEST_DATA_QUAD_INDEX 0xFFFFF
#else
    #define LARGEST_DATA_QUAD_INDEX LARGEST_QUAD_INDEX
#endif
#define LARGEST_SYM_INDEX           0xFFFF

#define SYM_BUF_SIZE            1024
#define SYMS_PER_BUF            (SYM_BUF_SIZE/sizeof(SYM_ENTRY))
#define SYMBUFS_PER_SEG         16
#define SYM_SEG_SIZE            (SYM_BUF_SIZE*SYMBUFS_PER_SEG)

#define MAX_SYM_SEGS    (LARGEST_SYM_INDEX/(SYMS_PER_BUF*SYMBUFS_PER_SEG)+1)

global seg_info     SymSegs[MAX_SYM_SEGS];  /* segments for symbols */

global STR_HANDLE   StringHash[STRING_HASH_SIZE]; /* string literals */
global char         *TextSegName;       /* name of the text segment */
global char         *DataSegName;       /* name of the data segment */
global char         *CodeClassName;     /* name of the code class */
global char         *ModuleName;        /* name of module */
global char         *ObjectFileName;    /* name of object file */
global char         *DependFileName;    /* name of make style auto depend file */
global char         *DependHeaderPath;  /* path to prepend if included header has none */
global char         *DependTarget;      /* name of target in make style autodep */
global char         *SrcDepName;        /* name of first depend (sourcefile)*/
global char         DependForceSlash;   /* type of slash to force to in depend output */
global align_type   PackAmount;         /* current packing alignment */
global align_type   GblPackAmount;      /* packing alignment given on command line */
global textsegment  *TextSegList;       /* list of #pragma alloc_text segs*/

typedef enum {
    SEGTYPE_CODE       = 1,             /* #pragma code_seg("segname","class") */
    SEGTYPE_DATA       = 2,             /* #pragma data_seg("segname","class") */
    SEGTYPE_BASED      = 3,             /* __based(__segname("segname")) */
    SEGTYPE_INITFINI   = 4,             /* "XI" or "YI" segment */
    SEGTYPE_INITFINITR = 5,             /* thread data */
} seg_type;

typedef struct extref_info {
    struct extref_info  *next;
    SYM_HANDLE          symbol;
    char                name[1];
} extref_info;

global extref_info  *ExtrefInfo;

global int          (*NextChar)( void );
global void         (*UnGetChar)( int c );
global int          (*GetCharCheck)( int );

global struct debug_fwd_types *DebugNameList;

global unsigned     Column;             /* skip to Column when reading */
global unsigned     Trunc;              /* stop at Trunc when reading  */
global size_t       EnumRecSize;        /* size of largest enum entry */
global int          PreProcChar;
global TYPEPTR      PrevProtoType;      /* prev func prototype */

global align_type   SegAlignment[FIRST_PRIVATE_SEGMENT];
global unsigned     TargSys;
global segment_id   DefDataSegment;     /* #pragma data_seg("segname","class") */
global textsegment  *DefCodeSegment;    /* #pragma code_seg("seg","c") */

global unroll_type  UnrollCount;        /* #pragma unroll(#); */
global macro_flags  InitialMacroFlag;
global unsigned char Stack87;
global char         *ErrorFileName;

typedef struct undef_names {
    struct undef_names  *next;
    char                *name;
} undef_names;

global undef_names  *UndefNames;

global TREEPTR      CurFuncNode;

//================= Function Prototypes ========================

/* casian */
extern void         SetDBChar(int);

/*ccheck*/
extern bool         ChkCompatibleFunction( TYPEPTR typ1, TYPEPTR typ2, bool topLevelCheck );
extern bool         ChkCompatibleLanguage( type_modifiers typ1, type_modifiers typ2 );
extern void         TernChk( TYPEPTR typ1, TYPEPTR typ2 );
extern void         ChkCallParms(void);
extern void         ChkRetType(TREEPTR);
extern void         ChkConst(TREEPTR);
extern void         CompatiblePtrType(TYPEPTR,TYPEPTR,TOKEN);
extern bool         IdenticalType(TYPEPTR,TYPEPTR);
extern bool         VerifyType(TYPEPTR,TYPEPTR,SYMPTR);
extern TYPEPTR      SkipTypeFluff( TYPEPTR typ );
extern void         ParmAsgnCheck( TYPEPTR typ1, TREEPTR opnd2, int parmno, bool asgn_check );

//ccmain.c
extern void         FreeRDir( void );
extern void         FrontEndInit( bool reuse );
extern int          FrontEnd(char **);
extern void         FrontEndFini( void );
extern void         CppComment(int);
extern bool         CppPrinting(void);
extern void         CppPutc(int);
extern void         CppPrtf(char *,...);
extern void         SetCppWidth(unsigned);
extern void         CppPrtChar(int);
extern void         CppPrtToken(void);
extern bool         OpenSrcFile(const char *,bool);
extern void         OpenDefFile(void);
extern FILE         *OpenBrowseFile(void);
extern void         CloseFiles(void);
extern void         CClose( FILE *fp );
extern void         FreeFNames(void);
extern char         *ErrFileName(void);
extern char         *DepFileName(void);
extern char         *ObjFileName(void);
extern char         *CppFileName(void);
extern char         *ForceSlash(char *, char );
extern char         *CreateFileName( char *template, char *extension, bool forceext );
extern char         *GetSourceDepName( void );
extern FNAMEPTR     NextDependency( FNAMEPTR );

extern FNAMEPTR     AddFlist(char const *);
extern FNAMEPTR     FileIndexToFName(unsigned);
extern char         *FNameFullPath( FNAMEPTR flist );
extern char         *FileIndexToCorrectName( unsigned file_index );
extern int          FListSrcQue(void);
extern void         SrcFileReadOnlyDir( char const *dirs );
extern void         SrcFileReadOnlyFile( char const *file );
extern bool         SrcFileInRDir( FNAMEPTR flist );
extern void         SrcFileIncludeAlias( const char *alias_name, const char *real_name, bool is_lib );
extern int          SrcFileTime(char const *,time_t *);
extern void         SetSrcFNameOnce( void );
extern void         GetNextToken(void);
extern void         EmitLine(unsigned,const char *);
extern void         EmitPoundLine(unsigned,const char *,int);

extern void         AddIncFile( INCFILE * );
extern void         AddIncFileList( const char *filename );
extern void         FreeIncFileList( void );

// cdata.c
extern void         InitGlobalVars( void );

// from cdebug.c
extern dbug_type    DBType(TYPEPTR);
extern void         EmitDBType(void);

extern void         ParsePgm(void);
extern void         AdjParmType(SYMPTR sym);
extern void         Chk_Struct_Union_Enum(TYPEPTR);
extern void         Declarator( SYMPTR sym, type_modifiers mod, TYPEPTR typ, decl_state state );
extern bool         DeclList(SYM_HANDLE *);
extern FIELDPTR     FieldDecl( TYPEPTR typ, type_modifiers mod, decl_state state );
extern TYPEPTR      TypeName(void);

// cdecl2.c
extern void         InvDecl( void );
extern parm_list    *NewParm( TYPEPTR, parm_list * );
extern TYPEPTR      *MakeParmList( parm_list *, bool );
extern FIELDPTR     FieldCreate( const char *name );

// cinfo.c
extern segment_id   SymSegId( SYMPTR sym );
extern void         SetSegSymHandle( SYM_HANDLE sym_handle, segment_id segment );

/* cdinit */
extern void         InitDataQuads(void);
extern void         FreeDataQuads(void);
extern bool         DataQuadsAvailable(void);
extern void         *StartDataQuadAccess( void );
extern void         EndDataQuadAccess( void * );
extern DATA_QUAD    *NextDataQuad(void);
extern void         InitSymData(TYPEPTR,TYPEPTR,int);
extern void         StaticInit(SYMPTR,SYM_HANDLE);
extern void         VarDeclEquals(SYMPTR,SYM_HANDLE);

/* cdump */
extern void         DumpFuncDefn(void);
extern void         SymDump(void);
extern char         *DiagGetTypeName(TYPEPTR typ);

/* cems */
extern SEGADDR_T    AccessSegment(seg_info *);
extern SEGADDR_T    AllocSegment(seg_info *);

/* cenum */
extern TYPEPTR      EnumDecl(type_modifiers);
extern ENUMPTR      EnumLookup(id_hash_idx,const char *);
extern void         EnumInit(void);
extern void         FreeEnums(void);

//cerror.c
extern void         CErr1(int msgnum);
extern void         CErr2(int msgnum,int);
extern void         CErr2p(int msgnum,const char *);
extern void         CErr3p(int msgnum,const char *,const char *);
extern void         CErr4p(int msgnum,const char *,const char *,const char *);
extern void         CErrP1(int parmno,int msgnum);
extern void         SetErrLoc(source_loc *);
extern void         InitErrLoc(void);
extern void         CWarn1(int level,int msgnum);
extern void         CWarn2(int level,int msgnum,int);
extern void         CWarn2p(int level,int msgnum,const char *);
extern void         CWarnP1(int parmno,int level,int msgnum);
extern void         PCHNote( int msgnum, ... );
extern void         CInfoMsg(int,...);
extern void         CSuicide(void);
extern void         OpenErrFile(void);
extern void         FmtCMsg( char *buff, cmsg_info *info );
extern void         SetDiagSymbol(SYMPTR sym, SYM_HANDLE handle);
extern void         SetDiagEnum(ENUMPTR);
extern void         SetDiagMacro(MEPTR);
extern void         SetDiagType1(TYPEPTR typ_source);
extern void         SetDiagType2(TYPEPTR typ_target, TYPEPTR typ_source);
extern void         SetDiagType3(TYPEPTR typ_first, TYPEPTR typ_second, TOKEN opr);
extern void         SetDiagPop(void);

//  cexpr.c
extern void         ExprInit(void);
extern void         ChkCallNode( TREEPTR tree );
extern TREEPTR      Expr(void);
extern TREEPTR      AddrExpr(void);
extern TREEPTR      BoolExpr(TREEPTR);
extern TREEPTR      CommaExpr(void);
extern int          ConstExpr(void);
typedef struct {
    int64     value;
    DATA_TYPE type;
} const_val;
extern bool         ConstExprAndType(const_val *);
extern TREEPTR      SingleExpr(void);
extern TREEPTR      IntLeaf(target_int);
extern TREEPTR      RValue(TREEPTR);
extern TREEPTR      LongLeaf(target_long);
extern TREEPTR      UIntLeaf(target_uint);
extern TREEPTR      VarLeaf(SYMPTR,SYM_HANDLE);
extern TREEPTR      BasedPtrNode(TYPEPTR,TREEPTR);
extern bool         IsLValue(TREEPTR);
extern op_flags     OpFlags( type_modifiers  flags );
extern type_modifiers FlagOps( op_flags ops );
extern FIELDPTR     SearchFields( TYPEPTR *class_typ, target_size *field_offset, const char *name );

//cfold.c
extern int64        LongValue64( TREEPTR leaf );
extern void         CastFloatValue( TREEPTR leaf, DATA_TYPE newtype );
extern void         CastConstValue(TREEPTR,DATA_TYPE);
extern void         CastConstNode( TREEPTR leaf, TYPEPTR newtyp );
extern void         DoConstFold(TREEPTR);
extern void         FoldExprTree(TREEPTR);
extern bool         BoolConstExpr( void );

//cgen.c
extern void         DoCompile(void);
extern void         EmitInit(void);
extern void         EmitAbort(void);
extern int          EmitBytes(STR_HANDLE);
extern void         GenInLineFunc( SYM_HANDLE sym_handle );
extern bool         IsInLineFunc( SYM_HANDLE sym_handle );

/* cgendata */
extern void         EmitDataQuads(void);
extern void         EmitZeros(target_size);
extern void         AlignIt(TYPEPTR);

/* cgetch */
extern int          GetNextChar(void);
extern void         GetNextCharUndo(int);
extern int          GetCharCheckFile(int);
extern int          getCharAfterBackSlash(void);
extern void         CloseSrcFile(FCB *);

// cinfo.c
extern void         SegInit(void);
extern segment_id   AddSegName(const char *,const char *,int);
extern segment_id   DefThreadSeg( void );
extern void         EmitSegLabels(void);
extern void         FiniSegLabels(void);
extern void         FiniSegBacks(void);
extern void         SetSegs(void);
extern SYM_HANDLE   SegSymHandle(segment_id);
extern void         SetFuncSegment(SYMPTR,segment_id);
extern void         SetFarHuge(SYMPTR,int);
extern char         *SegClassName(segment_id);
extern hw_reg_set   *SegPeggedReg(segment_id);
extern void         SetSegment(SYMPTR);
extern void         SetSegAlign(SYMPTR);
extern void         AssignSeg(SYMPTR);

// cintmain.c
extern void         ConsErrMsg( cmsg_info  *info );
extern void         ConsErrMsgVerbatim( char const  *line );
extern void         ConsMsg( char const  *line );
extern void         BannerMsg( char const  *line );
extern void         DebugMsg( char const  *line );
extern void         NoteMsg( char const  *line );
extern void         ConBlip( void );
extern bool         ConTTY( void );
extern void         MyExit( int ret );

//cmac1.c
extern void         EnlargeBuffer(size_t);
extern void         MacroInit(void);
extern void         MacroAddComp(void);
extern void         MacroFini(void);
extern void         MacroPurge(void);
extern void         GetMacroToken(void);
extern TOKEN        SpecialMacro( special_macros spc_macro );
extern void         DoMacroExpansion(void);

//cmac2.c
extern TOKEN        ChkControl(void);
extern bool         MacroDel( const char *name );
extern void         CppStackInit( void );
extern void         CppStackFini(void);

//cmacadd.c
extern void         AllocMacroSegment(size_t);
extern void         FreeMacroSegments(void);
extern MEPTR        CreateMEntry(const char *, size_t len);
extern void         FreeMEntry( MEPTR mentry );
extern void         MacLkAdd( MEPTR mentry, size_t len, macro_flags flags );
extern void         MacroAdd( MEPTR mentry, const char *buf, size_t len, macro_flags flags );
extern int          MacroCompare(MEPTR,MEPTR);
extern void         MacroCopy(const void *,MACADDR_T,size_t);
extern MEPTR        MacroLookup(const char *);
extern void         MacroOverflow(size_t,size_t);
extern SYM_HASHPTR  SymHashAlloc(size_t);

//cmath.c
extern TREEPTR      AddOp(TREEPTR,TOKEN,TREEPTR);
extern TREEPTR      InitAsgn( TYPEPTR,TREEPTR );
extern TREEPTR      AsgnOp(TREEPTR,TOKEN,TREEPTR);
extern TREEPTR      BinOp(TREEPTR,TOKEN,TREEPTR);
extern bool         IsPtrConvSafe( TREEPTR, TYPEPTR, TYPEPTR );
extern TREEPTR      CnvOp(TREEPTR,TYPEPTR,int);
extern TREEPTR      FlowOp(TREEPTR,opr_code,TREEPTR);
extern TREEPTR      IntOp(TREEPTR,TOKEN,TREEPTR);
extern TREEPTR      RelOp(TREEPTR,TOKEN,TREEPTR);
extern TREEPTR      ShiftOp(TREEPTR,TOKEN,TREEPTR);
extern TYPEPTR      TernType(TREEPTR,TREEPTR);
extern TYPEPTR      TypeOf(TREEPTR);
extern TREEPTR      UComplement(TREEPTR);
extern TREEPTR      UMinus(TREEPTR);
extern DATA_TYPE    BinExprType(TYPEPTR,TYPEPTR);
extern DATA_TYPE    DataTypeOf(TYPEPTR);
extern bool         IsFuncPtr(TYPEPTR);
extern TREEPTR      FixupAss( TREEPTR opnd, TYPEPTR newtyp );
extern pointer_class ExprTypeClass( TYPEPTR typ );
extern TREEPTR      LCastAdj(  TREEPTR tree );

/* cmodel */
extern char         *BadCmdLine(int,const char *);
extern char         *Define_Macro(const char *);
extern char         *Define_UserMacro(const char *);
extern void         PreDefine_Macro(const char *);
extern char         *AddUndefName(const char *);
extern void         InitModInfo(void);
extern void         MiscMacroDefs(void);

//cmsg.c
extern char const   *CGetMsgStr(  msg_codes msgcode );
extern void         CGetMsg( char *msgbuf, msg_codes msgnum );
extern void         InitMsg( void );
extern void         FiniMsg( void );
extern char const   *UsageText(void);   // GET INTERNATIONAL USAGE TEXT
extern msgtype      CGetMsgType( msg_codes msgcode );
extern char const   *CGetMsgPrefix( msg_codes msgcode );

/* cname */
extern int          NameCmp(const void *,const void *,size_t);

/* coptions */
extern bool         EqualChar(int);
extern void         GenCOptions(char **);
extern void         MergeInclude(void);

/* cpragma */
extern void         CPragmaInit( void );
extern void         CPragmaFini( void );
extern bool         SetToggleFlag( char const *name, int const value );
extern void         CPragma(void);
extern textsegment  *LkSegName(const char *,const char *);
extern textsegment  *NewTextSeg(const char *,const char *,const char *);
extern void         PragmaInit(void);
extern void         PragmaFini(void);
extern void         PragmaAuxInit(void);
extern void         PragInit(void);
extern void         PragEnding(void);
extern void         PragObjNameInfo(char **);
extern bool         PragRecog(const char *);
extern hw_reg_set   PragRegList(void);
extern int          PragRegIndex(const char *,const char *,size_t,bool);
extern int          PragRegNumIndex( const char *name, int max_reg );
extern hw_reg_set   *PragManyRegSets(void);
extern void         PragCurrAlias(const char *);
extern TOKEN        PragRegSet(void);
extern void         ChkPragmas(void);
extern void         CreateAux(const char *);
extern void         SetCurrInfo(const char *);
extern void         XferPragInfo(const char*,const char*);
extern void         EnableDisableMessage(int,unsigned);
extern void         AddLibraryName( const char *, const char );
extern void         AddExtRefN( const char * );
extern void         AddExtRefS( SYM_HANDLE );
extern void         SetPackAmount( unsigned amount );

/* cprag??? */
extern void         AsmStmt(void);
extern void         PragAux(void);
extern hw_reg_set   PragRegName(const char *, size_t);

/* cpurge */
extern void         InitPurge(void);
extern void         PurgeMemory(void);

/* cscan */
extern void         ScanInit( void );
extern bool         InitPPScan( void );
extern void         FiniPPScan( bool );
extern id_hash_idx  CalcHash( const char *, size_t );
extern unsigned     hashpjw( const char * );
extern int          ESCChar( int, const unsigned char **, bool * );
extern void         SkipAhead( void );
extern TOKEN        ScanToken( void );
extern void         ReScanInit( const char * );
extern bool         ReScanToken( void );
extern char         *ReScanPos( void );
extern TOKEN        KwLookup( const char *, size_t );
extern TOKEN        IdLookup( const char *, size_t );
extern TOKEN        NextToken( void );
extern TOKEN        PPNextToken( void );

/* csizeof */
extern target_size  SizeOfArg(TYPEPTR);

/* cstats */
extern void         InitStats(void);
extern void         PrintStats(void);

/* cstmt */
extern void         LookAhead(void);
extern void         Statement(void);
extern void         AddStmt(TREEPTR);
extern void         GenFunctionNode(SYM_HANDLE);
extern LABEL_INDEX  NextLabel(void);
extern void         StmtInit( void );

/* cstring */
extern void         FreeLiteral(STR_HANDLE);
extern STR_HANDLE   GetLiteral(void);
extern void         LoadUnicodeTable(unsigned);
extern void         StringInit(void);
extern TREEPTR      StringLeaf(string_flags);

/* csym */
extern void         SymInit(void);
extern void         SpcSymInit(void);
extern void         SymFini(void);
extern void         SymCreate(SYMPTR,const char *);
extern SYM_HANDLE   SegSymbol(const char *,segment_id);
extern SYM_HANDLE   SpcSymbol(const char *,TYPEPTR,stg_classes);
extern SYM_HANDLE   SymAdd(id_hash_idx,SYMPTR);
extern SYM_HANDLE   SymAddL0(id_hash_idx,SYMPTR);
extern SYM_HANDLE   SymLook(id_hash_idx,const char *);
extern SYM_HANDLE   Sym0Look(id_hash_idx,const char *);
extern SYM_HANDLE   SymLookTypedef(id_hash_idx,const char *,SYMPTR);
extern void         SymGet(SYMPTR,SYM_HANDLE);
extern SYMPTR       SymGetPtr(SYM_HANDLE);
extern void         SymReplace(SYMPTR,SYM_HANDLE);
extern void         EndBlock(void);
extern SYM_HANDLE   MakeFunction(const char *,TYPEPTR);
extern SYM_HANDLE   MakeNewSym(SYMPTR,char,TYPEPTR,stg_classes);
extern LABELPTR     LkLabel(const char *);
extern void         FreeLabels(void);
extern XREFPTR      NewXref(XREFPTR);
extern void         FreeXrefs(XREFPTR);
extern unsigned     SymGetNumSyms( void );
extern unsigned     SymGetNumSpecialSyms( void );
extern SYM_HANDLE   SymGetFirst( void );
extern SYM_HANDLE   SymGetNext( SYM_HANDLE sym_handle );

/* ctimepc */
extern void         TimeInit(void);

//ctype.c
extern void         CTypeInit(void);
extern void         InitTypeHashTables(void);
extern void         SetSignedChar(void);
extern TYPEPTR      GetType(DATA_TYPE);
extern TYPEPTR      ArrayNode(TYPEPTR);
extern TYPEPTR      FuncNode(TYPEPTR, type_modifiers, TYPEPTR *);
extern TYPEPTR      TypeDefault(void);
extern TYPEPTR      PtrNode(TYPEPTR,type_modifiers,segment_id);
extern TYPEPTR      BPtrNode(TYPEPTR,type_modifiers,segment_id,SYM_HANDLE, BASED_KIND);
extern TYPEPTR      TypeNode(DATA_TYPE,TYPEPTR);
extern type_modifiers TypeQualifier(void);
extern void         TypeSpecifier( decl_info *info );
extern void         FullDeclSpecifier( decl_info *info );
extern TAGPTR       NullTag(void);
extern TAGPTR       TagLookup(void);
extern void         FreeTags(void);
extern target_size  TypeSize(TYPEPTR);
extern target_size  TypeSizeEx( TYPEPTR, bitfield_width *pFieldWidth );
extern TYPEPTR      GetIntTypeBySize( target_size size, bool sign, bool exact );
extern TAGPTR       VfyNewTag( TAGPTR, DATA_TYPE );
extern void         VfyNewSym( id_hash_idx, const char * );
extern align_type   GetTypeAlignment( TYPEPTR );
extern void         TypesPurge( void );
extern void         AddTypeHash( TYPEPTR );
extern void         AddPtrTypeHash( TYPEPTR );

/* cusage */
extern void         CCusage( void );

/* cutil */
extern void         CErrSymName( int, SYMPTR,SYM_HANDLE );
extern void         Expecting( char * );
extern bool         ExpectingToken( TOKEN );
extern void         ExpectingAfter( TOKEN, TOKEN );
extern bool         ExpectingConstant( void );
extern void         ExpectEndOfLine( void );
extern void         ExpectIdentifier( void );
extern void         ExpectString( void );
extern void         ExpectStructUnionTag( void );
extern void         MustRecog( TOKEN );
extern SYM_NAMEPTR  SymName( SYMPTR, SYM_HANDLE );

/* dwarf */
extern void         DwarfBrowseEmit( void );

/* ftoa */
extern char         *ftoa( FLOATVAL * );

/* jis2unic */
extern unsigned int JIS2Unicode( unsigned );

// pchdr.c
extern int          UsePreCompiledHeader( const char * );
extern void         InitBuildPreCompiledHeader( void );
extern void         BuildPreCompiledHeader( const char * );
extern void         FreePreCompiledHeader( void );

/* watcom */
extern void         CBanner( void );

// cstmt.c
extern SYM_HANDLE   GetBlockSymList( void );
extern void         InitStmt( void );
extern void         SwitchPurge( void );

/* Macro to skip all typedefs and arrive at the underlying type */
#define SKIP_TYPEDEFS( typeptr )                        \
    while( typeptr->decl_type == TYPE_TYPEDEF ) {       \
        typeptr = typeptr->object;                      \
    }
#define SKIP_DUMMY_TYPEDEFS( typeptr )                  \
    while( typeptr->decl_type == TYPE_TYPEDEF && (typeptr->type_flags & TF2_DUMMY_TYPEDEF) ) { \
        typeptr = typeptr->object;                      \
    }
#define SKIP_ENUM( typeptr )                            \
    if( typeptr->decl_type == TYPE_ENUM ) {             \
        typeptr = typeptr->object;                      \
    }
#define SKIP_ARRAYS( typeptr )                          \
    while( typeptr->decl_type == TYPE_ARRAY ) {         \
        typeptr = typeptr->object;                      \
    }
