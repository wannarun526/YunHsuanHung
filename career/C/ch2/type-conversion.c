#include <stdio.h>

int main(void)
{
    int neg = -1;
    unsigned int one = 1;
    if (neg < one)                       // ⚠ -1 被轉成 unsigned，變成 4294967295
        printf("expected\n");
    else
        printf("surprise: -1 >= 1u\n");  // 實際會印這行（-Wextra 會警告）

    int big = 300;
    unsigned char small = big;           // 截斷：300 = 0x12C，只留下 0x2C
    printf("%d\n", small);               // 44

    int total = 7, n = 2;
    double avg1 = total / n;             // 3.0：先做整數除法，才轉 double
    double avg2 = (double)total / n;     // 3.5：強制轉型（cast）
    printf("%.1f %.1f\n", avg1, avg2);

    char digit = '7';
    int value = digit - '0';             // 字元轉數字：'7' - '0' = 7
    printf("%d\n", value);
    return 0;
}