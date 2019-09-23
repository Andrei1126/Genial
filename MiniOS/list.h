/* Micut Andrei-Ion-Grupa 311CB */

#ifndef _LIST_H_
#define _LIST_H_

#include <stdio.h>


// Generic function used for printing
typedef void (*print_f)(const void *, FILE *);

// Generic comparator function
typedef int (*compare_f)(const void *, const void *);

// Generic delete function
typedef void (*delete_f)(const void *);

typedef struct _list_cell_t {
	void *p_data;
	struct _list_cell_t *next;
} list_cell_t;

// Create a new cell that holds the given data
list_cell_t *make_list_cell(void *p_data);
 	
// Delete the cell and return its content
void *erase_list_cell(list_cell_t *p_cell);

#endif  // // _LIST_H_