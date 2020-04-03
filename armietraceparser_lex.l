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
#include "armietraceparser_yacc.h"
%}

HEX  [0123456789abcdef]
DEC  [0123456789]

%%
0x{HEX}+   { yylval.ll = strtoll(yytext, NULL, 16); /* printf("0x%llx ", yylval.ll); */ return HEXADECIMAL; }

{DEC}+     { yylval.ll = strtoll(yytext, NULL, 10); /* printf("%lld ", yylval.ll); */ return DECIMAL; }

;        { return SEMICOLON; }

,        { return COMMA; }

Instrace.* { return HEADER; }

.*<PC>.* { return HEADER; }

SVE.* { return HEADER; }

\n

.          { return yytext[0]; }

%%
