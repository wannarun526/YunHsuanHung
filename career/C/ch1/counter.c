/* counter.c */
int g_error_count = 0;          // 定義（definition）：真的配置記憶體，只能有一處

void report_error(void) { ++g_error_count; }

