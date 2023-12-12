#include "thread_pool.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

enum {
	TASK_CREATED = 0,
	TASK_PUSHED,
	TASK_PICKED,
	TASK_RELEASED,
	TASK_JOINED,

	TASK_DETACHED,
	TASK_DELETED,
};

struct thread_task {
	thread_task_f function;
	void *arg;
	// result of function(arg)
	void *result;
	// next task in the queue of tasks;
	struct thread_task *next;
	// previous task in the queue of tasks;
	struct thread_task *prev;
	// task state
	int state;
	// task id
	int id;
	// auto-delete
	bool auto_delete;
	// thread which picks the task
	pthread_t thread;
	// mutex for modifying the shared data
	pthread_mutex_t *mutex;
	// cond for joining the task
	pthread_cond_t *cond;
;
};

struct thread_pool {
	pthread_t *threads;
	// maximum number of threads the pool can have
	int capacity;
	// number of threads currently created by thread pool
	int threads_count;
	// number of threads which are currently working on some tasks
	int busy_threads_count;
	// head_task is the next task to give to a thread
	struct thread_task *head_task;
	// tail_task is the last task added so far
	struct thread_task *tail_task;
	// number of currenly available tasks to do. number of tasks in between head_task and tail_task
	int task_count;
	
	pthread_mutex_t *mutex;

	// a signal sent when a new task appears or a broadcast sent for shutting down all threads
	pthread_cond_t *cond;
	// if user requested to delete the pool
	bool is_shutdown;
};

struct thread_args
{
	struct thread_pool *pool;
	int thread_id;
};

struct thread_task* pick_task(struct thread_pool *pool, int thread_id)
{	
	struct thread_task *task = pool->head_task;
	pthread_mutex_lock(task->mutex);
	// printf("pick id: %d \t state: %d\n", task->id, task->state);
	pool->head_task = pool->head_task->next;
	pool->task_count --;
	pool->busy_threads_count++;
	if (pool->head_task == NULL){
		pool->tail_task = NULL;
	}

	task->thread = pool->threads[thread_id];
	task->state = TASK_PICKED;
	pthread_mutex_unlock(task->mutex);

	return task;
}

void release_task(struct thread_task *task, void* result) 
{
	pthread_mutex_lock(task->mutex);
	task->result = result;
	task->thread = 0;
	if (task->auto_delete){
		task->state = TASK_DETACHED;
		pthread_mutex_unlock(task->mutex);
		thread_task_delete(task);
	}else{
		task->state = TASK_RELEASED;
		pthread_cond_broadcast(task->cond);
		pthread_mutex_unlock(task->mutex);
	}
}

void* thread_lifecycle(void *args_ptr)
{
	struct thread_args *args = (struct thread_args *)args_ptr;

	struct thread_pool *pool = args->pool;
	int thread_id = args->thread_id;

	while(true){

		pthread_mutex_lock(pool->mutex);

		// wait until there is a task or shutdown
		while(pool->task_count == 0 && !pool->is_shutdown){
			// printf("thread %d waiting\n", thread_id);
			pthread_cond_wait(pool->cond, pool->mutex);
			// printf("thread %d woke up:)\n", thread_id);
		}

		// if there is a task, pick the task
		if (pool->task_count > 0){
			// printf(">>>>>> thread %d \t task_count: %d\n", thread_id, pool->task_count);
			struct thread_task* task = pick_task(pool, thread_id);
			
			// printf("****** thread %d \t task: %d \t picked\n", thread_id, task->id);
	
			pthread_mutex_unlock(pool->mutex);
			// printf("------ thread %d \t task: %d \t running...\n", thread_id, task->id);
			void *result = task->function(task->arg);
			// printf("++++++ thread %d \t task: %d \t finished\n", thread_id, task->id);
			
			pthread_mutex_lock(pool->mutex);
			pool->busy_threads_count --;
			pthread_mutex_unlock(pool->mutex);

			release_task(task, result);
			
			// printf("////// thread %d \t task: %d \t releasing...\n", thread_id, task->id);
			// printf(",,,,,, thread %d \t task: %d \t released\n", thread_id, task->id);	
		} 
		// if there is shutdown
		else if (pool->is_shutdown){
			// printf("@@@@ thread %d shutdown\n", thread_id);
			pthread_mutex_unlock(pool->mutex);
			break;	
		}
	}
	free(args);
	pthread_exit(NULL);	
}

int
thread_pool_new(int max_thread_count, struct thread_pool **pool)
{

	if (max_thread_count > TPOOL_MAX_THREADS)
		return TPOOL_ERR_INVALID_ARGUMENT;

	// pool should have at aleast one thread
	if (max_thread_count < 1)
		return TPOOL_ERR_INVALID_ARGUMENT;

	struct thread_pool *new_pool = malloc(sizeof(struct thread_pool));

	new_pool->threads = malloc(sizeof(pthread_t)*max_thread_count);
	new_pool->capacity = max_thread_count;
	new_pool->threads_count = 0;
	new_pool->busy_threads_count = 0;
	new_pool->task_count = 0;
	new_pool->is_shutdown = false;
	new_pool->mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	new_pool->cond = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
	pthread_mutex_init(new_pool->mutex, NULL);
	pthread_cond_init(new_pool->cond, NULL);

	*pool = new_pool;
	return 0;
}

int
thread_pool_thread_count(const struct thread_pool *pool)
{
	return pool->threads_count;
}

int
thread_pool_delete(struct thread_pool *pool)
{
	if (pool == NULL)
		return TPOOL_ERR_INVALID_ARGUMENT;;

	pthread_mutex_lock(pool->mutex);
	// if there is still some task to do or there are some threads working on some tasks
	if (pool->task_count > 0 || pool->busy_threads_count > 0){
		pthread_mutex_unlock(pool->mutex);
		return TPOOL_ERR_HAS_TASKS;
	}
	pool->is_shutdown = true;
	// broadcast to all threads that they should shutdown
	pthread_cond_broadcast(pool->cond);
	pthread_mutex_unlock(pool->mutex);

	// join threads
	for (int i = 0; i < pool->threads_count; i ++ ){
		pthread_join(pool->threads[i], NULL);
	}
	free(pool->threads);
	pthread_mutex_destroy(pool->mutex);
	pthread_cond_destroy(pool->cond);
	free(pool->mutex);
	free(pool->cond);
	free(pool);
	return 0;
}


int thread_pool_launch_thread(struct thread_pool *pool) {
	
	struct thread_args *args = malloc(sizeof(struct thread_args));
	args->thread_id = pool->threads_count;
	args->pool = pool;
	pthread_create(&(pool->threads[pool->threads_count]), NULL, thread_lifecycle, (void *)args);
	pool->threads_count ++;
	return 0;
}

int
thread_pool_push_task(struct thread_pool *pool, struct thread_task *task)
{

	if (task == NULL || pool == NULL)
		return TPOOL_ERR_INVALID_ARGUMENT;

	pthread_mutex_lock(task->mutex);
	if (task->state != TASK_CREATED && task->state != TASK_JOINED){
		pthread_mutex_unlock(task->mutex);
		return TPOOL_ERR_TASK_IN_POOL;
	}

	if (task->state == TASK_DETACHED){
		pthread_mutex_unlock(task->mutex);
		return TPOOL_ERR_TASK_DETACHED;
	}
	// printf(">>> push \t id:%d \t task->state %d \t task_count: %d\t thread_count: %d \t busy_threads_count: %d\n", task->id, task->state, pool->task_count, pool->threads_count, pool->busy_threads_count);

	pthread_mutex_lock(pool->mutex); 
	// there are too many tasks in the pool
	if (pool->task_count >= TPOOL_MAX_TASKS){
		pthread_mutex_unlock(task->mutex);
		pthread_mutex_unlock(pool->mutex); 
		return TPOOL_ERR_TOO_MANY_TASKS;
	}
	// These two lines of code took a week of my life  ¯⁠\⁠_⁠(⁠ツ⁠)⁠_⁠/⁠¯
	task->next = NULL;
	task->prev = NULL;
	//----------------
	task->state = TASK_PUSHED;
	if (pool->tail_task == NULL){
		pool->tail_task = task;
		pool->head_task = task;
	}else{
		pool->tail_task->next = task;
		task->prev = pool->tail_task;
		pool->tail_task = task;
	}
	pool->task_count ++;
	// printf("--- push task->state %d \t task_count: %d\t thread_count: %d \t busy_threads_count: %d \t capacity: %d\n", task->state, pool->task_count, pool->threads_count, pool->busy_threads_count, pool->capacity);
	pthread_mutex_unlock(task->mutex);
	// if there is no free thread and a new thread can be created, then create a thread
	if (pool->busy_threads_count == pool->threads_count && pool->threads_count < pool->capacity){
		thread_pool_launch_thread(pool);
		pthread_mutex_unlock(pool->mutex);
	} // otherwise singal threads for a new task 
	else{
		// printf("*** push task->state %d \t task_count: %d\t thread_count: %d \t busy_threads_count: %d\n", task->state, pool->task_count, pool->threads_count, pool->busy_threads_count);
		pthread_cond_signal(pool->cond);
		pthread_mutex_unlock(pool->mutex);
		
	}
	// printf("<<< push task->state %d \t task_count: %d\t thread_count: %d \t busy_threads_count: %d\n", task->state, pool->task_count, pool->threads_count, pool->busy_threads_count);
	return 0;
}

int
thread_task_new(struct thread_task **task, thread_task_f function, void *arg)
{
	static int task_counter = 0;
	task_counter ++;
	// printf("new task\n");
	struct thread_task *new_task = malloc(sizeof(struct thread_task));
	new_task->function = function;
	new_task->arg = arg;
	new_task->id = task_counter;
	new_task->state = TASK_CREATED;
	new_task->next = NULL;
	new_task->prev = NULL;
	new_task->auto_delete = false;
	new_task->mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	new_task->cond = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
	pthread_mutex_init(new_task->mutex, NULL);
	pthread_cond_init(new_task->cond, NULL);
	*task = new_task;
	return 0;
}

bool
thread_task_is_finished(const struct thread_task *task)
{
	return task->state == TASK_RELEASED;
}

bool
thread_task_is_running(const struct thread_task *task)
{
	return task->state == TASK_PICKED;
}

int
thread_task_join(struct thread_task *task, void **result)
{
	if (task == NULL)
		return TPOOL_ERR_INVALID_ARGUMENT;

	pthread_mutex_lock(task->mutex);
	// printf("join \t id: %d \t state: %d\n", task->id, task->state);
	// can't join a task which is yet to be pushed to a pool
	if (task->state < TASK_PUSHED){
		pthread_mutex_unlock(task->mutex);
		return TPOOL_ERR_TASK_NOT_PUSHED;
	}
	// wait untill the task is picked and released by a thread
	while (task->state < TASK_RELEASED)
		pthread_cond_wait(task->cond, task->mutex);
	
	*result = task->result;
	task->state = TASK_JOINED;
	pthread_mutex_unlock(task->mutex);
	return 0;
}

#ifdef NEED_TIMED_JOIN

int timespec_compare(struct timespec ts1, struct timespec ts2) {
    if (ts1.tv_sec < ts2.tv_sec) {
        return -1;
    }
    if (ts1.tv_sec > ts2.tv_sec) {
        return 1;
    }
	if (ts1.tv_nsec < ts2.tv_nsec) {
		return -1;
	}
	if (ts1.tv_nsec > ts2.tv_nsec) {
		return 1;
	}
	return 0;
}

long show(struct timespec ts){
	return ts.tv_sec * 1e9 + ts.tv_nsec;
}

void diff(struct timespec ts1, struct timespec ts2){
	printf("%ld  %ld  %ld  %d\n", show(ts1), show(ts2), (show(ts1) - show(ts2)), timespec_compare(ts1, ts2));
}

int
thread_task_timed_join(struct thread_task *task, double timeout, void **result)
{
	if (task == NULL)
		return TPOOL_ERR_INVALID_ARGUMENT;
	struct timespec start_ts;
	clock_gettime(CLOCK_MONOTONIC, &start_ts);

	pthread_mutex_lock(task->mutex);

	if (task->state < TASK_PUSHED){
		pthread_mutex_unlock(task->mutex);
		return TPOOL_ERR_TASK_NOT_PUSHED;
	}

	struct timespec timeout_ts;
	clock_gettime(CLOCK_MONOTONIC, &timeout_ts);
	time_t seconds = (time_t)timeout;
	timeout_ts.tv_sec += seconds;
	timeout_ts.tv_nsec += (long)((timeout - seconds)*1000000000);

	timeout_ts.tv_sec += timeout_ts.tv_nsec / 1000000000;
	timeout_ts.tv_nsec %= 1000000000;

	diff(start_ts, timeout_ts);
	
	while (task->state < TASK_RELEASED){
		struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
		// if timeout_ts has expired
		diff(now, timeout_ts);
		if (timespec_compare(now, timeout_ts) > -1){
			pthread_mutex_unlock(task->mutex);
			return TPOOL_ERR_TIMEOUT;
		}
		bool is_timeout_err = pthread_cond_timedwait(task->cond, task->mutex, &timeout_ts) == ETIMEDOUT;
		clock_gettime(CLOCK_MONOTONIC, &now);
		printf("----: ");diff(now, timeout_ts);
		if (is_timeout_err){
			pthread_mutex_unlock(task->mutex);
			return TPOOL_ERR_TIMEOUT;
		}
	}
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	diff(now, timeout_ts);			
	
	*result = task->result;
	task->state = TASK_JOINED;
	
	pthread_mutex_unlock(task->mutex);
	return 0;
}

#endif

int
thread_task_delete(struct thread_task *task)
{
	if (task == NULL)
		return TPOOL_ERR_INVALID_ARGUMENT;
	
	pthread_mutex_lock(task->mutex);
	// can't delete a task which is in a pool and not joined yet
	if (task->state > TASK_CREATED && task->state < TASK_JOINED){
		pthread_mutex_unlock(task->mutex);
		return TPOOL_ERR_TASK_IN_POOL;
	}
	// printf("delete id: %d \t state: %d\n",task->id, task->state);
	task->state = TASK_DELETED;
	pthread_mutex_unlock(task->mutex);

	pthread_mutex_destroy(task->mutex);
	pthread_cond_destroy(task->cond);
	free(task->mutex);
	free(task->cond);
	free(task);
	return 0;
}

#ifdef NEED_DETACH

int
thread_task_detach(struct thread_task *task)
{
	if (task == NULL)
		return TPOOL_ERR_INVALID_ARGUMENT;
	
	pthread_mutex_lock(task->mutex);
	// if the task is just created but yet to be pushed to a pool, return error
	if (task->state == TASK_CREATED){
		pthread_mutex_unlock(task->mutex);
		return TPOOL_ERR_TASK_NOT_PUSHED;
	}
	// put the task on auto delete so that it could be deleted in the future.
	task->auto_delete = true;

	// if the task is finished or already joined then immediately delete the task
	if (task->state == TASK_RELEASED || task->state == TASK_JOINED){
		task->state = TASK_DETACHED;
		pthread_mutex_unlock(task->mutex);
		thread_task_delete(task);
	}
	pthread_mutex_unlock(task->mutex);
	return 0;
}

#endif

