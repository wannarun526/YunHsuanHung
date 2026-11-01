# K&R C 練習：怎麼 build 跟 run

這個資料夾是《The C Programming Language》（K&R）的練習程式，筆記在 [`../K&R.html`](../K&R.html)。

```
C/
├── ch1/            第一章範例
│   └── exercise/   第一章習題（1-1.c …）
├── ch2/            第二章範例
└── output/         編譯出來的執行檔都放這裡（不要放進 git）
```

以下指令都在 **`career/C/` 這一層**執行。

---

## 1. 環境準備（只需要做一次）

| 環境                | 指令                                     | 備註                                                    |
| ------------------- | ---------------------------------------- | ------------------------------------------------------- |
| macOS（本機）       | `xcode-select --install`               | macOS 的`gcc` 其實是 Apple clang 的別名，指令一樣能用 |
| Ubuntu（Fusion VM） | `sudo apt install build-essential gdb` | 真正的 GCC                                              |

確認裝好了：

```bash
gcc --version
```

### 連進 Fusion VM：`ssh fw-lab`

在 Mac 的終端機輸入：

```bash
ssh fw-lab
```

就會免密碼登入 Fusion 裡的 Ubuntu VM，輸入 `exit` 離開。

這需要先在 Mac 的 `~/.ssh/config` 設定好 `fw-lab`（一次就好）：

```
Host fw-lab
    HostName <VM_IP 或 主機名稱.local>
    User <VM 的帳號>
    IdentityFile ~/.ssh/id_ed25519
    AddKeysToAgent yes
    UseKeychain yes
```

並且已經用 `ssh-keygen -t ed25519` 產生金鑰、用 `ssh-copy-id -i ~/.ssh/id_ed25519.pub <帳號>@<VM_IP>` 把公鑰放進 VM。VM 的 IP 用 `ip -4 addr show` 查；IP 變了就更新 `HostName`，或在 VM 裡裝 `avahi-daemon` 改用 `<主機名稱>.local`。

#### 把練習檔同步到 VM 上編譯

程式碼放在 Mac 上，要在 VM 裡編譯的話，先在 **`career/C/` 這一層**把檔案同步過去（`output/` 不用傳，執行檔要在 VM 上重新編譯）：

```bash
rsync -av --exclude output/ ./ fw-lab:~/C/
```

再連進 VM 編譯執行：

```bash
ssh fw-lab
cd ~/C && ./serve.sh ch2/behavior.c
```

也可以不登入，一行直接在 VM 上執行，結果會印在 Mac 的終端機：

```bash
ssh fw-lab 'cd ~/C && ./serve.sh ch2/behavior.c'
```

`serve.sh` 沒有執行權限的話，把 `./serve.sh` 換成 `bash serve.sh`。用 VS Code 的話，裝 **Remote - SSH** 擴充套件後選 `fw-lab`，就能直接在 VM 上編輯和執行，不需要同步。

---

## 2. 單一檔案：編譯並執行

```bash
mkdir -p output
gcc -std=c11 -Wall -Wextra -g ch1/hello.c -o output/hello
./output/hello
```

各參數的意思：

| 參數                | 作用                                                       |
| ------------------- | ---------------------------------------------------------- |
| `-std=c11`        | 指定用 C11 標準，讓 Mac 和 Ubuntu 的行為一致               |
| `-Wall -Wextra`   | 開啟警告。**每個警告都要處理**，C 的警告常常就是 bug |
| `-g`              | 加上除錯資訊，之後才能用 gdb／lldb 逐行除錯                |
| `-o output/hello` | 執行檔的名稱和位置；沒寫的話會在目前目錄產生`a.out`      |

想讓警告直接變成錯誤（比較嚴格），再加上 `-Werror`。

### 一行完成：編譯成功才執行

```bash
gcc -std=c11 -Wall -Wextra -g ch2/const.c -o output/const && ./output/const
```

`&&` 表示前面成功才會執行後面，編譯失敗就不會跑到舊的執行檔。

---

## 3. 會讀取輸入的程式（`getchar`）

`getCharPutChar.c`、`count.c`、`array.c`、`characterArray.c` 會一直讀到 **EOF** 才結束。有三種給輸入的方式：

```bash
# ① 互動輸入：打完按 Enter，最後按 Ctrl-D 送出 EOF
./output/count

# ② 用管線（pipe）餵一段文字
echo "hello firmware" | ./output/count
printf 'one\ntwo\n' | ./output/characterArray

# ③ 把檔案內容當作輸入（重新導向）
./output/count < ch1/count.c
```

> ⚠ 程式看起來「卡住」不動，通常不是壞掉，而是在等輸入。按 `Ctrl-D` 結束，或按 `Ctrl-C` 強制中止。

---

## 4. 多個檔案一起編譯

`ch1/main.c` 用 `extern` 引用了 `ch1/counter.c` 裡的變數和函式，所以兩個檔案要**一起**編譯：

```bash
gcc -std=c11 -Wall -Wextra -g ch1/main.c ch1/counter.c -o output/app && ./output/app
```

只編譯 `main.c` 的話，會出現 `undefined reference to 'g_error_count'`（Mac 上的訊息是 `Undefined symbols`），因為 linker 找不到那個變數的定義。

---

## 5. 每個檔案的指令速查

| 檔案                               | 編譯                                                                            | 執行                                      | 需要輸入？ |
| ---------------------------------- | ------------------------------------------------------------------------------- | ----------------------------------------- | ---------- |
| `ch1/hello.c`                    | `gcc -std=c11 -Wall -Wextra -g ch1/hello.c -o output/hello`                   | `./output/hello`                        | —         |
| `ch1/while.c`                    | `gcc -std=c11 -Wall -Wextra -g ch1/while.c -o output/while`                   | `./output/while`                        | —         |
| `ch1/for.c`                      | `gcc -std=c11 -Wall -Wextra -g ch1/for.c -o output/for`                       | `./output/for`                          | —         |
| `ch1/getCharPutChar.c`           | `gcc -std=c11 -Wall -Wextra -g ch1/getCharPutChar.c -o output/getCharPutChar` | `echo hi \| ./output/getCharPutChar`     | ✅         |
| `ch1/count.c`                    | `gcc -std=c11 -Wall -Wextra -g ch1/count.c -o output/count`                   | `./output/count < ch1/count.c`          | ✅         |
| `ch1/array.c`                    | `gcc -std=c11 -Wall -Wextra -g ch1/array.c -o output/array`                   | `echo deadbeef42 \| ./output/array`      | ✅         |
| `ch1/callByValue.c`              | `gcc -std=c11 -Wall -Wextra -g ch1/callByValue.c -o output/callByValue`       | `./output/callByValue`                  | —         |
| `ch1/characterArray.c`           | `gcc -std=c11 -Wall -Wextra -g ch1/characterArray.c -o output/characterArray` | `./output/characterArray < ch1/hello.c` | ✅         |
| `ch1/main.c` + `ch1/counter.c` | `gcc -std=c11 -Wall -Wextra -g ch1/main.c ch1/counter.c -o output/app`        | `./output/app`                          | —         |
| `ch2/behavior.c`                 | `gcc -std=c11 -Wall -Wextra -g ch2/behavior.c -o output/behavior`             | `./output/behavior`                     | —         |
| `ch2/const.c`                    | `gcc -std=c11 -Wall -Wextra -g ch2/const.c -o output/const`                   | `./output/const`                        | —         |
| `ch1/exercise/1-1.c`             | `gcc -std=c11 -Wall -Wextra -g ch1/exercise/1-1.c -o output/1-1`              | `./output/1-1`                          | 看題目     |

新增檔案時照同一個格式：`gcc … <來源檔> -o output/<檔名>`。

---

## 6. 除錯

### 抓記憶體錯誤：AddressSanitizer（最推薦）

陣列越界、用到已釋放的記憶體這類錯誤，C 平常不會報錯。加上 `-fsanitize=address` 之後，一發生就會停下來，並指出是哪一行：

```bash
gcc -std=c11 -Wall -Wextra -g -fsanitize=address ch1/array.c -o output/array && echo "abc" | ./output/array
```

練習時建議**一律加上**，等確定沒問題再拿掉。

### 逐行除錯：lldb（Mac）／gdb（Ubuntu）

Apple Silicon 的 Mac 上沒有 gdb，要改用 lldb。指令很像：

| 動作                   | lldb（Mac）                   | gdb（Ubuntu）                |
| ---------------------- | ----------------------------- | ---------------------------- |
| 啟動                   | `lldb ./output/callByValue` | `gdb ./output/callByValue` |
| 在 main 設中斷點       | `b main`                    | `b main`                   |
| 開始執行               | `run`                       | `run`                      |
| 執行下一行（不進函式） | `n`                         | `n`                        |
| 進入函式               | `s`                         | `s`                        |
| 印出變數               | `p x`                       | `p x`                      |
| 繼續執行               | `c`                         | `c`                        |
| 離開                   | `q`                         | `q`                        |

要餵輸入時，在 lldb 用 `process launch -i ch1/count.c`，gdb 用 `run < ch1/count.c`。

---

## 7. 清理

```bash
rm -rf output/*
```

在 Mac 上用 `-g` 編譯時，會多產生 `output/*.dSYM` 資料夾（除錯符號），可以一起刪掉，下次編譯會再產生。

---

## 8. 在 Mac 和 Ubuntu 上結果可能不同

同一支程式在兩個環境跑出不同結果是正常的，這正是 K&R 第二章在講的「由平台決定」。例如 `ch2/behavior.c` 的 `CHAR_MIN`：

| 環境                      | `CHAR_MIN` | 原因                          |
| ------------------------- | ------------ | ----------------------------- |
| macOS（Apple Silicon）    | -128         | macOS 規定`char` 有號       |
| Ubuntu ARM64（Fusion VM） | 0            | ARM64 Linux 規定`char` 無號 |
| Ubuntu x86-64（GCP VM）   | -128         | x86 Linux 規定`char` 有號   |

兩邊都跑一次，親眼看看差異。

---

## 9. UEFI 實驗：UEFI Shell 與 debugcon log（在 Fusion VM 裡）

以下指令都在 **Fusion VM** 裡執行（先 `ssh fw-lab`），不是在 Mac 上。QEMU 指令都在 `~/uefi-lab` 這個資料夾執行。

| 名詞 | 是什麼 |
|---|---|
| **UEFI Shell** | 韌體開機完成後執行的命令列（一支 `.efi` 程式），可以下 `memmap`、`pci`、`map` 等指令查看硬體 |
| **debugcon log** | 韌體在開機過程中（SEC → PEI → DXE → BDS）用 `DEBUG()` 印出的除錯紀錄，QEMU 把寫到 I/O port `0x402` 的內容存成 `debug.log` |

### 9.1 開進 UEFI Shell（用 Ubuntu 的 OVMF）

Ubuntu 26.04 的 `ovmf` 套件**沒有內建 Shell**，要另外裝 `efi-shell-x64`，並放到虛擬磁碟的 `EFI/BOOT/BOOTX64.EFI`。

準備（只需要做一次）：

```bash
sudo apt install -y efi-shell-x64
```

```bash
mkdir -p ~/uefi-lab/esp/EFI/BOOT && cd ~/uefi-lab && cp /usr/share/efi-shell-x64/shellx64.efi esp/EFI/BOOT/BOOTX64.EFI && cp /usr/share/OVMF/OVMF_VARS_4M.fd vars.fd
```

開機：

```bash
qemu-system-x86_64 -machine q35 -m 512 -drive if=pflash,format=raw,unit=0,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd -drive if=pflash,format=raw,unit=1,file=vars.fd -drive file=fat:rw:esp,format=raw,media=disk -debugcon file:debug.log -global isa-debugcon.iobase=0x402 -nographic -net none
```

倒數 5 秒後會出現 `Shell>`，可以試 `ver`、`map`、`memmap`、`pci`、`help`。

> ⚠ Ubuntu 的 OVMF 是 **RELEASE 版**，編譯時已經把 `DEBUG()` 拿掉，所以這一步的 `debug.log` 會是 **0 bytes**。要看到 log，要做 9.2 和 9.3。

各參數的意思：

| 參數 | 意思 |
|---|---|
| `-machine q35` | 模擬現代的 Intel 晶片組（有 PCIe） |
| `pflash unit=0`、`readonly=on` | 韌體程式本身（`OVMF_CODE`），相當於主機板上的 BIOS flash |
| `pflash unit=1`、`vars.fd` | 存 UEFI 變數的 NVRAM。用**複製的一份**，因為會被寫入 |
| `-drive file=fat:rw:esp` | 把 `esp/` 資料夾變成一顆 FAT 虛擬磁碟 |
| `-debugcon file:debug.log -global isa-debugcon.iobase=0x402` | 把寫到 I/O port `0x402` 的內容存成 `debug.log` |
| `-nographic` | 把序列埠（serial console）接到終端機，透過 SSH 也能操作 |

### 9.2 編出 DEBUG 版 OVMF（從 EDK II 原始碼）

準備（只需要做一次）：

```bash
sudo apt install -y uuid-dev
```

```bash
cd ~ && git clone https://github.com/tianocore/edk2.git && cd edk2 && git submodule update --init
```

```bash
make -C BaseTools -j2
```

編譯（2 個 vCPU 大約 20–40 分鐘，看到 `- Done -` 就是成功）：

```bash
cd ~/edk2 && export GCC_BIN=x86_64-linux-gnu- && . edksetup.sh && build -a X64 -t GCC -p OvmfPkg/OvmfPkgX64.dsc -b DEBUG -n 2
```

| 部分 | 意思 |
|---|---|
| `export GCC_BIN=x86_64-linux-gnu-` | 指定 x86-64 交叉編譯器。新版 EDK II 從 `GCC_BIN` 讀這個值（`tools_def.txt` 裡的 `DEFINE GCC_X64_PREFIX = ENV(GCC_BIN)`），**不是** `GCC5_X64_PREFIX` |
| `. edksetup.sh` | 用 source 設定 `WORKSPACE`、`PATH` 等環境變數（前面的 `.` 不能省） |
| `-a X64` | 目標架構 x86-64 |
| `-t GCC` | 工具鏈標籤。新版 EDK II 已經移除舊的 `GCC5` |
| `-p OvmfPkg/OvmfPkgX64.dsc` | 要編的平台：QEMU 用的 x86-64 OVMF |
| `-b DEBUG` | DEBUG 版，保留 `DEBUG()` 輸出 |
| `-n 2` | 同時編 2 個工作（對應 2 個 vCPU） |

輸出位置：`~/edk2/Build/OvmfX64/DEBUG_GCC/FV/`（裡面有 `OVMF_CODE.fd`、`OVMF_VARS.fd`）。

常見錯誤：

| 錯誤訊息 | 原因與解法 |
|---|---|
| `Tool chain [GCC5] is not defined` | 新版 EDK II 已經沒有 `GCC5`，改用 `-t GCC` |
| `gcc: error: unrecognized command-line option '-m64'` | 用到 ARM 的 gcc：沒設定 `GCC_BIN`，或設成舊的 `GCC5_X64_PREFIX` |
| `uuid/uuid.h: No such file or directory` | 沒裝 `uuid-dev` |
| `objcopy: error: the input file '…dll' is empty` | **同時跑了兩個 build**，互相寫壞了檔案。確認只有一個 build 在跑，執行 `rm -rf ~/edk2/Build` 後重新編譯 |

> 💡 編譯要跑很久，建議在 `tmux new -s edk2` 裡執行。SSH 斷線時 build 不會中斷，之後用 `tmux attach -t edk2` 接回去。

### 9.3 用 DEBUG 版開機，拿到 debugcon log

先複製 DEBUG 版的 vars（一定要跟 DEBUG 版的 `OVMF_CODE.fd` 搭配，不能用 Ubuntu 的 `OVMF_VARS_4M.fd`）：

```bash
cd ~/uefi-lab && cp ~/edk2/Build/OvmfX64/DEBUG_GCC/FV/OVMF_VARS.fd vars-debug.fd
```

開機：

```bash
qemu-system-x86_64 -machine q35 -m 512 -drive if=pflash,format=raw,unit=0,readonly=on,file=$HOME/edk2/Build/OvmfX64/DEBUG_GCC/FV/OVMF_CODE.fd -drive if=pflash,format=raw,unit=1,file=vars-debug.fd -drive file=fat:rw:esp,format=raw,media=disk -debugcon file:debug.log -global isa-debugcon.iobase=0x402 -nographic -net none
```

進到 `Shell>` 後離開 QEMU，再看 log：

```bash
wc -l debug.log && head -20 debug.log
```

這次 `debug.log` 應該有好幾千行。可以用 `grep` 找各個開機階段的開始點：

```bash
grep -n -m1 -E "SecCoreStartup" debug.log; grep -n -m1 "PeiCore" debug.log; grep -n -m1 "DxeCore" debug.log; grep -n -m1 "\[Bds\]" debug.log
```

### 9.4 離開 QEMU

| 方式 | 怎麼做 |
|---|---|
| 快捷鍵 | 按 `Control-A`，**放開**，再按 `X`（Mac 也是用 Control，不是 ⌘） |
| 在 UEFI Shell 裡 | 輸入 `reset -s` 關機 |
| QEMU monitor | `Control-A` 放開再按 `C`，輸入 `quit` |
| 從 Mac 強制結束 | `ssh fw-lab 'pkill -f qemu-system-x86_64'` |
| 整個卡住 | 先按 Enter，再輸入 `~.` 斷開 SSH 連線 |
