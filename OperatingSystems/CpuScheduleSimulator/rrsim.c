#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linkedlist.h"

#define MAX_BUFFER_LEN 80

#define minfl(a, b) a < b ? a : b

taskval_t *event_list = NULL;
taskval_t *ready_q = NULL;
taskval_t *finished_q = NULL;
int time = -1;

void print_task(taskval_t *t, void *arg) {
    printf("task %03d: %5d %3.2f %3.2f\n",
        t->id,
        t->arrival_time,
        t->cpu_request,
        t->cpu_used
    );  
}


void increment_count(taskval_t *t, void *arg) {
    int *ip;
    ip = (int *)arg;
    (*ip)++;
}

/**
 *  This function increments the time counter and handles the arrival of new tasks. 
 */
int time_tick(int time) {
    time++;                                                             /* Increment time counter. */
    taskval_t *next_to_arrive;                             
    while ((next_to_arrive = peek_front(event_list)) != NULL) {         /* Check for incoming tasks. */
        if (time == event_list->arrival_time) {
            event_list = remove_front(event_list);                      /* Remove arriving task from event_list. */
            ready_q = add_end(ready_q, next_to_arrive);                 /* Add arriving task to the ready_q. */
        } 
        else break;
    }
    return time;
}

/**
 *  This function simulates the cost of dispatching a task for cpu time.
 */
void dispatch_task(taskval_t *task, int dispatch_cost) {
    if (dispatch_cost == 0) fprintf(stdout,"[%05d] DISPATCHING\n", time);
    for (int i = 0; i < dispatch_cost; i++) {                           /* Simulate dispatching time. */
        fprintf(stdout,"[%05d] DISPATCHING\n", time);
        time = time_tick(time);                                         /* Increment the time to account for dispatch cost. */
    }
}

/**
 *      This function simulates the completion of a task after running in the current
 * time tick. It assigns the task finish_time field, attributing to the task the whole
 * time tick. It also calculates the the waiting and turnaround times of the task. Then
 * it checks that if there are more tasks to be run. If there are not, then it exits the
 * program successfuly.
 */
void complete_task(taskval_t *task) {
    task->finish_time = time + 1;                                       /* Task finishes at the end of the current tick. */
    float turnaround = task->finish_time - task->arrival_time;          /* Calculate the turnaround time. */
    float waiting = turnaround - task->cpu_used;                        /* Calculate the waiting time. */

    fprintf(stdout, "[%05d] id=%05d EXIT w=%.2f ta=%.2f\n",             /* Print exit information */
        task->finish_time,
        task->id,
        waiting,
        turnaround
        );

    finished_q = add_end(finished_q, task);                             /* Add task to finished_q. Used for debugging. */

    return;
}

/**
 *  This function simulates the execution of a task for one time tick. This updates the
 * field indicating the amount of cpu time a task has used.
 */
void execute_task(taskval_t *task, int quantum) {
    for (int i = 0; i < quantum; i++) {                                 /* Simulate cpu burst time. */
        fprintf(stdout, "[%05d] id=%05d req=%.2f used=%.2f\n",          /* Print execution information. */
        time,
        task->id,
        task->cpu_request,
        task->cpu_used
        );
                
        float burst = minfl(1.0, task->cpu_request - task->cpu_used);   /* Calculate the actual amount of burst time required this tick. */
        task->cpu_used += burst;                                        /* Update task cpu_used field. */

        if (task->cpu_used == task->cpu_request) break;                 /* Task run to completion. */
        if (i != quantum - 1) time = time_tick(time);                   /* Increment time for all except the final tick since */
    }                                                                   /* time will be incremented at the beginning of the next loop. */
    return;
}

void run_simulation(int qlen, int dlen) {
    for (;;) {
        time = time_tick(time);                                         /* Increment the time counter. */
        
        taskval_t *task_to_run = peek_front(ready_q);                   /* Determine the text task to run. */
        if (task_to_run == NULL) {                                      /* Nothing in ready_q => cpu is idle. */
            fprintf(stdout, "[%05d] IDLE\n", time);         
        }
        else {
            ready_q = remove_front(ready_q);                            /* Remove task from the ready_q. */
            dispatch_task(task_to_run, dlen);                           /* Simulate dispatch cost. */
            execute_task(task_to_run, qlen);                            /* Simulate task execution. */
            
            if (task_to_run->cpu_used == task_to_run->cpu_request) {    /* Task is complete. Print exit information. */
                complete_task(task_to_run);
                if (event_list == NULL && ready_q == NULL) {            /* No more tasks, exit simulotion. */
                    break;
                }
            }
            else {                                                      /* Task is incomplete. */
                ready_q = add_end(ready_q, task_to_run);                /* replace it on the end of the ready_q */
            }
        }
    }
}

int main(int argc, char *argv[]) {
    char   input_line[MAX_BUFFER_LEN];
    int    i;
    int    task_num;
    int    task_arrival;
    float  task_cpu;
    int    quantum_length = -1;
    int    dispatch_length = -1;

    taskval_t *temp_task;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--quantum") == 0 && i+1 < argc) {
            quantum_length = atoi(argv[i+1]);
        }
        else if (strcmp(argv[i], "--dispatch") == 0 && i+1 < argc) {
            dispatch_length = atoi(argv[i+1]);
        }
    }

    if (quantum_length == -1 || dispatch_length == -1) {
        fprintf(stderr, 
            "usage: %s --quantum <num> --dispatch <num>\n",
            argv[0]);
        exit(1);
    }


    while(fgets(input_line, MAX_BUFFER_LEN, stdin)) {
        sscanf(input_line, "%d %d %f", &task_num, &task_arrival,
            &task_cpu);
        temp_task = new_task();
        temp_task->id = task_num;
        temp_task->arrival_time = task_arrival;
        temp_task->cpu_request = task_cpu;
        temp_task->cpu_used = 0.0;
        event_list = add_end(event_list, temp_task);
    }

#ifdef DEBUG
    int num_events;
    apply(event_list, increment_count, &num_events);
    printf("DEBUG: # of events read into list -- %d\n", num_events);
    printf("DEBUG: value of quantum length -- %d\n", quantum_length);
    printf("DEBUG: value of dispatch length -- %d\n", dispatch_length);
#endif

    run_simulation(quantum_length, dispatch_length);

    return (0);
}
