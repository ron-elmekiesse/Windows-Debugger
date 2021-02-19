#include "debugger_actions.h"

BREAK_POINT BP_TABLE[MAX_BP] = { 0 };
PROCESS_INFORMATION pi = { 0 };
unsigned long long entry_point = 0;

int handle_break_point(DEBUG_EVENT* d_event)
{
	int i = 0;

	for (i = 0; i < MAX_BP; i++) // recover break point.
	{
		if (BP_TABLE[i].address == d_event->u.Exception.ExceptionRecord.ExceptionAddress)
		{
			if (recover_break_point(pi.hProcess, BP_TABLE[i].address, BP_TABLE[i].data_taken))
			{
				printf("Failed to recover break point\n");
				return -1;
			}
			else
			{
				sub_Rip(pi.hThread);
				remove_bp_from_table(BP_TABLE[i].address);
				return 0;
			}
		}
	}
	return -1;
}

int insert_break_point(HANDLE process_handle, unsigned long long address, BYTE* byte_to_save)
{
	int res = 0;
	BYTE buf = 0;
	int c = SISI;

	res = ReadProcessMemory(process_handle, address, &buf, sizeof(BYTE), NULL);
	if (!res)
	{
		printf("Failed to read BYTE in break point with: %d\n", GetLastError());
		return -1;
	}
	*byte_to_save = buf; // returning the BYTE to recover.
	res = WriteProcessMemory(process_handle, address, &c, sizeof(BYTE), NULL);
	if (!res)
	{
		printf("Failed to insert break point with: %d\n", GetLastError());
		return -1;
	}
	return 0;
}

int recover_break_point(HANDLE process_handle, unsigned long long address, BYTE byte_to_recover)
{
	int res = 0;

	res = WriteProcessMemory(process_handle, address, &byte_to_recover, sizeof(BYTE), NULL);
	if (!res)
	{
		printf("Failed to recover break point with: %d\n", GetLastError());
		return -1;
	}
	return 0;
}

int sub_Rip(HANDLE thread_handle)
{
	int res = 0;
	CONTEXT t_ctx = { 0 };

	t_ctx.ContextFlags = CONTEXT_ALL;
	res = GetThreadContext(thread_handle, &t_ctx);
	if (!res)
	{
		printf("Failed to Get Thread Context with: %d\n", GetLastError());
		return -1;
	}
	t_ctx.Rip--; // sub Rip to recover run the skipped instruction.
	res = SetThreadContext(thread_handle, &t_ctx);
	if (!res)
	{
		printf("Failed to Set Thread Context with: %d\n", GetLastError());
		return -1;
	}
	return 0;
}

int make_stepi(HANDLE thread_handle)
{
	int res = 0;
	CONTEXT t_ctx = { 0 };

	t_ctx.ContextFlags = CONTEXT_ALL;
	res = GetThreadContext(thread_handle, &t_ctx);
	if (!res)
	{
		printf("Failed to Get Thread Context with: %d\n", GetLastError());
		return -1;
	}
	t_ctx.EFlags |= 0x0100; // turn on the trap flag in order to make the stepi.
	res = SetThreadContext(thread_handle, &t_ctx);
	if (!res)
	{
		printf("Failed to Set Thread Context with: %d\n", GetLastError());
		return -1;
	}
	return 0;
}

int run_process()
{
	char path[MAX_PATH] = { 0 };
	char* ptr = NULL;
	STARTUPINFO si = { 0 };
	si.cb = sizeof(si);

	printf("Enter file path: ");
	fgets(path, MAX_PATH, stdin);
	ptr = strchr(path, '\n');
	if (ptr)
		*ptr = '\0';

	if (!CreateProcessA(path, NULL, NULL, NULL, FALSE, DEBUG_PROCESS, NULL, NULL, &si, &pi))
	{ // if failed.
		return -1;
	}
	return 0;
}

int remove_bp_from_table(unsigned long long address)
{
	int i = 0;

	for (i = 0; i < MAX_BP; i++)
	{
		if (BP_TABLE[i].address == address)
		{
			BP_TABLE[i].address = 0; // remove the bp.
			BP_TABLE[i].data_taken = 0;
			return 1;
		}
	}
	printf("Invalid address to remove bp!\n");
	return 0;
}

int insert_bp_to_table(unsigned long long address, BYTE byte_to_save)
{
	int i = 0;
	for (i = 0; i < MAX_BP; i++)
	{
		if (BP_TABLE[i].address == 0) // if empty place
		{
			BP_TABLE[i].address = address;
			BP_TABLE[i].data_taken = byte_to_save;
			return 1;
		}
	}
	printf("Reached the max amount of break points -> 5!\n");
	return 0; // if there is no empty place.
}

int exists_in_bp_table(unsigned long long address)
{
	int i = 0;
	for (i = 0; i < MAX_BP; i++)
	{
		if (BP_TABLE[i].address == address)
		{
			printf("Break point already exists!\n");
			return 1;
		}
	}
	return 0;
}

int empty_place_in_bp_table()
{
	int i = 0;
	for (i = 0; i < MAX_BP; i++)
	{
		if (BP_TABLE[i].address == 0)
			return 1;
	}
	printf("Reached the max amount of break points -> 5!\n");
	return 0;
}

int handle_delete_break_point(unsigned long long full_address)
{
	int i = 0, res = 0;

	for (i = 0; i < MAX_BP; i++)
	{
		if (BP_TABLE[i].address == full_address)
		{
			res = recover_break_point(pi.hProcess, full_address, BP_TABLE[i].data_taken);
			if (!res) // if success.
			{
				BP_TABLE[i].address = 0; // remove the bp.
				BP_TABLE[i].data_taken = 0;
				return 0;
			}
		}
	}
	return -1;
}

int handle_print_stack_frame(HANDLE thread_handle)
{
	int res = 0;
	unsigned long long size = 0;
	unsigned char* buffer = NULL;
	CONTEXT ctx = { 0 };

	ctx.ContextFlags = CONTEXT_ALL;

	res = GetThreadContext(thread_handle, &ctx);
	if (!res)
	{
		printf("Failed to Get Thread Context to print current stack frame!\n");
		return -1;
	}

	size = ctx.Rbp - ctx.Rsp;
	if (size < 0)
	{
		printf("Failed to display memory!\n");
		return -1;
	}
	printf("Displaying %llu bytes\n", size);
	buffer = (unsigned char*)malloc(size);

	res = ReadProcessMemory(pi.hProcess, ctx.Rsp, buffer, size, NULL);
	if (!res)
	{
		printf("Failed to read memory while displaying stack frame with: %d\n", GetLastError());
		return -1;
	}

	display_stack_frame(buffer, size);
	free(buffer);
	return 0;
}


void display_stack_frame(unsigned char* buffer, int size)
{
	int i = 0;

	for (i = 0; i < size; i += 8)
	{
		if (i % 8 == 0 && i != 0)
		{
			if (i % 32 == 0)
				puts(""); // for new line.
			else
				printf("  ");
		}
		printf("%016llx", *(unsigned long long*)&buffer[i]); // TODO: push to git hub again!!!!!
	}
		//
	puts("");
}

int show_info_reg(HANDLE thread_handle)
{
	int res = 0;
	CONTEXT ctx = { 0 };

	ctx.ContextFlags = CONTEXT_ALL;

	res = GetThreadContext(thread_handle, &ctx);
	if (!res)
	{
		printf("Failed to Get Thread Context to show registers info.\n");
		return -1;
	}
	
	printf("[Rbp]: %#llx\n", ctx.Rbp);
	printf("[Rsp]: %#llx\n", ctx.Rsp);
	printf("[Rip]: %#llx\n", ctx.Rip);
	printf("[Rax]: %llu\n", ctx.Rax);
	printf("[Rbx]: %llu\n", ctx.Rbx);
	printf("[Rcx]: %llu\n", ctx.Rcx);
	printf("[Rdx]: %llu\n", ctx.Rdx);
	printf("[Rdi]: %llu\n", ctx.Rdi);
	printf("[Rsi]: %llu\n", ctx.Rsi);
	return 0;
}

void print_help()
{
	printf("Commands:\n");
	printf("'run' -> to run the debuggee\n");
	printf("'stepi' -> to make a single step\n");
	printf("'b {offset - hex}' -> to set a break point at the offset from LpStartAddress given (in hex)\n");
	printf("'db {offset - hex}' -> to delete a break point at the offset from LpStartAddress given (in hex)\n");
	printf("'reg' -> to display the register's values\n");
	printf("'stack' -> to display the current stack frame\n");
	printf("'x/s {offset - hex}' -> to display a string at an offset from LpStartAddress given\n");
	printf("'q' -> to quit - close the debugger\n");
}

void examine_string(unsigned long long full_address)
{
	int res = 0, tav = 1, read = 0;

	printf("%#016llx: '", full_address);

	while (tav != 0)
	{
		res = ReadProcessMemory( // reading 1 bytes until the null byte.
			pi.hProcess, 
			full_address++, 
			&tav, 
			sizeof(BYTE), 
			NULL
		);

		if (!res)
		{
			printf("Failed to examine string with: %d\n", GetLastError());
			return;
		}
		printf("%c", tav);
	}
	puts("'");
}