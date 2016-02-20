# Symbols and types: Bottom-up traversal

In this part of our compiler, we traverse the syntax tree created in previous steps of our project, and with the help of the symbol table created in previous steps, we generate 3-address instructions represented as 4-tuples, or quads.

## Authors
- SeokJun Bing
- Juan C. Torres

Some code provided by Sean W. Smith with some originally written by Thomas Cormen.

## How to build and run

- Execute `make` to build the program, `make clean` to delete object files and other executable files.

- Execute make test to build the program and run relevant tests. The folder `./testinputs` contains several example programs written in our C grammar, a subset of the full C grammar. Our test script should produce several files `{testfile}.output.{date}` with the results of each test.

## Program execution

*For descriptions of previous parts of this compiler, please see previous folders.*

-`quad.c` and `quad.h` define the data structures and functions required to build the array of quads that we will eventually use to generate assembly code.

- `symtab.c` and `symtab.h` define the data structures and functions required to build the symbol tables, as well as the functions to check for errors related to such symbol table,
e.g., checking that a variable has been defined before it is used.

- `parser.l` is a flex specification so that flex returns the next token from the input stream and returns other relevant information to the creation of a parse tree, such as string and int values.

- `parser.y` is a bison file, including a grammar and executable code. Each production in the grammar is accompanied by some code used to build a parse tree.

- `ast.c` and `ast.h` define the relevant data structures and functions needed to build a parse tree.

The program traverses the syntax tree created in previous steps, assuming that there have been no errors until this point (if there were, the program halts).


### Generating a collection of literals

We begin by generating a collection of literals, stored as a hash table in the main symbol table in the `->literal_collection` field. Here we collect the strings and integer literals found in the source file to compile. Each of them is a symbol node (`symnode`), which we will use in the following step.

Temporary variables and labels are added to this collection. Temporary variables are named `__T` followed by a unique number.

Labels are either named `__L` followed by a number corresponding to the number of the node that generated them, or named after function names.

Integer literals are named `__` followed by their integer value.

The names of string literal do not change.

Different types of `symnode` symbol nodes are named differently to preserve the ability to differentiate them from other types of nodes that possibly have equal names. For instance, this allows us to differentiate between the integer `1` (represented as `__1`) and the string literal `1`, represented as `1`. There remains the possibility for clashes if the user names a string `__1`, for instance, but we hope to minimize those.

### Generating three-address instructions, or quads

These are four-tuples or quads, specifying one destination, and up to two sources, for the operations we define. For instance, `(ADD, t1, c, d)` adds `c` and `d`, and stores the result in `t1`.Â

Not all instructions need to use three addresses: `(GOTO, a, -, -)`, which makes the execution of our program jump, only needs access to one address.

The resulting quads are stored in an array in the order they should be executed. They are created by traversing the abstract syntax tree created in previous steps in a post-order way (visiting all children before visiting the parent node), with some exceptions.

#### Generating labels

This function takes as a parameter an input string, an `ast_node`, and a pointer to the collection of literals. It creates a `symnode` with the name `__L_` concatenated with the node number, concatenated with an input string. An example of a label is `__L_23_DO_WHILE`.

#### Function prologue and epilogues

Since we are at this stage generating intermediate code, function prologues and epilogues are by necessity high-level.
In a function prologue, a caller pushes all the arguments to the function, then transfers control to the function.

Function prologues are, at the moment, just placeholders.

Both of these are handled by the caller.

An example for the function call `my_func(b + 3 + d, c)` follows:

```
62: (LABEL, __L_121_FUNC_PROLOGUE_BEGIN, -, -)
63: (ADD, __T16, b, __3)
64: (ADD, __T17, __T16, d)
65: (ASSIGN, __T18, __T17, -)
66: (PUSH, __T18, -, -)
67: (ASSIGN, __T19, c, -)
68: (PUSH, __T19, -, -)
69: (CALL, my_func, -, -)
70: (LABEL, __L_121_FUNC_EPILOGUE, -, -)
```


#### Quad generation examples for selected cases

##### a < 10
```
18: (GT, __T12, a, __10)
19: (ifTrue, __L_48_TRUE, __T12, -)
20: (ASSIGN, __T12, __0, -)
21: (GOTO, __L_48_DONE, -, -)
22: (LABEL, __L_48_TRUE, -, -)
23: (ASSIGN, __T12, __1, -)
24: (LABEL, __L_48_DONE, -, -)
```

#####	Do while
```
do{
  a = 3;
} while(b == 3);
```
generates the following quads
```
38: (LABEL, __L_84_DO_WHILE_BEGIN, -, -)
39: (ASSIGN, a, __3, -)
40: (EQ, __T43, b, __3)
41: (ifTrue, __L_85_TRUE, __T43, -)
42: (ASSIGN, __T43, __0, -)
43: (GOTO, __L_85_DONE, -, -)
44: (LABEL, __L_85_TRUE, -, -)
45: (ASSIGN, __T43, __1, -)
46: (LABEL, __L_85_DONE, -, -)
47: (ifTrue, __L_84_DO_WHILE_BEGIN, __T43, -)
```

#### Arrays

Array elements are accessed as specified in lecture 13:

```
t2 = a[t1] turns into
t3 = t1 * elem_size(a)
t4 = &a + t3
t2 = *t4
```
That is, the offset into the memory chunk of the array is calculated first, followed
by adding this to the address of the array, and finally dereferencing the object
stored at that address in the array.

An example for  `	my_func(fff[1], v + (y * t *(v - ppp)))` follows.

```
81: (LABEL, __L_127_FUNC_PROLOGUE_BEGIN, -, -)
82: (MULT, __T29, __1, __4)
83: (ADDR, __T30, fff, -)
84: (ADD, __T31, __T29, __T30)
85: (DEREF, __T32, __T31, -)
86: (MULT, __T33, y, t)
87: (SUB, __T34, v, ppp)
88: (MULT, __T35, __T33, __T34)
89: (ADD, __T36, v, __T35)
90: (ASSIGN, __T37, __T32, -)
91: (PUSH, __T37, -, -)
92: (ASSIGN, __T38, __T36, -)
93: (PUSH, __T38, -, -)
94: (CALL, my_func, -, -)
95: (LABEL, __L_127_FUNC_EPILOGUE, -, -)

```



## Changes from previous programs

- Added a couple of helper functions to `symtab.c`

- Added a condition to halt in driver if there were certain errors.

- Changed the way empty returns are handled a bit.