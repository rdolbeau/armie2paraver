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

#include <stdio.h>
#include <stdlib.h>

#include "armietraceparser_yacc.h"
#include "armstuff.h"

extern FILE *yyin, *yyout;
extern int yydebug;


arminstruction* allinsts;
size_t instcount;
size_t cinstcount;
size_t bufsize;

int main(int argc, char **argv) {
  int i;
  //yydebug=1;

  FILE *inputfile = stdin;
  FILE *outputfile = stdout;
  int closein = 0, closeout = 0;
  if (argc > 1) {
	  inputfile = fopen(argv[1], "r");
	  if (!inputfile) {
		  fprintf(stderr, "no file\n");
		  return -1;
	  }
	  closein = 1;
  }

  if (argc > 2) {
          char temp[256];
	  snprintf(temp, 256, "%s.prv", argv[2]);
	  outputfile = fopen(temp, "w");
	  if (!outputfile) {
		  fprintf(stderr, "no file\n");
		  return -1;
	  }
	  closeout = 1;
  }
  bufsize = BUFINC;
  allinsts = calloc(BUFINC, sizeof(arminstruction));
  instcount = 0;

  fprintf(stdout, "#Paraver (30/03/2020 at 09:50):19859_ns:1(1):1:1:(1:1)\n");

  yyin = inputfile;
  do {
	  yyparse(outputfile);
  } while (!feof(yyin));
  if (closein)
    fclose(inputfile);

  inc_instcount(1, outputfile);  

  if (closeout)
    fclose(outputfile);

  {
    char temp[256];
    if (argc > 2)
      snprintf(temp, 256, "%s.pcf", argv[2]);
    else
      snprintf(temp, 256, "%s.pcf", "mytrace");
      
    generate_pcf(temp);
  }
  return 0;
}


void inc_instcount(const int flush, FILE* outputfile) {
	if (flush || ((cinstcount >= BUFINC/2) && (!allinsts[cinstcount].ismem))) {
	  for (size_t i = 0 ; i <= cinstcount ; i++) {
	    printinst_paraver(outputfile, allinsts[i].opcode, &allinsts[i]);
	  }
	  cinstcount = 0;
	} else {
	  cinstcount ++;
	}
	instcount ++;
}
