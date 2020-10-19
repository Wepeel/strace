#include "traced_process.h"
#include <stdio.h>
#include <sys/user.h>
#include <errno.h>

void print_string(pid_t pid, unsigned long long addr, unsigned long long count)
{
	for (unsigned long long i = 0; i < count; i++)
	{
		char print = (char)ptrace(PTRACE_PEEKDATA, pid, reinterpret_cast<const char *>(addr) + i);

		if (print == '\n')
		{
			fprintf(stderr, "\\n");
		}

		else if (print == '\t')
		{
			fprintf(stderr, "\\t");
		}

		else
		{
			fprintf(stderr, "%c", print);
		}
	}
}

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
		wait(&wstatus);

		ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_EXITKILL);

		while (true)
		{

			if (WIFSTOPPED(wstatus))
			{
				user_regs_struct regs;

				ptrace(PTRACE_SYSCALL, pid, 0, 0);
				wait(&wstatus);
				ptrace(PTRACE_GETREGS, pid, 0, &regs);

				switch (regs.orig_rax)
				{
				case 0: // Read
					fprintf(stderr, "read(fd=%llu, buf=0x%llx, count=%llu)",
						regs.rdi, regs.rsi, regs.rdx);
					break;
				case 1: // Write
					fprintf(stderr, "write(fd=%llu, buf=",
											regs.rdi);
					print_string(pid, regs.rsi, regs.rdx);
					 fprintf(stderr,", count=%llu)", regs.rdx);
					break;
				case 2: // Open
					fprintf(stderr, "open(filename=%s, flags=%llu, mode=%llu)",
						(char const *)regs.rdi, regs.rsi, regs.rdx);
					break;
				case 3: // Close
					fprintf(stderr, "close(fd=%llu)", regs.rdi);
					break;
				default:
					fprintf(stderr, "System call number - %llu\n", regs.orig_rax);
				}

				if (0 <= regs.orig_rax && 3 >= regs.orig_rax)
				{
					ptrace(PTRACE_SYSCALL, pid, 0, 0);
					wait(&wstatus);
					ptrace(PTRACE_GETREGS, pid, 0, &regs);
					fprintf(stderr, " = %llu\n", regs.rax);
				}

				else
				{
					ptrace(PTRACE_SYSCALL, pid, 0, 0);
					wait(&wstatus);
					ptrace(PTRACE_GETREGS, pid, 0, &regs);
					fprintf(stderr, "System call returned - %llx\n", regs.rax);
				}

			}

			else if (WIFEXITED(wstatus))
			{
				fprintf(stderr, "Exited\n");
				return;
			}

			else
			{
				fprintf(stderr, "Unknown status\n");
				return;
			}
		}
	}
}
