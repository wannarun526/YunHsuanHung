#include <stdio.h>

int main(void)
{
    int km;                    // 變數必須先宣告型別才能用
    float miles;

    km = 0;
    while (km <= 50) {
        miles = km * 0.621371f;
        printf("%3d km = %6.2f mi\n", km, miles);
        km = km + 10;
    }
    return 0;
}