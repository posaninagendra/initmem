#include<iostream>

using namespace std;
int foo();
int main(){
	int a, b;
	a = 10;
	int c = 0;
	int d;
	int e;
	int k = foo();

	cout << "b = " << b << "\n"; 
	cout << "d = " << d << "\n";
	cout << "e = " << e << "\n";
	return 0;
}

int foo(){
	int x = 0;
	int y;
	cout << "y = " << y << "\n";
	return x;
}
