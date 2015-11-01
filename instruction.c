#include <stdint.h>
#include <string.h>

#include "instruction.h"
#include "emulator.h"
#include "emulator_function.h"

#include "modrm.h"

instruction_func_t* instructions[256];

/* MOV命令 */
static void mov_r32_imm32(Emulator* emu)
{
  uint8_t reg = get_code8(emu, 0) - 0xB8;
  uint32_t value = get_code32(emu, 1);
  emu->registers[reg] = value;
  emu->eip += 5;
}

static void mov_rm32_imm32(Emulator* emu)
{
  emu->eip += 1;
  ModRM modrm;
  parse_modrm(emu, &modrm);
  uint32_t value = get_code32(emu, 0);
  emu->eip += 4;
  set_rm32(emu, &modrm, value);
}

static void mov_rm32_r32(Emulator* emu)
{
  emu->eip += 1;
  ModRM modrm;
  parse_modrm(emu, &modrm);
  uint32_t r32 = get_r32(emu, &modrm);
  set_rm32(emu, &modrm, r32);
}

static void mov_r32_rm32(Emulator* emu)
{
  emu->eip += 1;
  ModRM modrm;
  parse_modrm(emu, &modrm);
  uint32_t rm32 = get_rm32(emu, &modrm);
  set_r32(emu, &modrm, rm32);
}

/* ADD命令 */
static void add_rm32_r32(Emulator* emu)
{
  emu->eip += 1;
  ModRM modrm;
  parse_modrm(emu, &modrm);
  uint32_t r32 = get_r32(emu, &modrm);
  uint32_t rm32 = get_rm32(emu, &modrm);
  set_rm32(emu, &modrm, rm32 + r32);
}

/* JMP命令(ショートジャンプ) */
static void short_jump(Emulator* emu)
{
  int8_t diff = get_sign_code8(emu, 1);
  /* eipに足しこむ．2はjmp命令分 */
  emu->eip += (diff + 2);
}

/* JMP命令(ニアジャンプ) */
static void near_jump(Emulator* emu)
{
  int32_t diff = get_sign_code32(emu, 1);
  emu->eip += (diff + 5);
}

/* 関数ポインタテーブル */
void init_instructions(void)
{
  int i;
  memset(instructions, 0, sizeof(instructions));
  instructions[0x01] = add_rm32_r32;
  instructions[0x89] = mov_rm32_r32;
  instructions[0x8B] = mov_r32_rm32;
  for (i = 0; i < 8; i++) {
    instructions[0xB8 + i] = mov_r32_imm32;
  }
  instructions[0xC7] = mov_rm32_imm32;
  instructions[0xE9] = near_jump;
  instructions[0xEB] = short_jump;
}