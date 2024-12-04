#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NOT_IMPLEMENTED -1



/*
 *  Stduent functions - Full in each so that they return the integer value of the requested instruction
 */
int generate_load_instruction(int width, int destination, int address, int offset) {
  int opcode = 0x03; // Load opcode
  int funct3 = width & 0x7; 

  // Mask the values to ensure no extra bits are introduced
  destination &= 0x1F;
  address &= 0x1F;
  offset &= 0xFFF;

  //printf("Destination: 0x%x, Address: 0x%x, Offset: 0x%x\n", destination, address, offset);
  return (offset << 20) | (address << 15) | (funct3 << 12) | (destination << 7) | opcode;
  
}

int generate_store_instruction(int width, int address, int source, int offset) {
  int opcode = 0x23; // Store opcode
  int funct3 = width & 0x7; 
  int imm11_5 = (offset >> 5) & 0x7F;
  int imm4_0 = offset & 0x1F;

  // Make sure to mask both address and source to fit within 5 bits (0x1F)
  address = address & 0x1F;  // Store address is a 5-bit field
  source = source & 0x1F;    // Store source is a 5-bit field

  return (imm11_5 << 25) | (source << 20) | (address << 15) | (funct3 << 12) | (imm4_0 << 7) | opcode;
}

int generate_math_instruction(int function, int a, int b, int destination) {
  int opcode = 0x33; // R-type opcode

  // Funct7 is determined based on the function
  int funct7 = (function == 0x0) ? 0x00 : ((function == 0x1 || function == 0x5) ? 0x00 : 0x20);
  
  // Funct3 is the lower 3 bits of function
  int funct3 = function & 0x7;

  // Mask A, B, and destination to ensure they fit in 5 bits
  a = a & 0x1F;            // 5 bits for source A
  b = b & 0x1F;            // 5 bits for source B
  destination = destination & 0x1F;

  return (funct7 << 25) | (b << 20) | (a << 15) | (funct3 << 12) | (destination << 7) | opcode;
}


int generate_constant_instruction(int value, int destination) {
   int opcode = 0x37; // LUI opcode

    // Mask the value to only use the upper 20 bits
    value = value & 0xFFFFF;

    // Mask the destination to ensure it's a 5-bit value
    destination = destination & 0x1F;

    // Shift the value left by 12 bits, combine with destination and opcode
    return (value << 12) | (destination << 7) | opcode;
}


/*                                                                                    *
 *  Helper functions used by main to convert string instructions into numeric values  *
 *                                                                                    */
/*  Calculate the integer that represents the instruction bit width based on its text  */
int get_width(const char* str) {
  switch (str[1]) {
  case 'b': {
      if (str[2] == 'u')
	return 0x4;
      return 0x0;
    }
  case 'h': {
      if (str[2] == 'u')
	return 0x5;
      return 0x1;
    }
  case 'w': return 0x2;
  default:
    printf("[ERROR] Unknown width index\n");
    exit(-2);
  }
}

/*  Calculate the integer index that represents the instruction function based on its text  */
int get_func(const char* str) {
  const char* fns[] = {"add", "sll", "slt", "sltu", "xor", "srl", "or", "and"};
  for (int i=0; i<8; i++)
    if (strcmp(fns[i], str) == 0)
      return i;
  printf("[ERROR] Unknown function name\n");
  exit(-5);
}

/*  Calculates the integer register index based on the register name  */
static const char* regs[] = {"zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};
int get_reg(const char* str) {
  int i = 0;
  for (; str[i] != '\0'; i++)
    if (str[i] == ',')
      break;
  
  for (int j=0; j<32; j++) {
    if (strncmp(str, regs[j], i) == 0)
      return j;
  }
  printf("[ERROR] Unknown register index\n");
  exit(-3);
}

/*  Parses the integer register index and integer offset of a register from a memory style instruction i.e. 8(a0)  */
int get_mem_reg(const char* str, int* offset) {
  int i = 0;
  char* ptr;
  *offset = strtol(str, &ptr, 0);
  if (str == ptr) {
    printf("[ERROR] Bad memory address syntax\n");
    exit(-4);
  }
  
  for (i=0; ptr[i] != ')' && ptr[i] != '\0'; i++);
  if (ptr[0] != '(' || ptr[i] != ')') {
    printf("[ERROR] Bad memory address syntax\n");
    exit(-4);
  }

  ++ptr;
  for (int j=0; j<32; j++)
    if (strncmp(ptr, regs[j], i-1) == 0)
      return j;
  printf("[ERROR] Unknown register index\n");
  exit(-3);
}

/*  Check if a function has been implemented  */
int is_implemented(const char* name, int (*fn)(int, int, int, int), int width, int destination, int source, int offset) {
  int opcode = fn(width, destination, source, offset);
  if (opcode == NOT_IMPLEMENTED) {
    printf("[ERROR] `%s` has not been implemented\n", name);
    exit(-2);
  }
  return opcode;
}
int is_constant_implemented(const char* name, int (*fn)(int, int), int value, int destination) {
  int opcode = fn(value, destination);
  if (opcode == NOT_IMPLEMENTED) {
    printf("[ERROR] `%s` has not been implemented\n", name);
    exit(-2);
  }
  return opcode;
}


/*
 *  Main function converts an assembly instruction into an integer and prints the result
 */
int main(int argc, char** argv) {
  int j = 1;
  char* args[5];
  args[j++] = argv[1];
  if (argc != 2) {
    printf("[ERROR] Too many arguments\n");
    return -1;
  }

  for (int i=0; argv[1][i] != '\0'; i++) {
    if (argv[1][i] == ' ') {
      argv[1][i] = '\0';
      args[j++] = &(argv[1][++i]);
    }
  }
  
  if (strcmp(args[1], "lui") == 0) {
    if (j != 4) {
      printf("[ERROR] Incorrect arguments for constant\n");
      return -2;
    }

    int value = strtol(args[3], NULL, 0), dst = get_reg(args[2]);
    printf("Breakdown:\n  [type=constant] [value=0x%x] [dst=0x%x]\n", value, dst);
    printf("\n  Instruction: 0x%x\n\n", is_constant_implemented("generate_constant_instruction", generate_constant_instruction, value, dst));
    return 0;
  }
  else if (args[1][0] == 's') {
    if (j != 4) {
      printf("[ERROR] Incorrect arguments for store\n");
      return -2;
    }
    int offset, width = get_width(args[1]);
    int reg = get_mem_reg(args[3], &offset);
    printf("Breakdown:\n  [type=store] [width=0x%x] [addr=0x%x] [src=0x%x] [offset=0x%x]\n", width, reg, get_reg(args[2]), offset);
    printf("\n  Instruction: 0x%x\n\n", is_implemented("generate_store_instruction", generate_store_instruction, width, reg, get_reg(args[2]), offset));
  }
  else if (args[1][0] == 'l') {
    if (j != 4) {
      printf("[ERROR] Incorrect arguments for load\n");
      return -2;
    }
    int width = get_width(args[1]);
    int offset;
    int reg = get_mem_reg(args[3], &offset);
    printf("Breakdown:\n  [type=load] [width=0x%x] [dst=0x%x] [addr=0x%x] [offset=0x%x]\n", width, get_reg(args[2]), reg, offset);
    printf("\n  Instruction: 0x%x\n\n", is_implemented("generate_load_instruction", generate_load_instruction, width, get_reg(args[2]), reg, offset));
  }
  else {
    printf("Breakdown:\n  [type=math] [func=0x%x] [a=0x%x] [b=0x%x] [dst=0x%x]\n", get_func(args[1]), get_reg(args[3]), get_reg(args[4]), get_reg(args[2]));
    printf("\n  Instruction: 0x%x\n\n", is_implemented("generate_math_instruction", generate_math_instruction, get_func(args[1]), get_reg(args[3]), get_reg(args[4]), get_reg(args[2])));
  }
}
