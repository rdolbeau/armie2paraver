/*

Arm Instruction Emulator traces to Paraver traces translator.

Copyright © 2020 romain Dolbeau <romain.dolbeau@sipearl.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

%{
#include <stdio.h>
#include <stdlib.h>
#include "armstuff.h"
#define YYDEBUG 1
%}

%parse-param {FILE *outputfile}

%union
{
  unsigned long long ll;
}

%token <ll> HEXADECIMAL
%token <ll> DECIMAL
%token COMMA
%token SEMICOLON
%token HEADER

%%
input:      /* empty */ {  }
| input line           {  }
;

line:  HEADER { }
|      HEXADECIMAL SEMICOLON HEXADECIMAL SEMICOLON DECIMAL {
	allinsts[cinstcount].instnum = instcount;
	allinsts[cinstcount].pc = $1;
	allinsts[cinstcount].opcode = $3;
	allinsts[cinstcount].tid = 0;
	allinsts[cinstcount].issve = 0;
	allinsts[cinstcount].ismem = $5;
	allinsts[cinstcount].isbundle = 0;
	allinsts[cinstcount].iswrite = 0;
	allinsts[cinstcount].isscattergather = 0; // dunno yet
	allinsts[cinstcount].address = 0; // dunno yet
	allinsts[cinstcount].size = 0; // dunno yet
	allinsts[cinstcount].indices = NULL; // dunno yet
	inc_instcount(0, outputfile);
}
|      HEXADECIMAL COMMA HEXADECIMAL COMMA DECIMAL {
	allinsts[cinstcount].instnum = instcount;
	allinsts[cinstcount].pc = $1;
	allinsts[cinstcount].opcode = $3;
	allinsts[cinstcount].tid = 0;
	allinsts[cinstcount].issve = 1;
	allinsts[cinstcount].ismem = $5;
	allinsts[cinstcount].isbundle = 0;
	allinsts[cinstcount].iswrite = 0;
	allinsts[cinstcount].isscattergather = 0; // dunno yet
	allinsts[cinstcount].address = 0; // dunno yet
	allinsts[cinstcount].size = 0; // dunno yet
	allinsts[cinstcount].indices = NULL; // dunno yet
	inc_instcount(0, outputfile);
}
|      DECIMAL SEMICOLON DECIMAL SEMICOLON DECIMAL SEMICOLON DECIMAL SEMICOLON HEXADECIMAL SEMICOLON HEXADECIMAL {
        if (cinstcount == 0) {
	  fprintf(stderr, "Can't have memory before instruction...\n");
	  exit(-11);
	}
	if (allinsts[cinstcount-1].ismem == 0) {
	  fprintf(stderr, "Previous instruction is not a memory instruction...\n");
	  exit(-3);
	}
	if (allinsts[cinstcount-1].pc != $11) {
	  fprintf(stderr, "PC mismatch between instruction & memory... (%p vs. %p at inst %llu)\n", allinsts[cinstcount].pc, $11, instcount);
	  exit(-5);
	}
	
	allinsts[cinstcount-1].tid = $1;
	allinsts[cinstcount-1].isbundle = $3;
	allinsts[cinstcount-1].iswrite = $5;
	allinsts[cinstcount-1].isscattergather = 0; // ???
	allinsts[cinstcount-1].address = $9;
	allinsts[cinstcount-1].size = $7;
	allinsts[cinstcount-1].indices = NULL; // ???
}
;
%%

int
yyerror(FILE* outputfile, char *s)
{
  fprintf(stderr, "error: %s\n", s);
  return(0);
}

int
yywrap(void)
{
  return(-1);
}
