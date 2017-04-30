#include<iostream>

using namespace std;

int foo1();

int main() {
	int mvar0 ;
	int mvar1 ;
	int mvar2 ;
	cout << "mvar0 = " << mvar0 << "\n";
	cout << "mvar1 = " << mvar1 << "\n";
	cout << "mvar2 = " << mvar2 << "\n";
foo1();
return 0;
}

int foo1(){
	int f1var0 ;
	int f1var1 ;
	int f1var2 ;
	cout << "f1var0 = " << f1var0 << "\n";
	cout << "f1var1 = " << f1var1 << "\n";
	cout << "f1var2 = " << f1var2 << "\n";
return 0;
}