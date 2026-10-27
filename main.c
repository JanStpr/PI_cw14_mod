#include <stdio.h>
#include <time.h> //do losowania liczb na podstawie czasu
#include <stdlib.h> //do rand() i srand
#include "functions.h"

int main(void){
    srand(time(NULL));
    bst_showcase();
    return 0;
}