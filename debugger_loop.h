#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "debugger_actions.h"
#include "user_commands.h"

int start_debugger();
void close_debugger();
int handle_event(DEBUG_EVENT* d_event);