%{
//-- don't change *any* of these: if you do, you'll break the compiler.
#include <algorithm>
#include <memory>
#include <cstring>
#include <cdk/compiler.h>
#include <cdk/types/types.h>
#include ".auto/all_nodes.h"
#define LINE                         compiler->scanner()->lineno()
#define yylex()                      compiler->scanner()->scan()
#define yyerror(compiler, s)         compiler->scanner()->error(s)
//-- don't change *any* of these --- END!
%}

%parse-param {std::shared_ptr<cdk::compiler> compiler}

%union {
  //--- don't change *any* of these: if you do, you'll break the compiler.
  YYSTYPE() : type(cdk::primitive_type::create(0, cdk::TYPE_VOID)) {}
  ~YYSTYPE() {}
  YYSTYPE(const YYSTYPE &other) { *this = other; }
  YYSTYPE& operator=(const YYSTYPE &other) { type = other.type; return *this; }

  std::shared_ptr<cdk::basic_type> type;        /* expression type */
  //-- don't change *any* of these --- END!

  int                                            i;          /* integer value */
  double                                         d;          /* integer value */
  std::string                                   *s;          /* symbol name or string literal */
  cdk::basic_node                               *node;       /* node pointer */
  cdk::sequence_node                            *sequence;
  cdk::expression_node                          *expression; /* expression nodes */
  cdk::lvalue_node                              *lvalue;
  til::block_node                               *block;
  std::vector<std::shared_ptr<cdk::basic_type>> *type_vec;
};

%token <i> tINTEGER
%token <d> tDOUBLE
%token <s> tIDENTIFIER tSTRING
%token tTYPE_INT tTYPE_DOUBLE tTYPE_STRING tTYPE_VOID
%token tEXTERNAL tFORWARD tPUBLIC tPRIVATE tVAR
%token tBLOCK tIF tLOOP tSTOP tNEXT tRETURN tPRINT tPRINTLN
%token tREAD tNULL tINDEX tOBJECTS tSIZEOF tFUNCTION
%token tLE tGE tEQ tNE tAND tOR tSET
%token tPROGRAM

%type <node> file_decl decl program func_arg instr
%type <sequence> file_decls decls func_args instrs exprs
%type <expression> func_def expr
%type <lvalue> lval
%type <block> decls_instrs block
%type <type> type referable_type ptr_to_type func_type func_return_type void_ptr_type
%type <type_vec> types

%{
//-- The rules below will be included in yyparse, the main parsing function.
%}
%%

file : file_decls program { compiler->ast(new cdk::sequence_node(LINE, $2, $1)); }
     | file_decls         { compiler->ast($1); }
     |            program { compiler->ast(new cdk::sequence_node(LINE, $1)); }
     | /* empty */        { compiler->ast(new cdk::sequence_node(LINE)); }
     ;

file_decls : file_decls file_decl { $$ = new cdk::sequence_node(LINE, $2, $1); }
           |            file_decl { $$ = new cdk::sequence_node(LINE, $1); }
           ;

file_decl : '(' tEXTERNAL type tIDENTIFIER      ')' { $$ = new til::declaration_node(LINE, tEXTERNAL, $3, *$4, nullptr); delete $4; }
          | '(' tFORWARD  type tIDENTIFIER      ')' { $$ = new til::declaration_node(LINE, tFORWARD,  $3, *$4, nullptr); delete $4; }
          | '(' tPUBLIC   type tIDENTIFIER      ')' { $$ = new til::declaration_node(LINE, tPUBLIC,   $3, *$4, nullptr); delete $4; }
          | '(' tPUBLIC   type tIDENTIFIER expr ')' { $$ = new til::declaration_node(LINE, tPUBLIC,   $3, *$4,      $5); delete $4; }
          | '(' tPUBLIC   tVAR tIDENTIFIER expr ')' { $$ = new til::declaration_node(LINE, tPUBLIC,   nullptr, *$4, $5); delete $4; }
          | '(' tPUBLIC        tIDENTIFIER expr ')' { $$ = new til::declaration_node(LINE, tPUBLIC,   nullptr, *$3, $4); delete $3; }
          | decl                                    { $$ = $1; }
          ;

decls : decls decl { $$ = new cdk::sequence_node(LINE, $2, $1); }
      |       decl { $$ = new cdk::sequence_node(LINE, $1); }
      ;

decl : '(' type tIDENTIFIER      ')' { $$ = new til::declaration_node(LINE, tPRIVATE, $2, *$3, nullptr); delete $3; }
     | '(' type tIDENTIFIER expr ')' { $$ = new til::declaration_node(LINE, tPRIVATE, $2, *$3,      $4); delete $3; }
     | '(' tVAR tIDENTIFIER expr ')' { $$ = new til::declaration_node(LINE, tPRIVATE, nullptr, *$3, $4); delete $3; }
     ;

program : '(' tPROGRAM decls_instrs ')' { $$ = new til::program_node(LINE, $3); }
        ;

decls_instrs : decls instrs { $$ = new til::block_node(LINE, $1,                           $2); }
             | decls        { $$ = new til::block_node(LINE, $1,                           new cdk::sequence_node(LINE)); }
             |       instrs { $$ = new til::block_node(LINE, new cdk::sequence_node(LINE), $1); }
             | /* empty */  { $$ = new til::block_node(LINE, new cdk::sequence_node(LINE), new cdk::sequence_node(LINE)); }
             ;

func_def : '(' tFUNCTION '(' func_return_type func_args ')' decls_instrs ')' { $$ = new til::function_node(LINE, $5,                           $4, $7); }
         | '(' tFUNCTION '(' func_return_type           ')' decls_instrs ')' { $$ = new til::function_node(LINE, new cdk::sequence_node(LINE), $4, $6); }
         ;

func_args : func_args func_arg { $$ = new cdk::sequence_node(LINE, $2, $1); }
          |           func_arg { $$ = new cdk::sequence_node(LINE, $1); }
          ;

func_arg : '(' type tIDENTIFIER ')' { $$ = new til::declaration_node(LINE, tPRIVATE, $2, *$3, nullptr); delete $3; }
         ;

types : types type { $$ = $1; $$->push_back($2); }
      |       type { $$ = new std::vector<std::shared_ptr<cdk::basic_type>>(1, $1); }
      ;

type : referable_type { $$ = $1; }
     | void_ptr_type  { $$ = $1; }
     ;

referable_type : tTYPE_INT    { $$ = cdk::primitive_type::create(4, cdk::TYPE_INT); }
               | tTYPE_DOUBLE { $$ = cdk::primitive_type::create(8, cdk::TYPE_DOUBLE); }
               | tTYPE_STRING { $$ = cdk::primitive_type::create(4, cdk::TYPE_STRING); }
               | ptr_to_type  { $$ = $1; }
               | func_type    { $$ = $1; }
               ;

ptr_to_type : referable_type '!' { $$ = cdk::reference_type::create(4, $1); }
            ;

func_type : '(' func_return_type               ')' { $$ = cdk::functional_type::create($2); }
          | '(' func_return_type '(' types ')' ')' { $$ = cdk::functional_type::create(*$4, $2); delete $4; }

func_return_type : type       { $$ = $1; }
                 | tTYPE_VOID { $$ = cdk::primitive_type::create(0, cdk::TYPE_VOID); }
                 ;

void_ptr_type : void_ptr_type '*' { $$ = $1; }
              | tTYPE_VOID '*'    { $$ = cdk::reference_type::create(4, cdk::primitive_type::create(0, cdk::TYPE_VOID)); }

block : '(' tBLOCK decls_instrs ')' { $$ = $3; }

instrs : instrs instr { $$ = new cdk::sequence_node(LINE, $2, $1); }
       |        instr { $$ = new cdk::sequence_node(LINE, $1); }
       ;

instr : expr                           { $$ = new til::evaluation_node(LINE, $1); }
      | '(' tPRINT   exprs    ')'      { $$ = new til::print_node(LINE, $3, false); }
      | '(' tPRINTLN exprs    ')'      { $$ = new til::print_node(LINE, $3, true); }
      | '(' tSTOP    tINTEGER ')'      { $$ = new til::stop_node(LINE, $3); }
      | '(' tSTOP             ')'      { $$ = new til::stop_node(LINE, 1); }
      | '(' tNEXT    tINTEGER ')'      { $$ = new til::next_node(LINE, $3); }
      | '(' tNEXT             ')'      { $$ = new til::next_node(LINE, 1); }
      | '(' tRETURN  expr     ')'      { $$ = new til::return_node(LINE, $3); }
      | '(' tRETURN           ')'      { $$ = new til::return_node(LINE, nullptr); }
      | '(' tIF   expr instr instr ')' { $$ = new til::if_else_node(LINE, $3, $4, $5); }
      | '(' tIF   expr instr       ')' { $$ = new til::if_node(LINE, $3, $4); }
      | '(' tLOOP expr instr       ')' { $$ = new til::loop_node(LINE, $3, $4); }
      | block                          { $$ = $1; }
      ;

exprs : exprs expr { $$ = new cdk::sequence_node(LINE, $2, $1); }
      |       expr { $$ = new cdk::sequence_node(LINE, $1); }
      ;

expr : // Literal expressions:
       tINTEGER { $$ = new cdk::integer_node(LINE, $1); }
     | tDOUBLE  { $$ = new cdk::double_node(LINE, $1); }
     | tSTRING  { $$ = new cdk::string_node(LINE, $1); }
     | tNULL    { $$ = new til::null_ptr_node(LINE); }
       // Functions invocation:
     | '(' expr exprs ')' { $$ = new til::function_call_node(LINE, $2,      $3); }
     | '(' expr       ')' { $$ = new til::function_call_node(LINE, $2,      new cdk::sequence_node(LINE)); }
     | '(' '@'  exprs ')' { $$ = new til::function_call_node(LINE, nullptr, $3); }
     | '(' '@'        ')' { $$ = new til::function_call_node(LINE, nullptr, new cdk::sequence_node(LINE)); }
       // Identifier, read, functions:
     | lval          { $$ = new cdk::rvalue_node(LINE, $1); }
     | '(' tREAD ')' { $$ = new til::read_node(LINE); }
     | func_def      { $$ = $1; }
       // Operators evaluation table:
     | '(' '+'  expr      ')' { $$ = new cdk::unary_plus_node(LINE, $3); }
     | '(' '-'  expr      ')' { $$ = new cdk::unary_minus_node(LINE, $3); }
     | '(' '?'  lval      ')' { $$ = new til::address_of_node(LINE, $3); }
     | '(' '*'  expr expr ')' { $$ = new cdk::mul_node(LINE, $3, $4); }
     | '(' '/'  expr expr ')' { $$ = new cdk::div_node(LINE, $3, $4); }
     | '(' '%'  expr expr ')' { $$ = new cdk::mod_node(LINE, $3, $4); }
     | '(' '+'  expr expr ')' { $$ = new cdk::add_node(LINE, $3, $4); }
     | '(' '-'  expr expr ')' { $$ = new cdk::sub_node(LINE, $3, $4); }
     | '(' '<'  expr expr ')' { $$ = new cdk::lt_node(LINE, $3, $4); }
     | '(' '>'  expr expr ')' { $$ = new cdk::gt_node(LINE, $3, $4); }
     | '(' tLE  expr expr ')' { $$ = new cdk::le_node(LINE, $3, $4); }
     | '(' tGE  expr expr ')' { $$ = new cdk::ge_node(LINE, $3, $4); }
     | '(' tEQ  expr expr ')' { $$ = new cdk::eq_node(LINE, $3, $4); }
     | '(' tNE  expr expr ')' { $$ = new cdk::ne_node(LINE, $3, $4); }
     | '(' '~'  expr      ')' { $$ = new cdk::not_node(LINE, $3); }
     | '(' tAND expr expr ')' { $$ = new cdk::and_node(LINE, $3, $4); }
     | '(' tOR  expr expr ')' { $$ = new cdk::or_node(LINE, $3, $4); }
     | '(' tSET lval expr ')' { $$ = new cdk::assignment_node(LINE, $3, $4); }
       // Memory reservation, dimension:
     | '(' tOBJECTS  expr ')' { $$ = new til::objects_node(LINE, $3); }
     | '(' tSIZEOF   expr ')' { $$ = new til::sizeof_node(LINE, $3); }
     ;

lval : tIDENTIFIER              { $$ = new cdk::variable_node(LINE, $1); }
     | '(' tINDEX expr expr ')' { $$ = new til::ptr_index_node(LINE, $3, $4); }
     ;

%%
