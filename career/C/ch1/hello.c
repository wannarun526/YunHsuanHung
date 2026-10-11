#include <stdio.h>      // 引入標準輸入輸出的宣告（printf 在這裡）

int main(void)         // 程式從 main 開始執行，回傳 int 給作業系統
{
    printf("hi, firmware\n");   // \n 是換行，printf 不會自動換行
    return 0;          // 0 = 成功；非 0 = 失敗（shell 的 $? 看得到）
}