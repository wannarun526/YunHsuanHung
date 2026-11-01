#include <stdio.h>
#include <stdint.h>

#define BIT(n)  (1u << (n))          // 第 n 個 bit 的遮罩；用 1u 而不是 1

/* 假想的控制暫存器：bit0 = 啟用，bit3 = 中斷，bit[7:4] = 速度等級 */
#define CTRL_ENABLE      BIT(0)
#define CTRL_IRQ         BIT(3)
#define CTRL_SPEED_SHIFT 4
#define CTRL_SPEED_MASK  (0xFu << CTRL_SPEED_SHIFT) // 1111 0000

static void print_bin8(uint8_t v)
{
    for (int i = 7; i >= 0; i--)
        putchar((v & BIT(i)) ? '1' : '0');
    putchar('\n');
}

int main(void)
{
    uint8_t ctrl = 0;
    printf("ctrl = 0x%02X\n", ctrl);      // ctrl = 0x00
    print_bin8(ctrl);                     // 00000000


    ctrl |= CTRL_ENABLE;                  // 設定：OR 上遮罩
    printf("ctrl = 0x%02X\n", ctrl);      // ctrl = 0x01
    printf("ctrl = %d\n", ctrl);          // ctrl = 1
    print_bin8(ctrl);                     // 00000001

    ctrl |= CTRL_IRQ;
    printf("ctrl = 0x%02X\n", ctrl);      // ctrl = 0x09
    printf("ctrl = %d\n", ctrl);          // ctrl = 9
    print_bin8(ctrl);                     // 00001001

    ctrl &= (uint8_t)~CTRL_IRQ;           // 清除：AND 上「反轉後的遮罩」
    printf("ctrl = 0x%02X\n", ctrl);      // ctrl = 0x01
    printf("ctrl = %d\n", ctrl);          // ctrl = 1
    print_bin8(ctrl);                     // 00000001

    ctrl ^= CTRL_ENABLE;                  // 翻轉：XOR 遮罩
    printf("ctrl = 0x%02X\n", ctrl);      // ctrl = 0x00
    printf("ctrl = %d\n", ctrl);          // ctrl = 0
    print_bin8(ctrl);                     // 00000000

    if ((ctrl & CTRL_ENABLE) == 0)        // 測試：AND 後看是不是 0（括號不能少）
        printf("disabled\n");

    /* 寫入一個多 bit 欄位：先清掉舊值，再放入新值 */
    unsigned speed = 5;
    ctrl = (ctrl & ~CTRL_SPEED_MASK) | ((speed << CTRL_SPEED_SHIFT) & CTRL_SPEED_MASK);
    print_bin8(ctrl);                     // 01010000

    /* 讀出多 bit 欄位：先遮罩，再右移 */
    printf("speed = %u\n", (ctrl & CTRL_SPEED_MASK) >> CTRL_SPEED_SHIFT);  // 5
    return 0;
}