#pragma once

typedef void (*yield_f)(void *);

void merge(int* dst, int* src1, int size1, int* src2, int size2);

int* copy(int* src, int size);

void sort(int*src, int size, yield_f func, void* arg);

struct vector;

void describe_vector(struct vector* v);

struct vector* create_vector(int capacity);

void free_vector(struct vector *v);

void push_back(struct vector*v, int e);

int at(struct vector*v, int i);

struct vector* read_vector(char* filename);

void write_vector(struct vector* v, char* filename);

void print_vector(struct vector* v);

void sort_vector(struct vector* v, yield_f func, void* arg);


struct vector* merge_vector(struct vector* v1, struct vector* v2);

struct file;

struct file* create_file(char* name);

void free_file(struct file* f);

void free_file_rec(struct file* f);

void sort_file(struct file* f, yield_f func, void* arg);

struct vector* merge_file(struct file* f);

void add_file(struct file** head, struct file* f);

void traverse(struct file* f);

struct file* get_unsorted_file(struct file* f);

void describe_file(struct file* f);

