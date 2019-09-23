/* Micut Andrei-Ion-Grupa 311CB */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "list.h"
#include "queue.h"
#include "stack.h"

#define ERROR_BAD_PARAMS -1
#define ERROR_OPEN_FILES -2
#define ERROR_MISC -3

#define BUFFER_SIZE 256
#define MAX_PID 32767
#define MAX_MEMORY (3 * 1024 * 1024)

#define PROCESS_WAITING 0
#define PROCESS_RUNNING 1
#define PROCESS_FINISHED 2

//Define the structure of the process
typedef struct {
	unsigned short pid;
	unsigned char priority;
	unsigned int total_time, executed_time;
	int memory_begin, memory_size;
	int state, stack_bytes;
	stack_t stack;
} process_t;

//Print the waiting queue at current moment
void print_waiting_process(const void *p_process, FILE *file)
{
	process_t *p = (process_t *)p_process;
	fprintf(file, "(%hu: priority = %hhu, remaining_time = %u)",
	        p->pid, p->priority, p->total_time - p->executed_time);
}

//Print the queue of processes finished
void print_finished_process(const void *p_process, FILE *file)
{
	process_t *p = (process_t *)p_process;
	fprintf(file, "(%hu: priority = %hhu, executed_time = %u)",
	        p->pid, p->priority, p->executed_time);
}

//Print only the content of the stack
void print_stack_content(const void *p_content, FILE *file)
{
	fprintf(file, "%d", *((int *)p_content));
}

//Compare the waiting processes
int compare_waiting_process(const void *p_process1, const void *p_process2)
{
	process_t *p1 = (process_t *)p_process1;
	process_t *p2 = (process_t *)p_process2;

	// Sort by priority first
	if (p1->priority != p2->priority)
		return (int)p2->priority - p1->priority;

	// By the remaining time second
	if (p1->total_time - p1->executed_time !=
	    p2->total_time - p2->executed_time) {
		return (int)(p1->total_time - p1->executed_time) -
		    (int)(p2->total_time - p2->executed_time);
	}

	// And by pid last
	return (int)p1->pid - p2->pid;
}


//Compare the pids
int compare_process_pid(const void *p_process1, const void *p_process2)
{
	process_t *p1 = (process_t *)p_process1;
	process_t *p2 = (process_t *)p_process2;

	return (int)p1->pid - p2->pid;
}

// Add a new process in memory
process_t *add_process(queue_t *p_memory_queue,
                      process_t **pid_to_process,
                      unsigned int memory_size,
                      unsigned int total_time,
					  unsigned char priority)
{
	process_t *p = malloc(sizeof(process_t));
	if (p == NULL)
		return NULL;

	// Find the first free pid
	for (int i = 1; i <= MAX_PID; ++i) {
		if (pid_to_process[i] == NULL ||
		    pid_to_process[i]->state == PROCESS_FINISHED) {
			p->pid = i;
			break;
		}
	}

	// Initialize the process that is being created
	p->priority = priority;
	p->total_time = total_time;
	p->executed_time = 0;
	p->state = PROCESS_WAITING;
	p->memory_begin = -1;
	p->memory_size = memory_size;
	p->stack_bytes = 0;
	stack_initialize(&p->stack);

	// Try to find free space in memory
	// Create a process with the following atributes and 
	//insert it in the waiting queue
	// Descending by priority
	// Ascending by execution time(equal priority)
	// Ascending by PID(equal priority and execution time)
	int was_added = 0;
	queue_t tmp;
	queue_initialize(&tmp);
	unsigned int last_address = 0;
	while (!queue_is_empty(p_memory_queue)) {
		process_t *q = queue_pop(p_memory_queue);
		if (q->state == PROCESS_FINISHED) continue;
		if (!was_added && q->memory_begin - last_address >= memory_size) {
			was_added = 1;
			//I'm moving on the last address
			p->memory_begin = last_address;
			queue_push(&tmp, p);
		}
		last_address = q->memory_begin + q->memory_size;
		queue_push(&tmp, q);
	}
	if (!was_added && MAX_MEMORY - last_address >= memory_size) {
		was_added = 1;
		p->memory_begin = last_address;
		queue_push(&tmp, p);
	}

	if (!was_added) {
		// Attempt to defragment
		queue_sort(&tmp, compare_process_pid);
		last_address = 0;
		while (!queue_is_empty(&tmp)) {
			process_t *q = queue_pop(&tmp);
			q->memory_begin = last_address;
			last_address += q->memory_size;
			queue_push(p_memory_queue, q);
		}
		if (MAX_MEMORY - last_address >= memory_size) {
			p->memory_begin = last_address;
			queue_push(p_memory_queue, p);
		}
	} else {
		while (!queue_is_empty(&tmp)) {
			queue_push(p_memory_queue, queue_pop(&tmp));
		}
	}
	return p;
}

// Count how many times the given pid appears in queue
int count_pid(queue_t *p_queue, unsigned short pid)
{
	int ans = 0;
	queue_t tmp;
	queue_initialize(&tmp);

	// "Iterate" thorugh the queue by popping it into tmp
	while (!queue_is_empty(p_queue)) {
		process_t *q = queue_pop(p_queue);
		if (q->pid == pid) ++ans;
		queue_push(&tmp, q);
	}

	// Put all the elements back in the original queue
	while (!queue_is_empty(&tmp)) {
		queue_push(p_queue, queue_pop(&tmp));
	}

	return ans;
}

void free_simple(const void *p)
{
	free((void *)p);
}

void free_process(const void *p_process)
{
	if (p_process == NULL)
		return;
	// Free the stack and then the process
	stack_free(&((process_t *)p_process)->stack, free_simple);
	free((void *)p_process);
}

int main(int argc, char *argv[])
{
	int e;
	FILE *input_file, *output_file;
	char buffer[BUFFER_SIZE];

	// Check if the parameters where correctly passed
	if (argc != 3) {
		fprintf(stderr, "Run as ./%s in_file out_file\n", argv[0]);
		return ERROR_BAD_PARAMS;
	}

	// Open the files
	input_file = fopen(argv[1], "rt");
	output_file = fopen(argv[2], "wt");
	if (input_file == NULL || output_file == NULL)
		return ERROR_OPEN_FILES;

	// Initialize the needed structures
	unsigned int time_quanta, remaining_quanta = 0;
	queue_t memory_queue, waiting_queue, finished_queue;
	process_t *p_running_process = NULL;
	process_t *pid_to_process[MAX_PID + 1];
	for (int i = 0; i <= MAX_PID; ++i) pid_to_process[i] = NULL;

	queue_initialize(&memory_queue);
	queue_initialize(&waiting_queue);
	queue_initialize(&finished_queue);

	e = fscanf(input_file, "%u\n", &time_quanta);
	// Check the number of the parameters
	if (e != 1)
		goto exit_failure;

	// Suppose the input is more or less valid
	while (fscanf(input_file, "%s", buffer) == 1) {
		// Add the process in the waiting queue
		if (strcmp(buffer, "add") == 0) {
			unsigned int memory_size, total_time;
			unsigned char priority;
			e = fscanf(input_file, "%u %u %hhu\n",
			       &memory_size, &total_time, &priority);
			process_t *p = add_process(&memory_queue, pid_to_process,
			                           memory_size, total_time, priority);
			// If the process don't exist or e hasn't the specify atributes
			if (p == NULL || e != 3)
				//Exit failure
				goto exit_failure;
			// If it can't reserve memory for PID
			if (p->memory_begin < 0) {
				fprintf(output_file, "Cannot reserve memory for PID %hu.\n",
				        p->pid);
				free(p);
			} else {
				// If it can reserve memory for PID
				fprintf(output_file, "Process created successfully: PID: %hu, "
				        "Memory starts at 0x%x.\n", p->pid, p->memory_begin);
				pid_to_process[p->pid] = p;
				// If it doesn't exist a process
				if (p_running_process == NULL) {
					p_running_process = p;
					p->state = PROCESS_RUNNING;
				} else {
					// Else if it exist, it will be insert in the queue
					queue_insert(&waiting_queue, p, compare_waiting_process);
				}
			}
		} else if (strcmp(buffer, "get") == 0) {
			// Determination the status of a process
			unsigned short pid;
			e = fscanf(input_file, "%hu\n", &pid);
			if (e != 1)
				goto exit_failure;
			// Check if the PID of the process exist
			if (pid_to_process[pid] == NULL) {
				fprintf(output_file, "Process %hu not found.\n", pid);
			} else if (pid_to_process[pid]->state == PROCESS_RUNNING) {
				// Else the process exist and it is running
				// Print the atributes of the process
				fprintf(output_file, "Process %hu is running "
				        "(remaining_time: %u).\n", pid,
						pid_to_process[pid]->total_time -
						pid_to_process[pid]->executed_time);
				// Else the process exist and it is waiting
			} else if (pid_to_process[pid]->state == PROCESS_WAITING) {
				fprintf(output_file, "Process %hu is waiting "
				        "(remaining_time: %u).\n", pid,
						pid_to_process[pid]->total_time -
						pid_to_process[pid]->executed_time);
			}
			// Print the finish process with a specify pid 
			int n_finished = count_pid(&finished_queue, pid);
			for (int i = 0; i < n_finished; ++i) {
				fprintf(output_file, "Process %hu is finished.\n", pid);
			}
		} else if (strcmp(buffer, "push") == 0) {
			// Save the data on stack
			unsigned short pid;
			int value;
			e = fscanf(input_file, "%hu %d\n", &pid, &value);
			if (e != 2)
				goto exit_failure;
			if (pid_to_process[pid] == NULL ||
			    pid_to_process[pid]->state == PROCESS_FINISHED) {
				fprintf(output_file, "PID %hu not found.\n", pid);
			} else {
				process_t *p = pid_to_process[pid];
				// If the stack + 4 is full, then print the next message
				if (p->stack_bytes + 4 > p->memory_size) {
					fprintf(output_file, "Stack overflow PID %hu.\n", pid);
				} else {
					//Else allocate p_value
					int *p_value = malloc(sizeof(int));
					// Check the allocation
					if (p_value == NULL)
						goto exit_failure;
					p->stack_bytes += 4;
					*p_value = value;
					stack_push(&p->stack, p_value);
				}
			}
		} else if (strcmp(buffer, "pop") == 0) {
			// Delete the data from the stack
			unsigned short pid;
			e = fscanf(input_file, "%hu\n", &pid);
			if (e != 1)
				goto exit_failure;
			if (pid_to_process[pid] == NULL ||
			    pid_to_process[pid]->state == PROCESS_FINISHED) {
				fprintf(output_file, "PID %hu not found.\n", pid);
			} else {
				process_t *p = pid_to_process[pid];
				if (p->stack_bytes == 0) {
					fprintf(output_file, "Empty stack PID %hu.\n", pid);
				} else {
					// Eliminate the first 4 octets from the begining of the stack
					free(stack_pop(&p->stack));
					p->stack_bytes -= 4;
				}
			}
		} else if (strcmp(buffer, "print") == 0) {
			// Print the waiting queue
			e = fscanf(input_file, "%s", buffer);
			if (e != 1)
				goto exit_failure;
			// Print only the full content of the stack
			if (strcmp(buffer, "stack") == 0) {
				unsigned short pid;
				e = fscanf(input_file, "%hu\n", &pid);
				if (e != 1)
					goto exit_failure;
				process_t *p = pid_to_process[pid];
				if (p == NULL || p->state == PROCESS_FINISHED) {
					fprintf(output_file, "PID %hu not found.\n", pid);
				} else if (p->stack_bytes == 0) {
					fprintf(output_file, "Empty stack PID %hu.\n", pid);
				} else {
					fprintf(output_file, "Stack of PID %hu: ", pid);
					stack_print(&p->stack, print_stack_content,
					            " ", output_file);
					fprintf(output_file, ".\n");
				}
			} else if (strcmp(buffer, "waiting") == 0) {
				// Print the waiting queue
				fprintf(output_file, "Waiting queue:\n[");
				queue_print(&waiting_queue, print_waiting_process,
				            ",\n", output_file);
				fprintf(output_file, "]\n");
			} else if (strcmp(buffer, "finished") == 0) {
				// Print the finish queue of processes
				fprintf(output_file, "Finished queue:\n[");
				queue_print(&finished_queue, print_finished_process,
					        ",\n", output_file);
				fprintf(output_file, "]\n");
			}
		} else if (strcmp(buffer, "run") == 0) {
			// Execution units of time
			unsigned int t;
			e = fscanf(input_file, "%u\n", &t);
			if (e != 1)
				goto exit_failure;
			// Organize the queue by time
			while (t > 0) {
				if (remaining_quanta == 0) remaining_quanta = time_quanta;
				unsigned int running_time = t;
				if (running_time > remaining_quanta) {
					running_time = remaining_quanta;
				}
				// Check if exist the process
				if (p_running_process == NULL) {
					if (queue_is_empty(&waiting_queue)) break;
					p_running_process = queue_pop(&waiting_queue);
					p_running_process->state = PROCESS_RUNNING;
				}
				unsigned int remaining_time = p_running_process->total_time;
				remaining_time -= p_running_process->executed_time;
				// Calculate the remaining time for a process
				if (running_time > remaining_time)
					running_time = remaining_time;
				t -= running_time;
				p_running_process->executed_time += running_time;
				// Check if the process has finished
				if (p_running_process->executed_time ==
				    p_running_process->total_time) {
					p_running_process->state = PROCESS_FINISHED;
					queue_push(&finished_queue, p_running_process);
					if (!queue_is_empty(&waiting_queue)) {
						p_running_process = queue_pop(&waiting_queue);
						p_running_process->state = PROCESS_RUNNING;
					} else {
						p_running_process = NULL;
					}
				} else {
					remaining_quanta = remaining_quanta - running_time;
					if (remaining_quanta == 0) {
						p_running_process->state = PROCESS_WAITING;
						process_t *p = p_running_process;
						if (!queue_is_empty(&waiting_queue)) {
							p_running_process = queue_pop(&waiting_queue);
							p_running_process->state = PROCESS_RUNNING;
							queue_insert(&waiting_queue, p,
										 compare_waiting_process);
						} else {
							p_running_process->state = PROCESS_RUNNING;
						}
					}
				}
			}
		} else if (strcmp(buffer, "finish") == 0) {
			// Finish the execution of all processes
			unsigned int total_time = 0;
			while (p_running_process != NULL ||
				   !queue_is_empty(&waiting_queue)) {
				unsigned int running_time = time_quanta;
				if (p_running_process == NULL) {
					p_running_process = queue_pop(&waiting_queue);
					p_running_process->state = PROCESS_RUNNING;
				}
				unsigned int remaining_time = p_running_process->total_time;
				remaining_time -= p_running_process->executed_time;
				if (running_time > remaining_time)
					running_time = remaining_time;
				// Calculate the time remained to finish the process
				total_time += running_time;
				p_running_process->executed_time += running_time;
				if (p_running_process->executed_time ==
				    p_running_process->total_time) {
					p_running_process->state = PROCESS_FINISHED;
					queue_push(&finished_queue, p_running_process);
				} else {
					p_running_process->state = PROCESS_WAITING;
					queue_insert(&waiting_queue, p_running_process,
					             compare_waiting_process);
				}
				p_running_process = NULL;
			}
			fprintf(output_file, "Total time: %u\n", total_time);
		} else {
			// Ignore the unknown command but log it
			fprintf(stderr, "Unknown command '%s'", buffer);
		}
	}
	// Free the queue
	queue_free(&finished_queue, free_process);
	while (!queue_is_empty(&memory_queue))
		queue_pop(&memory_queue);
	fclose(input_file);
	fclose(output_file);

	return 0;

exit_failure:
	queue_free(&finished_queue, free_process);
	queue_free(&waiting_queue, free_process);
	free_process(p_running_process);
	while (!queue_is_empty(&memory_queue))
		queue_pop(&memory_queue);
	fclose(input_file);
	fclose(output_file);

	return ERROR_MISC;
}