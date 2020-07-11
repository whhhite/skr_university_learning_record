/*
 *  The scanner definition for COOL.
 */

/*
 *  Stuff enclosed in %{ %} in the first section is copied verbatim to the
 *  output, so headers and global definitions are placed here to be visible
 * to the code in the file.  Don't remove anything that was here initially
 */
%{
#include <cool-parse.h>
#include <stringtab.h>
#include <utilities.h>
#include <string.h>
/* The compiler assumes these identifiers. */
#define yylval cool_yylval
#define yylex  cool_yylex

/* Max size of string constants */
#define MAX_STR_CONST 1025
#define YY_NO_UNPUT   /* keep g++ happy */

extern FILE *fin; /* we read from this file */

/* define YY_INPUT so we read from the FILE fin:
 * This change makes it possible to use this scanner in
 * the Cool compiler.
 */
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
	if ( (result = fread( (char*)buf, sizeof(char), max_size, fin)) < 0) \
		YY_FATAL_ERROR( "read() in flex scanner failed");

char string_buf[MAX_STR_CONST]; /* to assemble string constants */
char *string_buf_ptr;

extern int curr_lineno;
extern int verbose_flag;
static int comment_count=0;

extern YYSTYPE cool_yylval;

/*
 *  Add Your own definitions here
 */

%}

/*
 * Define names for regular expressions here.
 */

DARROW          =>
ASSIGN			<-
LE				<=
DIGIT           [0,9]

%%

 /*
  *  Nested comments
  */

<INITIAL,COMMENTS,INLINE_COMMENTS>"(*"{
	comment_count++;
	BEGIN COMMENTS;
}

<COMMENTS>[^\n(*]* {}

<COMMENTS>[()*] { }

<COMMENTS> "*)"{
	comment_count--;
	if(comment_count==0)
	{
		BEGIN 0;
	}
}

<COMMENTS><<EOF>>{
	yylval.error_msg="EOF in comment";
	BEGIN 0;
	return ERROR;
}

"*)"{
	yylval.error_msg="Unmatched *)";
	return ERROR;
}

/*INLINE*/
<INITIAL>"--"{
	BEGIN INLINE_COMMENTS;
}


<INLINE_COMMENTS>[^\n]* {}

<INLINE_COMMENTS>\n{
	curr_lineno++;
	BEGIN 0;
}


 /*
  *  The multiple-character operators.
  */
{DARROW}		{ return (DARROW); }
{ASSIGN}		{ return (ASSIGN); }
{LE}			{ return (LE); }

/* single-character operators*/

"+"			{ return '+'; }
"-"			{ return '-'; }
"*"			{ return '*'; }
"/"			{ return '/'; }
"{"			{ return '{'; }
"}"			{ return '}'; }
"("			{ return '('; }
")"			{ return ')'; }
"~"			{ return '~'; }
","			{ return ','; }
";"			{ return ';'; }
":"			{ return ':'; }
"%"			{ return '%'; }
"."			{ return '.'; }
"<"			{ return '<'; }
"="			{ return '='; }
"@"			{ return '@'; }

 /*
  * Keywords are case-insensitive except for the values true and false,
  * which must begin with a lower-case letter.
  */
(?i:CLASS)		{ return (CLASS); }
(?i:ELSE)		{ return (ELSE); }
(?i:FI)			{ return (FI); }
(?i:IF)			{ return (IF); }
(?i:IN)			{ return (IN); }
(?i:INHERITS)	{ return (INHERITS); }
(?i:LET)		{ return (LET); }
(?i:LOOP)		{ return (LOOP); }
(?i:POOL)		{ return (POOL); }
(?i:THEN)		{ return (THEN); }
(?i:WHILE)		{ return (WHILE); }
(?i:CASE)		{ return (CASE); }
(?i:ESAC)		{ return (ESAC); }
(?i:OF)			{ return (OF); }
(?i:NEW)		{ return (NEW); }
(?i:LE)			{ return (LE); }
(?i:NOT)		{ return (NOT); }

t(?i:rue) {
    cool_yylval.boolean = 1;
    return BOOL_CONST;
}

f(?i:alse) {
    cool_yylval.boolean = 0;
    return BOOL_CONST;
}

 /* INT_CONST */
{DIGIT}+ {
    cool_yylval.symbol = inttable.add_string(yytext);
    return INT_CONST;
}

/* WHITE SPACE*/
[\f\r\t\v]+ {}

"\n"{
	curr_lineno++;
}

 /* OBJECTID */
[a-z][A-Za-z0-9_]* {
    cool_yylval.symbol = idtable.add_string(yytext);
    return OBJECTID;
}

 /* TYPEID */
[A-Z][A-Za-z0-9_]* {
    cool_yylval.symbol = idtable.add_string(yytext);
    return TYPEID;
}

 /*
  *  String constants (C syntax)
  *  Escape sequence \c is accepted for all characters c. Except for 
  *  \n \t \b \f, the result is c.
  *
  */
<INITIAL>(\"){
	BEGIN STRING;
	yymore();
}

<STRING>[^\\\"\n]*{
	yymore();
}

 /* normal escape characters, not \n */
<STRING>\\[^\n] { yymore(); }

<STRING>\\\n {
    curr_lineno++;
    yymore();
}

<STRING><<EOF>> {
    yylval.error_msg = "EOF in string constant";
    BEGIN 0;
    yyrestart(yyin);
    return ERROR;
}

<STRING>\n {
    yylval.error_msg = "Unterminated string constant";
    BEGIN 0;
    curr_lineno++;
    return ERROR;
}

<STRING>\\0 {
    yylval.error_msg = "Unterminated string constant";
    BEGIN 0;
    curr_lineno++;
    return ERROR;
}
/*end*/
<STRING>\" {
    std::string input(yytext, yyleng);

    // remove the '\"'s on both sizes.
    input = input.substr(1, input.length() - 2);

    std::string output = "";
    std::string::size_type pos;
    
    if (input.find_first_of('\0') != std::string::npos) {
        yylval.error_msg = "String contains null character";
        BEGIN 0;
        return ERROR;    
    }

    while ((pos = input.find_first_of("\\")) != std::string::npos) {
        output += input.substr(0, pos);

        switch (input[pos + 1]) {
        case 'b':
            output += "\b";
            break;
        case 't':
            output += "\t";
            break;
        case 'n':
            output += "\n";
            break;
        case 'f':
            output += "\f";
            break;
        default:
            output += input[pos + 1];
            break;
        }

        input = input.substr(pos + 2, input.length() - 2);
    }

    output += input;

    if (output.length() >= MAX_STR_CONST) {
        yylval.error_msg = "String constant too long";
        BEGIN 0;
        return ERROR;    
    }

    cool_yylval.symbol = stringtable.add_string((char*)output.c_str());
    BEGIN 0;
    return STR_CONST;

}
%%

