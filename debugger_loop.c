#include "debugger_loop.h"

static int first_process = 1, debugge_ready = 0;
static BYTE temp = 0;

int start_debugger()
{
	int res = 0;
	DEBUG_EVENT d_event = { 0 };
	
	res = run_process();
	if (res)
	{
		printf("Invalid file given!\n");
		return -1;
	}
	printf("Enter 'help' to see the commands\n");

	d_event.dwProcessId = pi.dwProcessId;
	d_event.dwThreadId = pi.dwThreadId;

	while (1)
	{
		res = WaitForDebugEvent(&d_event, INFINITE);
		if (!res)
		{
			printf("Failed in Wait For Debug Event with: %d\n", GetLastError());
			return -1;
		}

		res = handle_event(&d_event); // handle the event.
		switch (res)
		{
		case -1: // If failed.
			printf("Failed to handle event!\n");
			return -1;
		case 2:
			return 0; // finished debugging -> stop the loop.
		}

		if (debugge_ready && !run_debuggee_non_stop) // if the debuggee is ready (after init) and don't run the program without stoping.
		{ 
			res = make_user_commands(); // get and make commands from user.
			if (res) // if pressed 'q' to stop debugging.
			{
				close_debugger();
				return 0;
			}
		}

		res = ContinueDebugEvent(d_event.dwProcessId, d_event.dwThreadId, DBG_CONTINUE); // continue the debuggee process.
		if (!res)
		{
			printf("Failed in Continue Debug Event with: %d\n", GetLastError());
			return -1;
		}
	}
	return 0;
}

void close_debugger()
{
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	printf("Closed the debuggee Process\n");
}

int handle_event(DEBUG_EVENT* d_event)
{
	int i = 0, res = 0;

	switch (d_event->dwDebugEventCode)
	{
	case CREATE_PROCESS_DEBUG_EVENT:
		if (first_process) // main process created.
		{
			entry_point = d_event->u.CreateProcessInfo.lpStartAddress;
			printf("Entry point: %#llx\n", entry_point);
			first_process = 0;
			res = insert_break_point(pi.hProcess, entry_point, &temp); // insert bp at the entry point.
			if (res)
			{
				printf("Failed to insert bp at entry point!\n");
				return -1;
			}
		}
		printf("New process created!\n");
		break;

	case CREATE_THREAD_DEBUG_EVENT:
		printf("New thread created!\n");
		break;

	case EXCEPTION_DEBUG_EVENT:
		switch (d_event->u.Exception.ExceptionRecord.ExceptionCode) // TODO: fix the break point problem!!!!!
		{
		case EXCEPTION_BREAKPOINT:
			if (debugge_ready) // a real bp.
			{
				res = handle_break_point(d_event);
				if (res)
				{
					printf("Failed to handle break point!\n");
					return -1;
				}
			}
			if (d_event->u.Exception.ExceptionRecord.ExceptionAddress == entry_point) // bp for initialize.
			{
				res = recover_break_point(pi.hProcess, entry_point, temp);
				if (res)
					return -1;
				res = sub_Rip(pi.hThread);
				if (res)
					return -1;
				debugge_ready = 1;
			}
			printf("Reached a break point at: %#llx\n", (unsigned long long)d_event->u.Exception.ExceptionRecord.ExceptionAddress);
			break;
		case EXCEPTION_SINGLE_STEP:
			printf("Made a single step\n");
			break;
		case EXCEPTION_ACCESS_VIOLATION:
			printf("Invalid address!\n");
			break;
		default:
			printf("Shit Exception!\n");
			break;
		}
		break;
	case EXIT_PROCESS_DEBUG_EVENT:
		printf("Program finished with exit code: %d\n", d_event->u.ExitProcess.dwExitCode);
		return 2; // finished to debug the process.

	case EXIT_THREAD_DEBUG_EVENT:
		printf("Exited thread\n");
		break;
	case LOAD_DLL_DEBUG_EVENT:
		printf("Loaded a DLL\n");
		CloseHandle(d_event->u.LoadDll.hFile); // closes DLL handle.
		break;
	default:
		printf("In deafult!\n");
		break;
	}
	return 0;
}