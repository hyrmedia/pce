/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/cpu/arm/arm.c                                          *
 * Created:       2004-11-03 by Hampa Hug <hampa@hampa.ch>                   *
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arm.h"
#include "internal.h"


static unsigned arm_spsr_map[32] = {
  0,  /* 00 */
  0,  /* 01 */
  0,  /* 02 */
  0,  /* 03 */
  0,  /* 04 */
  0,  /* 05 */
  0,  /* 06 */
  0,  /* 07 */
  0,  /* 08 */
  0,  /* 09 */
  0,  /* 0a */
  0,  /* 0b */
  0,  /* 0c */
  0,  /* 0d */
  0,  /* 0e */
  0,  /* 0f */
  0,  /* 10 usr */
  1,  /* 11 fiq */
  2,  /* 12 irq */
  3,  /* 13 svc */
  0,  /* 14 */
  0,  /* 15 */
  0,  /* 16 */
  4,  /* 17 abt */
  0,  /* 18 */
  0,  /* 19 */
  5,  /* 1a und */
  0,  /* 1b */
  0,  /* 1c */
  0,  /* 1d */
  0,  /* 1e */
  0,  /* 1f sys */
};

static unsigned arm_reg_map[32][8] = {
  { 0,  0,  0,  0,  0,  0,  0,  0 },  /* 00 */
  { 0,  0,  0,  0,  0,  0,  0,  0 },  /* 01 */
  { 0,  0,  0,  0,  0,  0,  0,  0 },  /* 02 */
  { 0,  0,  0,  0,  0,  0,  0,  0 },  /* 03 */
  { 0,  0,  0,  0,  0,  0,  0,  0 },  /* 04 */
  { 0,  0,  0,  0,  0,  0,  0,  0 },  /* 05 */
  { 0,  0,  0,  0,  0,  0,  0,  0 },  /* 06 */
  { 0,  0,  0,  0,  0,  0,  0,  0 },  /* 07 */
  { 0,  0,  0,  0,  0,  0,  0,  0 },  /* 08 */
  { 0,  0,  0,  0,  0,  0,  0,  0 },  /* 09 */
  { 0,  0,  0,  0,  0,  0,  0,  0 },  /* 0a */
  { 0,  0,  0,  0,  0,  0,  0,  0 },  /* 0b */
  { 0,  0,  0,  0,  0,  0,  0,  0 },  /* 0c */
  { 0,  0,  0,  0,  0,  0,  0,  0 },  /* 0d */
  { 0,  0,  0,  0,  0,  0,  0,  0 },  /* 0e */
  { 0,  0,  0,  0,  0,  0,  0,  0 },  /* 0f */
  { 1,  2,  3,  4,  5,  6,  7,  8 },  /* 10 usr */
  { 9, 10, 11, 12, 13, 14, 15,  8 },  /* 11 fiq */
  { 1,  2,  3,  4,  5, 16, 17,  8 },  /* 12 irq */
  { 1,  2,  3,  4,  5, 18, 19,  8 },  /* 13 svc */
  { 0,  0,  0,  0,  0,  0,  0,  0 },  /* 14 */
  { 0,  0,  0,  0,  0,  0,  0,  0 },  /* 15 */
  { 0,  0,  0,  0,  0,  0,  0,  0 },  /* 16 */
  { 1,  2,  3,  4,  5, 20, 21,  8 },  /* 17 abt */
  { 0,  0,  0,  0,  0,  0,  0,  0 },  /* 18 */
  { 0,  0,  0,  0,  0,  0,  0,  0 },  /* 19 */
  { 1,  2,  3,  4,  5, 22, 23,  8 },  /* 1a und */
  { 0,  0,  0,  0,  0,  0,  0,  0 },  /* 1b */
  { 0,  0,  0,  0,  0,  0,  0,  0 },  /* 1c */
  { 0,  0,  0,  0,  0,  0,  0,  0 },  /* 1d */
  { 0,  0,  0,  0,  0,  0,  0,  0 },  /* 1e */
  { 1,  2,  3,  4,  5,  6,  7,  8 }   /* 1f sys */
};

void arm_init (arm_t *c)
{
  c->mem_ext = NULL;

  c->get_uint8 = NULL;
  c->get_uint16 = NULL;
  c->get_uint32 = NULL;

  c->set_uint8 = NULL;
  c->set_uint16 = NULL;
  c->set_uint32 = NULL;

  c->log_ext = NULL;
  c->log_opcode = NULL;
  c->log_undef = NULL;
  c->log_exception = NULL;

  arm_set_opcodes (c);

  c->reg_map = 0;

  c->cpsr = 0;

  c->oprcnt = 0;
  c->clkcnt = 0;
}

arm_t *arm_new (void)
{
  arm_t *c;

  c = (arm_t *) malloc (sizeof (arm_t));
  if (c == NULL) {
    return (NULL);
  }

  arm_init (c);

  return (c);
}

void arm_free (arm_t *c)
{
}

void arm_del (arm_t *c)
{
  if (c != NULL) {
    arm_free (c);
    free (c);
  }
}


void arm_set_mem_fct (arm_t *c, void *ext,
  void *get8, void *get16, void *get32,
  void *set8, void *set16, void *set32)
{
  c->mem_ext = ext;

  c->get_uint8 = get8;
  c->get_uint16 = get16;
  c->get_uint32 = get32;

  c->set_uint8 = set8;
  c->set_uint16 = set16;
  c->set_uint32 = set32;
}

unsigned long long arm_get_opcnt (arm_t *c)
{
  return (c->oprcnt);
}

unsigned long long arm_get_clkcnt (arm_t *c)
{
  return (c->clkcnt);
}

unsigned long arm_get_delay (arm_t *c)
{
  return (c->delay);
}


void arm_set_reg_map (arm_t *c, unsigned mode)
{
  unsigned i;
  unsigned m1, m2;
  unsigned *map1, *map2;

  m1 = c->reg_map;
  m2 = mode & 0x1f;

  if (m1 == m2) {
    return;
  }

  c->spsr_alt[arm_spsr_map[m1]] = c->spsr;
  c->spsr = c->spsr_alt[arm_spsr_map[m2]];

  map1 = arm_reg_map[m1];
  map2 = arm_reg_map[m2];

  for (i = 0; i < 8; i++) {
    c->reg_alt[map1[i]] = c->reg[i + 8];
    c->reg[i + 8] = c->reg_alt[map2[i]];
  }

  c->reg_map = m2;
}

void arm_reset (arm_t *c)
{
  unsigned i;

  c->cpsr = ARM_MODE_SVC;

  arm_set_reg_map (c, ARM_MODE_SVC);

  for (i = 0; i < 16; i++) {
    c->reg[i] = 0;
  }

  for (i = 0; i < ARM_REG_ALT_CNT; i++) {
    c->reg_alt[i] = 0;
  }

  for (i = 0; i < ARM_SPSR_CNT; i++) {
    c->spsr_alt[i] = 0;
  }

  c->ir = 0;

  c->interrupt = 0;

  c->delay = 1;

  c->oprcnt = 0;
  c->clkcnt = 0;
}

void arm_undefined (arm_t *c)
{
  if (c->log_undef != NULL) {
    c->log_undef (c->log_ext, c->ir);
  }
}

void arm_exception (arm_t *c, uint32_t addr, uint32_t ret, unsigned mode)
{
  uint32_t cpsr;

  if (c->log_exception != NULL) {
    c->log_exception (c->log_ext, addr);
  }

  c->delay += 1;

  cpsr = arm_get_cpsr (c);

  cpsr &= ~(ARM_PSR_T | ARM_PSR_I | ARM_PSR_M);
  cpsr |= mode & 0x1f;

  arm_write_cpsr (c, cpsr);
  arm_set_spsr (c, cpsr);
  arm_set_lr (c, ret);
  arm_set_pc (c, addr);
}

void arm_exception_reset (arm_t *c)
{
  arm_exception (c, 0x00000000UL, 0, ARM_MODE_SVC);

  arm_set_cpsr_f (c, 0);
}

void arm_exception_undefined (arm_t *c)
{
  arm_exception (c, 0x00000004UL, arm_get_pc (c) + 4, ARM_MODE_UND);
}

void arm_exception_swi (arm_t *c)
{
  arm_exception (c, 0x00000008UL, arm_get_pc (c) + 4, ARM_MODE_SVC);
}

void arm_exception_prefetch_abort (arm_t *c)
{
  arm_exception (c, 0x0000000cUL, arm_get_pc (c) + 4, ARM_MODE_ABT);
}

void arm_exception_data_abort (arm_t *c)
{
  arm_exception (c, 0x00000010UL, arm_get_pc (c) + 8, ARM_MODE_ABT);
}

void arm_interrupt (arm_t *c, unsigned char val)
{
  c->interrupt = (val != 0);
}

void arm_execute (arm_t *c)
{
  if (arm_ifetch (c, arm_get_pc (c), &c->ir)) {
    return;
  }

  if (c->log_opcode != NULL) {
    c->log_opcode (c->log_ext, c->ir);
  }

  if (arm_check_cond (c, arm_ir_cond (c->ir))) {
    c->opcodes[(c->ir >> 20) & 0xff] (c);
  }
  else {
    arm_set_clk (c, 4, 1);
  }

  c->oprcnt += 1;

  if (c->interrupt) {
    if (arm_get_cpsr_i (c)) {
      /* ... */
    }
  }
}

void arm_clock (arm_t *c, unsigned long n)
{
  while (n >= c->delay) {
    if (c->delay == 0) {
      fprintf (stderr, "warning: delay == 0 at %08lx\n", (unsigned long) arm_get_pc (c));
      fflush (stderr);
      c->delay = 1;
    }

    n -= c->delay;

    c->clkcnt += c->delay;
    c->delay = 0;

    arm_execute (c);
  }

  c->clkcnt += n;
  c->delay -= n;
}
