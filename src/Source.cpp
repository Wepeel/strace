#include "traced_process.h"

int main()
{
	char* const argv[] = { "-a" };
	traced_process::traced_process("ls", argv);

	return 0;
}