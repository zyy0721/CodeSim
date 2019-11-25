#include<stdio.h>

int fun(int n)
{
	if(n == 0)
		return 1;
	else
		return n*fun(n-1);
}

int main()
{
	int a = 1;
	int b = 2;
	a = fun(b);
	return a;
}
