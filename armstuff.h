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

#ifndef __ARMSTUFF_H__
#define __ARMSTUFF_H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	size_t instnum;
	unsigned long long pc;
	unsigned long long opcode; // should only use 32 bits
	unsigned long long tid; // might not be set
	int issve;
	int ismem;
	int isbundle; // ???
	int iswrite; // ???
	int isscattergather;
	unsigned long long address; // valid if ismem != 0
	unsigned long long size; // valid if ismem != 0
	unsigned long long *indices; // non-NULL if iscattergather != 0
	const char* decoded;
} arminstruction;

extern arminstruction* allinsts;
extern size_t instcount;
extern size_t cinstcount;
extern size_t bufsize;
#define BUFINC (1024*1024)

const char* getinst(unsigned long long opcode);
  void inc_instcount(const int flush, FILE* outputfile);


void printinst_paraver(FILE* out, unsigned long long n, const arminstruction* theinst);
void generate_pcf(const char* pcffile);

int yylex (void);

#define NATIVE_OPCODE


#ifdef __cplusplus
}
#endif

#endif // __ARMSTUFF_H__
