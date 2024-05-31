#include <MKL25Z4.h>

void delayMs(int n);
void delayUs(int n);
void delayPar(int n, int factor);
void delayMsScalable(int n, int factor);

int ms = 7000;

void delayUs(int n){
int i;
int j;
for(i = 0 ; i < n; i++)
for (j = 0; j < 700; j++) {}
}

void delayMs(int n){
int i;
int j;
for(i = 0 ; i < n; i++)
for (j = 0; j < 7000; j++) {}
}

void delayPar(int n, int factor) {
    int i;
    int j;
    for (i = 0; i < n; i++)
        for (j = 0; j < factor; j++) {}
}

void delayMsScalable(int n, int factor) {
    int i;
    int j;
    for (i = 0; i < n; i++)
        for (j = 0; j < (ms*factor); j++) {}
}
