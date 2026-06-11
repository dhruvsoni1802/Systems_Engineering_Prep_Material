#include <stdio.h>
#include "math_utils.h"

int main(void) {
    printf("3 + 4 = %d\n",  add(3, 4));
    printf("3 * 4 = %d\n",  multiply(3, 4));
    printf("5^2  = %d\n",   square(5));
    return 0;
}