#!/usr/bin/env bash
#
# serve.sh — 給定 .c 檔，自動編譯並執行
#
# 用法：
#   ./serve.sh <檔案.c> [更多檔案.c ...] [選項] [-- 程式參數 ...]
#
# 範例：
#   ./serve.sh ch1/hello.c
#   ./serve.sh ch1/main.c ch1/counter.c          # 多個檔案一起編譯
#   ./serve.sh -o app ch1/main.c ch1/counter.c   # 自訂執行檔名稱
#   echo "hello" | ./serve.sh ch1/count.c         # 用管線餵輸入
#   ./serve.sh ch1/count.c < ch1/count.c          # 用檔案當輸入
#   ./serve.sh --asan ch1/array.c                 # 開啟 AddressSanitizer
#   ./serve.sh --no-ubsan ch2/div0.c              # 關掉預設的 UndefinedBehaviorSanitizer
#   ./serve.sh ch1/hello.c -- foo bar             # -- 之後的參數交給程式
#
# 選項：
#   -a, --asan     加上 -fsanitize=address，抓陣列越界等記憶體錯誤
#   -U, --no-ubsan 關掉預設開啟的 UndefinedBehaviorSanitizer（見下方說明）
#   -W, --werror   把警告當成錯誤
#   -b, --build    只編譯，不執行
#   -o, --out 名稱 執行檔名稱（預設取第一個 .c 檔的檔名）
#   -h, --help     顯示說明
#
# 預設開啟 UndefinedBehaviorSanitizer（-fsanitize=undefined），執行時會抓出
# 除以 0、有號數溢位、位移超過寬度等 undefined behavior。等同於手動執行：
#   gcc -std=c11 -g -fsanitize=undefined div0.c -o div0 && ./div0
#   # div0.c:5:41: runtime error: division by zero
# 想看「沒有 sanitizer 時程式實際會怎樣」（例如比較 -O2 的結果），用 --no-ubsan。
#
# 執行檔會放在這個腳本旁邊的 output/，檔名預設取自第一個 .c 檔。
# 可以用環境變數 CC 換編譯器，例如：CC=clang ./serve.sh ch1/hello.c
#
# 註：macOS 內建的是 bash 3.2，這個腳本刻意寫成 3.2 也能跑。

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUT_DIR="$SCRIPT_DIR/output"
CC="${CC:-gcc}"
CFLAGS=(-std=c11 -Wall -Wextra -g)

# 狀態訊息一律輸出到 stderr，讓程式本身的 stdout 保持乾淨（可以再接管線）
if [ -t 2 ]; then
    C_INFO=$'\033[36m'; C_OK=$'\033[32m'; C_ERR=$'\033[31m'; C_DIM=$'\033[2m'; C_OFF=$'\033[0m'
else
    C_INFO=''; C_OK=''; C_ERR=''; C_DIM=''; C_OFF=''
fi
info() { printf '%s▶ %s%s\n' "$C_INFO" "$*" "$C_OFF" >&2; }
err()  { printf '%s✗ %s%s\n' "$C_ERR" "$*" "$C_OFF" >&2; }

usage() {
    sed -n '3,35p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'
}

# ---- 解析參數 ----
sources=()
run_args=()
build_only=0
ubsan=1
out_name=""

while [ $# -gt 0 ]; do
    case "$1" in
        -a|--asan)   CFLAGS+=(-fsanitize=address -fno-omit-frame-pointer) ;;
        -u|--ubsan)  ubsan=1 ;;   # 預設已開啟，保留這個選項只是為了相容
        -U|--no-ubsan) ubsan=0 ;;
        -W|--werror) CFLAGS+=(-Werror) ;;
        -b|--build)  build_only=1 ;;
        -o|--out)
            if [ $# -lt 2 ]; then err "-o 後面要接執行檔名稱"; exit 2; fi
            out_name="$2"; shift ;;
        -h|--help)   usage; exit 0 ;;
        --)          shift; run_args=("$@"); break ;;
        -*)          err "不認得的選項：$1（用 -h 看說明）"; exit 2 ;;
        *.c)
            if [ ! -f "$1" ]; then
                err "找不到檔案：$1"; exit 2
            fi
            sources+=("$1") ;;
        *)           err "只接受 .c 檔：$1"; exit 2 ;;
    esac
    shift
done

if [ ${#sources[@]} -eq 0 ]; then
    err "請至少給一個 .c 檔"
    echo >&2
    usage >&2
    exit 2
fi

if [ "$ubsan" -eq 1 ]; then
    CFLAGS+=(-fsanitize=undefined)
fi

name="${out_name:-$(basename "${sources[0]}" .c)}"
bin="$OUT_DIR/$name"
mkdir -p "$OUT_DIR"

# ---- 編譯 ----
info "build: $CC ${CFLAGS[*]} ${sources[*]} -o output/$name"
if ! "$CC" "${CFLAGS[@]}" "${sources[@]}" -o "$bin"; then
    err "編譯失敗"
    # 只給一個檔案而且出現 undefined symbol 時，多半是漏了另一個檔案
    if [ ${#sources[@]} -eq 1 ] && grep -q '^[[:space:]]*extern' "${sources[0]}"; then
        printf '%s  提示：這個檔案有用到 extern，可能要跟定義它的 .c 一起編譯，例如：\n  ./serve.sh %s <另一個檔案.c>%s\n' \
            "$C_DIM" "${sources[0]}" "$C_OFF" >&2
    fi
    exit 1
fi

if [ "$build_only" -eq 1 ]; then
    printf '%s✓ 編譯完成：%s%s\n' "$C_OK" "output/$name" "$C_OFF" >&2
    exit 0
fi

# ---- 執行 ----
shown_args=""
for a in ${run_args[0]+"${run_args[@]}"}; do shown_args="$shown_args $(printf '%q' "$a")"; done
info "run:   output/$name$shown_args"
if [ -t 0 ]; then
    printf '%s  （如果程式在等輸入：打完按 Ctrl-D 送出 EOF，按 Ctrl-C 中止）%s\n' "$C_DIM" "$C_OFF" >&2
fi
printf '%s──────────────────────────────%s\n' "$C_DIM" "$C_OFF" >&2

# 不讓 set -e 在程式回傳非 0 時直接中斷，才能印出 exit code
set +e
"$bin" ${run_args[0]+"${run_args[@]}"}
code=$?
set -e

printf '%s──────────────────────────────%s\n' "$C_DIM" "$C_OFF" >&2
if [ "$code" -eq 0 ]; then
    printf '%s✓ exit code 0%s\n' "$C_OK" "$C_OFF" >&2
else
    printf '%s✗ exit code %d%s\n' "$C_ERR" "$code" "$C_OFF" >&2
fi
exit "$code"
