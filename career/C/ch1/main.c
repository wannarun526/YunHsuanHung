/* main.c */
#include <stdio.h>
extern int g_error_count;       // 宣告（declaration）：「這個變數在別的檔案裡」
void report_error(void);

int main(void)
{
    printf("%d\n", EOF);  // 0
    report_error();
    report_error();
    printf("%d\n", g_error_count);  // 2
    return 0;
}