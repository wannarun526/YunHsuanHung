#include <stdio.h>

int main(void)
{
    int c;
    int hex[16];                        // 固定大小，宣告後不能變長

    // for (int i = 0; i < 16; ++i)
    //     hex[i] = 0;                     // ⚠ 區域陣列不會自動清 0，一定要自己初始化

    for (size_t i = 0; i < sizeof hex / sizeof hex[0]; ++i) // 用 sizeof hex / sizeof hex[0] 取代硬寫的 16
        hex[i] = 0;

    while ((c = getchar()) != EOF) {
        if (c >= '0' && c <= '9')
            ++hex[c - '0'];             // 用字元算出陣列索引
        else if (c >= 'a' && c <= 'g')
            ++hex[c - 'a' + 10];
    }
    for (int i = 0; i < 16; ++i)
        printf("%X: %d\n", i, hex[i]);  // %X 以大寫十六進位印出
    return 0;
}