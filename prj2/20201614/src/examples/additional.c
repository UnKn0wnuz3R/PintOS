#include <stdio.h>
#include <syscall.h>
#include <stdlib.h>
#include <stdbool.h>
// #include "userprog/syscall.h"

int main(int argc, char *argv[]){
    int x, y, z, w, max, fibo;
    bool success;

    if(argc != 5){
        printf("Input Error!\n");
        success = EXIT_FAILURE;
    }
    else{
        x = atoi(argv[1]);
        y = atoi(argv[2]);
        z = atoi(argv[3]);
        w = atoi(argv[4]);
        fibo = fibonacci(x);
        max = max_of_four_int(x,y,z,w);
        printf("%d %d\n",fibo, max);
        success = EXIT_SUCCESS;
    }
    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}