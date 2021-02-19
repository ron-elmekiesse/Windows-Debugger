#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "debugger_actions.h"

#define MAX_COMMAND_LEN 100
#define MAX_COMMANDS 9
#define RUN "run"
#define BREAK "b"
#define DB "db"
#define STEPI "stepi"
#define INFO_REG "reg"
#define STACK "stack"
#define HELP "help"
#define QUIT "q"
#define XS "x/s"

enum EXIT_CODES {SUCCESS, FAILED, EXIT, RUN_PROC};

typedef struct _COMMAND_INFO
{
	char* command;
	int needs_extra_data;
} COMMAND_INFO;


extern COMMAND_INFO COMMANDS[MAX_COMMANDS];
extern int run_debuggee_non_stop; // if needed to run the debuggee process entirely without breaks.


int make_user_commands();
void parse_command(char* buf, char* command_out, char* extra_data_out);
int check_valid_command(char* command, char* extra_data);
int make_command(char* command, char* extra_data);