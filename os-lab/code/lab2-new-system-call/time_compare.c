#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

long my_oper(int* result, int num1, int num2, char* op)
{
	if(op)
	{
		if(*op == '+')
			*result = num1 + num2;
		else if(*op == '-')
			*result = num1 - num2;
		else if(*op == '*')
			*result = num1 * num2;
		else if(*op == '\\')
		{
			if(num2 != 0)
				*result = num1 / num2;
			else
				printf("divided number can't be zero.\n");
		}
	}
	else
		printf("operator is empth.\n");
	
	return 0;
}


long optime(int count)
{
	printf("count is:%d\n", count);
	struct timeval tstart,tend;
	gettimeofday(&tstart, NULL);
	int i;
	int result;
	int times = count/4;
	for(i=0;i<times;++i)
	{
		char op_add = '+';
		my_oper(&result,10,10,&op_add);
	}
	printf("my_func op_add is ok. op_add count is:%d\n",i);
	
	for(i=0;i<times;++i)
	{
		char op_sub = '-';
		my_oper(&result,20,10,&op_sub);
	}
	printf("my_func op_sub is ok. op_sub count is:%d\n",i);
	
	for(i=0;i<times;++i)
	{
		char op_mul = '*';
		my_oper(&result,10,10,&op_mul);
	}
	printf("my_func op_mul is ok. op_mul count is:%d\n",i);
    
	for(i=0;i<times;++i)
	{
		char op_div = '\\';
		my_oper(&result,20,10,&op_div);
	}
	printf("my_func op_div is ok. op_div count is:%d\n",i);
	
	gettimeofday(&tend, NULL);

	long exeTime = (tend.tv_sec-tstart.tv_sec)*1000000+(tend.tv_usec-tstart.tv_usec);

	printf("my_func running time is %ld usec\n", exeTime);
	
	return 0;
}

int main()
{
    int count = 10000*10000;
    long ret = syscall(400,count);
    printf("result is %ld\n",ret);
    optime(count);
    return 0;
}
