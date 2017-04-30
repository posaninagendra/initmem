#include<stdio.h>

int z;
int foo();
int main(){
	int a, b;
	//a = 10;
	int c = 0;
	int d;
	int e;
	int k = foo();

	printf("a = %d\n",a); 
	printf("b = %d\n",b); 
	printf("d = %d\n",d); 
	printf("e = %d\n",e);
	printf("z = %d\n",z);
	return 0;
}

int foo(){
	int x = 0;
	int y;
	printf("y = %d\n",y); 
	return x;
}
