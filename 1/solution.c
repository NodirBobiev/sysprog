#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "libcoro.h"
#include "mine.h"


/**
 * You can compile and run this code using the commands:
 *
 * $> gcc solution.c libcoro.c
 * $> ./a.out
 */

struct my_context {
	char *name;
	struct file* f;
	long worktime;
	struct timespec start_time; 
	long quota;
	/** ADD HERE YOUR OWN MEMBERS, SUCH AS FILE NAME, WORK TIME, ... */
};

static struct my_context *
my_context_new(const char *name, struct file* f, long quota)
{
	struct my_context *ctx = malloc(sizeof(*ctx));
	ctx->f = f;
	ctx->name = strdup(name);
	ctx->quota = quota;
	ctx->worktime = 0;
	return ctx;
}

static void
my_context_delete(struct my_context *ctx)
{
	free(ctx->name);
	free(ctx);
}

/**
 * A function, called from inside of coroutines recursively. Just to demonstrate
 * the example. You can split your code into multiple functions, that usually
 * helps to keep the individual code blocks simple.
 */
static void
other_function(const char *name, int depth)
{
	printf("%s: entered function, depth = %d\n", name, depth);
	coro_yield();
	if (depth < 3)
		other_function(name, depth + 1);
}


long get_miliseconds(struct timespec* begin, struct timespec* end) {
	long seconds = end->tv_sec - begin->tv_sec;
	long nanoseconds = end->tv_nsec - begin->tv_nsec;
	if(nanoseconds < 0){
		seconds -= 1;
        nanoseconds += 1000000000L;
	}
	return seconds * 1000 + nanoseconds / 1000000;
}

long get_microseconds(struct timespec* begin, struct timespec* end) {
	long seconds = end->tv_sec - begin->tv_sec;
	long nanoseconds = end->tv_nsec - begin->tv_nsec;
	if(nanoseconds < 0){
		seconds -= 1;
        nanoseconds += 1000000000L;
	}
	return seconds * 1000000 + nanoseconds / 1000;
}

long get_nanoseconds(struct timespec* begin, struct timespec* end) {
	long seconds = end->tv_sec - begin->tv_sec;
	long nanoseconds = end->tv_nsec - begin->tv_nsec;
	if(nanoseconds < 0){
		seconds -= 1;
        nanoseconds += 1000000000L;
	}
	return seconds * 1000000000L + nanoseconds;
}

void yield(void* context) {
	struct my_context *ctx = context;
	struct timespec cur_time;
	clock_gettime(CLOCK_MONOTONIC, &cur_time);
	long time_passed = get_microseconds(&ctx->start_time, &cur_time);
	// printf("%s: time_passed: %ld, quota: %ld\n", ctx->name, time_passed, ctx->quota);
	if(time_passed >= ctx->quota){
		ctx->worktime += time_passed;
		coro_yield();
		ctx->start_time = cur_time;
	}
}

/**
 * Coroutine body. This code is executed by all the coroutines. Here you
 * implement your solution, sort each individual file.
 */
static int
coroutine_func_f(void *context)
{
	/* IMPLEMENT SORTING OF INDIVIDUAL FILES HERE. */

	struct coro *this = coro_this();
	struct my_context *ctx = context;
	clock_gettime(CLOCK_MONOTONIC, &ctx->start_time);
	char *name = ctx->name;
	printf("Started coroutine %s\n", name);
	struct file* f = ctx->f;
	while(f != NULL){
		f = get_unsorted_file(f);
		if(f != NULL){
			sort_file(f, yield, context);
		} else{
			break;
		}
		yield(context);
	}
	printf("%s: switch count %lld\n", name, coro_switch_count(this));
	printf("%s: worktime: %ldµs\n", name, ctx->worktime);

	my_context_delete(ctx);
	/* This will be returned from coro_status(). */
	return 0;
}

int
main(int argc, char **argv)
{
	struct timespec start_time, end_time;
	clock_gettime(CLOCK_MONOTONIC, &start_time);

	struct file* head = NULL;
	
    for(int i = 1; i < argc; i++ ){
        struct file* f = create_file(argv[i]);
        add_file(&head, f);
    }

	/* Initialize o	ur coroutine global cooperative scheduler. */
	coro_sched_init();

	// latency in µs (microseconds). 1000000µs = 1s. 
	long T = 30000; 
	printf("Latency: T=%ldµs\n", T);

	// number of coroutines
	int N = 6; 
	printf("Coroutines: N=%d\n", N);

	printf("Quota: T/N=%ldµs\n", (T/N));

	printf("\n");
	/* Start several coroutines. */
	for (int i = 0; i < N; ++i) {
		/*
		 * The coroutines can take any 'void *' interpretation of which
		 * depends on what you want. Here as an example I give them
		 * some names.
		 */
		char name[16];
		sprintf(name, "coro_%d", i);
		/*
		 * I have to copy the name. Otherwise all the coroutines would
		 * have the same name when they finally start.
		 */
		coro_new(coroutine_func_f, my_context_new(name, head, T / N));
	}
	/* Wait for all the coroutines to end. */
	struct coro *c;
	while ((c = coro_sched_wait()) != NULL) {
		/*
		 * Each 'wait' returns a finished coroutine with which you can
		 * do anything you want. Like check its exit status, for
		 * example. Don't forget to free the coroutine afterwards.
		 */
		printf("Finished %d\n", coro_status(c));
		coro_delete(c);
	}
	/* All coroutines have finished. */
	

	/* IMPLEMENT MERGING OF THE SORTED ARRAYS HERE. */
	struct vector* sorted_elements = merge_file(head);
	
	// delete all files after merging their sorted elements;
	free_file_rec(head);

	
	// Writing sorted arrays into result.txt.

	// char* result_file = "result.txt";
	// write_vector(sorted_elements, result_file);
	
	// delete sorted elements after writing to the fiel;
	free_vector(sorted_elements);
	
	clock_gettime(CLOCK_MONOTONIC, &end_time);

	// printf("start_time: %lds, %ldns\n", start_time.tv_sec, start_time.tv_nsec);
	// printf("end_time: %lds, %ldns\n", end_time.tv_sec, end_time.tv_nsec);
	printf("\nTime elapsed: %ldµs\n", get_microseconds(&start_time, &end_time));
	return 0;

}
