%{
#include "compiler.hpp"
#include "logger.hpp"

void yyerror(const char *);
int yylex();
%}

%code requires
{
  struct Expr;
}

%union
{
  int int_val;
  char* str_val;
  Expr* expr_val;
}

/** Comparison Tokens */
%token TOK_EQ
%token TOK_GT
%token TOK_GTE
%token TOK_NEQ
%token TOK_LT
%token TOK_LTE

/** Data Tokens */
%token TOK_CHAR
%token TOK_IDENTIFIER
%token TOK_INTEGER
%token TOK_STRING

/** Keyword Tokens */
%token TOK_ARRAY
%token TOK_BEGIN
%token TOK_CHR
%token TOK_CONST
%token TOK_DO
%token TOK_DOWN_TO
%token TOK_ELSE
%token TOK_ELSE_IF
%token TOK_END
%token TOK_FOR
%token TOK_FORWARD
%token TOK_FUNCTION
%token TOK_IF
%token TOK_OF
%token TOK_ORD
%token TOK_PRED
%token TOK_PROCEDURE
%token TOK_READ
%token TOK_RECORD
%token TOK_REF
%token TOK_REPEAT
%token TOK_RETURN
%token TOK_STOP
%token TOK_SUCC
%token TOK_THEN
%token TOK_TO
%token TOK_TYPE
%token TOK_UNTIL
%token TOK_VAR
%token TOK_WHILE
%token TOK_WRITE

/** Logic Tokens */
%token TOK_AND
%token TOK_OR
%token TOK_NOT

/** Operator Tokens */
%token TOK_ASSIGN
%token TOK_DIVIDE
%token TOK_MINUS
%token TOK_MODULO
%token TOK_MULTIPLY
%token TOK_PLUS
%token TOK_UNARY_MINUS

/** Symbol Tokens */
%token TOK_BRACKET_L
%token TOK_BRACKET_R
%token TOK_COLON
%token TOK_COMMA
%token TOK_DOT
%token TOK_PARENTHESIS_L
%token TOK_PARENTHESIS_R
%token TOK_SEMICOLON

/** Associativity and Precedence */
%left TOK_OR TOK_AND
%right TOK_NOT
%nonassoc TOK_GTE TOK_GT TOK_LTE TOK_LT TOK_NEQ TOK_EQ
%left TOK_MINUS TOK_PLUS TOK_MODULO TOK_DIVIDE TOK_MULTIPLY
%right TOK_UNARY_MINUS

/** Types */
%type <int_val> TOK_CHAR
%type <expr_val> ConstExpr
%type <expr_val> Expr
%type <int_val> ForCondition
%type <expr_val> LValue
%type <int_val> SimpleType
%type <int_val> TOK_INTEGER
%type <str_val> TOK_IDENTIFIER
%type <str_val> TOK_STRING
%type <int_val> Type
%type <expr_val> VarExpr


%%

Program : OptConstExprs OptTypeExprs OptVarExprs ProcOrFuncExprs Block TOK_DOT { endProgram(); };

OptConstExprs : TOK_CONST ConstExprs | ;
ConstExprs : ConstExprs ConstExpr | ConstExpr;
ConstExpr : TOK_IDENTIFIER TOK_EQ Expr TOK_SEMICOLON { addConst($1, $3); };

ProcOrFuncExprs : ProcOrFuncExprs ProcOrFunc | ;
ProcOrFunc : Procedure | Function;

Procedure : TOK_PROCEDURE ProcFuncCommon1 ProcFuncCommon2;

Function : TOK_FUNCTION ProcFuncCommon1 TOK_COLON Type ProcFuncCommon2;

ProcFuncCommon1 : TOK_IDENTIFIER TOK_PARENTHESIS_L OptFormalParams TOK_PARENTHESIS_R;
ProcFuncCommon2 : TOK_SEMICOLON ProcFuncCommon3 TOK_SEMICOLON;
ProcFuncCommon3 : TOK_FORWARD | Body;

OptFormalParams : FormalParams | ;
FormalParams : FormalParams TOK_SEMICOLON FormalParam | FormalParam;
FormalParam : OptVarRef IdentList TOK_COLON Type;

OptVarRef : TOK_VAR | TOK_REF | ;

Body : OptConstExprs OptTypeExprs OptVarExprs Block;

Block : TOK_BEGIN Statements TOK_END { noop(); }; 

OptTypeExprs : TOK_TYPE TypeExprs | ;
TypeExprs : TypeExprs TypeExpr | TypeExpr;
TypeExpr : TOK_IDENTIFIER TOK_EQ Type TOK_SEMICOLON { noop("Type Expr"); };
Type : SimpleType { $$ = $1; }
     | RecordType { $$ = noop(); }
     | ArrayType  { $$ = noop(); };

SimpleType: TOK_IDENTIFIER { $$ = simpleType($1); };

RecordType : TOK_RECORD OptFields TOK_END { noop("Record Type"); };

OptFields : OptFields Field | ;
Field: IdentList TOK_COLON Type TOK_SEMICOLON { noop("Field"); };

ArrayType : TOK_ARRAY TOK_BRACKET_L Expr TOK_COLON Expr TOK_BRACKET_R TOK_OF Type { noop("Array Type"); };

OptVarExprs : TOK_VAR VarExprs | ;
VarExprs : VarExprs VarExpr | VarExpr;
VarExpr : IdentList TOK_COLON Type TOK_SEMICOLON { addVars($3); };

IdentList : IdentList TOK_COMMA TOK_IDENTIFIER { addId($3); }
          | TOK_IDENTIFIER                     { addId($1); };

Statements : Statements TOK_SEMICOLON Statement { noop(); }
           | Statement                          { noop(); };

Statement : Assignment
          | IfStatement
          | WhileStatement
          | RepeatStatement
          | ForStatement
          | StopStatement
          | ReturnStatement
          | ReadStatement
          | WriteStatement
          | ProcedureCall
          | ;

Assignment : LValue TOK_ASSIGN Expr { assignExpr($1, $3); };

IfStatement : If Then ElseIfs Else TOK_END { ifEnd(); };

If : IfBegin Expr { ifCondition($2); };

IfBegin : TOK_IF { ifBegin(); };

Then : TOK_THEN Statements { ifThen(); };

ElseIfs : ElseIfs ElseIf Then | ;

ElseIf : TOK_ELSE_IF Expr { ifCondition($2); };

Else : TOK_ELSE Statements | ;

WhileStatement : WhileBegin WhileCondition TOK_DO Statements TOK_END { whileEnd(); };

WhileBegin : TOK_WHILE { whileBegin(); };

WhileCondition : Expr { whileCondition($1); };

RepeatStatement : RepeatBegin Statements RepeatCondition;

RepeatBegin : TOK_REPEAT { repeatBegin(); };

RepeatCondition : TOK_UNTIL Expr { repeatCondition($2); };

ForStatement: ForInit ForCondition TOK_DO Statements TOK_END { forCounter($2); };

ForInit : TOK_FOR TOK_IDENTIFIER TOK_ASSIGN Expr { forInit($2, $4); };

ForCondition : TOK_TO Expr      { $$ = forTo($2); }
             | TOK_DOWN_TO Expr { $$ = forDownTo($2); };

StopStatement : TOK_STOP { noop("Stop"); };

ReturnStatement : TOK_RETURN OptExprs { noop("Return Optional Expressions"); };

ReadStatement : TOK_READ TOK_PARENTHESIS_L ReadExprs TOK_PARENTHESIS_R;

ReadExprs : ReadExprs TOK_COMMA LValue { readExpr($3); }
          | LValue                     { readExpr($1); };

WriteStatement : TOK_WRITE TOK_PARENTHESIS_L WriteExprs TOK_PARENTHESIS_R;

WriteExprs : WriteExprs TOK_COMMA Expr { writeExpr($3); }
           | Expr                      { writeExpr($1); };

ProcedureCall : TOK_IDENTIFIER TOK_PARENTHESIS_L OptExprs TOK_PARENTHESIS_R { noop("Procedure Call"); };

OptExprs : Exprs { noop("Optional Expressions"); }
         |       { noop("No Optional Expressions"); };
         
Exprs : Exprs TOK_COMMA Expr
      | Expr;

Expr : Expr TOK_AND Expr                                           { $$ = andExpr($1, $3); }
     | Expr TOK_DIVIDE Expr                                        { $$ = divExpr($1, $3); }
     | Expr TOK_EQ Expr                                            { $$ = eqExpr($1, $3); }
     | Expr TOK_GT Expr                                            { $$ = gtExpr($1, $3); }
     | Expr TOK_GTE Expr                                           { $$ = gteExpr($1, $3); }
     | Expr TOK_LT Expr                                            { $$ = ltExpr($1, $3); }
     | Expr TOK_LTE Expr                                           { $$ = lteExpr($1, $3); }
     | Expr TOK_MINUS Expr                                         { $$ = subExpr($1, $3); }
     | Expr TOK_MODULO Expr                                        { $$ = modExpr($1, $3); }
     | Expr TOK_MULTIPLY Expr                                      { $$ = multExpr($1, $3); }
     | Expr TOK_NEQ Expr                                           { $$ = neqExpr($1, $3); }
     | Expr TOK_OR Expr                                            { $$ = orExpr($1, $3); }
     | Expr TOK_PLUS Expr                                          { $$ = addExpr($1, $3); }
     | TOK_CHAR                                                    { $$ = charExpr($1); }
     | TOK_CHR TOK_PARENTHESIS_L Expr TOK_PARENTHESIS_R            { $$ = chrExpr($3); }
     | TOK_IDENTIFIER TOK_PARENTHESIS_L OptExprs TOK_PARENTHESIS_R { $$ = expr("Function Call"); }
     | LValue                                                      { $$ = loadExpr($1); }
     | TOK_INTEGER                                                 { $$ = intExpr($1); }
     | TOK_MINUS Expr %prec TOK_UNARY_MINUS                        { $$ = negExpr($2); }
     | TOK_NOT Expr                                                { $$ = notExpr($2); }
     | TOK_ORD TOK_PARENTHESIS_L Expr TOK_PARENTHESIS_R            { $$ = ordExpr($3); }
     | TOK_PARENTHESIS_L Expr TOK_PARENTHESIS_R                    { $$ = $2; }
     | TOK_PRED TOK_PARENTHESIS_L Expr TOK_PARENTHESIS_R           { $$ = predExpr($3); }
     | TOK_STRING                                                  { $$ = strExpr($1); }
     | TOK_SUCC TOK_PARENTHESIS_L Expr TOK_PARENTHESIS_R           { $$ = succExpr($3); };

LValue : LValue TOK_DOT TOK_IDENTIFIER           { $$ = expr("Lvalue.Property"); }     
       | LValue TOK_BRACKET_L Expr TOK_BRACKET_R { $$ = expr("Lvalue[Expr]"); }
       | TOK_IDENTIFIER                          { $$ = lvalueExpr($1); };

%%

void yyerror(const char* s)
{
  extern char *yytext;	// defined and maintained in scanner.cpp

  logger::compileError(std::string(s) + " at symbol '" + std::string(yytext) + "'");
  exit(1);
}
