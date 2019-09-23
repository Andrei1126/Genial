/* Micut Andrei-Ion-Grupa 311CB */

#ifndef _QUEUE_H_

#define _QUEUE_H_

#include "list.h"


typedef struct {
	list_cell_t *p_front;
	list_cell_t *p_back;
	int size;
} queue_t;

// Initialize the given queue
void queue_initialize(queue_t *p_queue);

// Return 1 if the queue is empty and 0 otherwise
int queue_is_empty(queue_t *p_queue);

// Add a new element to the back of the queue
void queue_push(queue_t *p_queue, void *p_data);

// Remove the first element in the queue
void *queue_pop(queue_t *p_queue);

// Return the data that is stored in the first element of the queue
void *queue_front(queue_t *p_queue);

// Insert the given element somewhere in the queue, according to comp
void queue_insert(queue_t *p_queue, void *p_data, compare_f compare);

// Sort the queue by the given criteria
void queue_sort(queue_t *p_queue, compare_f compare);

// Print the queue using the given function
void queue_print(queue_t *p_queue, print_f print,
                 const char* separator, FILE *file);


// Free the memory used by the queue
void queue_free(queue_t *p_queue, delete_f del);

#endif  // _QUEUE_H_