/* Micut Andrei-Ion-Grupa 311CB */

#include <stdio.h>

#include "queue.h"



void queue_initialize(queue_t *p_queue)
{
	p_queue->size = 0;
	p_queue->p_front = p_queue->p_back = NULL;
}

int queue_is_empty(queue_t *p_queue)
{
	return p_queue->p_front == NULL;
}

void queue_push(queue_t *p_queue, void *p_data)
{
	list_cell_t *p = make_list_cell(p_data);
	// Push the element if and only if the cell was correctly built
	if (p != NULL && queue_is_empty(p_queue)) {
		p_queue->p_front = p_queue->p_back = p;
		++p_queue->size;
	} else if (p != NULL) {
		p_queue->p_back->next = p;
		p_queue->p_back = p;
		++p_queue->size;
	}
}

void *queue_pop(queue_t *p_queue)
{
	list_cell_t *p = p_queue->p_front;
	p_queue->p_front = p->next;
	--p_queue->size;
	return erase_list_cell(p);
}

void *queue_front(queue_t *p_queue)
{
	return p_queue->p_front->p_data;
}

void queue_insert(queue_t *p_queue, void *p_data, compare_f compare)
{
	// Create two auxiliary queues
	queue_t tmp1, tmp2;
	queue_initialize(&tmp1);
	queue_initialize(&tmp2);

	// Put smaller elements in the first queue
	// And put the bigger elements in the second queue
	while (!queue_is_empty(p_queue)) {
		void *p = queue_pop(p_queue);
		if (compare(p, p_data) < 0)
			queue_push(&tmp1, p);
		else
			queue_push(&tmp2, p);
	}

	// Insert back the smaller elements
	while (!queue_is_empty(&tmp1)) {
		queue_push(p_queue, queue_pop(&tmp1));
	}

	// Insert the given element
	queue_push(p_queue, p_data);

	// Insert back the bigger elements
	while (!queue_is_empty(&tmp2)) {
		queue_push(p_queue, queue_pop(&tmp2));
	}
}

void queue_sort(queue_t *p_queue, compare_f compare)
{
	queue_t tmp;
	queue_initialize(&tmp);

	while (!queue_is_empty(p_queue)) {
		// Find the current minimum in the queue
		void *p_min = queue_pop(p_queue);
		int n = p_queue->size;
		while (n--) {
			void *p = queue_pop(p_queue);
			if (compare(p_min, p) > 0) {
				queue_push(p_queue, p_min);
				p_min = p;
			} else {
				queue_push(p_queue, p);
			}
		}
		queue_push(&tmp, p_min);
	}
	// Indirectly return the sorted queue
	p_queue->p_front = tmp.p_front;
	p_queue->p_back = tmp.p_back;
}


void queue_print(queue_t *p_queue, print_f print,
                 const char* separator, FILE *file)
{
	// Create an auxiliary queue
	queue_t tmp;
	queue_initialize(&tmp);

	// Put everything in the auxiliary queue
	while (!queue_is_empty(p_queue)) {
		queue_push(&tmp, queue_pop(p_queue));
	}

	// Print the elements and put them back in the original queue
	while (!queue_is_empty(&tmp)) {
		void *p = queue_pop(&tmp);
		print(p, file);
		queue_push(p_queue, p);
		if (!queue_is_empty(&tmp))
			fprintf(file, "%s", separator);
	}
}

void queue_free(queue_t *p_queue, delete_f del)
{
	while (!queue_is_empty(p_queue)) {
		del(queue_pop(p_queue));
	}
}