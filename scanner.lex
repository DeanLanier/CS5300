%{
#include "logger.hpp"
#include "parser.hpp"
%}

DIGIT  [0-9]
HEX    [0-9a-fA-F]
OCTAL  [0-7]
LETTER [A-Za-z]

%option nounput

%%

array|ARRAY           { return TOK_ARRAY; }
begin|BEGIN           { return TOK_BEGIN; }
chr|CHR               { return TOK_CHR; }
const|CONST           { return TOK_CONST; }
do|DO                 { return TOK_DO; }
downto|DOWNTO         { return TOK_DOWN_TO; }
else|ELSE             { return TOK_ELSE; }
elseif|ELSEIF         { return TOK_ELSE_IF; }
end|END               { return TOK_END; }
for|FOR               { return TOK_FOR; }
forward|FORWARD       { return TOK_FORWARD; }
function|FUNCTION     { return TOK_FUNCTION; }
if|IF                 { return TOK_IF; }
of|OF                 { return TOK_OF; }
ord|ORD               { return TOK_ORD; }
pred|PRED             { return TOK_PRED; }
procedure|PROCEDURE   { return TOK_PROCEDURE; }
read|READ             { return TOK_READ; }
record|RECORD         { return TOK_RECORD; }
ref|REF               { return TOK_REF; }
repeat|REPEAT         { return TOK_REPEAT; }
return|RETURN         { return TOK_RETURN; }
stop|STOP             { return TOK_STOP; }
succ|SUCC             { return TOK_SUCC; }
then|THEN             { return TOK_THEN; }
to|TO                 { return TOK_TO; }
type|TYPE             { return TOK_TYPE; }
until|UNTIL           { return TOK_UNTIL; }
var|VAR               { return TOK_VAR; }
while|WHILE           { return TOK_WHILE; }
write|WRITE           { return TOK_WRITE; }

"("  { return TOK_PARENTHESIS_L; }
")"  { return TOK_PARENTHESIS_R; }
"["  { return TOK_BRACKET_L; }
"]"  { return TOK_BRACKET_R; }
"."  { return TOK_DOT; }
"*"  { return TOK_MULTIPLY; }
"/"  { return TOK_DIVIDE; }
"%"  { return TOK_MODULO; }
"+"  { return TOK_PLUS; }
"-"  { return TOK_MINUS; }
":=" { return TOK_ASSIGN; }
"="  { return TOK_EQ; }
"<>" { return TOK_NEQ; }
"<=" { return TOK_LTE; }
"<"  { return TOK_LT; }
">=" { return TOK_GTE; }
">"  { return TOK_GT; }
"&"  { return TOK_AND; }
"|"  { return TOK_OR; }
"~"  { return TOK_NOT; }
":"  { return TOK_COLON; }
";"  { return TOK_SEMICOLON; }
","  { return TOK_COMMA; }

{LETTER}({LETTER}|{DIGIT}|_)* { yylval.str_val = strdup(yytext); return TOK_IDENTIFIER; }

0{OCTAL}*  { yylval.int_val = strtol(yytext, nullptr, 0); return TOK_INTEGER; }
0x{HEX}+   { yylval.int_val = strtol(yytext, nullptr, 0); return TOK_INTEGER; }
{DIGIT}+   { yylval.int_val = strtol(yytext, nullptr, 0); return TOK_INTEGER; }

'\\b'     { yylval.int_val = '\b'; return TOK_CHAR; }
'\\f'     { yylval.int_val = '\f'; return TOK_CHAR; }
'\\n'     { yylval.int_val = '\n'; return TOK_CHAR; }
'\\r'     { yylval.int_val = '\r'; return TOK_CHAR; }
'\\t'     { yylval.int_val = '\t'; return TOK_CHAR; }
'[^\\\n]' { yylval.int_val = yytext[1]; return TOK_CHAR; }
'\\.'     { yylval.int_val = yytext[2]; return TOK_CHAR; }

\"([^\\\n\"]|\\[^\n\"])*\" { yylval.str_val = strdup(yytext); return TOK_STRING; }

\$.*   {}
\n     { logger::incLineNumber(); }
[ \t]+ {}
.      { logger::compileError(std::string("Unknown Token: ") + yytext); }

%%
