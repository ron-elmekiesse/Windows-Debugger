#include "user_commands.h"

COMMAND_INFO COMMANDS[MAX_COMMANDS] = 
{	{RUN, 0},
	{B, 1},
	{DB, 1},
	{STEPI, 0},
	{INFO_REG, 0},
	{STACK, 0},
	{HELP, 0},
	{QUIT, 0}
};

int run_debuggee_non_stop = 0; // extern.
static int just_run = 1;

int make_user_commands()
{
	int i = 0, res = 0, finished = 0, run_debuggee = 0;
	char buf[MAX_COMMAND_LEN] = { 0 };
	char command[MAX_COMMAND_LEN] = { 0 };
	char extra_data[MAX_COMMAND_LEN] = { 0 };
	char* ptr = NULL;

	just_run = 1;

	while (!finished)
	{
		printf("$");
		fgets(buf, MAX_COMMAND_LEN, stdin); // get command
		ptr = strchr(buf, '\n');
		if (ptr)
			*ptr = '\0';

		parse_command(buf, command, extra_data); // parse command.

		res = check_valid_command(command, extra_data);
		if (!res)
		{
			printf("Invalid command!\n");
		}
		else
		{
			res = make_command(command, extra_data);
			if (res == EXIT)
				return 1;
			if (res == RUN_PROC)
			{
				run_debuggee = 1;
				finished = 1; // stop asking -> start running.
			}
		}
	}
	if (just_run && run_debuggee == 1) // if didn't set a bp or stepi so run non stop.
		run_debuggee_non_stop = 1;

	return 0;
}

void parse_command(char* buf, char* command_out, char* extra_data_out)
{
	char* temp = NULL;

	if (strchr(buf, ' ') && buf[strnlen(buf, MAX_COMMAND_LEN) - 1] != ' ') // if command is with ' ' and not ends with ' '.
	{
		temp = strtok(buf, " ");
		strncpy(command_out, temp, MAX_COMMAND_LEN);
		temp = strtok(NULL, " ");
		strncpy(extra_data_out, temp, MAX_COMMAND_LEN);
	}
	else
	{
		strncpy(command_out, buf, MAX_COMMAND_LEN);
		strncpy(extra_data_out, "", MAX_COMMAND_LEN);
	}
}

int check_valid_command(char* command, char* extra_data)
{
	int i = 0;

	for (i = 0; i < MAX_COMMANDS; i++)
	{
		// if command needs extra data and there is extra data, or no data is needed and there is no data given.
		if ((COMMANDS[i].needs_extra_data && strncmp(extra_data, "", MAX_COMMAND_LEN) != 0) || (COMMANDS[i].needs_extra_data == 0 && strncmp(extra_data, "", MAX_COMMAND_LEN) == 0))
		{
			if (strncmp(command, COMMANDS[i].command, MAX_COMMAND_LEN) == 0) // cmp the commands.
			{
				if (strnlen(command, MAX_COMMAND_LEN) == strnlen(COMMANDS[i].command, MAX_COMMAND_LEN))
				{
					return 1;
				}
			}
		}
	}
	return 0;
}

int make_command(char* command, char* extra_data) // could have made a struct with a function ptr in order to replace the if's.
{
	int i = 0, res = 0;
	unsigned long long address = 0;
	BYTE byte = 0;

	if (strncmp(command, RUN, MAX_COMMAND_LEN) == 0) // TODO: Make stepi for bps.
	{
		return RUN_PROC;
	}
	else if (strncmp(command, B, MAX_COMMAND_LEN) == 0) // insert a break point.
	{
		res = empty_place_in_bp_table();
		if (!res)
			return FAILED;
		
		address = strtol(extra_data, NULL, 16);
		if (address == 0)
		{
			printf("Invalid address!\n");
			return FAILED;
		}

		res = exists_in_bp_table(entry_point + address);
		if (res)
			return FAILED;

		res = insert_break_point(pi.hProcess, entry_point + address, &byte);
		if (res)
			return FAILED;

		res = insert_bp_to_table(entry_point + address, byte);
		if (!res)
			return FAILED;
		printf("Break point at: %#llx\n", entry_point + address);
		just_run = 0;
	}
	else if (strncmp(command, DB, MAX_COMMAND_LEN) == 0) // 
	{
		address = strtol(extra_data, NULL, 16);
		if (address == 0)
		{
			printf("Invalid address!\n");
			return FAILED;
		}

		res = handle_delete_break_point(entry_point + address);
		if (res)
		{
			printf("Failed to handle delete break point\n");
			return FAILED;
		}
		printf("Removed break point at: %#llx\n", entry_point + address);
	}
	else if (strncmp(command, STEPI, MAX_COMMAND_LEN) == 0)
	{
		res = make_stepi(pi.hThread);
		if (res)
			return FAILED;
		just_run = 0;
		return RUN_PROC; // after turned on the trap flag -> run the program.
	}
	else if (strncmp(command, INFO_REG, MAX_COMMAND_LEN) == 0)
	{
		res = show_info_reg(pi.hThread);
		if (res)
		{
			printf("Failed to show registern information!\n");
			return FAILED;
		}
	}
	else if (strncmp(command, STACK, MAX_COMMAND_LEN) == 0)
	{
		// print the stack frame.
		res = handle_print_stack_frame(pi.hThread);
		if (res)
		{
			printf("Failed to display current stack frame!\n");
			return FAILED;
		}
	}
	else if (strncmp(command, HELP, MAX_COMMAND_LEN) == 0)
	{
		print_help();
	}
	else if (strncmp(command, QUIT, MAX_COMMAND_LEN) == 0)
	{
		return EXIT;
	}
	return SUCCESS;
}