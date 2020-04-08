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

#include "llvm/Config/llvm-config.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/MC/MCContext.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/raw_os_ostream.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/MC/MCInstrInfo.h"

#include "armstuff.h"

#include <vector>

#include <iostream>

llvm::MCDisassembler *disassembler;
llvm::MCInstPrinter *instprinter;
llvm::MCSubtargetInfo *subtargetinfo;
llvm::MCInstrInfo *instrinfo;
llvm::MCRegisterInfo *reginfo;

llvm::raw_os_ostream myerr(std::cerr);
std::string buf;
llvm::raw_string_ostream bufstream(buf);
std::vector<const char*> unique_opcodes;

static void init() {
  llvm::InitializeAllTargets(); 
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllDisassemblers();

  std::string triplestr("aarch64-unknown-linux-gnu");
  llvm::Twine tripletwine(triplestr);
  std::string error;

  const llvm::Triple triple(tripletwine);

  const llvm::Target *target = llvm::TargetRegistry::lookupTarget(triplestr, error);
  //fprintf(stderr, "%s: %d: %p (%s)\n", __PRETTY_FUNCTION__, __LINE__, target, error.c_str());


  //llvm::MCDisassembler disas = llvm::createAArch64Disassembler(); // (const MCSubtargetInfo &STI, MCContext &Ctx)

  /* llvm::MCRegisterInfo * */
  reginfo = target->createMCRegInfo(triplestr);
  //fprintf(stderr, "%s: %d: %p (%u)\n", __PRETTY_FUNCTION__, __LINE__, reginfo, reginfo->getNumRegs() );

#if LLVM_VERSION_MAJOR > 10
  llvm::MCTargetOptions mco;
  llvm::MCAsmInfo *asminfo = target->createMCAsmInfo(*reginfo, triplestr, mco);
#else
  llvm::MCAsmInfo *asminfo = target->createMCAsmInfo(*reginfo, triplestr);
#endif
  //fprintf(stderr, "%s: %d: %p\n", __PRETTY_FUNCTION__, __LINE__, asminfo);

  /* llvm::MCInstrInfo * */
  instrinfo = target->createMCInstrInfo();
  //fprintf(stderr, "%s: %d: %p\n", __PRETTY_FUNCTION__, __LINE__, instrinfo);

  /* const llvm::MCSubtargetInfo * */
  subtargetinfo = target->createMCSubtargetInfo(triplestr, "cortex-a72", "+sve");
  //fprintf(stderr, "%s: %d: %p\n", __PRETTY_FUNCTION__, __LINE__, subtargetinfo);

  llvm::MCContext *context = new llvm::MCContext(asminfo, reginfo, NULL);
  //fprintf(stderr, "%s: %d: %p\n", __PRETTY_FUNCTION__, __LINE__, context);

  /* llvm::MCDisassembler * */
  disassembler = target->createMCDisassembler(*subtargetinfo, *context);
  //fprintf(stderr, "%s: %d: %p\n", __PRETTY_FUNCTION__, __LINE__, disassembler);

  /* llvm::MCInstPrinter * */
  instprinter = target-> createMCInstPrinter(triple, 0, *asminfo, *instrinfo, *reginfo);
  //fprintf(stderr, "%s: %d: %p\n", __PRETTY_FUNCTION__, __LINE__, instprinter);

  
#if 0
  for (unsigned i = 1 ; i <  reginfo->getNumRegs() ; i++) { // 0 breaks down
    myerr << "register '" << i << "' is '";
    instprinter->printRegName(myerr, i);
    myerr << "'\n";
    myerr.flush();
  }
#endif
}

static int init_done = 0;

void printinst_paraver(FILE* out, unsigned long long n, const arminstruction* theinst) {

  if (!init_done)
    init();
  init_done ++;

//   if (theinst->issve)
//     fprintf(stderr, "THIS IS SVE!\n");

  llvm::MCInst *llvminst = new llvm::MCInst();
  
  unsigned char data[4];
  data[3] = (n & 0xFF000000) >> 24;
  data[2] = (n & 0x00FF0000) >> 16;
  data[1] = (n & 0x0000FF00) >>  8;
  data[0] = (n & 0x000000FF) >>  0;
  llvm::ArrayRef<uint8_t> dataref(data, 4);
  uint64_t isize = 0;

  //fprintf(stderr, "0x%016llx -> 0x%02x 0x%02x 0x%02x 0x%02x\n", n, data[0], data[1], data[2], data[3]);

#if LLVM_VERSION_MAJOR > 10
  llvm::MCDisassembler::DecodeStatus ds = disassembler->getInstruction(*llvminst, isize, dataref, 0x0, myerr);
#else
  llvm::MCDisassembler::DecodeStatus ds = disassembler->getInstruction(*llvminst, isize, dataref, 0x0, myerr, myerr);
#endif

  if (ds != 0x3) {
    fprintf(stderr, "%s: %d: DS is %d for 0x%08x\n", __PRETTY_FUNCTION__, __LINE__, ds, n & 0xFFFFFFFF);
    return;
  }

  const llvm::MCInstrDesc& llvminstdesc = instrinfo->get(llvminst->getOpcode());

    
#if 0
  fprintf(stderr, "%s: %d: %d - %lld\n", __PRETTY_FUNCTION__, __LINE__, ds, isize);
  fprintf(out, " -> %u operands (%u numdefs)\n", llvminst->getNumOperands(), llvminstdesc.getNumDefs());
  //llvminst->dump_pretty(myerr, instprinter);fprintf(stderr, "\n");
  instprinter->printInst(llvminst, myerr, "", *subtargetinfo);std::cerr << std::endl;
  fprintf(out, "  -> opcode is 0x%x (%s)\n", llvminst->getOpcode(), instprinter->getOpcodeName(llvminst->getOpcode()).str().c_str());
  for (unsigned i = 0 ; i < llvminst->getNumOperands() ; i++) {
    const llvm::MCOperand& op = llvminst->getOperand(i);
    if (op.isValid() && op.isReg())
      fprintf(out, "    -> op %u is register %u ()\n", i, op.getReg() /*, instprinter->getRegisterName(op.getReg()) */ );
    else if (op.isValid() && op.isImm())
      fprintf(out, "    -> op %u is immediate 0x%llx\n", i, op.getImm());
    else
      fprintf(out, "    -> op %u is %s\n", i, op.isValid() ? "valid" : "(invalid)");
  }
#endif


  unsigned theopcode = llvminst->getOpcode();
 
#ifdef NATIVE_OPCODE
  buf.clear();
#if LLVM_VERSION_MAJOR > 10
  instprinter->printInst(llvminst, 0, "", *subtargetinfo, bufstream);
#else
  instprinter->printInst(llvminst, bufstream, "", *subtargetinfo);
#endif
  bufstream.flush(); // required before using buf
  char temp[128];
  sscanf(buf.c_str(), "%s", temp);

  std::vector<const char*>::iterator it =
    std::find_if(unique_opcodes.begin(), unique_opcodes.end(),
		 [&temp](const char* &val){
		   if (strcmp(val, temp) == 0) return true;
		   return false;
		 });
  if (it != unique_opcodes.end()) {
    theopcode = std::distance(unique_opcodes.begin(), it);
  } else {
    unique_opcodes.push_back(strdup(temp));
    theopcode = unique_opcodes.size() - 1;
  }
#endif

  /* header ; no idea for the first 5 fields, then inst number */	
  fprintf(out, "2:1:1:1:1:%zd", theinst->instnum);
  /* 47000001 PC */
  fprintf(out, ":47000001:%llu", (unsigned long long)theinst->pc);
  /* 47000015 instruction type */
  fprintf(out, ":47000015:%llu", (unsigned long long)theopcode);
  /* 47000006     RDest, 47000007     RSrc1, 47000008     RSrc2, 47000009     RSrc3, 47000010     Immediate1, 47000011     Immediate2 */
  unsigned regs = 0;
  unsigned imms = 0;
  for (unsigned i = 0 ; i < llvminst->getNumOperands() ; i++) {
    const llvm::MCOperand& op = llvminst->getOperand(i);
    unsigned numdefs = llvminstdesc.getNumDefs();
    if (op.isValid()) {
      if (op.isReg()) {
	if (regs < numdefs)
	  fprintf(out, ":47000006:%llu", (unsigned long long)op.getReg());
	else {
	  int num = 47000007 + regs - numdefs;
	  if (num > 47000009)
	    num = 47000009;
	  fprintf(out, ":%d:%llu", num, (unsigned long long)op.getReg());
	}
	regs ++;
      }
      if (op.isImm()) {
	fprintf(out, ":%d:%llu", 47000010 + imms, (unsigned long long)op.getImm());
	imms ++;
      }
    }
  }
  if (theinst->ismem) {
    fprintf(out, ":47000005:%llu", (unsigned long long)theinst->address);
    fprintf(out, ":47000025:%llu", (unsigned long long)theinst->size);
  }
  if (theinst->issve) {
    fprintf(out, ":47000026:%d", theinst->issve);
  }

  fprintf(out, "\n");
  
  delete llvminst;

//   if (init_done > 3)
//     exit(1);
}

const static char* pcfstart = "DEFAULT_OPTIONS\n\
\n\
LEVEL               THREAD\n\
UNITS               NANOSEC\n\
LOOK_BACK           100\n\
SPEED               1\n\
FLAG_ICONS          ENABLED\n\
NUM_OF_STATE_COLORS 1000\n\
YMAX_SCALE          37\n\
\n\
\n\
DEFAULT_SEMANTIC\n\
\n\
THREAD_FUNC          State As Is\n\
\n\
\n\
STATES\n\
0    Idle\n\
1    Running\n\
2    Not created\n\
3    Waiting a message\n\
4    Blocking Send\n\
5    Synchronization\n\
6    Test/Probe\n\
7    Scheduling and Fork/Join\n\
8    Wait/WaitAll\n\
9    Blocked\n\
10    Immediate Send\n\
11    Immediate Receive\n\
12    I/O\n\
13    Group Communication\n\
14    Tracing Disabled\n\
15    Others\n\
16    Send Receive\n\
17    Memory transfer\n\
18    Profiling\n\
19    On-line analysis\n\
20    Remote memory access\n\
21    Atomic memory operation\n\
22    Memory ordering operation\n\
23    Distributed locking\n\
24    Overhead\n\
25    One-sided op\n\
26    Startup latency\n\
27    Waiting links\n\
28    Data copy\n\
29    RTT\n\
30    Allocating memory\n\
31    Freeing memory\n\
\n\
\n\
STATES_COLOR\n\
0    {117,195,255}\n\
1    {0,0,255}\n\
2    {255,255,255}\n\
3    {255,0,0}\n\
4    {255,0,174}\n\
5    {179,0,0}\n\
6    {0,255,0}\n\
7    {255,255,0}\n\
8    {235,0,0}\n\
9    {0,162,0}\n\
10    {255,0,255}\n\
11    {100,100,177}\n\
12    {172,174,41}\n\
13    {255,144,26}\n\
14    {2,255,177}\n\
15    {192,224,0}\n\
16    {66,66,66}\n\
17    {255,0,96}\n\
18    {169,169,169}\n\
19    {169,0,0}\n\
20    {0,109,255}\n\
21    {200,61,68}\n\
22    {200,66,0}\n\
23    {0,41,0}\n\
24    {139,121,177}\n\
25    {116,116,116}\n\
26    {200,50,89}\n\
27    {255,171,98}\n\
28    {0,68,189}\n\
29    {52,43,0}\n\
30    {255,46,0}\n\
31    {100,216,32}\n\
\n\
EVENT_TYPE\n\
9   47000000     id\n\
\n\
EVENT_TYPE\n\
9   47000001     program-counter\n\
\n\
EVENT_TYPE\n\
9   47000002     disasm\n\
\n\
EVENT_TYPE\n\
9   47000005     address\n\
\n\
EVENT_TYPE\n\
9   47000010     Immediate1\n\
\n\
EVENT_TYPE\n\
9   47000011     Immediate2\n\
\n\
EVENT_TYPE\n\
9   47000025     memory-size\n\
\n\
EVENT_TYPE\n\
9   47000026     issve\n\
\n\
";

void generate_pcf(const char* pcffile) {
  if (!init_done)
    init();
  init_done ++;

  llvm::StringRef Filename(pcffile);
  std::error_code EC;
  llvm::raw_fd_ostream pcf(Filename, EC);


  pcf << pcfstart;
  pcf.flush();

  pcf << "EVENT_TYPE\n9   " << 47000006 << "      " << "RDest" << "\nVALUES\n";
  for (unsigned i = 1 ; i <  reginfo->getNumRegs() ; i++) { // 0 breaks down
    pcf << i << " ";
    instprinter->printRegName(pcf, i);
    pcf << "\n";
  }
    pcf << "\n";

  pcf << "EVENT_TYPE\n9   " << 47000007 << "      " << "RSrc1" << "\nVALUES\n";
  for (unsigned i = 1 ; i <  reginfo->getNumRegs() ; i++) { // 0 breaks down
    pcf << i << " ";
    instprinter->printRegName(pcf, i);
    pcf << "\n";
  }
    pcf << "\n";

  pcf << "EVENT_TYPE\n9   " << 47000008 << "      " << "RSrc2" << "\nVALUES\n";
  for (unsigned i = 1 ; i <  reginfo->getNumRegs() ; i++) { // 0 breaks down
    pcf << i << " ";
    instprinter->printRegName(pcf, i);
    pcf << "\n";
  }
    pcf << "\n";

  pcf << "EVENT_TYPE\n9   " << 47000009 << "      " << "RSrc3" << "\nVALUES\n";
  for (unsigned i = 1 ; i <  reginfo->getNumRegs() ; i++) { // 0 breaks down
    pcf << i << " ";
    instprinter->printRegName(pcf, i);
    pcf << "\n";
  }
    pcf << "\n";

  pcf << "EVENT_TYPE\n9   " << 47000015 << "      " << "Instruction" << "\nVALUES\n";
#ifdef NATIVE_OPCODE
  for (size_t i = 0 ; i < unique_opcodes.size() ; i++) {
    pcf << i << " ";
    pcf << unique_opcodes[i];
    pcf << "\n";
  }
#else
  for (unsigned i = 1 ; i < instrinfo->getNumOpcodes() ; i++) {
    pcf << i << " ";
    //pcf << instrinfo->getName(i);
    pcf << instprinter->getOpcodeName(i);
    pcf << "\n";
  }
#endif

    pcf << "\n";
  
  pcf.close();
}
