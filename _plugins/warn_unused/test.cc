struct A{
	int x, y;
};

__attribute__((warn_unused_result)) A foo();

void h(const A&);
void m(int);

void test_foo(){
	foo();
	h(foo());
	int z = foo().y;
	foo().y;
	m(foo().y);
}

struct B{
	~B();
};

__attribute__((warn_unused_result)) B bar();

void test_bar(){
	bar();
}
A quux(){
	return foo();
}
