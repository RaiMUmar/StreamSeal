#include "../include/header.h"

int main(){

    if (sodium_init() < 0){
        printf("Program Failed!");
        return 0;
    }

    

    return 1;
}