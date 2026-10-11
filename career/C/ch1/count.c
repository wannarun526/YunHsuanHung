#include <stdio.h>

int main(void)
{
    int c;
    long chars = 0, lines = 0, tabs = 0;

    while ((c = getchar()) != EOF) {
        ++chars;
        if (c == '\n') ++lines;     // 'x' 是字元常數，本質就是一個整數（ASCII 碼）
        if (c == '\t') ++tabs;
    }
    printf("chars=%ld lines=%ld tabs=%ld\n", chars, lines, tabs);
    return 0;
}