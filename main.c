#define _CRT_SECURE_NO_WARNINGS 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "hardware.h"

/*function declerations*/
void read_file(char file_name[30]);
int tokenize_cmd(char str[MAX_CMD_TOKENS * TOKEN_LEN + 1], char tokens[MAX_CMD_TOKENS][TOKEN_LEN]);
void print_tokens(char tokens[MAX_FILE_LEN][MAX_CMD_TOKENS][TOKEN_LEN]);
int updateSymbolTable(symbol ** head, char name[SYMBOL_LEN], int isInstruction, int isExternal);
int updateFlag(int * flag, int );
void initSymbols(symbol** h);
void initData(data** h);
symbol * create_symbol(symbol * new_node, char name[SYMBOL_LEN], int isInstruction, int isExternal);
int symbolExists(char name[SYMBOL_LEN]);
int getTokenType(char token[TOKEN_LEN]);
void print_dataTable(data ** head);
void print_symbolTable(symbol ** head);
data * create_data(data * new_node, char val[TOKEN_LEN], int tt);
int handleLine(int fields, char tokens[MAX_CMD_TOKENS][TOKEN_LEN]);
int updateDataTable(data** head, char* val, int tt);
/*function declerations end*/

int dc = 0, ic = 0, line = 0, symbolFlag = 0;
enum tokenType { Symbol, Extern, Entry, Data, String };
enum flagState { Off, On };
enum char_type { NUM, STR };
char operations[OPERATIONS_CNT][2][5] = {
	{"mov", "2"},{"cmp", "2"},{"add", "2"},{"sub", "2"},{"lea", "2"},{"clr", "2"},{"not", "2"},{"inc", "1"},
	{"dec", "1"},{"jmp", "1"},{"bne", "1"},{"red", "2"},{"prn", "1"},{"jsr", "2"},{"rts", "2"}, {"stop", "0"}
};
symbol * symbolsTable;
data * dataTable;
 



void main() {
 
	initSymbols(&symbolsTable);
	initData(&dataTable);

	read_file("input1.as");
	printf("Data Table:\n");
	print_dataTable(&dataTable);
	printf("Sybols Table:\n");
	print_symbolTable(&symbolsTable);
}


void read_file(char file_name[30]) {
	char tokens[MAX_FILE_LEN][MAX_CMD_TOKENS][TOKEN_LEN] = {'\0'};
	char buffer[MAX_CMD_TOKENS * TOKEN_LEN + 1];
	FILE *fp;
	fp = fopen(file_name, "r+");
	if (fp == NULL) {
		printf("Error->unable to open file.");
		return;
	}
	/*read line*/
	while (fgets(buffer, MAX_CMD_TOKENS * TOKEN_LEN + 1, fp) != NULL) {
		int fields;
		fields = tokenize_cmd(buffer, tokens[line]);/*separate to fields*/
		handleLine(fields, tokens[line++]);
		buffer[0] = 0;/*clear the buffer to be ready for next line*/
	}
	//print_tokens(tokens);
	fclose(fp);
}

int handleLine(int fields, char tokens[MAX_CMD_TOKENS][TOKEN_LEN]) {
	//enum tokenType { Symbol, Extern, Entry, Data, String };
	int ff;//ff == first field
	ff = getTokenType(tokens[0]);/*get field type*/
	if ((ff == Extern || ff == Symbol) && symbolExists(tokens[0])) {
		printf("%s -> Symbol already exists.\n", tokens[0]);
		return 0;
	}
	if (ff == Extern) {/*if extern, need to ignore on first round*/
		int tmp;
		tmp = updateSymbolTable(&symbolsTable, tokens[1], 0, 1);
		if (tmp == 0) printf("Error saving symbol %s\n", tokens[1]);
		return 1;
	}
	else if (ff == Entry) {/*if entry, update symbol table with second field*/
		return 1;
	}
	else if (ff == Symbol) {
		int i, sf;//second flag
		updateFlag(&symbolFlag, On);/*turn on symbol flag*/
		sf = getTokenType(tokens[1]);
		if (sf == String || sf == Data) {//if instruction, parse parameters to word and move to data table
				if (updateSymbolTable(&symbolsTable, tokens[0], 1, 0)) {
					for (i = 2; i < fields; i++) {
						if (strcmp(tokens[i], ",") == 0) continue;
						if (sf == Data) {
							updateDataTable(&dataTable, tokens[i], Data);
						}
						else if (sf == String) {
							int k;
							for (k = 0; k < strlen(tokens[i]); k++) {
								updateDataTable(&dataTable, tokens[i][k], String);
							}
						}
					}
				}
		}
		else if (sf >= 10) {//if operation
			//handle_operation(tokens);
			int cnt = 0;
			for (int i = 2; i < fields; i++)
				if (strcmp(tokens[i], ",") == 0) cnt++;
			int validArgNum = fields - 2 - cnt - atoi(operations[sf - 10][1]);//0 == valid
		
			printf(">>> %s ", operations[sf - 10][0]);
			if (validArgNum == 0)
				printf("- valid\n");
			else
				printf("- Not valid\n");

		}
	}

}

void translateToWord(char token[TOKEN_LEN], word * dest, int charType) {
	int i, val;
	if (charType == Data) {
		val = atoi(token);
	}
	else {
		val = (int)token;
	}
	for (i = 0; i < 15; i++) {
		int tmp;
		tmp = val >> i;
		if (tmp & 1)
			dest->bits[i] = '1';
		else
			dest->bits[i] = '0';
	}
	dest->bits[15] = '\0';
}


void print_tokens(char tokens[MAX_FILE_LEN][MAX_CMD_TOKENS][TOKEN_LEN]) {
	int i, j;
	for (i = 0; i < line && tokens[i][0] != '\0'; i++) {
		for (j = 0; j < MAX_CMD_TOKENS && tokens[i][j] != '\0'; j++) {
			printf("%s ", tokens[i][j]);
		}
		putchar('\n');
	}
}

int tokenize_cmd(char str[MAX_CMD_TOKENS * TOKEN_LEN + 1], char tokens[MAX_CMD_TOKENS][TOKEN_LEN]) {
	int i, k, t;
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
	/*TODO -step 3 - check field for symbol at tocken[0]*/
	/*TODO step- 11 - if definition symbol, insert into symbol table with code property, value= IC+100, check if symbol exist (yes=error) */
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
	//printf("address => %d, Value: %s\n", new_symbol, name);
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


int symbolExists(char name[SYMBOL_LEN]) {
	symbol * current_symbol = &symbolsTable;
	int i = 0;
	while (0) {
		int tmp ;
		printf("%d> %s\n",i++,name);
		tmp = strcmp(name, current_symbol->name);
		if (tmp == 0) {
			return 1;
			break;
		}
		current_symbol = current_symbol->next;
	}
	return 0;
	//TODO - step 6.1 - check if symbol exists at symbol table 
	//TODO step- 11 - if definition symbol, insert into symbol table with code property, value= IC+100, check if symbol exist (yes=error) 
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
	data * tmp = *head;
	while (tmp)
	{
		printf("> value: %s\n",&tmp->value);
		tmp = tmp->next;
	}
}
void print_symbolTable(symbol ** head) {
	symbol * tmp = *head;
	while (tmp)
	{
		printf("> value: %s\n", tmp->name);
		tmp = tmp->next;
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
		if (isExternal)
			new_node->address = 0;
		else
			new_node->address = dc;
	}
	else
		new_node->address = ic;
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