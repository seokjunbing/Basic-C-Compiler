void main(void){
	int b[10];
	int a=foo(b);
	int c = a+ ++b[1]*4-5;
	a=++b[++a];
	a=-a;
	print b[-a];
}
int foo(int bar[]){
	bar[1]=3;
	return bar[1];
}