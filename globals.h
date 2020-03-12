
#define MAX_FILE_LEN 100
#define TOKEN_LEN 31
#define MAX_CMD_TOKENS 10
#define OPERATIONS_CNT 16
#define SYMBOL_LEN 21
#define MEMORY_SIZE 4096
#define WORD_SIZE 16
#define HIGHEST_POSITIVE_15 16383
#define LOWEST_NEGATIVE_15 -16384
#define HIGHEST_POSITIVE_12 2047
#define LOWEST_NEGATIVE_12 -2048
#define FIRST_ITERATION 1
#define OCTAL_LEN 6
#define SECOND_ITERATION 2
#define MAX_FILE_NAME 50
#define ASCII_DEC_DIFF 48
#define IC_OFFSET 100
#define OUTPUT_LINE_SIZE 30





extern int dc, ic, line, symbolFlag;

typedef struct word {
	char bits[WORD_SIZE];
}word;

typedef struct external {
	char name[SYMBOL_LEN];
	int address;
	struct external* next;
}external;


typedef struct ram {
	word memory[MEMORY_SIZE];
}ram;

typedef struct Symbol {
	char name[SYMBOL_LEN];
	int address;
	int isInstruction;
	int isExternal;
	int isEntry;
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
	
	struct Instruction * next;
} instruction;


