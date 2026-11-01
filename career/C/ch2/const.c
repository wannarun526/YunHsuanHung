#include <stdio.h>

enum power_state { S0, S3 = 3, S4, S5 };     // S0=0, S3=3, S4=4, S5=5

int main(void)
{
    int count = 0, limit = 100;        // 宣告時就初始化
    const double ratio = 0.62;         // const：之後不能再修改
    const char banner[] = "BOOT";      // 陣列大小由初始值決定（5，含 '\0'）
    const char tempString[] = "A";

    int length(tempString);        // 承諾這個函式不會改動 s 的內容




    unsigned int a = 255;          // 十進位
    unsigned int b = 0xFF;         // 十六進位：0x 開頭
    unsigned int c = 0377;         // ⚠ 八進位：0 開頭！不是 377
    printf("%u %u %u\n", a, b, c);             // 255 255 255
    printf("is a == b? %d\n", a == b);          // 1 (true)
    printf("is b == c? %d\n", b == c);          // 1 (true)
    printf("is a == c? %d\n", a == c);          // 1 (true)

    unsigned long big = 4000000000UL;          // U = unsigned，L = long
    printf("%lu\n", big);

    char nl = '\n', esc = '\x1B', nul = '\0';  // 字元常數：跳脫寫法、十六進位寫法
    printf("%d %d %d\n", nl, esc, nul);        // 10 27 0

    printf("%s\n", "split " "string");         // 相鄰字串常數會在編譯時自動接起來
    printf("sleep state = %d\n", S4);
    return 0;
}