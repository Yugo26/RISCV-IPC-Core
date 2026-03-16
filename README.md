# 2023RVOS

# Linux 開發環境

1. install crossed compiler(GCC)
```bash
$sudo apt update
$sudo apt install gcc-riscv64-unknown-elf
```
2. install some tools
```bash
$sudo apt install gcc make perl dkms git
```
3. install gdb-multiarch
```bash
$sudo apt install gdb-multiarch
```
4. install qemu
```bash
$sudo apt install qemu-system-misc
```

# windows 開發環境 msys2

https://www.msys2.org
Download and install MSYS2: https://sourceforge.net/projects/msys2/
Open MSYS2 shell and run 
```
pacman -Syu
```
After some time you will be prompted to close the MSYS2 shell by clicking the "X" button, which you should do :-)
Relaunch MSYS2 shell and run 
```
pacman -Su
```

1. crossed complie(Gcc)
```
pacman -S mingw-w64-x86_64-riscv64-unknown-elf-gcc
```

2. install some tools
```
pacman -S git make autoconf gcc flex bison man perl python3
```

3. install gdb-multiarh 
```
pacman -S mingw-w64-x86_64-gdb-multiarch
```
4. install qemu
```
pacman -S mingw-w64-x86_64-qemu
```

# vscode 安裝 (windows)

下載 vscode https://code.visualstudio.com
並自行安裝


#### 特技

1.  使用 Readme\_XXX.md 来支持不同的语言，例如 Readme\_en.md, Readme\_zh.md
2.  Gitee 官方博客 [blog.gitee.com](https://blog.gitee.com)
3.  你可以 [https://gitee.com/explore](https://gitee.com/explore) 这个地址来了解 Gitee 上的优秀开源项目
4.  [GVP](https://gitee.com/gvp) 全称是 Gitee 最有价值开源项目，是综合评定出的优秀开源项目
5.  Gitee 官方提供的使用手册 [https://gitee.com/help](https://gitee.com/help)
6.  Gitee 封面人物是一档用来展示 Gitee 会员风采的栏目 [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)
