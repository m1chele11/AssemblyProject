#include <stdlib.h>
#include <stdio.h>
#define NOT_IMPLEMENTED -1

/*
 *  Student functions - Fill in each so they extract the correct value from the instruction
 */

int get_opcode(int instruction) {
  return (instruction >> 2) & 0x1F;
}

/*  Returns an integer representing the type of instruction  *
 *    0 - Load     (opcode == 0x0)                           *
 *    1 - Store    (opcode == 0x8)                           *
 *    2 - Math     (opcode == 0xC)                           *
 *    3 - Constant (opcode == 0xD)                           */
int get_instruction_type(int instruction) {
  int opcode = get_opcode(instruction);  

  switch (opcode) {
    case 0x0: 
      return 0;  // Load
    case 0x8: 
      return 1;  // Store
    case 0xC: 
      return 2;  // Math
    case 0xD: 
      return 3;  // Constant
    default: 
      return -1; // Invalid or unknown
  }
}

int get_width(int instruction) {
  return (instruction >> 12) & 0x7;
}

int get_destination(int instruction) {
  return (instruction >> 7) & 0x1F;
}

int get_load_address(int instruction) {
  return (instruction >> 15) & 0x1F;
}

int get_load_offset(int instruction) {
  return (instruction >> 20) & 0xFFF;
}

int get_store_offset(int instruction) {
  int upper_offset = (instruction >> 25) & 0x7F;  // Bits 11 to 5
  int lower_offset = (instruction >> 7) & 0x1F;   // Bits 4 to 0 
  return (upper_offset << 5) | lower_offset;
}

int get_store_source(int instruction) {
  return (instruction >> 20) & 0x1F;
}

int get_store_address(int instruction) {
  return (instruction >> 15) & 0x1F;
}

int get_math_function(int instruction) {
  return (instruction >> 12) & 0x7;
}

int get_math_operand_a(int instruction) {
  return (instruction >> 15) & 0x1F;
}

int get_math_operand_b(int instruction) {
  return (instruction >> 20) & 0x1F;
}

int get_constant_value(int instruction) {
  return instruction >> 12;
}




/*                                                                                       *
 *  Helper functions for converting integers into the human readable text they represent *
 *                                                                                       */
/*  Convert an integer index into the name of a register  */

const char* int_to_register(int idx) {
  if (idx >= 32) {
    printf("[ERROR] Bad register index\n");
    exit(-3);
  }
  return ((const char*[]){"zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
			  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"})[idx];
}

/*  Convert an integer index into the name of a function  */
const char* int_to_function(int idx) {
  if (idx >= 8) {
    printf("[ERROR] Bad function index\n");
    exit(-3);
  }
  return ((const char*[]){"add", "sll", "slt", "sltu", "xor", "srl", "or", "and"})[idx];
}

/*  Convert an integer index into a textual explanation of the function's bit width  */
const char* int_to_width(int idx) {
  if (idx >= 5 || idx == 0x3) {
    printf("[ERROR] Bad width index\n");
    exit(-3);
  }
  return ((const char*[]){"8-bits", "16-bits", "32-bits", "", "8-bits (unsigned)", "16-bits (unsigned)"})[idx];
}

/*  Check if a function has been implemented  */
int is_implemented(const char* name, int (*fn)(int), int instruction) {
  int opcode = fn(instruction);
  if (opcode == NOT_IMPLEMENTED) {
    printf("[ERROR] `%s` has not been implemented\n", name);
    exit(-2);
  }
  return opcode;
}

/*
 *  Main function calculates and prints the instruction passed in through argument 1
 */
int main(int argc, char** argv) {
  /*  Ensure the command line arguments are valid  */
  if (argc != 2) {
    printf("[ERROR] Invalid arguments\n");
    return -1;
  }
  int instruction  = strtol(argv[1], NULL, 0);
  if (instruction == 0) {
    printf("[ERROR] Instruction was not a valid number");
    return -2;
  }

  printf("\n\n    Instruction  ==  ");
  for (int i=31; i>=0; i--) {
    printf("%d", (instruction >> i) & 0x1);
    if (i % 4 == 0) printf(" ");
  }
  printf("\n\n");
  switch (is_implemented("get_instruction_type", get_instruction_type, instruction)) {
  case 0x0 : {
    int width = is_implemented("get_width", get_width, instruction);
    int src = is_implemented("get_load_address", get_load_address, instruction);
    int offset = is_implemented("get_load_offset", get_load_offset, instruction);
    int dst = is_implemented("get_destination", get_destination, instruction);
    printf("Breakdown:\n  [offset=0x%x] [source=0x%x] [width=0x%x] [dst=0x%x] opcode=0x%x\n", offset, src, width, dst, is_implemented("get_opcode", get_opcode, instruction));
    printf("\nTranslation:\n  Load %s from the address in register [%s]+0x%x into register [%s]\n\n",
	   int_to_width(width), int_to_register(src), offset, int_to_register(dst));
    break;
  }
  case 0x1 : {
    int width = is_implemented("get_width", get_width, instruction);
    int src = is_implemented("get_store_source", get_store_source, instruction);
    int addr = is_implemented("get_store_address", get_store_address, instruction);
    int offset = is_implemented("get_store_offset", get_store_offset, instruction);
    printf("Breakdown:\n  [offset=0x%x] [addr=0x%x] [source=0x%x] [width=0x%x] opcode=0x%x\n", offset, addr, src, width, is_implemented("get_opcode", get_opcode,instruction));
    printf("\nTranslation:\n  Store %s into the address in register [%s]+0x%x from register [%s]\n\n",
	   int_to_width(width), int_to_register(addr), offset, int_to_register(src));
    break;
  }
  case 0x2 : {
    int a = is_implemented("get_math_operand_a", get_math_operand_a, instruction);
    int b = is_implemented("get_math_operand_b", get_math_operand_b, instruction);
    int op = is_implemented("get_math_function", get_math_function, instruction);
    int dst = is_implemented("get_destination", get_destination, instruction);
    printf("Breakdown:\n  [b=0x%x] [a=0x%x] [funcion=0x%x] [dst=0x%x] opcode=0x%x\n", b, a, op, dst, is_implemented("get_opcode", get_opcode, instruction));
    printf("\nTranslation:\n  Perform '%s' on registers [%s] and [%s] and store result in [%s]\n\n",
	   int_to_function(op),
	   int_to_register(a),
	   int_to_register(b),
	   int_to_register(dst));
    break;
  }
  case 0x3 : {
    int val = is_implemented("get_constant_value", get_constant_value, instruction);
    int dst = is_implemented("get_destination", get_destination, instruction);
    printf("Breakdown:\n  [value=0x%x] [dst=0x%x] opcode=0x%x\n", val, dst, is_implemented("get_opcode", get_opcode, instruction));
    printf("\nTranslation:\n  Set register [%s] to the constant value %d\n\n",
	   int_to_register(dst), val);
    break;
  }
  default :
    printf("[ERROR] Unknown error code\n");
    return -2;
  }
}
