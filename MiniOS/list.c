/* Micut Andrei-Ion-Grupa 311CB */

#include <stdlib.h>

#include "list.h"

list_cell_t *make_list_cell(void *p_data)
{
	// Attempt to allocate the memory
	list_cell_t *p = malloc(sizeof(list_cell_t));
	if (p == NULL)
		return NULL;

	// Initialize the fields
	p->p_data = p_data;
	p->next = NULL;

	return p;
}

void *erase_list_cell(list_cell_t *p_cell)
{
	// Delete the cell and return its data
	void *p = p_cell->p_data;
	free(p_cell);

	return p;
}