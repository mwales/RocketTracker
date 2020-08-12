#ifndef LORATEST_H
#define LORATEST_H


#include <stdio.h>
#include <string.h>

class Ser
{
public:
	void print(char const * var)
	{
		printf("%s", var);
	}

	void println(char const * var)
	{
		printf("%s\n", var);
	}

	void print(int var)
	{
		printf("%d", var);
	}

	void println(int var)
	{
		printf("%d\n", var);
	}

	void print(char var)
	{
		printf("%c", var);
	}

	void println(char var)
	{
		printf("%c\n", var);
	}
};

#endif

