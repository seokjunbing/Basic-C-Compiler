int foo(int bar[]){
	bar[1]=3;
	return bar[1];
}

void main(void){
	int b[10];
	int a;
	int c;
	//a=foo(b);
	b;
	c = a+ ++b[1]*4-5;
	a=++b[++a];
	a=-a;
	print b[-a];
}
