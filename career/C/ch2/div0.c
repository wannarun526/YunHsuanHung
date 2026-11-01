#include <stdio.h>
int main(void)
{
    int a = 17, b = 0;                                       // 把 b 改成 0

    if(b != 0) {
        printf("a / b            = %d\n", a / b);                // ⚠ undefined behavior
        printf("a %% b            = %d\n", a % b);               // ⚠ undefined behavior
        printf("-17 / 5          = %d\n", -17 / 5);
        printf("-17 %% 5          = %d\n", -17 % 5);
        printf("(a>b)+(a==b)     = %d\n", (a > b) + (a == b));
        printf("!a               = %d\n", !a);
        printf("b!=0 && a/b>2    = %d\n", (b != 0 && a / b > 2)); // && 短路，除法不會執行
        printf("17.0 / b         = %f\n", 17.0 / b);              // 浮點數：inf，有定義
    }

    return 0;
}
