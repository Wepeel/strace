#include "traced_process.h"
#include <stdio.h>
#include <sys/user.h>
#include <errno.h>

void traced_process::traced_process(const char* path, char* const argv[])
{
	pid_t pid = fork();

	if (-1 == pid)
	{
		printf("Failed to fork process\n");
		return;
	}

	else if (0 == pid)
	{
		// Child Process
		ptrace(PTRACE_TRACEME, 0, 0, 0);
		int result = execvp(path, argv);
		if (-1 == result)
		{
			printf("Failed to execute command - %d\n", errno);
		}
	}

	else
	{
		bool first = true;
		// Parent Process
		int wstatus;

		while (true)
		{
			wait(&wstatus);

			if (WIFSTOPPED(wstatus))
			{
				user_regs_struct regs;

				ptrace(PTRACE_SYSCALL, pid, 0, 0);
				wait(&wstatus);
				ptrace(PTRACE_GETREGS, pid, 0, &regs);
				printf("System call number - %llu\n", regs.orig_rax);

				ptrace(PTRACE_SYSCALL, pid, 0, 0);
				wait(&wstatus);
				ptrace(PTRACE_GETREGS, pid, 0, &regs);
				printf("System call returned - %llu\n", regs.orig_rax);
			}
		}
	}
}
