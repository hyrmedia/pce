/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/simarm/cmd_arm.c                                  *
 * Created:       2004-11-04 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-11-08 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2004 Hampa Hug <hampa@hampa.ch>                        *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  General *
 * Public License for more details.                                          *
 *****************************************************************************/

/* $Id$ */


#include "main.h"


static const char *arm_modes[32] = {
  "0x00", "0x01", "0x02", "0x03",
  "0x04", "0x05", "0x06", "0x07",
  "0x08", "0x09", "0x0a", "0x0b",
  "0x0c", "0x0d", "0x0e", "0x0f",
  "usr",  "fiq",  "irq",  "svc",
  "0x14", "0x15", "0x16", "abt",
  "0x18", "0x19", "und",  "0x1b",
  "0x1c", "0x1d", "0x1e", "sys"
};

int sarm_match_reg (cmd_t *cmd, simarm_t *sim, uint32_t **reg)
{
  arm_t *cpu;

  cpu = sim->cpu;

  cmd_match_space (cmd);

  if (cmd_match (cmd, "r")) {
    unsigned short n;

    if (!cmd_match_uint16b (cmd, &n, 10)) {
      cmd_error (cmd, "missing register number");
      return (0);
    }

    *reg = &cpu->reg[n & 0x0f];

    return (1);
  }
  else if (cmd_match (cmd, "pc")) {
    *reg = &cpu->reg[15];
    return (1);
  }
  else if (cmd_match (cmd, "cpsr")) {
    *reg = &cpu->cpsr;
    return (1);
  }
  else if (cmd_match (cmd, "spsr")) {
    *reg = &cpu->spsr;
    return (1);
  }

  return (0);
}

void sarm_dasm_str (char *dst, arm_dasm_t *op)
{
  switch (op->argn) {
    case 0:
      sprintf (dst, "%08lX  %s", (unsigned long) op->ir, op->op);
      break;

    case 1:
      sprintf (dst, "%08lX  %-8s %s", (unsigned long) op->ir, op->op, op->arg[0]);
      break;

    case 2:
      sprintf (dst, "%08lX  %-8s %s, %s",
        (unsigned long) op->ir, op->op, op->arg[0], op->arg[1]
      );
      break;

    case 3:
      sprintf (dst, "%08lX  %-8s %s, %s, %s",
        (unsigned long) op->ir, op->op,
        op->arg[0], op->arg[1], op->arg[2]
      );
      break;

    case 4:
      sprintf (dst, "%08lX  %-8s %s, %s, %s, %s",
        (unsigned long) op->ir, op->op,
        op->arg[0], op->arg[1], op->arg[2], op->arg[3]
      );
      break;

    case 5:
      sprintf (dst, "%08lX  %-8s %s, %s, %s, %s, %s",
        (unsigned long) op->ir, op->op,
        op->arg[0], op->arg[1], op->arg[2], op->arg[3], op->arg[4]
      );
      break;

    default:
      strcpy (dst, "---");
      break;
  }
}

void sarm_prt_state_cpu (arm_t *c, FILE *fp)
{
  unsigned long long opcnt, clkcnt;
  unsigned long      delay;
  arm_dasm_t         op;
  char               str[256];

  prt_sep (fp, "ARM");

  opcnt = arm_get_opcnt (c);
  clkcnt = arm_get_clkcnt (c);
  delay = arm_get_delay (c);

  fprintf (fp, "CLK=%llx  OP=%llx  DLY=%lx  CPI=%.4f\n",
    clkcnt, opcnt, delay,
    (opcnt > 0) ? ((double) (clkcnt + delay) / (double) opcnt) : 1.0
  );

  fprintf (fp, "r00=%08lX  r04=%08lX  r08=%08lX  r12=%08lX  CPSR=%08lX\n",
    (unsigned long) arm_get_reg (c, 0),
    (unsigned long) arm_get_reg (c, 4),
    (unsigned long) arm_get_reg (c, 8),
    (unsigned long) arm_get_reg (c, 12),
    (unsigned long) arm_get_cpsr (c)
  );

  fprintf (fp, "r01=%08lX  r05=%08lX  r09=%08lX  r13=%08lX  SPSR=%08lX\n",
    (unsigned long) arm_get_reg (c, 1),
    (unsigned long) arm_get_reg (c, 5),
    (unsigned long) arm_get_reg (c, 9),
    (unsigned long) arm_get_reg (c, 13),
    (unsigned long) arm_get_spsr (c)
  );

  fprintf (fp, "r02=%08lX  r06=%08lX  r10=%08lX  r14=%08lX    CC=[%c%c%c%c]\n",
    (unsigned long) arm_get_reg (c, 2),
    (unsigned long) arm_get_reg (c, 6),
    (unsigned long) arm_get_reg (c, 10),
    (unsigned long) arm_get_reg (c, 14),
    (arm_get_cc_n (c)) ? 'N' : '-',
    (arm_get_cc_z (c)) ? 'Z' : '-',
    (arm_get_cc_c (c)) ? 'C' : '-',
    (arm_get_cc_v (c)) ? 'V' : '-'
  );

  fprintf (fp, "r03=%08lX  r07=%08lX  r11=%08lX  r15=%08lX     M=%02X (%s)\n",
    (unsigned long) arm_get_reg (c, 3),
    (unsigned long) arm_get_reg (c, 7),
    (unsigned long) arm_get_reg (c, 11),
    (unsigned long) arm_get_reg (c, 15),
    (unsigned) arm_get_cpsr_m (c),
    arm_modes[arm_get_cpsr_m (c) & 0x1f]
  );

  arm_dasm_mem (c, &op, arm_get_pc (c), par_xlat);
  sarm_dasm_str (str, &op);

  fprintf (fp, "%08lX  %s\n", (unsigned long) arm_get_pc (c), str);
}


static
void sarm_exec (simarm_t *sim)
{
  unsigned long long old;

  old = arm_get_opcnt (sim->cpu);

  while (arm_get_opcnt (sim->cpu) == old) {
    sarm_clock (sim, 1);
  }
}

static
void sarm_run_bp (simarm_t *sim)
{
  pce_start();

  while (1) {
    sarm_exec (sim);

    if (bp_check (&sim->brkpt, arm_get_pc (sim->cpu), 0)) {
      break;
    }

    if (sim->brk) {
      break;
    }
  }

  pce_stop();
}

static
int sarm_exec_to (simarm_t *sim, unsigned long addr)
{
  while (arm_get_pc (sim->cpu) != addr) {
    sarm_clock (sim, 1);

    if (sim->brk) {
      return (1);
    }
  }

  return (0);
}

static
int sarm_exec_off (simarm_t *sim, unsigned long addr)
{
  while (arm_get_pc (sim->cpu) == addr) {
    sarm_clock (sim, 1);

    if (sim->brk) {
      return (1);
    }
  }

  return (0);
}


#if 0
static
void sarm_log_opcode (void *ext, unsigned long ir)
{
  simarm_t *sim = ext;
}
#endif

static
void sarm_log_undef (void *ext, unsigned long ir)
{
  simarm_t *sim = ext;

  pce_log (MSG_DEB,
    "%08lX: undefined operation [%08lX]\n",
    (unsigned long) arm_get_pc (sim->cpu), ir
  );

  sarm_break (sim, PCE_BRK_STOP);
}

static
void sarm_log_trap (void *ext, unsigned long addr)
{
  simarm_t *sim = ext;
  char     *name;

  switch (addr) {
    default:
      name = "unknown";
      break;
  }

  pce_log (MSG_DEB, "%08lX: exception %lx (%s)\n",
    (unsigned long) arm_get_pc (sim->cpu), addr, name
  );
}

static
void do_bc (cmd_t *cmd, simarm_t *sim)
{
  unsigned long addr;

  if (cmd_match_eol (cmd)) {
    bp_clear_all (&sim->brkpt);
    return;
  }

  if (!cmd_match_uint32 (cmd, &addr)) {
    cmd_error (cmd, "expecting address");
    return;
  }

  if (!cmd_match_end (cmd)) {
    return;
  }

  if (bp_clear (&sim->brkpt, addr, 0)) {
    printf ("no breakpoint cleared at %08lX\n", addr);
  }
}

static
void do_bl (cmd_t *cmd, simarm_t *sim)
{
  if (!cmd_match_end (cmd)) {
    return;
  }

  bp_list (sim->brkpt, 0);
}

static
void do_bs (cmd_t *cmd, simarm_t *sim)
{
  unsigned long  addr;
  unsigned short pass, reset;

  addr = arm_get_pc (sim->cpu);
  pass = 1;
  reset = 0;

  if (!cmd_match_uint32 (cmd, &addr)) {
    cmd_error (cmd, "expecting address");
    return;
  }

  cmd_match_uint16 (cmd, &pass);
  cmd_match_uint16 (cmd, &reset);

  if (!cmd_match_end (cmd)) {
    return;
  }

  if (pass > 0) {
    printf ("Breakpoint at %08lX  %04X  %04X\n",
      addr, pass, reset
    );

    bp_add (&sim->brkpt, addr, 0, pass, reset);
  }
}

static
void do_b (cmd_t *cmd, simarm_t *sim)
{
  if (cmd_match (cmd, "l")) {
    do_bl (cmd, sim);
  }
  else if (cmd_match (cmd, "s")) {
    do_bs (cmd, sim);
  }
  else if (cmd_match (cmd, "c")) {
    do_bc (cmd, sim);
  }
  else {
    cmd_error (cmd, "b: unknown command");
  }
}

static
void do_c (cmd_t *cmd, simarm_t *sim)
{
  unsigned long cnt;

  cnt = 1;

  cmd_match_uint32 (cmd, &cnt);

  if (!cmd_match_end (cmd)) {
    return;
  }

  while (cnt > 0) {
    sarm_clock (sim, 1);
    cnt -= 1;
  }

  sarm_prt_state_cpu (sim->cpu, stdout);
}

static
void do_d (cmd_t *cmd, simarm_t *sim)
{
  unsigned long        i, j;
  unsigned long        cnt;
  unsigned long        addr1, addr2;
  static int           first = 1;
  static unsigned long saddr = 0;
  unsigned long        p, p1, p2;
  char                 buf[256];

  if (first) {
    first = 0;
    saddr = arm_get_pc (sim->cpu);
  }

  addr1 = saddr;
  cnt = 256;

  if (cmd_match_uint32 (cmd, &addr1)) {
    cmd_match_uint32 (cmd, &cnt);
  }

  if (!cmd_match_end (cmd)) {
    return;
  }

  addr2 = (addr1 + cnt - 1) & 0xffffffffUL;
  if (addr2 < addr1) {
    addr2 = 0xffffffffUL;
    cnt = addr2 - addr1 + 1;
  }

  saddr = addr1 + cnt;

  p1 = addr1 / 16;
  p2 = addr2 / 16 + 1;

  for (p = p1; p < p2; p++) {
    j = 16 * p;

    sprintf (buf,
      "%08lX  xx xx xx xx xx xx xx xx-xx xx xx xx xx xx xx xx  xxxxxxxxxxxxxxxx\n",
      j
    );

    for (i = 0; i < 16; i++) {
      if ((j >= addr1) && (j <= addr2)) {
        uint8_t  val;
        unsigned val1, val2;

        if (arm_get_mem8 (sim->cpu, j, par_xlat, &val)) {
          val = 0xff;
        }

        val1 = (val >> 4) & 0x0f;
        val2 = val & 0x0f;

        buf[10 + 3 * i + 0] = (val1 < 10) ? ('0' + val1) : ('A' + val1 - 10);
        buf[10 + 3 * i + 1] = (val2 < 10) ? ('0' + val2) : ('A' + val2 - 10);

        if ((val >= 32) && (val <= 127)) {
          buf[59 + i] = val;
        }
        else {
          buf[59 + i] = '.';
        }
      }
      else {
        buf[10 + 3 * i] = ' ';
        buf[10 + 3 * i + 1] = ' ';
        buf[59 + i] = ' ';
      }

      j += 1;
    }

    fputs (buf, stdout);
  }
}

static
void do_e (cmd_t *cmd, simarm_t *sim)
{
  unsigned long  addr;
  unsigned short val;

  addr = 0;

  if (!cmd_match_uint32 (cmd, &addr)) {
    cmd_error (cmd, "need an address");
    return;
  }

  while (cmd_match_uint16 (cmd, &val)) {
    if (arm_set_mem8 (sim->cpu, addr, par_xlat, val)) {
      printf ("TLB miss: %08lx <- %02x\n", addr, (unsigned) val);
    }

    addr += 1;
  }
}

static
void do_g (cmd_t *cmd, simarm_t *sim)
{
  int           run;
  unsigned long addr;

  if (cmd_match (cmd, "b")) {
    run = 0;
  }
  else {
    run = 1;
  }

  if (cmd_match_uint32 (cmd, &addr)) {
    bp_add (&sim->brkpt, addr, 0, 1, 0);
    run = 0;
  }

  if (!cmd_match_end (cmd)) {
    return;
  }

  if (run) {
    pce_run();
  }
  else {
    sarm_run_bp (sim);

    fputs ("\n", stdout);
    sarm_prt_state_cpu (sim->cpu, stdout);
  }
}

static
void do_h (cmd_t *cmd, simarm_t *sim)
{
  fputs (
    "bc [addr]                 clear a breakpoint or all\n"
    "bl                        list breakpoints\n"
    "bs addr [pass [reset]]    set a breakpoint [pass=1 reset=0]\n"
    "c [cnt]                   clock\n"
    "d [addr [cnt]]            dump memory\n"
    "e addr [val...]           enter bytes into memory\n"
    "g [b]                     run with or without breakpoints (ESC to stop)\n"
    "key [val...]              send keycodes to the serial console\n"
    "p [cnt]                   execute cnt instructions, skip calls [1]\n"
    "q                         quit\n"
    "r reg [val]               set a register\n"
    "s [what]                  print status (cpu)\n"
    "t [cnt]                   execute cnt instructions [1]\n"
    "u [addr [cnt]]            disassemble\n"
    "x [c|r|v]                 set the translation mode (cpu, real, virtual)\n",
    stdout
  );
}

static
void do_key (cmd_t *cmd, simarm_t *sim)
{
  unsigned short c;

  while (cmd_match_uint16 (cmd, &c)) {
    sarm_set_keycode (sim, c);
  }

  if (!cmd_match_end (cmd)) {
    return;
  }
}

static
void do_p (cmd_t *cmd, simarm_t *sim)
{
  unsigned long cnt;
  arm_dasm_t    da;

  cnt = 1;

  cmd_match_uint32 (cmd, &cnt);

  if (!cmd_match_end (cmd)) {
    return;
  }

  pce_start();

  while (cnt > 0) {
    arm_dasm_mem (sim->cpu, &da, arm_get_pc (sim->cpu), ARM_XLAT_CPU);

    if (da.flags & ARM_DFLAG_CALL) {
      if (sarm_exec_to (sim, arm_get_pc (sim->cpu) + 4)) {
        break;
      }
    }
    else {
      uint32_t cpsr;

      cpsr = arm_get_cpsr (sim->cpu);

      if (sarm_exec_off (sim, arm_get_pc (sim->cpu))) {
        break;
      }

      if ((cpsr & ARM_PSR_M) == ARM_MODE_USR) {
        /* check if exception occured */
        while (arm_get_cpsr_m (sim->cpu) != ARM_MODE_USR) {
          sarm_clock (sim, 1);

          if (sim->brk) {
            break;
          }
        }
      }
    }

    cnt -= 1;
  }

  pce_stop();

  sarm_prt_state_cpu (sim->cpu, stdout);
}

static
void do_r (cmd_t *cmd, simarm_t *sim)
{
  unsigned long val;
  uint32_t      *reg;

  if (!sarm_match_reg (cmd, sim, &reg)) {
    printf ("missing register\n");
    return;
  }

  if (cmd_match_eol (cmd)) {
    printf ("%08lx\n", (unsigned long) *reg);
    return;
  }

  if (!cmd_match_uint32 (cmd, &val)) {
    printf ("missing value\n");
    return;
  }

  if (!cmd_match_end (cmd)) {
    return;
  }

  *reg = val;

  sarm_prt_state_cpu (sim->cpu, stdout);
}

static
void do_s (cmd_t *cmd, simarm_t *sim)
{
  if (cmd_match_eol (cmd)) {
    sarm_prt_state_cpu (sim->cpu, stdout);
    return;
  }

  prt_state (sim, stdout, cmd_get_str (cmd));
}

static
void do_t (cmd_t *cmd, simarm_t *sim)
{
  unsigned long i, n;

  n = 1;

  cmd_match_uint32 (cmd, &n);

  if (!cmd_match_end (cmd)) {
    return;
  }

  pce_start();

  for (i = 0; i < n; i++) {
    sarm_exec (sim);
  }

  pce_stop();

  sarm_prt_state_cpu (sim->cpu, stdout);
}

static
void do_u (cmd_t *cmd, simarm_t *sim)
{
  unsigned             i;
  int                  to;
  unsigned long        addr, cnt;
  static unsigned int  first = 1;
  static unsigned long saddr = 0;
  arm_dasm_t           op;
  char                 str[256];

  if (first) {
    first = 0;
    saddr = arm_get_pc (sim->cpu);
  }

  to = 0;
  addr = saddr;
  cnt = 16;

  if (cmd_match (cmd, "-")) {
    to = 1;
  }

  if (cmd_match_uint32 (cmd, &addr)) {
    cmd_match_uint32 (cmd, &cnt);
  }

  if (!cmd_match_end (cmd)) {
    return;
  }

  if (to) {
    addr -= 4 * (cnt - 1);
  }

  for (i = 0; i < cnt; i++) {
    arm_dasm_mem (sim->cpu, &op, addr, par_xlat);
    sarm_dasm_str (str, &op);

    fprintf (stdout, "%08lX  %s\n", addr, str);

    addr += 4;
  }

  saddr = addr;
}

int sarm_do_cmd (cmd_t *cmd, simarm_t *sim)
{
  if (cmd_match (cmd, "b")) {
    do_b (cmd, sim);
  }
  else if (cmd_match (cmd, "c")) {
    do_c (cmd, sim);
  }
  else if (cmd_match (cmd, "d")) {
    do_d (cmd, sim);
  }
  else if (cmd_match (cmd, "e")) {
    do_e (cmd, sim);
  }
  else if (cmd_match (cmd, "g")) {
    do_g (cmd, sim);
  }
  else if (cmd_match (cmd, "h")) {
    do_h (cmd, sim);
  }
  else if (cmd_match (cmd, "key")) {
    do_key (cmd, sim);
  }
  else if (cmd_match (cmd, "p")) {
    do_p (cmd, sim);
  }
  else if (cmd_match (cmd, "r")) {
    do_r (cmd, sim);
  }
  else if (cmd_match (cmd, "s")) {
    do_s (cmd, sim);
  }
  else if (cmd_match (cmd, "t")) {
    do_t (cmd, sim);
  }
  else if (cmd_match (cmd, "u")) {
    do_u (cmd, sim);
  }
  else {
    cmd_error (cmd, "unknown command");
    return (1);
  }

  return (0);
}

void sarm_cmd_init (simarm_t *sim)
{
  sim->cpu->log_ext = sim;
  sim->cpu->log_opcode = NULL;
  sim->cpu->log_undef = &sarm_log_undef;
  sim->cpu->log_exception = &sarm_log_trap;
}
