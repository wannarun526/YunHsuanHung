#include <stdio.h>

int clamp(int x, int lo, int hi);       // 函式原型（prototype）：先宣告，定義寫在後面
void try_swap(int a, int b);

int main(void)
{
    printf("%d\n", clamp(150, 0, 100)); // 100

    int x = 1, y = 2;
    try_swap(x, y);
    printf("x=%d y=%d\n", x, y);        // 還是 x=1 y=2！
    return 0;
}

int clamp(int x, int lo, int hi)
{
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

void try_swap(int a, int b)             // a、b 只是複本，改了不影響呼叫端
{
    int tmp = a;
    a = b;
    b = tmp;
}