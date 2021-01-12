#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define SISI 0xcc
#define MAX_BP 5

typedef struct _BREAK_POINT
{
	BYTE data_taken;
	unsigned long long address;
} BREAK_POINT;


extern BREAK_POINT BP_TABLE[MAX_BP];
extern PROCESS_INFORMATION pi;
extern unsigned long long entry_point;

// handle a bp event.
int handle_break_point(DEBUG_EVENT* d_event);
// Inserts break point in a specific address.
int insert_break_point(HANDLE process_handle, unsigned long long address, BYTE* byte_to_save);
// Inserts the byte to recover the break point.
int recover_break_point(HANDLE process_handle, unsigned long long address, BYTE byte_to_recover);
// Sub Rip. Rip--, after handling bp go one instruction back.
int sub_Rip(HANDLE thread_handle);
// Turns on the Trap FLag to make the stepi.
int make_stepi(HANDLE thread_handle);
// runs a process.
int run_process();
// to remove bp from the table.
int remove_bp_from_table(unsigned long long address);
// insert bp to table if empty place -> will return 1.
int insert_bp_to_table(unsigned long long address, BYTE byte_to_save);
// checks if the address already has break point at it.
int exists_in_bp_table(unsigned long long address);
// checks if there is an empty place in the bp table.
int empty_place_in_bp_table();
// handle delete break point command.
int handle_delete_break_point(unsigned long long full_address);
// handle func to print the current stack frame.
int handle_print_stack_frame(HANDLE thread_handle);
// function to display the stack frame.
void display_stack_frame(unsigned char* buffer, int size);
// prints the registers.
int show_info_reg(HANDLE thread_handle);
// prints the commands available for the debugger.
void print_help();