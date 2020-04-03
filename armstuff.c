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

#include "armstuff.h"
#include <string.h>
#include <stdlib.h>

/* for debug, slow and text only */
const char* getinst(unsigned long long opcode) {
	char temp[128];
	char res[128];
	unsigned long long opcode2;
	FILE *r;
	snprintf(temp, 128, "echo 0x%llx | enc2instr.py", opcode);
	r = popen(temp, "r");
	int x = fscanf(r, "0x%llx : %128[^\n]", &opcode2, res);
	pclose(r);

	if (opcode2 != opcode) {
		fprintf(stderr, "opcode mismatch\n");
		exit(-7);
	}

	//printf("%s\n", res);

	return strndup(res, 127);
}
