// hello.c
#include <stdio.h>

int ADC_EOC_CHECK() {
    // wait for end of conversion
    // hardware delay emulation
    int a; 
    for(int i=0; i<100000; i++) 
        a = 10; // do something.
    
    return 1; // end of conversion
}
int main() {
    // ADC_DATA = .... will be executed by Hardware
    int ADC_DATA=3;
    // blocking until status is matched
    while(ADC_EOC_CHECK() == 0); 

    int compensated_ADC = ADC_DATA + 7;
    printf("ADC_DATA is %d\n", compensated_ADC);

    return 0;
}

