/* Micut Andrei-Ion-Grupa 311CB */

#ifndef _STACK_H_
#define _STACK_H_

#include "list.h"


typedef struct {
	list_cell_t *p_top;
} stack_t;

// Initialize the stack
void stack_initialize(stack_t *p_stack);

// Return 1 if the stack is empty (and 0 otherwise)
int stack_is_empty(stack_t *p_stack);

// Add another element at the top of the stack
void stack_push(stack_t *p_stack, void *p_data);

// Remove the element that is at the top of the stack
void *stack_pop(stack_t *p_stack);

// Return the data stored at the top of the stack
void *stack_top(stack_t *p_stack);

// Print the elements of the stack using the given print function
void stack_print(stack_t *p_stack, print_f print,
                 const char *separator, FILE *file);

// Free all the memory used by the stack
void stack_free(stack_t *p_stack, delete_f del);

#endif  // _STACK_H_