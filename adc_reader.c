#include <stdio.h>
#include "pico/stdlib.h"

int main() {
    stdio_init_all();

    printf("Hello, worlds!\n");
    while(1){
	printf("...\n");
	sleep_ms(2500);
    }
    //unreachable
    return 0;
}
