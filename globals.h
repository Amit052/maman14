#pragma once
#include "hardware.h"
#define MAX_FILE_LEN 20
#define TOKEN_LEN 31
#define MAX_CMD_TOKENS 20
#define OPERATIONS_CNT 16
#define SYMBOL_LEN 21
extern int dc, ic, line, symbolFlag;

typedef struct Symbol {
	char name[SYMBOL_LEN];
	int address;
	int isInstruction;
	int isExternal;
	struct symbol * next;
}symbol;
typedef struct Data {
	int address;
	word value;
	struct data* next;
} data;
typedef struct Instruction {
	int address;
	word value;
<<<<<<< HEAD
	struct Instruction* next;
=======
	struct Instruction * next;
>>>>>>> amit
} instruction;

 
/*TODO - ADD flags like "symbol definition"*/

