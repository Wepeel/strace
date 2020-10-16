#pragma once

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>

namespace traced_process
{
	void traced_process(const char* path, char* const argv[]);
}