#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "targetcode.h"
#include "symtab.h"
#include "unescape.h"

#define MAINSTART 20
#define DEBUG 1
#define DEFAULT_ARRAY_PARAM_SIZE 20
extern int temp_counter;

/* In our memory space, first will come the program instructions, then the
temps, then strings, then variables, and later functions. Anything else? */
int endoftemp;
int endofstr;

FILE *ofile = NULL;

/* Translates array of quads into series of assembly instructions */
int gen_target_code (quad_type **array, char argv[], symboltable_t* symboltable) {

	char *filename = calloc(sizeof(char), 100);
	if(NULL == strcat(filename, "./ys/")) {
		assert(0);
	}

	if(NULL == strcat(filename, argv)) {
		assert(0);
	}
	if(NULL == strcat(filename, ".ys")) {
		assert(0);
	}

	ofile = fopen(filename, "w");

	assert(ofile != NULL);

	calculate_var_offsets(symboltable->root);
	calculate_string_addrs(symboltable->literal_collection);
	calculate_global_var_addrs(symboltable);
#if DEBUG
	fprintf(ofile, "#END OF PROG: %d\n\n", ENDOFPROG);
#endif
	print_initialization();

	for(int i = 0; i < quad_index; i++){

		int type1;
		int type2;
#if DEBUG
		fprintf(ofile, "#");
		print_quad(array[i]);
#endif
		switch(array[i]->op){
			case Q_ASSIGN:
				/*
				   Move the value of the right hand side into %eax,
				   then call assign_immediate(), which will take care of moving it
				   into the location of the symnode (somewhere in memory,
				   depending on whether the left hand side is a temp, a global,
				   etc.)
				*/
				// fprintf(ofile, "\n\n#arlaksdgja;lksdfj as;df\n\n#");
				// print_quad(array[i]);
				// fprintf(ofile, "\n\n#arlaksdgja;lksdfj as;df\n\n");

				move_to_reg(array[i]->src1, LEFT_OPERAND_REG);
				//assign_immediate(array[i]->dest);
				if(array[i]->dest->is_array){
					assign_to_addr(array[i]->dest);
				} else{
					assign_immediate(array[i]->dest);
				}

				break;

			/*~~~~~~~~~ Binary operations ~~~~~~~~~*/
			case Q_ADD:
				move_to_reg_bin(array[i]);
				fprintf(ofile, "\taddl %s, %s\n", RIGHT_OPERAND_REG, LEFT_OPERAND_REG);
				assign_immediate(array[i]->dest);
				fprintf(ofile, "\n");
				break;

			case Q_SUB:
				move_to_reg_bin(array[i]);
				fprintf(ofile, "\tsubl %s, %s\n", RIGHT_OPERAND_REG, LEFT_OPERAND_REG);
				assign_immediate(array[i]->dest);
				fprintf(ofile, "\n");
				break;

			case Q_MULT:
				move_to_reg_bin(array[i]);
				fprintf(ofile, "\tmull %s, %s\n", RIGHT_OPERAND_REG, LEFT_OPERAND_REG);
				assign_immediate(array[i]->dest);
				fprintf(ofile, "\n");
				break;

			case Q_DIV:
				move_to_reg_bin(array[i]);
				fprintf(ofile, "\tdivl %s, %s\n", RIGHT_OPERAND_REG, LEFT_OPERAND_REG);
				assign_immediate(array[i]->dest);
				fprintf(ofile, "\n");
				break;


			case Q_MOD:
				move_to_reg_bin(array[i]);
				fprintf(ofile, "\tmodl %s, %s\n", RIGHT_OPERAND_REG, LEFT_OPERAND_REG);
				assign_immediate(array[i]->dest);
				fprintf(ofile, "\n");
				break;

			/*~~~~~~~~~ Unary operations ~~~~~~~~~*/

			case Q_INC:
				// move to %eax
				//move_to_reg(array[i]->src1, LEFT_OPERAND_REG);
				move_to_reg(array[i]->dest, LEFT_OPERAND_REG); // rmmovl -8(%esp), %eax
				fprintf(ofile, "\tirmovl 1, %s\n", RIGHT_OPERAND_REG); // irmovl 1, %ecx
				fprintf(ofile, "\taddl %s, %s\n", RIGHT_OPERAND_REG, LEFT_OPERAND_REG); // addl...
				if(assign_immediate(array[i]->dest) == 0){ //
					fprintf(ofile, "ERROR. TRYING TO ASSIGN VALUE TO INT\n");
				}
				break;

			case Q_DEC:
				move_to_reg(array[i]->dest, LEFT_OPERAND_REG);
				fprintf(ofile, "\tirmovl 1, %s\n", RIGHT_OPERAND_REG);
				fprintf(ofile, "\tsubl %s, %s\n", RIGHT_OPERAND_REG, LEFT_OPERAND_REG);

				if(!assign_immediate(array[i]->dest)){
					fprintf(ofile, "\n\n\nERROR. TRYING TO ASSIGN VALUE TO INT\n\n\n");
				}
				break;

			case Q_IFF:
				if(array[i-1]->op == Q_EQ) {
					fprintf(ofile, "\tjne %s\n",array[i]->dest->name_clean);
				}
				else if(array[i-1]->op == Q_GT) {
					fprintf(ofile, "\tjle %s\n",array[i]->dest->name_clean);
				}
				else if(array[i-1]->op == Q_LT) {
					fprintf(ofile, "\tjge %s\n",array[i]->dest->name_clean);
				}
				else if(array[i-1]->op == Q_GTEQ) {
					fprintf(ofile, "\tjl %s\n",array[i]->dest->name_clean);
				}
				else if(array[i-1]->op == Q_LTEQ) {
					fprintf(ofile, "\tjg %s\n",array[i]->dest->name_clean);
				}
				else if(array[i-1]->op == Q_NEQ) {
					fprintf(ofile, "\tje %s\n",array[i]->dest->name_clean);
				}
				else {
					assert(array[i]->src1 != NULL);
					move_to_reg(array[i]->src1, LEFT_OPERAND_REG);
					fprintf(ofile, "\tirmovl 0, %s\n", RIGHT_OPERAND_REG);
					fprintf(ofile, "\tsubl %s, %s\n", RIGHT_OPERAND_REG, LEFT_OPERAND_REG);
					fprintf(ofile, "\tje %s\n", array[i]->dest->name_clean);
				}
				break;

			case Q_IFT:
				if(array[i-1]->op == Q_EQ) {
					fprintf(ofile, "\tje %s\n",array[i]->dest->name_clean);
				}
				else if(array[i-1]->op == Q_GT) {
					fprintf(ofile, "\tjg %s\n",array[i]->dest->name_clean);
				}
				else if(array[i-1]->op == Q_LT) {
					fprintf(ofile, "\tjl %s\n",array[i]->dest->name_clean);
				}
				else if(array[i-1]->op == Q_GTEQ) {
					fprintf(ofile, "\tjge %s\n",array[i]->dest->name_clean);
				}
				else if(array[i-1]->op == Q_LTEQ) {
					fprintf(ofile, "\tjle %s\n",array[i]->dest->name_clean);
				}
				else if(array[i-1]->op == Q_NEQ) {
					fprintf(ofile, "\tjne %s\n",array[i]->dest->name_clean);
				}
				else {
					assert(array[i]->src1 != NULL);
					move_to_reg(array[i]->src1, LEFT_OPERAND_REG);
					fprintf(ofile, "\tirmovl 0, %s\n", RIGHT_OPERAND_REG);
					fprintf(ofile, "\tsubl %s, %s\n", RIGHT_OPERAND_REG, LEFT_OPERAND_REG);
					fprintf(ofile, "\tjne %s\n", array[i]->dest->name_clean);
				}

				break;

			case Q_GOTO:
				fprintf(ofile, "\tjmp %s\n", array[i]->dest->name_clean);

				break;

			case Q_LABEL:
				// Function declarations (and hence calls wil end up here)
				if(!is_function(array[i]->dest)) {
					fprintf(ofile, "%s:\n\n", array[i]->dest->name_clean);
				}
				if(is_function(array[i]->dest)){
					fprintf(ofile, "%s:\n\n", array[i]->dest->name);
					// Move the frame pointer to the current stack pointer.
					//fprintf(ofile, "\trrmovl %s, %s\n", STACK_PTR, BASE_PTR);

					// Leave space for locals in the stack by increasing the stack
					// pointer
					fprintf(ofile, "\tirmovl %d, %s\n",
						array[i]->dest->needed_space, RIGHT_OPERAND_REG);
					fprintf(ofile, "\tsubl %s, %s\n", RIGHT_OPERAND_REG, STACK_PTR);

					// Copy the parameters into locals.
					// Starts reading at 4(%ebp)
					// Starts copying at -8(%ebp), since 8 bytes needed for return address, old frame pointer.
					#if DEBUG
					fprintf(ofile, "#NUM PARAMS: %d\n", array[i]->dest->num_parameters);
					#endif
					fprintf(ofile, "#### SOMETHING\n");

					if(array[i]->dest->num_parameters > 0){
					//
					// 	// Change # of params to let the for loop take care of every single
					// 	// slot in the arrays passed as parameters.
					// 	int num_arr = 0;
					// 	for(int j = 0; j < array[i]->dest->num_parameters; j++){
					// 		if(array[i]->dest->parameters[j] == VAR_ARRAY_INT_T){
					// 			num_arr++;
					// 		}
					// 	}
					//
					// 	if(num_arr > 0){
					// 		fprintf(ofile, "#Increasing num of params\n");
					// 		array[i]->dest->num_parameters += (num_arr * DEFAULT_ARRAY_PARAM_SIZE - 1);
					// 	}
					// 	fprintf(ofile, "#New num of params: %d\n", array[i]->dest->num_parameters);
					// 	for(int j = 0; j < array[i]->dest->num_parameters; j++){
					// 		fprintf(ofile, "#%d\n", j);
					// 		fprintf(ofile, "\tmrmovl %d(%s), %s\n",
					// 		 	((j * 4) + 4), BASE_PTR, LEFT_OPERAND_REG);
					// 		fprintf(ofile, "\trmmovl %s, %d(%s)\n",
					// 		 	LEFT_OPERAND_REG, (-j * 4) - 8, BASE_PTR); // -8; -; 8 - 8;
					// 	}
					//
					// 	// Change # of params back
					// 	if(num_arr > 0){
					// 		array[i]->dest->num_parameters-= (num_arr * DEFAULT_ARRAY_PARAM_SIZE - 1);
					// 	}
					//
					}
				}
				break;

			case Q_READ:
				fprintf(ofile, "\tmrmovl %s, %s\n", KHXR, LEFT_OPERAND_REG);
				move_from_reg(LEFT_OPERAND_REG, array[i]->dest);
				break;

			case Q_PRINT:
				if(array[i]->dest->type == STRING_T) { //for strings
					//for(int k = 0; k <strlen(array[i]->dest->name); k++) {
						//if(k%4 == 0) {
							// fprintf(ofile, "%s\n", array[i]->dest->name);
							fprintf(ofile, "\tirmovl %d, %s\n", array[i]->dest->addr, IO_REG);
							fprintf(ofile, "\trmmovl %s, %s\n", IO_REG, DSTR);
						//}
					//}
					//store at 0x00FFFE10
				}
				else if(get_symnode_type(array[i]->dest) == TEMP_SYMNODE) { //for lit values  RET_SYMNODE
					//store at 0x00FFFE14
					fprintf(ofile, "\tmrmovl %d, %s\n", get_temp_addr(array[i]->dest), IO_REG);
					fprintf(ofile, "\trmmovl %s, %s\n", IO_REG, DHXR);
				}
				else if(get_symnode_type(array[i]->dest) == VAR_SYMNODE){
					if(is_var_global(array[i]->dest) == 1){ //global variable
						fprintf(ofile, "\tmrmovl %d, %s\n", array[i]->dest->addr, IO_REG);
						fprintf(ofile, "\trmmovl %s, %s\n", IO_REG, DHXR);
					}
					else { //local variable
						fprintf(ofile, "\tmrmovl %d(%s), %s\n", array[i]->dest->offset, BASE_PTR, IO_REG);
						fprintf(ofile, "\trmmovl %s, %s\n", IO_REG, DHXR);
					}
				}
				else if(get_symnode_type(array[i]->dest) == INT_SYMNODE){
					fprintf(ofile, "\tirmovl %d, %s\n", array[i]->dest->num_val, IO_REG);
					fprintf(ofile, "\trmmovl %s, %s\n", IO_REG, DHXR);
				}
				else if(get_symnode_type(array[i]->dest) == RET_SYMNODE) {
					fprintf(ofile, "\trrmovl %s, %s\n", RETURN_REG, IO_REG);
					fprintf(ofile, "\trmmovl %s, %s\n", IO_REG, DHXR);
				}
				else {
					assert(0);
				}
				break;

			case Q_EQ:
				move_to_reg_bin(array[i]);
				fprintf(ofile, "\tsubl %s, %s\n", RIGHT_OPERAND_REG, LEFT_OPERAND_REG);
				assign_immediate(array[i]->dest);
				break;

			case Q_GT:
				move_to_reg_bin(array[i]);
				fprintf(ofile, "\tsubl %s, %s\n", RIGHT_OPERAND_REG, LEFT_OPERAND_REG);
				assign_immediate(array[i]->dest);

				break;

			case Q_LT:
				move_to_reg_bin(array[i]);
				fprintf(ofile, "\tsubl %s, %s\n", RIGHT_OPERAND_REG, LEFT_OPERAND_REG);
				assign_immediate(array[i]->dest);
				break;

			case Q_GTEQ:
				move_to_reg_bin(array[i]);
				fprintf(ofile, "\tsubl %s, %s\n", RIGHT_OPERAND_REG, LEFT_OPERAND_REG);
				assign_immediate(array[i]->dest);
				break;

			case Q_LTEQ:
				move_to_reg_bin(array[i]);
				fprintf(ofile, "\tsubl %s, %s\n", RIGHT_OPERAND_REG, LEFT_OPERAND_REG);
				assign_immediate(array[i]->dest);
				break;

			case Q_NEG:
				move_to_reg(array[i]->src1, RIGHT_OPERAND_REG);
				fprintf(ofile, "\tirmovl %d, %s\n", 0, LEFT_OPERAND_REG);
				fprintf(ofile, "\tsubl %s, %s\n", RIGHT_OPERAND_REG, LEFT_OPERAND_REG);
				assign_immediate(array[i]->dest);
				break;

			case Q_NEQ:
				move_to_reg_bin(array[i]);
				fprintf(ofile, "\tsubl %s, %s\n", RIGHT_OPERAND_REG, LEFT_OPERAND_REG);
				assign_immediate(array[i]->dest);

				break;

			/*~~~~~~ Functions ~~~~~~~*/

			case Q_CALL:
				// Push caller's base pointer
				//fprintf(ofile, "\tpushl %s\n", BASE_PTR);
				// And transfer control to the function
				fprintf(ofile, "\tcall %s\n", array[i]->dest->name);
				break;

			case Q_PUSH:
				// Move into %eax, then push %eax
				move_to_reg(array[i]->dest, LEFT_OPERAND_REG);
				fprintf(ofile, "\tpushl %s\n", LEFT_OPERAND_REG);
				break;

			case Q_RETURN:
				move_to_reg(array[i]->dest, RETURN_REG);
				fprintf(ofile, "\trrmovl %s, %s\n", BASE_PTR, STACK_PTR);

				// Pop into base pointer, i.e., get old one back.
				fprintf(ofile, "\tpopl %s\n", BASE_PTR);

				// Move stack pointer back down so return will pop return address
				// and not something else.
				fprintf(ofile, "\tirmovl %d, %s\n", -8, RIGHT_OPERAND_REG);
				fprintf(ofile, "\taddl %s, %s\n", RIGHT_OPERAND_REG, STACK_PTR);
				fprintf(ofile, "\tret\n");
				break;

			case Q_PRECALL:
				// Push caller's base pointer
				fprintf(ofile, "\tpushl %s\n", BASE_PTR);
				fprintf(ofile, "\trrmovl %s, %s\n", STACK_PTR, BASE_PTR);
				break;

			case Q_POSTCALL:
				fprintf(ofile, "\tirmovl %d, %s\n", (4 + array[i]->dest->needed_space),
					RIGHT_OPERAND_REG);
				fprintf(ofile, "\trrmovl %s, %s\n", BASE_PTR, STACK_PTR);
				fprintf(ofile, "\taddl %s, %s\n", RIGHT_OPERAND_REG, STACK_PTR);
				break;

			/*~~~~~~~ Arrays ~~~~~~~~~*/

			case Q_ADDR:
				if(get_symnode_type(array[i]->src1) != VAR_SYMNODE){
					assert(0);
				}
				if(is_var_global(array[i]->src1)){
					fprintf(ofile, "\tirmovl %d, %s\n", array[i]->src1->addr, LEFT_OPERAND_REG);
				}
				else{ // local
					// Have to calculate address in ref to frame pointer:
					// move offset to %eax, add frame pointer to it.
					fprintf(ofile, "\tirmovl %d, %s\n", array[i]->src1->offset, LEFT_OPERAND_REG);
					fprintf(ofile, "\taddl %s, %s\n", BASE_PTR, LEFT_OPERAND_REG);
				}

				assign_immediate(array[i]->dest);
				break;

			case Q_DEREF:
				fprintf(ofile, "\tmrmovl %d, %s\n",
					get_temp_addr(array[i]->src1), RIGHT_OPERAND_REG);
				fprintf(ofile, "\tmrmovl (%s), %s\n", RIGHT_OPERAND_REG, LEFT_OPERAND_REG);
				assign_immediate(array[i]->dest);
				break;

			default:
				break;
		}
	}

	put_strings_in_mem(symboltable->literal_collection);
	print_global_vars(symboltable);
	print_stack_setup();

	return 0;
}


/*~~~~~~~~~~~~~~~~~~~~ Helper functions ~~~~~~~~~~~~~~~~~~~~~~~~*/


int offset = -8;

int calculate_var_offsets(symhashtable_t* hashtable) {
	symhashtable_t *hash = NULL;
	for(int j = 0; j < HASHSIZE; j++ ) { //every index in hash
		for(symnode_t *node = hashtable->table[j]; node != NULL; node = node->next) { //every linked list at an index
			offset = -8;
			if(node->type == FUNC_INT_T || node->type == FUNC_VOID_T) {

				assert(node != NULL);
				assert(node->abnode != NULL); //ast_node
				assert(node->abnode->left_child != NULL); //formal_params
				assert(node->abnode->left_child->right_sibling != NULL); //SEQ
				assert(node->abnode->left_child->right_sibling->left_child != NULL); //LOCAL_DECLARATIONS
				// assert(node->abnode->left_child->right_sibling->left_child->left_child);

				// If function has params
				//if(node->abnode->left_child->left_child != NULL) {
				if(has_params(node)){
					// Get the hashtable that the params live in.
					hash = node->abnode->left_child->left_child->snode->parent;
					// Make space for them
					calculate_var_offsets_helper(hash);
					node->needed_space = -offset;
				}

				// If function has locals
				//else if(node->abnode->left_child->right_sibling->left_child->left_child != NULL) { //this means tht this function contains var declarations
				else if(has_locals(node)){

					// If local declaration contains an assignment, this node will be a '=' in the ast.
					// Use its left child, which is linked to the variable declaration in the symbol table.
					//if(node->abnode->left_child->right_sibling->left_child->left_child->left_child != NULL){
					if(node->abnode->left_child->right_sibling->left_child->left_child->node_type == OP_ASSIGN_N){
						hash = node->abnode->left_child->right_sibling->left_child->left_child->left_child->snode->parent; //hashtabel containing var declaration

					// If it does not contain an assignment, simply use the node itself, which is
					// linked to the variable declaration in the symbol table.
					} else{
						hash = node->abnode->left_child->right_sibling->left_child->left_child->snode->parent; //hashtabel containing var declaration
					}

					//fprintf(ofile, "%s\n", node->name);
					//assert(node->abnode->left_child->right_sibling->left_child->left_child->snode != NULL);
					//hash = node->abnode->left_child->right_sibling->left_child->left_child->snode->parent; //hashtabel containing var declaration
					calculate_var_offsets_helper(hash); //offset starts at eight
					node->needed_space = -offset;
				}

			}
		}
	}
	return 0;
}


void calculate_var_offsets_helper(symhashtable_t* hashtable){

	for(int j = 0; j < HASHSIZE; j++ ) {
		for(symnode_t *node = hashtable->table[j]; node != NULL; node = node->next) {
			if(node->type == VAR_INT_T){
				if(!node->offset){
					node->offset = offset;
					offset -= 4;
				}
			}
			else if(node->type == VAR_ARRAY_INT_T ||
				 (node->abnode->node_type == ID_N && node->abnode->return_type == ARRAY_TYPE_N)) { // problem here
				if(!node->offset){
					node->offset = offset;
					offset = offset - 4 * DEFAULT_ARRAY_PARAM_SIZE;
				}

				//if(node->abnode->array_length != 0){
				//	offset = offset - 4 * node->abnode->array_length;
				//}
				//else{
					// Allocate a default number of elements for arrays as parameters
					// (no way of knowing how long they are when they are params)

				//}
			}
		}
	}

	/* Recurse on each child of the subtree hashtable */
	symhashtable_t *child;
	for (child = hashtable->child; child != NULL; child = child->rightsib)
		calculate_var_offsets_helper(child);

}


/* Returns 1 if the function symnode has parameters, 0 otherwise*/
int has_params(symnode_t *node){
	if(node->abnode->left_child->left_child != NULL) {
		return 1;
	} else{
		return 0;
	}
}


/* Returns 1 if the function symnode has local var declarations, 0 otherwise*/
int has_locals(symnode_t *node){
	//this means tht this function contains var declarations
	if(node->abnode->left_child->right_sibling->left_child->left_child != NULL) {
		fprintf(ofile, "\n\n #This function has locals \n\n");
		return 1;
	} else{
		return 0;
	}
}


/* Allocates space for all the variables in the symbol table */
void calculate_global_var_addrs(symboltable_t *symboltable){
	int global_var_offset = 0;
	symhashtable_t *hashtable = symboltable->root;

  for(int j = 0; j < HASHSIZE; j++) {
    for(symnode_t *node = hashtable->table[j]; node != NULL; node = node->next) {

      if(node->type == VAR_INT_T){
			node->addr = endofstr + global_var_offset;
			global_var_offset += 4;
      }

		else if(node->type == VAR_ARRAY_INT_T) {
			node->addr = endofstr + global_var_offset;
			global_var_offset += (node->abnode->array_length * 4);
		}
    }
  }
}


/* Print the global variables into the file */
void print_global_vars(symboltable_t *symboltable){
	symhashtable_t *hashtable = symboltable->root;

	fprintf(ofile, "\n.pos 0x%x   # Start of global var space \n", endofstr);

	for(int j = 0; j < HASHSIZE; j++) {
    for(symnode_t *node = hashtable->table[j]; node != NULL; node = node->next) {

      if(node->type == VAR_INT_T){
      		if(node->abnode->right_sibling == NULL) {
				fprintf(ofile, "\t.long 0x%08x\n", node->num_val);
      		}
      		else {
      			fprintf(ofile, "\t.long 0x%08x\n", node->abnode->right_sibling->value_int);
      		}
      }

			if(node->type == VAR_ARRAY_INT_T) {
				for(int i = 0; i < node->abnode->array_length; i++){
					fprintf(ofile, "\t.long 0x%08x\n", 0);
				}
			}
    }
  }
}


/*
 * Gets the address of a temporary variable
 */
int get_temp_addr(symnode_t* temp){
	return temp->num_val * 4 + ENDOFPROG + MAINSTART;
}


/* Calculates the location of strings in memory and stores that location in the
 symnode for the particular string */
void calculate_string_addrs(symhashtable_t* hashtable){
	endoftemp = (temp_counter * 4 + ENDOFPROG + 4) + MAINSTART; // Where in memory the strings should start
	// +4 because that's the space for the last quad.
	int str_offset = 0;	// str_offset

	// Traverse the hashtable looking for strings (we have to put them in
	// memory).
	for(int j = 0; j < HASHSIZE; j++ ) {
		for(symnode_t *node = hashtable->table[j]; node != NULL; node = node->next) {


			if(node->type == STRING_T){
				// Allocate memory for it
				node->addr = str_offset + endoftemp;
				if(node->addr %2 ==1){
					// node->addr++;
				}
				str_offset += round_str_addr(node->name);
			}
		}
	}

	endofstr = endoftemp + str_offset + 4;

	// endofstr++;
}


/*
   Lays out the strings in their location in memory as .byte directives
*/
void put_strings_in_mem(symhashtable_t* hashtable){

	assert(ofile != NULL);

	//fprintf(ofile, ".pos 0x%x    #Start of string space\n", endoftemp);

	// Traverse the hashtable looking for strings (we have to put them in
	// memory).
	for(int j = 0; j < HASHSIZE; j++ ) {



    for(symnode_t *node = hashtable->table[j]; node != NULL; node = node->next) {

			if(node->type == STRING_T){
				// Allocate memory for it
				fprintf(ofile, ".pos 0x%x    #qwerty\n", node->addr);
				node->name = unescape(node->name);
				// node->name[0] = '\0';
				// node->name[strlen(node->name)-1] = '\0';

				int counter = 0;
				int len = strlen(node->name);

				for(int counter = 0; counter < len; counter++){
					fprintf(ofile, "\n\t.byte \t");
					fprintf(ofile, "0x%x\n", node->name[counter]);
				}
				fprintf(ofile, "\n\t.byte \t");
					fprintf(ofile, "0x%x\n", '\0');
			}
		}
  }
}



/* Takes a str, and returns the amount of bytes it will need in memory (a multiple
   of 4), since addresses must be aligned in 4-byte boundaries. */
int round_str_addr(char* str){
	int len = (strlen(str)+1)*2;
	return len + 2- (len % 2);
}


/*
  Returns the type of the symnode (variable, temp, int) so that it is treated
  properly by other functions.
*/
int get_symnode_type(symnode_t *snode){
	//assert(snode != NULL);
  char *substr1 = substring(snode->name, 3); // first 3 chars of name
  char *substr2 = substring(snode->name, 2); // first 2 chars of name

  // if name starts with anything other than __, it is a var
  if(strcmp(substr2, "__") != 0){
		free(substr2);
		return VAR_SYMNODE;
  }
  free(substr2);

  if(strcmp(substr1, "__T") == 0){
    free(substr1);
    return TEMP_SYMNODE;
	} else if(strcmp(substr1, "__R") == 0){
		free(substr1);
		return RET_SYMNODE;
	} else{
    free(substr1);
    return INT_SYMNODE;
  }
}


/*
  Returns a substring of str, from str[0] to str[len]
*/
char *substring(char *str, int len) {
  char *sub = calloc(len + 1, sizeof(char));
  assert(sub != NULL);
  strncpy(sub, str, len);
  sub[len] = '\0';
  return sub;
}


void print_quad(quad_type *quad){
	fprintf(ofile, "(%s, %s, ",OP_NAME(quad->op), quad->dest->name);
	if (quad->src1 != NULL)
		fprintf(ofile, "%s, ",quad->src1->name);
	else
		fprintf(ofile, "-, ");
	if (quad->src2 != NULL)
		fprintf(ofile, "%s)\n", quad->src2->name);
	else
		fprintf(ofile, "-)\n");
}


/*
  Moves the operands of a quad containing a binary operation into %eax and %ecx.
	The left operand is moved into %eax, while the right one is moved into %ecx.
*/
void move_to_reg_bin(quad_type *quad){
	move_to_reg(quad->src1, LEFT_OPERAND_REG);
	move_to_reg(quad->src2, RIGHT_OPERAND_REG);
}


/*
  Moves a unary operand into %eax.
*/
void move_to_reg_un(quad_type *quad){
	move_to_reg(quad->src1, LEFT_OPERAND_REG);
}


/*
   Handles register and memory moves to assign a value seen in a quad
	 to a particular variable or temp.
	 Returns 1 if successful, 0 if unsuccessful (when trying to assign to an int
   or a register used for returning values)
*/
int assign_immediate(symnode_t *left_val){
	assert(left_val != NULL);

	int left_type = get_symnode_type(left_val);

	// Can't assign to right value (int or return register from function)
	if(left_type == INT_SYMNODE || left_type == RET_SYMNODE){
		return 0;
	} else{
		if(left_type == TEMP_SYMNODE){
				fprintf(ofile, "\trmmovl %s, %d\n", LEFT_OPERAND_REG, get_temp_addr(left_val));
		}
		else{ // var
			if(is_var_global(left_val)){
				fprintf(ofile, "\trmmovl %s, %d\n", LEFT_OPERAND_REG, left_val->addr);
			} else{
				fprintf(ofile, "\trmmovl %s, %d(%s)\n", LEFT_OPERAND_REG, left_val->offset, BASE_PTR);
			}
		}
		return 1;
	}
}

int assign_to_addr(symnode_t *addr_node){

	int left_type = get_symnode_type(addr_node);

	if(left_type != TEMP_SYMNODE){
		fprintf(ofile, "Error: acessing a non-address variable as one.\n");
		return 1;
	}
	fprintf(ofile, "\tmrmovl %d, %s\n", get_temp_addr(addr_node), RIGHT_OPERAND_REG);
	fprintf(ofile, "\trmmovl %s, (%s)", LEFT_OPERAND_REG, RIGHT_OPERAND_REG);
	return 0;
}

/*
  Moves a value to stored in a register to the variable specified by target.
*/
void move_from_reg(char* reg, symnode_t* target){
	int type = get_symnode_type(target);

	fprintf(ofile, "\t");
	if(type == TEMP_SYMNODE){
		fprintf(ofile, "rmmovl %s, %d\n", reg, get_temp_addr(target));
	} else if(type == RET_SYMNODE){
		fprintf(ofile, "rrmovl %s, %s\n", reg, RETURN_REG);
	} else if(type == VAR_SYMNODE){
		if(is_var_global(target)){
			fprintf(ofile, "rmmovl %s, %d\n", reg, target->addr);
		} else{ // local
			fprintf(ofile, "rmmovl %s, %d(%s)\n", reg, target->offset, BASE_PTR);
		}
	} else{
		fprintf(ofile, "Error: trying to assign a value to a right value\n");
	}
}

/*
   Moves the value in operand (depending on whether it is a temp,
   a global, or a local variable), and issues an instruction to move
	 into the register specified by reg.
	 For possible registers, see targetcode.h's macros for registers.
*/
void move_to_reg(symnode_t *operand, char *reg){
	assert(operand != NULL);
	int type = get_symnode_type(operand);

	fprintf(ofile, "\t");
	if(type == INT_SYMNODE){
		fprintf(ofile, "irmovl %d, %s\n", operand->num_val, reg);
	} else if(type == TEMP_SYMNODE){
		fprintf(ofile, "mrmovl %d, %s\n", get_temp_addr(operand),reg);
	} else if(type == RET_SYMNODE){
		fprintf(ofile, "rrmovl %s, %s\n", RETURN_REG, reg);
	} else{ // var
		if(is_var_global(operand)){
			fprintf(ofile, "mrmovl %d, %s\n", operand->addr, reg);
		} else{
			fprintf(ofile, "mrmovl %d(%s), %s\n", operand->offset, BASE_PTR, reg);
		}
	}
}

/*
   Returns 1 if a label is a function, 0 otherwise
*/
int is_function(symnode_t *label){
	char *substr1 = substring(label->name, 3); // first 3 chars of label name

	if(strcmp(substr1, "__L") != 0){
		return 1;
	} else{
		return 0;
	}
}


void print_initialization() {
	fprintf(ofile, ".pos 0\n");
	fprintf(ofile, "\tirmovl stack, %s\n", STACK_PTR);
	fprintf(ofile, "\tirmovl stack, %s\n", BASE_PTR);
	fprintf(ofile, "\tcall main\n");
	fprintf(ofile, "\thalt\n");
	fprintf(ofile, ".pos 0x14\n");
}

void print_stack_setup() {
	fprintf(ofile, ".pos 0x0000FFFC\n");
	fprintf(ofile, "stack:\n");
}
