#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <setjmp.h>
#include <stdio.h>
#include <stdbool.h>
#include<stdio.h>
#include <iostream>
#include <chrono>

using namespace std;


struct task {
    int id;
    jmp_buf env;
    vector<int> v;
    FILE *fp;
    int i;
    int j;
    long int all_time;
    int temp;
    bool is_finished;
    char local_data[128];

};

int task_count;
static struct task *tasks;
static int current_task_i = 0;
static std::chrono::high_resolution_clock::time_point start;
static std::chrono::high_resolution_clock::time_point finish;
int T;

/**
 * Check not in a function, because setjmp result can not be used
 * after return.
 *
 * Another way how to do not put 'check_resched' after each line -
 * do "define ; check_resched;".
 */
#define check_resched {                        \
    finish = std::chrono::high_resolution_clock::now();  \
    int temp = chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count();\
    if (temp>=T){  \
    tasks[current_task_i].all_time += temp; \
    start = finish;   \
    int old_i = current_task_i;                \
    current_task_i = (current_task_i + 1) % task_count;    \
    if (setjmp(tasks[old_i].env) == 0)            \
        longjmp(tasks[current_task_i].env, 1); }} \


static void
my_coroutine() {


    tasks[current_task_i].fp = fopen(tasks[current_task_i].local_data, "r");

    check_resched;


    while (fscanf(tasks[current_task_i].fp, "%d", &tasks[current_task_i].temp) != EOF) {
        check_resched;
        tasks[current_task_i].v.push_back(tasks[current_task_i].temp);
        check_resched;

    }
    check_resched;

    //Bubble sort:)
    for (tasks[current_task_i].i = 0;
         tasks[current_task_i].i < tasks[current_task_i].v.size() - 1; tasks[current_task_i].i++) {
        // Last i elements are already in place
        //check_resched;
        for (tasks[current_task_i].j = 0; tasks[current_task_i].j < tasks[current_task_i].v.size() - 1 -
                                                                    tasks[current_task_i].i; tasks[current_task_i].j++) {
            check_resched;
            if (tasks[current_task_i].v[tasks[current_task_i].j] >
                tasks[current_task_i].v[tasks[current_task_i].j + 1]) {
                check_resched;
                tasks[current_task_i].temp = tasks[current_task_i].v[tasks[current_task_i].j];
                check_resched;
                tasks[current_task_i].v[tasks[current_task_i].j] = tasks[current_task_i].v[tasks[current_task_i].j + 1];
                check_resched;
                tasks[current_task_i].v[tasks[current_task_i].j + 1] = tasks[current_task_i].temp;
                check_resched;
            }
        }
    }

    check_resched;

    fclose(tasks[current_task_i].fp);

    check_resched;
    char str[12];
    sprintf(str, "%d", current_task_i);

    tasks[current_task_i].fp = fopen(str, "w"); // write mode;

    check_resched;

    for (tasks[current_task_i].i = 0;
         tasks[current_task_i].i < tasks[current_task_i].v.size(); tasks[current_task_i].i++) {
        check_resched;
        fprintf(tasks[current_task_i].fp, "%d ", tasks[current_task_i].v[tasks[current_task_i].i]);
        check_resched;
    }

    fclose(tasks[current_task_i].fp);

    check_resched;
    cout << "Task " << current_task_i << " finished" << endl;
    cout << "Total time is   " << tasks[current_task_i].all_time << " ns" << endl;
    tasks[current_task_i].is_finished = true;

    while (true) {
        bool is_all_finished = true;
        for (int i = 0; i < task_count; ++i) {
            if (!tasks[i].is_finished) {
                is_all_finished = false;
                break;
            }
        }
        if (is_all_finished) {
            return;
        }
        check_resched;
    }
}

struct MinHeapNode {
    int element;
    int i;
};

void swap_heap(MinHeapNode *x, MinHeapNode *y) {
    MinHeapNode temp = *x;
    *x = *y;
    *y = temp;
}

class MinHeap {
    MinHeapNode *harr;
    int heap_size;

public:
    MinHeap(MinHeapNode a[], int size);

    void MinHeapify(int);


    int left(int i) { return (2 * i + 1); }

    int right(int i) { return (2 * i + 2); }


    MinHeapNode getMin() { return harr[0]; }

    void replaceMin(MinHeapNode x) {
        harr[0] = x;
        MinHeapify(0);
    }
};

MinHeap::MinHeap(MinHeapNode a[], int size) {
    heap_size = size;
    harr = a;
    int i = (heap_size - 1) / 2;
    while (i >= 0) {
        MinHeapify(i);
        i--;
    }
}

//Recursive function to calculate mininum element of heap
void MinHeap::MinHeapify(int i) {
    int l = left(i);
    int r = right(i);
    int smallest = i;
    if (l < heap_size && harr[l].element < harr[i].element)
        smallest = l;
    if (r < heap_size && harr[r].element < harr[smallest].element)
        smallest = r;
    if (smallest != i) {
        swap_heap(&harr[i], &harr[smallest]);
        MinHeapify(smallest);
    }
}

void mergeFiles(const string &output_file, int k) {
    int n = 0;
    FILE *f = fopen("0", "r");
    int temp = 0;
    while (fscanf(f, "%d", &temp) != EOF) {
        n++;
    }
    fclose(f);
    FILE *in[k];
    for (int i = 0; i < k; i++) {
        char fileName[2];
        snprintf(fileName, sizeof(fileName), "%d", i);
        in[i] = fopen(fileName, "r");
    }

    FILE *out = fopen(output_file.c_str(), "w");

    MinHeapNode *harr = new MinHeapNode[k];
    int i;
    for (i = 0; i < k; i++) {
        if (fscanf(in[i], "%d ", &harr[i].element) != 1)
            break;

        harr[i].i = i;
    }
    MinHeap hp(harr, i);

    int count = 0;
    while (count != i) {
        MinHeapNode root = hp.getMin();
        fprintf(out, "%d ", root.element);

        if (fscanf(in[root.i], "%d ", &root.element) != 1) {
            root.element = INT8_MAX;
            count++;
        }

        hp.replaceMin(root);
    }

    // close input and output files
    for (int i = 0; i < k; i++)
        fclose(in[i]);

    fclose(out);
}

int main(int argc, char *argv[]) {
    T = atoi(argv[1]);
    task_count = argc - 2;
    tasks = new task[task_count];
    int i;
    for (i = 0; i < task_count; ++i) {
        tasks[i].id = i;
        tasks[i].is_finished = false;
        sprintf(tasks[i].local_data, "%s", argv[i + 2]);
        /* Entry point for new co-coutines. */
        setjmp(tasks[i].env);
    }
    start = std::chrono::high_resolution_clock::now();
    my_coroutine();

    mergeFiles("result.dat", task_count);
    printf("Finished\n");

    return 0;
}