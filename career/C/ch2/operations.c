#include <stdio.h>

int main(void)
{
    int a = 17, b = 5;
    printf("%d / %d = %d\n", a, b, a / b);   // 3, 整數除法，小數捨去
    printf("%d %% %d = %d\n", a, b, a % b);  // 2, 餘數（只能用在整數）
    printf("%d / %d = %d\n", -17, 5, -17 / 5);   // -3, C99 起規定往 0 的方向捨去
    printf("%d %% %d = %d\n", -17, 5, -17 % 5);  // -2, 餘數的正負號跟被除數一樣

    printf("%d\n", (a > b) + (a == b));    // 1, 比較結果就是 int：成立是 1，不成立是 0
    printf("%d\n", !a);                    // 0, a 不是 0，所以 !a 是 0
    printf("%d\n", (b != 0 && a / b > 2)); // && 短路：b 是 0 就不會做除法
    return 0;
}