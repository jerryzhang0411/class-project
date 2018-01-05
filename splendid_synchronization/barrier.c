/**
 * Splendid Synchronization Lab 
 * CS 241 - Spring 2017
 */
#include "barrier.h"

// The returns are just for errors if you want to check for them.
int barrier_destroy(barrier_t *barrier) {
  pthread_mutex_destroy(&(barrier->mtx));
  pthread_cond_destroy(&(barrier->cv));
  return 0;
}

int barrier_init(barrier_t *barrier, unsigned int num_threads) {
    barrier->n_threads = num_threads;
    barrier->count = 0;
    barrier->times_used = 0;
    pthread_mutex_init(&(barrier->mtx), NULL);
    pthread_cond_init(&(barrier->cv), NULL);
    return 0;
}

int barrier_wait(barrier_t *barrier) {
	unsigned int tempHold = barrier->n_threads;
	//unsigned int tmep = barrier->time_used;
	pthread_mutex_lock(&(barrier->mtx));
	barrier->count++; 
	barrier->times_used++;
	if(barrier->count < tempHold) {
	  pthread_cond_wait(&(barrier->cv),&(barrier->mtx)); 
	}

	if (barrier->count >= tempHold) {
		barrier->count = 0;
		pthread_cond_broadcast(&(barrier->cv));
	}
		/*
	if(tmep == barrier->times_used){
		barrier->time_used++;
	}
	*/
	pthread_mutex_unlock(&(barrier->mtx));
	return 0;
}