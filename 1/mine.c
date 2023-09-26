#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mine.h"

void merge(int* dst, int* src1, int size1, int* src2, int size2) {
    for( int i = 0, j = 0; i < size1 || j < size2; ){
        if(i < size1 && j < size2){
            if(src1[i]<src2[j])
                *dst = src1[i++];
            else
                *dst = src2[j++];
        }else if(i < size1){
            *dst = src1[i++];
        }else{
            *dst = src2[j++];
        }
        dst++;
    }
}

int* copy(int* src, int size){
    int *dst = (int*)malloc(size * sizeof(int));
    for(int i = 0; i < size; i ++){
        dst[i] = src[i];
    }
    return dst;
}

void sort(int*src, int size, yield_f func, void* arg){
    if(size <= 1) return;
    
    int lsize = size / 2;
    int rsize = size - lsize;

    int* left = copy(src, lsize);
    int* right = copy(src + lsize, rsize);

    

    sort(left, lsize, func, arg);

    func(arg);

    sort(right, rsize, func, arg);
    
    merge(src, left, lsize, right, rsize);
    
    func(arg);

    free(left);
    free(right);    
}

struct vector{
    int capacity;
    int size;
    int *elements;
};

void describe_vector(struct vector* v){
    printf("capacity: %d,   size: %d\n", v->capacity, v->size);  
}

struct vector* create_vector(int capacity){
    struct vector* vec = (struct vector*)malloc(sizeof(struct vector));

    vec->capacity = capacity;
    vec->size = 0;
    vec->elements = (int*)malloc(capacity * sizeof(int));
    return vec;
}

void free_vector(struct vector* v){
    // printf("free vector: ");
    // describe_vector(v);
    free(v->elements);
    free(v);
}

void push_back(struct vector*v, int e){
    if(v->size == v->capacity){
        v->capacity *= 2;
        int* old = v->elements;
        v->elements = (int*)malloc(v->capacity*sizeof(int));
        for(int i = 0; i < v->size; i ++){
            v->elements[i] = old[i];
        }
        free(old);
    }

    v->elements[v->size] = e;
    v->size ++;
}

int at(struct vector* v, int i){
   return v->elements[i]; 
}

struct vector* read_vector(char* filename){
    FILE* file = fopen(filename, "r");
    if(file == NULL){
        printf("ERROR: couldn't open %s to read array\n", filename);
        exit(1);
    }
    struct vector* v = create_vector(100);
    int x;
    while(fscanf(file, "%d", &x)==1){
        push_back(v, x);
    }
    fclose(file);
    return v;
}

void write_vector(struct vector* v, char* filename){
    FILE* file = fopen(filename, "w");
    if(file == NULL){
        printf("ERROR: couldn't open %s to write array\n", filename);
        exit(1);
    }
    for(int i = 0; i < v->size; i ++ ){
        fprintf(file, "%d ", at(v, i));
    }
    fclose(file);
}

void print_vector(struct vector* v){
    printf("capacity: %d\n", v->capacity);
    printf("size: %d\n", v->size);
    for(int i = 0; i < v->size; i++){
        printf("%d ", at(v,i));
    }
    printf("\n");
}

void sort_vector(struct vector* v, yield_f func, void* arg){
    sort(v->elements, v->size, func, arg);
}

struct vector* merge_vector(struct vector* v1, struct vector* v2){
    int total_size = v1->size + v2->size;
    struct vector* res_vector = create_vector(total_size);
    res_vector->size = total_size;
    merge(res_vector->elements, v1->elements, v1->size, v2->elements, v2->size);

    return res_vector;
}

struct file{
    char* name;
    struct vector* v;
    int is_sorted;
    struct file *prev, *next; 
};

void describe_file(struct file* f){
    printf("file_name: %s,  size: %d,  is_sorted: %d  \n", f->name, f->v->size, f->is_sorted);
}

struct file* create_file(char* name){
    struct file* f = (struct file*)malloc(sizeof(struct file));
    f->name = strdup(name);
    f->is_sorted = 0;
    f->v = read_vector(name);
    return f;
}

void free_file(struct file* f){
    // printf("free file: ");
    // describe_file(f);
    free_vector(f->v);
    free(f->name);
    free(f);
}

void free_file_rec(struct file* f){
    if(f == NULL)
        return;
    free_file_rec(f->next);
    free_file(f);
}

void sort_file(struct file* f, yield_f func, void* arg){
    f->is_sorted = 1;
    sort_vector(f->v, func, arg);
}

struct vector* merge_file(struct file* f){
    struct vector* result = create_vector(1);
	
	while(f != NULL){
        // describe_file(f);
		struct vector* temp = result;

		result = merge_vector(temp, f->v);
		
		free_vector(temp);

		f = f->next;
	}
    return result;
}

void add_file(struct file** head, struct file* f){
    f->next = *head;
    f->prev = NULL;
    if(*head != NULL){
        (*head)->prev = f;
    }
    *head = f;
}

void traverse(struct file* f){
    if(f == NULL)
        return;
    printf("%s ", f->name);
    traverse(f->next);
}

struct file* get_unsorted_file(struct file* f){
    if(f == NULL)
        return f;
    if(!f->is_sorted)
        return f;
    return get_unsorted_file(f->next);
}



// int main(int argc, char **argv) {
//     struct file* head = NULL;
    
//     for(int i = 1; i < argc; i++ ){
//         struct file* f = create_file(argv[i]);
//         add_file(&head, f);
//     }

//     traverse(head);

//     // char* filename = "test2.txt";
//     // struct vector* v = read_vector(filename);
//     // struct vector* v1 = read_vector(filename);
//     // // print(v1);
//     // // printf("------ -----");
//     // // print(v);
//     // sort_vector(v);
//     // // print(v);
    
//     // for(int i = 0; i < v->size; i ++ ){
//     //     int cnt = 0;
//     //     for(int j = 0; j < v1->size; j ++){
//     //         if(at(v,i) == at(v, j))
//     //             cnt++;
//     //     }
//     //     // printf("v[i]: %d, cnt: %d\n", at(v,i), cnt);
//     //     for(int j = 0; j < v->size; j ++ ){
//     //         if(at(v,i) == at(v,j))
//     //             cnt--;
//     //     }
//     //     if(cnt){
//     //         printf("Don't match: v[i]: %d, cnt: %d\n", at(v,i), cnt);
//     //         exit(1);
//     //     }
//     // }

//     // free_vector(v);
//     // free_vector(v1);
//     // return 0;
// }
