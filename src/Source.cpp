#include "traced_process.h"

int main(int argc, char * argv[])
{
	traced_process::traced_process(argv[1], argv + 1);

	return 0;
}