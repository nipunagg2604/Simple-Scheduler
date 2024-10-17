#include <stdio.h>

int main()
{
	if(fork() == 0)
	{
		char* path;
		char* args[2];
		execv(path, args);
	}
	printf("Hi1\n");
	printf("Hi1\n");
	printf("Hi1\n");
	printf("Hi1\n");


	return 0;
}
