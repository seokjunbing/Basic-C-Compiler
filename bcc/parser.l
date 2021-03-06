/*The file name: 
*   cscan.l
* The purpose of the file:
*   flex file containing the rules and the actions of a subset of C
* The names of all the authors:
*   SeokJun Bing
*   Charles Li
*   Juan Torres
* Attributions, as appropriate.
*   need toktype.h, cscan.c
*   use make to compile and link
*/

%option noyywrap

%{
#include <string.h>
#include "parser.tab.h"
int lexLookup(char *);
int oldstate;
int num_lines = 1;

#define MAXTOKENLENGTH 201

char savedIdText[MAXTOKENLENGTH];
char savedLiteralText[MAXTOKENLENGTH];



%}

white [ \t]+
letter [A-Za-z]
digit [0-9]
id ({letter}|_)({letter}|{digit}|_)*
integer {digit}+
signedint [-+]?{integer}


%x string
%x line_comment
%x multi_comment
%x num


%%





{white}      /* skip whitespace */

{id}         { strncpy(savedIdText, yytext, MAXTOKENLENGTH-1);
                oldstate = YY_START;
                BEGIN(num);
				        return lexLookup(yytext); }

{signedint}{id} {return OTHER_T;}

{signedint}  {  
    strncpy(savedLiteralText, yytext, MAXTOKENLENGTH-1); 
    oldstate = YY_START; 
    BEGIN(num);
    return INTCONST_T;} 

\n { ++num_lines; }

<num>[+-] {BEGIN(oldstate); return yytext[0];}
<num>[ \t]+
<num>[^+-] {yyless(0); BEGIN(oldstate); } 
<num>"++"|"--" {yyless(0); BEGIN(oldstate); }

  /* Handle string special case */
\"           { BEGIN(string); }
<string>\n   { ++num_lines; }
<string>\\\" { yymore(); }
<string>\"   {
  BEGIN(INITIAL);		/* go back to normal state */
  yytext[--yyleng] = '\0';	/* remove the trailing double-quote */
	strncpy(savedLiteralText, yytext, MAXTOKENLENGTH-1);
  return STRINGCONST_T;
 }
<string>.    { yymore(); }



 /* One line comments */
"//"                { oldstate = YY_START; BEGIN(line_comment); }
<line_comment>\n  { ++num_lines; BEGIN(oldstate); }
<line_comment>.   { ; } /* do nothing */



 /* Multi-line comments */
"/*"              { oldstate = YY_START; BEGIN(multi_comment); }
<multi_comment>.   { ; } /* do nothing */
<multi_comment>\n   { ++num_lines; }
<multi_comment>"*/"   { BEGIN(oldstate); }
<multi_comment><<EOF>> { ; } /* do nothing */

 /* changed single char to symbols */
";"          return ';';
"="          return '=';
"+"          return '+';
"-"          return '-';
"*"          return '*';
"<"          return '<';
">"          return '>';
"=="         return ISEQUAL_T;
"!="         return NOTEQUAL_T;
"\."         return '.';
"["          return '[';   /* a valid but odd alternate: */
"]"          return ']';   /* [][(){}]     return yytext[0]; */
"("          return '(';
")"          return ')';
"{"          return '{';
"}"          return '}';
"<="         return LESS_EQUAL_T;
">="         return GREATER_EQUAL_T;
"++"         return INCREMENT_T;
"--"         return DECREMENT_T;
"&&"         return AND_T;
"||"         return OR_T;
"!"          return '!';
","          return ',';
"%"          return '%';
"/"          return '/';

<<EOF>>      return EOF;
  .          return OTHER_T;

%%

int lexLookup(char *s) {
  struct combo {
    char *keyword;
    int token;
  };
  typedef struct combo combo;

  const int KWCOUNT = 11;

  static combo keywords[] =
    { /*{ "include", INCLUDE_T }, */
      /*{ "extern", EXTERN_T }, */
      { "int", INT_T },
      /*{ "char", CHAR_T },*/
      { "while", WHILE_T },
      { "for", FOR_T },
      { "if", IF_T},
      { "else", ELSE_T},
      { "return", RETURN_T},
      { "void", VOID_T},
      { "read", READ_T},
      { "print", PRINT_T},
      { "do", DO_T},
      { NULL, ID_T } // this should be the last one, as it's the fallback case.
    };

  combo *kw;

  keywords[KWCOUNT-1].keyword = s;

  for (kw = keywords; strcmp(kw->keyword, s) != 0; kw++)
    ;
	
	if(kw->token == ID_T) {
		strncpy(savedIdText, yytext, MAXTOKENLENGTH-1);
	}
  return kw->token;
}
