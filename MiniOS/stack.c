/* Micut Andrei-Ion-Grupa 311CB */

#include <stdio.h>

#include "stack.h"

void stack_initialize(stack_t *p_stack)
{
	p_stack->p_top = NULL;
}

int stack_is_empty(stack_t *p_stack)
{
	return p_stack->p_top == NULL;
}

void stack_push(stack_t *p_stack, void *p_data)
{
	list_cell_t *p = make_list_cell(p_data);
	if (p != NULL && stack_is_empty(p_stack)) {
		p_stack->p_top = p;
	} else if (p != NULL) {
		p->next = p_stack->p_top;
		p_stack->p_top = p;
	}
}

void *stack_pop(stack_t *p_stack)
{
	list_cell_t *p = p_stack->p_top;
	p_stack->p_top = p->next;
	return erase_list_cell(p);
}

void *stack_top(stack_t *p_stack)
{
	return p_stack->p_top->p_data;
}

void stack_print(stack_t *p_stack, print_f print,
                 const char *separator, FILE *file)
{
	// Use a temporary stack
	stack_t tmp;
	stack_initialize(&tmp);

	// Put the elements in the auxiliary stack (in reverse order)
	while (!stack_is_empty(p_stack)) {
		stack_push(&tmp, stack_pop(p_stack));
	}

	// Reverse back the element in the original stack and print them
	while (!stack_is_empty(&tmp)) {
		void *p = stack_pop(&tmp);
		print(p, file);
		stack_push(p_stack, p);
		if (!stack_is_empty(&tmp))
			fprintf(file, "%s", separator);
	}
}

void stack_free(stack_t *p_stack, delete_f del)
{
	while (!stack_is_empty(p_stack)) {
		del(stack_pop(p_stack));
	}
}