#ifndef JSCONTROL
#define JSCONTROL//µð¹ö±ë¿ë

#include <stdio.h>
#include <stdlib.h>

void error_handling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

#endif
