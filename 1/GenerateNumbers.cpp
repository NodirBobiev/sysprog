//
// Created by micheal on 10.10.18.
//

#include <random>
#include <fstream>
#include <iostream>
using namespace std;


int main () {
    int num_of_numbers = 16384;
    int* buffer = new int[num_of_numbers];
    for (int num_of_file = 0; num_of_file<10; num_of_file++){
        ofstream outfile("file"+to_string(num_of_file)+".dat");
        for (int i=0; i<num_of_numbers; i++){
            buffer[i]= rand();
        }
        for (int i=0; i<16384; i++ ){
            outfile<<buffer[i]<<" ";

        }
        outfile.close();
    }
    delete[] buffer;
    return 0;
}


