/**
 * Scheduler Lab
 * CS 241 - Spring 2017
 */
#include "libpriqueue.h"
#include "libscheduler.h"

static priqueue_t pqueue;
static core_t core;
static scheme_t scheme;
static int (*comparison_func)(const void *, const void *);

job_t* current_running_job;
job_t* pseudo_job; // job that is going to be pushed onto the queue
unsigned average_waiting_time = 0;
unsigned average_turnaround_time = 0;
unsigned average_response_time = 0;
double number_of_jobs = 0;

int comparer_fcfs(const void *a, const void *b) {
    job_t* a_job = (job_t*)a;
    job_t* b_job = (job_t*)b;
    if(a_job->arrival_time > b_job->arrival_time){
        return 1; // b first come
    }else if(a_job->arrival_time < b_job->arrival_time){
        return -1; // a first come
    }else{
        return 0; // arrive at the same time
    }
}

int break_tie(const void *a, const void *b) { return comparer_fcfs(a, b); }

int comparer_ppri(const void *a, const void *b) {
    // Complete as is
    return comparer_pri(a, b);
}

int comparer_pri(const void *a, const void *b) { 
    job_t* a_job = (job_t*)a;
    job_t* b_job = (job_t*)b;
    if(a_job->priority > b_job->priority){
        return 1; // b go first
    }else if(b_job->priority > a_job->priority){
        return -1;
    }else{
        return 0;
    }
}

int comparer_plrtf(const void *a, const void *b) { 
    job_t* a_job = (job_t*)a;
    job_t* b_job = (job_t*)b;
    if(a_job->remaining_time > b_job->remaining_time){
        return 1;
    }else if(b_job->remaining_time > a_job->remaining_time){
        return -1;
    }else{
        return 0;
    }
}

int comparer_rr(const void *a, const void *b) { 
    (void)a;
    (void)b;
    return 1;
}

int comparer_sjf(const void *a, const void *b) { 
    job_t* a_job = (job_t*)a;
    job_t* b_job = (job_t*)b;
    if(a_job->execution_time > b_job->execution_time){
        return 1; // b first
    }else if(b_job->execution_time > a_job->execution_time){
        return -1;
    }else{
        return 0;
    }
}

void scheduler_start_up(scheme_t s) {
    switch (s) {
    case FCFS:
        comparison_func = comparer_fcfs;
        break;
    case PRI:
        comparison_func = comparer_pri;
        break;
    case PPRI:
        comparison_func = comparer_ppri;
        break;
    case PLRTF:
        comparison_func = comparer_plrtf;
        break;
    case RR:
        comparison_func = comparer_rr;
        break;
    case SJF:
        comparison_func = comparer_sjf;
        break;
    default:
        printf("Did not recognize scheme\n");
        exit(1);
    }
    // for(int i=0; i<256; i++){
    //     id_list[i] = 0;
    // }
    priqueue_init(&pqueue, comparison_func);
}

bool scheduler_new_job(int job_number, unsigned time, unsigned running_time,
                       int priority) {
    pseudo_job = malloc(sizeof(job_t));
    pseudo_job->remaining_time = running_time;
    pseudo_job->id = job_number;
    pseudo_job->arrival_time = time;
    pseudo_job->priority = priority;
    pseudo_job->start_execution_time = -1;
    pseudo_job->complete_time = 0;
    pseudo_job->execution_time = running_time;
    pseudo_job->actual_exec_time = running_time;
    number_of_jobs++;
    //id_list[job_number] = 1;

    if(priqueue_size(&pqueue) == 0){
        pseudo_job->start_execution_time = time;
        priqueue_offer(&pqueue, pseudo_job);
        return true;
    }
    current_running_job = (job_t*)priqueue_peek(&pqueue);

    current_running_job->remaining_time -= (time - current_running_job->start_execution_time);


    if(comparison_func == comparer_pri){
        current_running_job->priority = -1;
        priqueue_offer(&pqueue, pseudo_job);
        return false;
    }else if(comparison_func == comparer_sjf){
        current_running_job->execution_time = -1;
        priqueue_offer(&pqueue, pseudo_job);
        return false;
    }else if(comparison_func == comparer_rr){
        priqueue_offer(&pqueue, pseudo_job);
        return false;
    }else{
        //int my_current_id = current_running_job->id;
        int status = pqueue.comparer(current_running_job, pseudo_job);
        if(status == 1){ // b first
            pseudo_job->start_execution_time = time;
            priqueue_offer(&pqueue, pseudo_job);
            return true;
        }else{
            //if(current_running_job->id != my_current_id)
            
            priqueue_offer(&pqueue, pseudo_job);
            return false;
        }
        priqueue_offer(&pqueue, pseudo_job);
        return false;
    }
    
}

int scheduler_job_finished(int job_number, unsigned time) {

    job_t* done_job = priqueue_poll(&pqueue);
    done_job->complete_time = time;
    if(comparison_func == comparer_sjf){
        average_waiting_time += (time - done_job->arrival_time - done_job->actual_exec_time);
    }else{
        average_waiting_time += (time - done_job->arrival_time - done_job->execution_time);
    }
    average_turnaround_time += (time - done_job->arrival_time);
    average_response_time += (done_job->start_execution_time - done_job->arrival_time);
    job_t* next_job = priqueue_peek(&pqueue);
    if(!next_job){
        return -1;
    }else{
        if(next_job->start_execution_time == -1){
            next_job->start_execution_time = time;
        }
        return next_job->id;
    }
}

int scheduler_quantum_expired(unsigned time) {
    current_running_job = priqueue_poll(&pqueue);
    current_running_job->remaining_time -= (time - current_running_job->start_execution_time);
    priqueue_offer(&pqueue, current_running_job);
    job_t* next_job = priqueue_peek(&pqueue);
    if(!next_job){
        return -1;
    }else{
        if(next_job->start_execution_time == -1){
            next_job->start_execution_time = time;
        }
        return next_job->id;
    }
}

float scheduler_average_waiting_time() {
    return (double)average_waiting_time/number_of_jobs;
}

float scheduler_average_turnaround_time() {
    return (double)average_turnaround_time/number_of_jobs;
}

float scheduler_average_response_time() {
    return (double)average_response_time/number_of_jobs;
}

void scheduler_clean_up() { priqueue_destroy(&pqueue); }

void scheduler_show_queue() {
    // This function is left entirely to you! Totally optional.
    // int size = pqueue.size;
    // entry* myhead = pqueue.head;
    // entry* temp = myhead;
    // int i;
    // if(current_running_job){
    //     printf("%d", current_running_job->id);
    // }
    // for(i=0; i<size; i++){
    //     job_t* temp_job = temp
    // }
}
