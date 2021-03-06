//taken from xcopilot
#ifndef NEWCPU_H
#define NEWCPU_H

#include "m68k.h"

#include "virtualhardware.h" //hack (remove me when possible)


extern void op_illg(ULONG);

void fatal(char *, int);
UWORD nextiword();
ULONG nextilong();
int cctrue(int cc);
ULONG get_disp_ea (ULONG base, UWORD dp);
void MakeSR();
void MakeFromSR();
void MC68000_oldstep(UWORD opcode);
void MC68000_setpc(CPTR newpc);
CPTR MC68000_getpc();
void MC68000_setstopped(int stop);
void MC68000_exception(int);
void MC68000_init(shared_img *shptr);
void MC68000_reset();
void MC68000_run();
void MC68000_step();
void MC68000_runtilladdr(CPTR);


//pilotcpu
int CPU(shared_img *shptr);
int CPU_init(shared_img *shptr);
void CPU_wait(shared_img *shptr);
void CPU_reset(shared_img *shptr);
void CPU_start(shared_img *shptr);
void CPU_stop(shared_img *shptr);
void CPU_getregs(shared_img *shptr, struct regstruct *r);
void CPU_setregs(shared_img *shptr, struct regstruct *r);
int CPU_setexceptionflag(shared_img *shptr, int exception, int flag);
void CPU_pushlongstack(ULONG val);
ULONG CPU_poplongstack();
void CPU_pushwordstack(UWORD val);
UWORD CPU_popwordstack();
//stack does not support byte ops
void CPU_68kfunction(CPTR addr, CPTR from);

//debuging
void printregs();
#endif
