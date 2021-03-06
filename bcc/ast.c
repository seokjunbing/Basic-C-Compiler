/* ast.c
 *
 * Written by THC for CS 57; massaged by SWS; modified by Seok Jun Bing and Juan Torres
 *
 * Relies on an enum and a struct for
 * various node types that appear in ast.h.  You should modify the
 * enum and struct as appropriate.
 *
 * This file contains functions to create a node and to print out an
 * abstract syntax tree, for debugging.
 *
 * Modified by Juan C. Torres, SeokJun Bing
 *
 */

#include "symtab.h"
#include "ast.h"


/* How much more space to allocate to the array to keep track of
  current sub-scope for each level.
*/
#define DELTA 10

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
//int sibl[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
/*
  Array to keep track of current sub-scope for each level. For instance,
  siblings[2] would contain the current sub-scope for level 2 in our tree. See
  README for more information.
 */
 static int *siblings;
 static unsigned arraylen = 0;

 int curr_number = 0;


/* Create a node with a given token type and return a pointer to the
   node. */
 ast_node create_ast_node(ast_node_type node_type) {
  ast_node new_node = calloc(1, sizeof(struct ast_node_struct));  // for zeros
  new_node->node_type = node_type;
  return new_node;
}

void label_nodes(ast_node root){
  root->node_no = curr_number++;
  ast_node child;

  // if(root->node_type == ARRAY_TYPE_N) {
  //   if( root->left_child != NULL ){
  //     root->array_length = root->left_child->value_int;
  //   }
  // }

  for(child = root->left_child; child != NULL; child = child->right_sibling){
    label_nodes(child);
  }
}


/* Print the contents of a subtree of an abstract syntax tree, given
   the root of the subtree and the depth of the subtree root. */
void print_ast(ast_node root, int depth, int lvl, int sublvl) {
  /* Print two spaces for every level of depth. */
  int i;
  for (i = 0; i < depth; i++)
    printf("  ");

  /* Print the node type. */
  printf("%s (uid: N%d) ", NODE_NAME(root->node_type), root->node_no);
  printf("inferred ret type: %s ", NODE_NAME(root->return_type));
  printf("isDecl = %d, ignore = %d ", root->isDecl, root->ignore_sym);


  /* Print attributes specific to node types. */
  switch (root->node_type) {
    case CALL_N:
      //printf("inferred ret type: %s", NODE_NAME(root->return_type));
      break;
    case SEQ_N:     // change main level when see a new sequence
      // lvl++;
      break;

    case FORMAL_PARAMS_N:
      // lvl++;
      // printf("[scope (%d,%d) <- (%d,%d)]", root->curr_level, root->curr_sib, root->parent_level, root->parent_sib);
      // printf("[in scope: (%d,%d) | child of (%d,%d)]", root->curr_level, root->curr_sib, root->parent_level, root->parent_sib);

      break;

    case ARRAY_TYPE_N:
      printf("%s ", root->value_string);
      if(root->return_type != 0) {
        // printf(" (type: %s)", NODE_NAME(root->return_type));
      }
      // printf("[in scope: (%d,%d) | child of (%d,%d)]", lvl, sublvl, MAX(lvl - 1, 0), siblings[lvl - 1]);
      printf("[scope (%d,%d) <- (%d,%d)]", root->curr_level, root->curr_sib, root->parent_level, root->parent_sib);
      printf(" declared at line %d ", root->line_declared);
      break;

    case FUNCTION_N:
      printf("%s ", root->value_string);
      if(root->return_type != 0) {
        // printf(" (type: %s)", NODE_NAME(root->return_type));
      }
      int i = 0;
      ast_node anode;
      for(anode = root->left_child; anode != NULL; anode = anode->right_sibling) {
        i++;
      }
      printf(" (%d params:", i);
      for(anode = root->left_child; anode != NULL; anode = anode->right_sibling) {
        printf(" %s", NODE_NAME(root->return_type));
      }
      printf(")");
      // printf("[in scope: (%d,%d) | child of (%d,%d)]", lvl, sublvl, MAX(lvl - 1, 0), siblings[lvl - 1]);
      printf("[scope (%d,%d) <- (%d,%d)]", root->curr_level, root->curr_sib, root->parent_level, root->parent_sib);
      printf(" declared at line %d ", root->line_declared);
      break;

    case STRING_LITERAL_N:
      printf("%s ", root->value_string);
      break;

    case ID_N:			/* print the id */
      printf("%s ", root->value_string);
      if(root->return_type != 0) {
        // printf(" (type: %s)", NODE_NAME(root->return_type));
      }
      // printf("[in scope: (%d,%d) | child of (%d,%d)]", lvl, sublvl, MAX(lvl - 1, 0), siblings[lvl - 1]);
      if(root->return_type == ARRAY_TYPE_N){
        printf(" array length: %d ", root->snode->abnode->array_length);
      }
      printf("[scope (%d,%d) <- (%d,%d)]", root->curr_level, root->curr_sib, root->parent_level, root->parent_sib);
      printf(" declared at line %d ", root->line_declared);
      break;

    case INT_LITERAL_N:		/* print the int literal */
      printf("%d", root->value_int);
      // printf(" (type: %s)", NODE_NAME(root->return_type));
      break;

    case FUNC_DECLARATION_N:
      printf("%s ", root->value_string);
      assert(root->return_type != 0);
      // if(root->return_type != 0) {
        // printf(" (type: %s)", NODE_NAME(root->return_type));
      // }
      printf(" (%d params:", root->snode->num_parameters);
      for(int k = 0; k < root->snode->num_parameters; k++) {
        printf(" %s ", TYPE_NAME(root->snode->parameters[k]));
      }
      printf(")");
      // printf("[in scope: (%d,%d) | child of (%d,%d)]", lvl, sublvl, MAX(lvl - 1, 0), siblings[lvl - 1]);
      printf("[scope (%d,%d) <- (%d,%d)]", root->curr_level, root->curr_sib, root->parent_level, root->parent_sib);
      printf(" declared at line %d ", root->line_declared);
      break;

    case RETURN_N:
      // printf("[in scope: (%d,%d) | child of (%d,%d)]", lvl, sublvl, MAX(lvl - 1, 0), siblings[lvl - 1]);
      printf("[scope (%d,%d) <- (%d,%d)]", root->curr_level, root->curr_sib, root->parent_level, root->parent_sib);
      printf("return to %s at line %d ", root->return_to->value_string, root->return_to->line_declared);
      break;

    case FOR_HEADER_EMPTY_N:
      printf("EMPTY FOR HEADER");
      break;

    default:
      //printf("at default of switch\n");
      break;
  }

  if(root->return_type != 0) {
    printf(" (type: %s) ", NODE_NAME(root->return_type));
  }

  /* If ran out of space in array to track current sub-scope,
  reallocate more memory for it.
  */
  if(arraylen == lvl) {
    arraylen = arraylen + DELTA;
    siblings = realloc(siblings, sizeof(int) * arraylen);

    assert(siblings != NULL);

    for(int k=0; k < DELTA; k++) {
      siblings[arraylen - (DELTA-k)] = 0;
    }
  }


  // print out the syntax tree, along with scope number and parent of each child.
  // printf("[in scope: (%d,%d) | (child of %d,%d)]", lvl, sublvl, MAX(lvl - 1, 0), siblings[lvl - 1]);

  printf("\n");


  /* Recurse on each child of the subtree root, with a depth one
     greater than the root's depth. */
  ast_node child;
  for (child = root->left_child; child != NULL; child = child->right_sibling)
    print_ast(child, depth + 1, lvl, siblings[lvl]);

  if(root->node_type == SEQ_N){//} || root->node_type == FORMAL_PARAMS_N){
    siblings[lvl]++;  // change sibling level after you're done printing all
                      // subtrees, i.e., after done recursing.
  }
}



void simple_print_ast(ast_node root, int depth, int lvl, int sublvl) {
  /* Print two spaces for every level of depth. */
  int i;
  for (i = 0; i < depth; i++)
    printf("  ");

  /* Print the node type. */
  printf("%s (uid: N%d) ", NODE_NAME(root->node_type), root->node_no);
  printf("inferred ret type: %s ", NODE_NAME(root->return_type));
  printf("isDecl = %d, ignore = %d ", root->isDecl, root->ignore_sym);


  /* Print attributes specific to node types. */
  switch (root->node_type) {
    case CALL_N:
      //printf("inferred ret type: %s", NODE_NAME(root->return_type));
      break;
    case SEQ_N:     // change main level when see a new sequence
      // lvl++;
      break;

    case FORMAL_PARAMS_N:
      // lvl++;
      // printf("[scope (%d,%d) <- (%d,%d)]", root->curr_level, root->curr_sib, root->parent_level, root->parent_sib);
      // printf("[in scope: (%d,%d) | child of (%d,%d)]", root->curr_level, root->curr_sib, root->parent_level, root->parent_sib);

      break;

    case ARRAY_TYPE_N:
      printf("%s ", root->value_string);
      if(root->return_type != 0) {
        // printf(" (type: %s)", NODE_NAME(root->return_type));
      }
      // printf("[in scope: (%d,%d) | child of (%d,%d)]", lvl, sublvl, MAX(lvl - 1, 0), siblings[lvl - 1]);
      printf("[scope (%d,%d) <- (%d,%d)]", root->curr_level, root->curr_sib, root->parent_level, root->parent_sib);
      //printf(" array length = %d ", root->snode->abnode->array_length);
      printf(" declared at line %d ", root->line_declared);
      break;

    case FUNCTION_N:
      printf("%s ", root->value_string);
      if(root->return_type != 0) {
        // printf(" (type: %s)", NODE_NAME(root->return_type));
      }
      int i = 0;
      ast_node anode;
      for(anode = root->left_child; anode != NULL; anode = anode->right_sibling) {
        i++;
      }
      printf(" (%d params:", i);
      for(anode = root->left_child; anode != NULL; anode = anode->right_sibling) {
        printf(" %s", NODE_NAME(root->return_type));
      }
      printf(")");
      // printf("[in scope: (%d,%d) | child of (%d,%d)]", lvl, sublvl, MAX(lvl - 1, 0), siblings[lvl - 1]);
      printf("[scope (%d,%d) <- (%d,%d)]", root->curr_level, root->curr_sib, root->parent_level, root->parent_sib);
      printf(" declared at line %d ", root->line_declared);
      break;

    case STRING_LITERAL_N:
      printf("%s ", root->value_string);
      break;

    case ID_N:			/* print the id */
      printf("%s ", root->value_string);
      if(root->return_type != 0) {
        // printf(" (type: %s)", NODE_NAME(root->return_type));
      }
      // printf("[in scope: (%d,%d) | child of (%d,%d)]", lvl, sublvl, MAX(lvl - 1, 0), siblings[lvl - 1]);
      //if(root->return_type == ARRAY_TYPE_N){
      //  printf(" array length: %d ", root->snode->abnode->array_length);
      //}
      //printf("[scope (%d,%d) <- (%d,%d)]", root->curr_level, root->curr_sib, root->parent_level, root->parent_sib);
      printf(" declared at line %d ", root->line_declared);
      break;

    case INT_LITERAL_N:		/* print the int literal */
      printf("%d", root->value_int);
      // printf(" (type: %s)", NODE_NAME(root->return_type));
      break;

    case FUNC_DECLARATION_N:
      printf("%s ", root->value_string);
      assert(root->return_type != 0);
      // if(root->return_type != 0) {
        // printf(" (type: %s)", NODE_NAME(root->return_type));
      // }
      printf(" (%d params:", root->snode->num_parameters);
      //for(int k = 0; k < root->snode->num_parameters; k++) {
      //  printf(" %s ", TYPE_NAME(root->snode->parameters[k]));
      //}
      printf(")");
      // printf("[in scope: (%d,%d) | child of (%d,%d)]", lvl, sublvl, MAX(lvl - 1, 0), siblings[lvl - 1]);
      printf("[scope (%d,%d) <- (%d,%d)]", root->curr_level, root->curr_sib, root->parent_level, root->parent_sib);
      printf(" declared at line %d ", root->line_declared);
      break;

    case RETURN_N:
      // printf("[in scope: (%d,%d) | child of (%d,%d)]", lvl, sublvl, MAX(lvl - 1, 0), siblings[lvl - 1]);
      printf("[scope (%d,%d) <- (%d,%d)]", root->curr_level, root->curr_sib, root->parent_level, root->parent_sib);
      //printf("return to %s at line %d ", root->return_to->value_string, root->return_to->line_declared);
      break;

    case FOR_HEADER_EMPTY_N:
      printf("EMPTY FOR HEADER");
      break;

    default:
      //printf("at default of switch\n");
      break;
  }

  if(root->return_type != 0) {
    printf(" (type: %s) ", NODE_NAME(root->return_type));
  }

  /* If ran out of space in array to track current sub-scope,
  reallocate more memory for it.
  */
  if(arraylen == lvl) {
    arraylen = arraylen + DELTA;
    siblings = realloc(siblings, sizeof(int) * arraylen);

    assert(siblings != NULL);

    for(int k=0; k < DELTA; k++) {
      siblings[arraylen - (DELTA-k)] = 0;
    }
  }


  // print out the syntax tree, along with scope number and parent of each child.
  // printf("[in scope: (%d,%d) | (child of %d,%d)]", lvl, sublvl, MAX(lvl - 1, 0), siblings[lvl - 1]);

  printf("\n");


  /* Recurse on each child of the subtree root, with a depth one
     greater than the root's depth. */
  ast_node child;
  for (child = root->left_child; child != NULL; child = child->right_sibling)
    simple_print_ast(child, depth + 1, lvl, siblings[lvl]);

  if(root->node_type == SEQ_N){//} || root->node_type == FORMAL_PARAMS_N){
    siblings[lvl]++;  // change sibling level after you're done printing all
                      // subtrees, i.e., after done recursing.
  }
}





void patch_return_types(ast_node root){
  switch(root->node_type){

    case ARRAY_TYPE_N:             // check for return types!
      if(root->left_child == NULL){
        root->return_type = ARRAY_TYPE_N;

      }
      else{
        root->return_type = INT_TYPE_N;
        //root->ignore_sym = 1;
      }
      break;

  default:
    break;

  }

  ast_node child;
  for(child = root->left_child; child != NULL; child = child->right_sibling){
    patch_return_types(child);
  }
}
