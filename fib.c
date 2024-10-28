#include <stdio.h>
#include "dummy_main.h"

int fib(int n) 
{
	if(n < 2) return n;
	return fib(n-1) + fib(n-2);
}

int main(int argc, char **argv)
{	printf("%s","hii");
	printf("%d\n", fib(40));

	return 0;
}

