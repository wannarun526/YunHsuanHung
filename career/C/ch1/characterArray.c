#include <stdio.h>

#define MAX_LINE 256

int read_line(char buf[], int max);     // 回傳讀到的長度，0 代表 EOF

int main(void)
{
    char line[MAX_LINE];
    int len;

    while ((len = read_line(line, MAX_LINE)) > 0)
        printf("[%3d] %s", len, line);  // %s 會一直印，直到遇到 '\0'
    return 0;
}

int read_line(char buf[], int max)
{
    int c, i = 0;

    while (i < max - 1 && (c = getchar()) != EOF && c != '\n')
        buf[i++] = c;                   // 保留最後一格給 '\0'
    if (c == '\n')
        buf[i++] = c;
    buf[i] = '\0';                      // ⚠ 沒有這行，%s 會一路印到記憶體深處
    return i;
}