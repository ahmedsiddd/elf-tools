#include <stdio.h>
int sq(int x) { return x * x; }
int main() { int s = 0; for (int i = 1; i <= 3; i++) s += sq(i); printf("sum=%d\n", s); return 0; }
