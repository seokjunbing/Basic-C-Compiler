CC = gcc
CFLAGS = -g -pedantic -ggdb3
BISONFL = -d -v
FLEXFLAGS = -ll

.PHONY: clean

.SUFFIXES: .c

.cc.o:
	$(CC) $(CFLAGS) -c $<

bcc : lex.yy.o parser.tab.o parser_driver.o ast.o symtab.o quad.o targetcode.o unescape.o
	$(CC) -o $@ $(CFLAGS) lex.yy.o parser.tab.o parser_driver.o ast.o symtab.o quad.o targetcode.o unescape.o $(FLEXFLAGS)

lex.yy.o : lex.yy.c
	$(CC) -c $(CFLAGS) $<

parser.tab.o : parser.tab.c
	$(CC) -c $(CFLAGS) $<

lex.yy.c : parser.l parser.tab.h
	flex parser.l

parser.tab.h : parser.y
	bison $(BISONFL) $<

parser.tab.c : parser.y
	bison $(BISONFL) $<

parser_driver.o : parser_driver.c

clean :
	rm -f parser parser.tab.h parser.tab.c lex.yy.c *~ parser.output
	rm -rf *.o
	rm -rf *.dSYM

depend :
	makedepend -- $(CFLAGS) -- parser_driver.c

# DO NOT DELETE THIS LINE -- make depend depends on it.

test:
	bash ./testscript.sh
