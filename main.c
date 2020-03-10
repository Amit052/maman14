#define _CRT_SECURE_NO_WARNINGS 1
#define _CRT_NONSTDC_NO_WARNINGS 1
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "globals.h"

//TODO -> FIX A.R.E FIELDS
/*function declerations*/
void read_file(char file_name[MAX_FILE_NAME], int iteration);
external* create_extern(external* new_node, int ic, char* val);
int update_instruction(instruction** int_head, symbol** sym_head, int ins_address, char* name);
int tokenize_cmd(char str[MAX_CMD_TOKENS * TOKEN_LEN + 1], char tokens[MAX_CMD_TOKENS][TOKEN_LEN]);
int updateSymbolTable(symbol ** head, char name[SYMBOL_LEN], int isInstruction, int isExternal);
int updateFlag(int * flag, int );
void print_externalsTable(  head);
int updateDataTable(data** head, char* val, int tt);
int get_operand_type(char* op);
int update_instruction_table(instruction** h, word w);
void print_entries(symbol** h);
void handle_symbol_and_op(int op, char* val_l, char* val_r);
int update_symbol(symbol** h, char* name);
void create_first_bin_word(word* w, int op_code, int op_l, int op_r);
void initSymbols(symbol** h);
void print_externals(symbol** h);
void fix_symbolTable_addresses(symbol** head);
void print_instructionsTable(instruction** head);
instruction* create_instruction(instruction* new_node, word w);
char* dec_2_bin(char* str, int num, int length);
char* op_2_str(int op, char* val);
void initData(data** h);
int is_op_valid(int op, int op_l, int op_r);
int prepare_instruction_words(int op_code, int op_l, int op_r, char* val_l, char* val_r);
void initInstructions(instruction** h);
symbol * create_symbol(symbol * new_node, char name[SYMBOL_LEN], int isInstruction, int isExternal);
int symbolExists(symbol ** h, char name[SYMBOL_LEN]);
int getTokenType(char token[TOKEN_LEN]);
void print_dataTable(data ** head);
void print_symbolTable(symbol ** head);
data * create_data(data * new_node, char val[TOKEN_LEN], int tt);
int handleLine(int fields, char tokens[MAX_CMD_TOKENS][TOKEN_LEN]);
int finishHim(int fields, char tokens[MAX_CMD_TOKENS][TOKEN_LEN]);
int updateDataTable(data** head, char* val, int tt);
int isRegister(char* reg);
int isNum(char* num);
void handle_operation(int start, int op, int fields, char tokens[MAX_CMD_TOKENS][TOKEN_LEN]);
/*function declerations end*/

int dc = 0, ic = 0, line = 0, symbolFlag = 0, errorFlag = 0;
enum tokenType { Symbol, Extern, Entry, Data, String };
enum flagState { Off, On };
enum char_type { NUM, STR };
enum operands { Num, Var, P_reg, Reg};
char operations[OPERATIONS_CNT][2][5] = {
	{"mov", "2"},{"cmp", "2"},{"add", "2"},{"sub", "2"},{"lea", "2"},{"clr", "1"},{"not", "1"},{"inc", "1"},{"dec", "1"},{"jmp", "1"},{"bne", "1"},
	{"red", "1"},{"prn", "1"},{"jsr", "1"},{"rts", "0"}, {"stop", "0"}
};
symbol * symbolsTable;
data * dataTable;
instruction * instructionTable;
external* externalTable;


void main(int argc, char *argv[]) {
	externalTable = NULL;  symbolsTable = NULL; dataTable = NULL; instructionTable = NULL;
	int i;
	char* fname;
	if (argc > 1) {
		
		for (i = 1; i < argc; i++){
			dc = ic = 0;
			fname = (char*)malloc(sizeof(char) * strlen(argv[i]) + 4);
			if (fname != NULL) {
				strcpy(fname, argv[i]);
				strcat(fname, ".as");
			}
			else {
				puts("Unable to allocate memory.\n");
				continue;
			}
			printf("_______________________%s_______________________\n", fname);
			read_file(fname, FIRST_ITERATION);
			ic = 0;
			if (errorFlag == 1)
				printf("Errors found on first iteration, program stops here :(\n");
			else {
				fix_symbolTable_addresses(&symbolsTable);
				read_file(fname, SECOND_ITERATION);
				printf("IC: %d\n", ic - 1);
				printf("DC: %d\n", dc);
				print_instructionsTable(&instructionTable);
				print_dataTable(&dataTable);
				print_entries(&symbolsTable);
				print_externalsTable(&externalTable);
			}
		}
	}
	else {
		externalTable = NULL; symbolsTable = NULL;  dataTable = NULL; instructionTable = NULL;
		read_file("badpath-indirect.as", FIRST_ITERATION);
		fix_symbolTable_addresses(&symbolsTable);
		ic = 0;
		if (errorFlag == 1)
			printf("Errors found on first iteration, program stops here :(\n");
		else
		{
			read_file("badpath-indirect.as", SECOND_ITERATION);
			printf("IC: %d\n", ic - 1);
			printf("DC: %d\n", dc);
			print_instructionsTable(&instructionTable);
			print_dataTable(&dataTable);
			print_entries(&symbolsTable);
			print_externalsTable(&externalTable);
		}

	}
}

char* bin_2_octal(char * bin, char octal[6]) {
	int i, len, p, tmp;
	char res[16];
	len = strlen(bin) - 1;
	for (tmp = 0,p = 1, i = len; i >= 0; i--) {
		if (bin[i] == '1') {
			tmp += p;
		}
		p *= 2;
	}
	if (tmp == 0) {
		strcpy(res, "00000");
	}
	else {
		p = 0;
		while (tmp != 0) {
			res[p++] = (tmp % 8) + ASCII_DEC_DIFF;
			tmp /= 8;
		}
	}
		while (p < 5)
			res[p++] = '0';
		res[5] = '\0';
		strcpy(octal, strrev(res));
	//printf("bin: %s =>%s\n", bin,  strrev(res));
	return octal;
}
void print_entries(symbol** head) {
	symbol* cur_node = NULL;
	puts("Entries:");
	while (*head) {
		if ((*head)->isEntry == 1) {
			printf("%d %s\n", (*head)->address, (*head)->name);
		}
		cur_node = *head;
		*head = (*head)->next;
		free(cur_node);
	}

}
void print_externals(symbol** h) {
	symbol* cur_node = *h;
	puts("Externals:");
	while (cur_node) {
		if (cur_node->isExternal == 1) {
			printf("%d %s\n", cur_node->address, cur_node->name);
		}
		cur_node = cur_node->next;
	}
}
void read_file(char file_name[MAX_FILE_NAME], int iteration) {
	char tokens[MAX_FILE_LEN][MAX_CMD_TOKENS][TOKEN_LEN] = {'\0'};
	char buffer[MAX_CMD_TOKENS * TOKEN_LEN + 1];
	int cnt = 0;
	FILE *fp;
	fp = fopen(file_name, "r+");
	if (fp == NULL) {
		printf("Error->unable to open file [%s].\n", file_name);
		return;
	}
	/*read line*/
	while (fgets(buffer, MAX_CMD_TOKENS * TOKEN_LEN + 1, fp) != NULL) {
		int fields;
		if (buffer[0] == ';') continue;//Comment line
		fields = tokenize_cmd(buffer, tokens[line]);/*separate to fields*/
		if (iteration == FIRST_ITERATION)
			handleLine(fields, tokens[line++]);
		else
			finishHim(fields, tokens[line++]);
		buffer[0] = 0;/*clear the buffer to be ready for next line*/
		if (++cnt == MAX_FILE_LEN) {
			printf("File too long...\n");
			break;
		}
	}
	line = 0;
	fclose(fp);
	
}

int finishHim(int fields, char tokens[MAX_CMD_TOKENS][TOKEN_LEN]) {
	int ff, i;//ff == first field
	ff = getTokenType(tokens[0]);/*get field type*/
	if (ff == Extern || ff == String || ff == Data) return 1;
	if (ff == Entry) {/**/
		for (i = 1; i < fields; i++) {
			if (strcmp(tokens[i], ",") == 0) continue;
			if (update_symbol(&symbolsTable, tokens[i]) == 0) {
				printf("Symbol was not found! [%s]\n", tokens[i]);
			}
		}
	}
	if (ff == Symbol) {/**/
		int sf;
		sf = getTokenType(tokens[1]);
		if (sf >= 10) {
			int tmp;
			tmp = atoi(operations[sf - 10][1]);
			if (tmp == 2)
				handle_symbol_and_op(sf - 10, tokens[2], tokens[4]);
			if (tmp == 1)
				handle_symbol_and_op(sf - 10, tokens[2], "");
			if (tmp == 0)
				handle_symbol_and_op(sf - 10, "", "");
		}
	}
	////////
	if (ff >= 10) {
		int tmp;
		tmp = atoi(operations[ff - 10][1]);
		if (tmp == 2)
			handle_symbol_and_op(ff - 10, tokens[1], tokens[3]);
		if (tmp == 1)
			handle_symbol_and_op(ff - 10, tokens[1], "");
		if (tmp == 0)
			handle_symbol_and_op(ff - 10, "", "");
	}
}

int update_if_extern(symbol ** h, external ** ex_head, char * val, int ic) {
	symbol* cur_node = *h;
	external* new_node = *ex_head;
	int flag = 0;
	////
	while (cur_node) {
		if (strcmp(cur_node->name, val) == 0 && cur_node->isExternal == 1) {
			flag = 1;
			break;
		}
		cur_node = cur_node->next;
	}
	///
	if (flag == 0) return 0;
	if (!new_node) {
		new_node = create_extern(new_node, ic, val);
		*ex_head = new_node;
	}
	else {
		while (new_node->next != NULL)
			new_node = new_node->next;
		new_node->next = create_extern(new_node, ic, val);
	}

	return 0;
}
external* create_extern(external * new_node, int ic, char * val) {
	new_node = (external*)malloc(sizeof(external));
	if (!new_node) return NULL;
	new_node->address = ic + IC_OFFSET;
	strcpy(new_node->name, val);
	new_node->next = NULL;
	return new_node;
}
void handle_symbol_and_op(int op, char * val_l, char * val_r ) {
	int op_l,  op_r;
	ic++;//first word for the operation
	if (strcmp(operations[op][1], "2") == 0) {
		op_l = get_operand_type(val_l);
		op_r = get_operand_type(val_r);
		if (op_l == Var) {
			update_if_extern(&symbolsTable, &externalTable, val_l, ic);
			update_instruction(&instructionTable, &symbolsTable, ic++, val_l);
		}
		if (op_l == Num) ic++;
		if (op_l == Reg) ic++;
		if (op_l == P_reg) ic++;
		if (op_r == Var) { 
			update_if_extern(&symbolsTable, &externalTable, val_r, ic); 
			update_instruction(&instructionTable, &symbolsTable, ic++, val_r);
		}
		if (op_r == Num) ic++;
		if (op_r == Reg && (op_l != Reg && op_l != P_reg)) ic++;
		if (op_r == P_reg && (op_l != Reg && op_l != P_reg)) ic++;
	}
	else if (strcmp(operations[op][1], "1") == 0) {
		op_l = get_operand_type(val_l);
		if (op_l == Var){
			update_if_extern(&symbolsTable, &externalTable, val_l, ic); 
			update_instruction(&instructionTable, &symbolsTable, ic++, val_l);
			}
		if (op_l == Num) ic++;
		if (op_l == Reg) ic++;
		if (op_l == P_reg) ic++;
	}
	else if (strcmp(operations[op][1], "0") == 0) ic++;
}

int update_instruction(instruction ** int_head, symbol** sym_head, int ins_address, char * name) {
	symbol * sym_node = *sym_head;
	instruction * ins_node = *int_head;
	char tmp_address[16];
	int flag = 0;
	while (sym_node) {
		if (strcmp(sym_node->name, name) == 0) {
			dec_2_bin(tmp_address, sym_node->address, 12);
			strcat(tmp_address, "010");
			flag = 1;
			break;
		}
		sym_node = sym_node->next;
	}
	if (flag == 0) { 
		puts(name);
		puts("NOT FOUND");
		return 0; 
	}
	while (ins_node) {
		if (ins_node->address == ins_address + IC_OFFSET) {
			strcpy(&ins_node->value, tmp_address);
			return 1;
		}
		ins_node = ins_node->next;
	}
	return 0;

}
int update_symbol(symbol** h, char * name) {
	symbol * cur_node = *h;
	while (cur_node) {
		if (strcmp(cur_node->name, name) == 0) {
			cur_node->isEntry = 1;
			return 1;
		}
		cur_node = cur_node->next;
	}
	return 0;
}

int handleLine(int fields, char tokens[MAX_CMD_TOKENS][TOKEN_LEN]) {
	int ff, t;//ff == first field
	ff = getTokenType(tokens[0]);/*get field type*/
	t = symbolExists(&symbolsTable, tokens[0]);
	if (ff == Extern || ff == Symbol) {
		if (t == 1) {
			printf("%s -> Symbol already exists.\n", tokens[0]);
			return 0;
		}
		else if (t == -1) {
			printf("%s -> cannot use operation name to a sybol.\n", tokens[0]);
			return 0;
		}
		else if (t == -2) {
			printf("%s -> cannot use Register name to a symbol.\n", tokens[0]);
			return 0;
		}
	}
	if (ff == Extern) {/*if Extern, update symbol table with second field*/
		int tmp;
		tmp = updateSymbolTable(&symbolsTable, tokens[1], 0, 1);
		if (tmp == 0) printf("Error saving symbol %s\n", tokens[1]);
		return 0;
	}
	/*if Entry, need to ignore on first round*/
	else if (ff == Entry) {
		return 1;
	}
	else if (ff == Symbol) {
		int i, sf;//second flag
		updateFlag(&symbolFlag, On);/*turn on symbol flag*/
		sf = getTokenType(tokens[1]);
		if (sf == String || sf == Data) {//if instruction, parse parameters to word and move to data table
				if (updateSymbolTable(&symbolsTable, tokens[0], 1, 0)) {
					int psikFlag = 0;
					for (i = 2; i < fields; i++) {
						if (strcmp(tokens[i], ",") == 0) {
							if (psikFlag == 1) {
								errorFlag = 1;
								printf("Too many psiks between the values!\n");
								return 0;
							}
							if (i == fields - 1) {
								errorFlag = 1;
								printf("Too many psiks between the values!\n");
								return 0;
							}
							psikFlag = 1;
							continue; 
						}
						if (sf == Data) {
							int j = 0;
							if (tokens[0] == '-') j = 1;
							for (; j < strlen(tokens[i]); j++) {
								if (tokens[i][j] < '0' || tokens[i][j] >'9') {
									errorFlag = 1;
									printf("invalid value: %s\n", tokens[i]);
									return 0;
								}
							}
							if (psikFlag == 0 && i > 2) {
								errorFlag = 1;
								printf("No psik between the values!\n");
								return 0;
							}
							else {
								psikFlag = 0;
								updateDataTable(&dataTable, tokens[i], Data);
							}
						}
						else if (sf == String) {
							unsigned k;
							for (k = 0; k <= strlen(tokens[i]); k++) {
								updateDataTable(&dataTable, tokens[i][k], String);
							}

						}
					}

				}
		}
		else if (sf >= 10) {//if operation
			int cnt = 0;
			for (int i = 2; i < fields; i++)
				if (strcmp(tokens[i], ",") == 0) cnt++;
			int validArgNum = fields - 2 - cnt - atoi(operations[sf - 10][1]);//0 == valid
			if (validArgNum == 0) {
				if (updateSymbolTable(&symbolsTable, tokens[0], 0, 0)) {
					handle_operation(2, sf - 10, fields, tokens);
				}
			}
			else {
				int i;
				for (i = 0; i < fields; i++) printf("%s ", tokens[i]);
				printf("- Not valid\n");
			}
		}
	}

if (ff == String || ff == Data) {//if instruction, parse parameters to word and move to data table
	int i;
		for (i = 1; i < fields; i++) {
			if (strcmp(tokens[i], ",") == 0) continue;
			if (ff == Data) {
				updateDataTable(&dataTable, tokens[i], Data);
			}
			else if (ff == String) {
				unsigned k;
				for (k = 0; k <= strlen(tokens[i]); k++) {
					updateDataTable(&dataTable, tokens[i][k], String);
				}

			}
		}
}
	else if (ff >= 10) {
		int cnt = 0;
		for (int i = 1; i < fields; i++)
			if (strcmp(tokens[i], ",") == 0) cnt++;
		int validArgNum = fields - 1 - cnt - atoi(operations[ff - 10][1]);//0 == valid
		if (validArgNum == 0) {
			handle_operation(1, ff - 10, fields, tokens);
		}
		else {
			int i;
			for (i = 0; i < fields; i++) printf("%s ", tokens[i]);
			printf("Not valid!\n\n");
		}
	}
	return 1;
}

void handle_operation(int start, int op, int fields, char tokens[MAX_CMD_TOKENS][TOKEN_LEN]) {
	int op_l, op_r, valid, num_of_words = 0;
	op_l = op_r = -2;
	if (op >= 0 && op <= 13) {//if op use 1 or 2 args
		op_l = get_operand_type(tokens[start]);
		if (op >= 0 && op <= 4) {
			op_r = get_operand_type(tokens[start + 2]);
		}
		else
			op_r = -2;//op not in use
		if (op_l == -1 || op_r == -1) {//-1 is when operand type not recognized
			int i;
			printf("Illegal operands= >");
			for (i = 0; i < fields; i++) {
				printf("%s ", tokens[i]);
			}
			putchar('\n');
			return;
		}
		valid = is_op_valid(op, op_l, op_r);
		if (valid) {//need to calculate how many memory words we need to reserve for the next iteration
			if (op >= 0 && op <= 4) {
				if (op_l == Reg && op_r == Reg)
					num_of_words += 2;//1 for operation + 1 for shared word
				else
					num_of_words += 3;//1 for operation + 1 for each operand(2)
			}
			else if (op > 4 && op <= 13)
				num_of_words += 2;//1 for operation + 1 for operand
		}
		else {
			for(int i = 0; i < fields; i++)
				printf("%s ", tokens[i]);
			puts(" - not valid");
		}
	}
	else {// if operation needs no operands
		num_of_words += 1;// only for the operation
	}



	prepare_instruction_words(op, op_l, op_r, tokens[start], tokens[start + 2]);
}


char * dec_2_bin(char * str, int num, int length) {
	int i = length - 1, tmp = num;
	while (i != -1) {
		if (num & 1) *(str + i--) = '1';
		else *(str + i--) = '0';
		num = num >> 1;
	}
	*(str + length) = '\0';
	return str;
}



int strip_number(char * str) {
	unsigned int i = 0, l = 0;
	l = strlen(str) - 1;
	char* tmp = (char *)malloc(sizeof(char) * l);
	if (tmp != NULL) {
		for (i = 1; i < strlen(str); i++) {
			tmp[i - 1] = str[i];
		}
		tmp[i - 1] = '\0';

		l = atoi(tmp);
	}
	return l;
}
int prepare_instruction_words(int op_code, int op_l, int op_r, char * val_l, char * val_r) {
	word w, w1, w2;
	w.bits[0] = w1.bits[0] = w2.bits[0] = '\0';
	char tmp_bin_left[13], tmp_bin_right[13];
	create_first_bin_word(&w, op_code, op_l, op_r);// set w as the first word needed
	strcpy(tmp_bin_left, op_2_str(op_l, val_l));
	strcpy(tmp_bin_right, op_2_str(op_r, val_r));
	//start assemble the words
	if (op_l == Reg || op_l == P_reg) {
		strcpy(w1.bits, "000000");
		strcat(w1.bits, tmp_bin_left);

		if (op_r == Reg || op_r == P_reg) {
			strcat(w1.bits, tmp_bin_right);
		}
		else {
			strcat(w1.bits, "000");
		}
	}
	if (op_l == Num) {
		strcpy(w1.bits, tmp_bin_left);
	}

	if (op_l == Var) {
		strcpy(w1.bits, "000000000000");
	}
	if (op_r == Reg || op_r == P_reg) {
		if (op_l != Reg && op_l != P_reg) {//this cases already taken care if they are equal to reg || P_reg
			strcpy(w2.bits, "000000000");
			strcat(w2.bits, tmp_bin_right);
		}
	}
	if (op_r == Num) {
		strcpy(w2.bits, tmp_bin_right);
	}

	if (op_r == Var) {
		strcpy(w2.bits, "000000000000");
	}
	update_instruction_table(&instructionTable, w);
	
	if (strlen((char*)w1.bits)) {
		strcat(w1.bits, "000");
		update_instruction_table(&instructionTable, w1);
		
	}
	if (strlen(w2.bits)) {
		strcat(w2.bits, "000");
		update_instruction_table(&instructionTable, w2);
		
	}
	return 1;
}

int update_instruction_table(instruction ** h, word w) {
	if ((ic + dc) >= MEMORY_SIZE) {
		errorFlag = 1;
		printf("The program has reached the maximum memory size!\n");
	}
	instruction* new_node = *h;
	if (!new_node) {
		new_node = create_instruction(new_node, w);
		*h = new_node;
	}
	else
	{
		while (new_node->next != NULL)
		{
			new_node = new_node->next;
		}

		new_node->next = create_instruction(new_node, w);
	}
	return 1;
}

instruction * create_instruction(instruction * new_node, word w) {
	new_node = (instruction*)malloc(sizeof(instruction));
	if (!new_node)
	{
		return NULL;
	}
	new_node->address = IC_OFFSET + ic++;
	strcpy(&new_node->value, w.bits);
	new_node->next = NULL;
	return new_node;
}

char* op_2_str(int op, char * val) {
	int bin_len = 0, value = 0;
	char tmp_bin[13];
	if (strlen(val)) {
		if (op == Reg || op == P_reg) {
			bin_len = 3;
			value = val[strlen(val) - 1] - ASCII_DEC_DIFF;
		}
		if (op == Num) {
			bin_len = 12;
			value = strip_number(val);
		}
		if (op == Var) {
			bin_len = 12;
			value = 0;
		}
		dec_2_bin(tmp_bin, value, bin_len);
	}
	else {
		tmp_bin[0] = '\0';
	}
	return tmp_bin;
}
 void create_first_bin_word(word * w, int op_code, int op_l, int op_r){
	 char tmp_bin[5];

	 dec_2_bin(tmp_bin, op_code, 4);
	 strcpy(w->bits, tmp_bin);
	 switch (op_l)
	 {
	 case -2: strcat(w->bits, "0000"); break;
	 case Num: strcat(w->bits, "0001"); break;
	 case Var: strcat(w->bits, "0010"); break;
	 case P_reg: strcat(w->bits, "0100"); break;
	 case Reg: strcat(w->bits, "1000"); break;
	 default:
		 break;
	 }
	 switch (op_r)
	 {
	 case -2: strcat(w->bits, "0000"); break;
	 case Num: strcat(w->bits, "0001"); break;
	 case Var: strcat(w->bits, "0010"); break;
	 case P_reg: strcat(w->bits, "0100"); break;
	 case Reg: strcat(w->bits, "1000"); break;
	 default:
		 break;
	 }
 
	 strcat(w->bits, "100");
 }



int is_op_valid(int op, int op_l, int op_r) {
	if (op == 1) {//cmp
		if (op_l >= Num && op_l <= Reg && op_r >= Num && op_r <= Reg)
			return 1;
	}
	if (op == 4) {//lea	
		if (op_l == Var && (op_r == Reg || op_r == P_reg))
			return 1;

		}
	if (op == 0 || op == 2 || op == 3) {//mov add sub
		if (op_l >= Num && op_l <= Reg && op_r >= Num && op_r <= Reg)
			return 1;
	}

	if (op == 5 || op == 6 || op == 7 || op == 8 || op == 11) {//clr not inc dec red
		if (op_l >= Var && op_l <= Reg)
			return 1;
	}
	if (op == 9 || op == 10 || op == 13) {//jmp bne jsr
		if (op_l >= Var && op_l <= P_reg)
			return 1;
	}
	if (op == 12) {//prn
		if (op_l >= Num && op_l <= Reg)
			return 1;
	}
	errorFlag = 1;
	return 0;
}
int get_operand_type(char * op) {
	int tmp;
	tmp = isRegister(op);
	if (tmp != -1) return tmp;
	tmp = isNum(op);
	if (tmp != -1) return tmp;
	tmp = looks_like_label(op);
	if (tmp != -1) return Var;
	return -1;
}
int looks_like_label(char * str) {
	int i;
	if ((str[0] < 'a' || str[0] > 'z') && (str[0] < 'A' || str[0] > 'Z')) return -1; // label must start with letter
	for (i = 0; i < strlen(str); i++) {
		if ((str[i] < 'a' || str[i] > 'z') && (str[i] < 'A' || str[i] > 'Z') && (str[i] < '0' || str[i] > '9'))
			return -1;
	}
	return Var;
}

int isRegister(char* reg){
	if (reg[0] == 'r') {
		if (reg[1] >= '0' && reg[1] <= '7' && reg[2] == '\0')
			return Reg;
	}
	if (reg[0] == '*' && reg[1] == 'r') {
		if (reg[2] >= '0' && reg[2] <= '7' && reg[3] == '\0')
			return P_reg;
	}
	return -1;
}

int isNum(char * num) {
	unsigned i, start;
	int tmp;
	start = 1;
	if (num[0] != '#') return -1;
	if (num[1] == '-') start = 2;
	for (i = start; i < strlen(num); i++) {
		if (num[i] < '0' || num[i] >'9')
			return -1;
	}
	tmp = strip_number(num);
	if (tmp > HIGHEST_POSITIVE_12 || tmp < LOWEST_NEGATIVE_12) {
		errorFlag = 1;
		printf("Number is out of 12 bits range -> %d\n", tmp);	
		return -1;
	}
	return Num;
}
void translateToWord(char token[TOKEN_LEN], word * dest, int charType) {
	int i, val;
	if (charType == Data) {
		val = atoi(token);
	}
	else {
		val = (int)token;
	}
	if (val > HIGHEST_POSITIVE_15 || val < LOWEST_NEGATIVE_15) { 
		errorFlag = 1;
		printf("Number is out of 15 bits range -> %d\n", val);
	}
	for (i = 14; i >=  0; i--) {
		if (val & 1)
			dest->bits[i] = '1';
		else
			dest->bits[i] = '0';
		val = val >> 1;
	}
	dest->bits[15] = '\0';

}



int tokenize_cmd(char str[MAX_CMD_TOKENS * TOKEN_LEN + 1], char tokens[MAX_CMD_TOKENS][TOKEN_LEN]) {
	unsigned i, k, t;
	char tmp[TOKEN_LEN];
	i = 0;
	while (str[i] == ' ' && i++ < strlen(str));//remove leading spaces
	for (k = t = 0; i <= strlen(str); i++) {
		if (str[i] != ' ' && str[i] != '\t' && str[i] != ',' && str[i] != '\0' && str[i] != '\n' && str[i] != EOF) {
			if (str[i] == '"') {
				while (str[++i] != '"')
				{
					tmp[k++] = str[i];
				}
			}
			else
				tmp[k++] = str[i];
		}
		else {				
			
			tmp[k] = '\0';
			if (tmp[0] != '\0') {
				strcpy(*(tokens + t++), tmp);
			}
			tmp[0] = '\0';
			k = 0;
			if (str[i] == ',') {
				strcpy(*(tokens + t++), ",\0");
			}
			if (str[i] == ' ' || str[i] == '\t') {
				continue;
			}
		}
	}
	return t;
}


int getTokenType(char token[TOKEN_LEN]) {
	int res = -1;
	if (token[strlen(token) - 1] == ':') res = Symbol;
	else
	if (strcmp(token, ".extern") == 0) res = Extern;
	else
	if (strcmp(token, ".entry") == 0) res = Entry;
	else
	if (strcmp(token, ".data") == 0) res = Data;
	else
	if (strcmp(token, ".string") == 0) res = String;
	else{
		int i;
		for (i = 0; i < OPERATIONS_CNT; i++) {
			if (strcmp(token, operations[i][0]) == 0) {
				res = i + 10;//operation number will start from 10
				break;
			}
		}
	
	}
	if (res == -1 && strlen(token) > 0) {
		errorFlag = 1;
		printf("undefined instruction -> \"%s\"\n", token);
	}
	return res;
}

int updateFlag(int * flag, int state) {
	*flag = state;
	return *flag;
	//step 4 return wih updated flag
}

//TODO - step 4 - "on" - symbol definition flag

//return 1 if success, 0 if fails
int updateSymbolTable(symbol ** head, char name[SYMBOL_LEN], int isInstruction, int isExternal) {
	symbol * new_symbol = *head; /*declaration*/
	if (name[strlen(name) - 1] == ':') name[strlen(name) - 1] = '\0';
	if (!new_symbol) /*head is empty*/
	{
		new_symbol = create_symbol(new_symbol, name, isInstruction, isExternal);
		*head = new_symbol;
	}
	else
	{
		while (new_symbol->next != NULL)
		{
			new_symbol = new_symbol->next;
		}

		new_symbol->next = create_symbol(new_symbol, name, isInstruction, isExternal);
	}
	return 1;
}


int symbolExists(symbol** h, char name[SYMBOL_LEN]) {
	symbol* current_symbol = *h;
	int tmp, i = 0;
	char* tmpStr = (char*)malloc(sizeof(char) * strlen(name));
	strcpy(tmpStr, name);
	tmp = strlen(name) - 1;
	tmpStr[tmp] = '\0';
	while (current_symbol) {
		tmp = strcmp(tmpStr, current_symbol->name);
		if (tmp == 0) {
			errorFlag = 1;
			return 1;
		}
		current_symbol = current_symbol->next;
	}
	for (i = 0; i < 15; i++) {
		tmp = strcmp(tmpStr, operations[i][0]);
		if (tmp == 0) {
			errorFlag = 1;
			return -1;
		}
	}
	if (isRegister(tmpStr) == Reg || isRegister(tmpStr) == P_reg) {
		errorFlag = 1;
		return -2;
	}
	return 0;
}

int updateDataTable(data ** head, char *val, int tt){
	data * new_data = *head;
		if(!new_data){

			new_data = create_data(new_data, val, tt);
			*head = new_data;
		}
		else {
			while (new_data->next != NULL)
				new_data = new_data->next;
			new_data->next = create_data(new_data, val, tt);;
		}
		dc++;
		return 1;
}

void printError()
{
	/*TODO step- 11 - if definition symbol, insert into symbol table with code property, value= IC+100, check if symbol exist (yes=error) */
}

	

void print_dataTable(data ** head) {
	data * tmp = NULL;
	char octal[6];
	while (*head)
	{
		int address = (*head)->address + ic + IC_OFFSET - 1;
	//	while()
		printf("%04d %s\n",address, bin_2_octal(&(*head)->value, octal));
		tmp = *head;
		*head = (*head)->next;
		free(tmp);

	}
}
void print_symbolTable(symbol** head) {
	symbol* tmp = NULL;
	while (*head)
	{
		printf(">address: %d,  value: %s\n", (*head)->address, (*head)->name);
		tmp = *head;
		*head = (*head)->next;
		free(tmp);
	}
}
void print_externalsTable(external** head) {
	external* tmp = NULL;
	puts("Externals:");
	while (*head)
	{
		printf("%s %d\n", (*head)->name, (*head)->address);
		tmp = *head;
		*head = (*head)->next;
		free(tmp);
	}
}
void fix_symbolTable_addresses(symbol** head) {
	symbol* tmp = *head;
	while (tmp)

	{
		if(tmp->isInstruction)
			tmp->address += (IC_OFFSET + ic);
		tmp = tmp->next;
	}
}


void print_instructionsTable(instruction** head) {
	instruction* tmp = NULL;
	char octal[6];
	printf("Instructions Table\n");
	while (*head)
	{
		printf("%04d %s\n", (*head)->address, bin_2_octal(&(*head)->value, octal));
		tmp = *head;
		*head = (*head)->next;
		free(tmp);
	}
}


symbol * create_symbol(symbol * new_node, char name[SYMBOL_LEN], int isInstruction, int isExternal) {
	new_node = (symbol*)malloc(sizeof(symbol));
	if (!new_node)
	{
		return NULL;
	}

	strcpy(new_node->name, name);
	new_node->next = NULL;
	if (isInstruction) {
		new_node->isInstruction = 1;
		if (isExternal) {
			new_node->address = 0;
		}
		else {
			new_node->address = dc;
		}
	}
	else {
		new_node->isInstruction = 0;
		new_node->isExternal = isExternal;
		new_node->isEntry = 0;
		new_node->address = ic + IC_OFFSET;
	}
	return new_node;
}



data * create_data(data * new_node, char val[TOKEN_LEN],int tt) {
	new_node = (data*)malloc(sizeof(data));
	if (!new_node)
	{
		return NULL;
	}
		translateToWord(val, &new_node->value, tt);
		new_node->address = dc;
		new_node->next = NULL;
	return new_node;
}



	void initSymbols(symbol** h) {
		*h = NULL;
	}
	void initData(data** h) {
		*h = NULL;
	}
	void initInstructions(instruction ** h) {
		*h = NULL;
	}




