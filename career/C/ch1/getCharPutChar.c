
#include <stdio.h>

int main(void)
{
    int c;                              // ⚠ 用 int，不是 char！

    while ((c = getchar()) != EOF)      // 讀一個字元，順便判斷是否結束
        putchar(c);
    return 0;
}