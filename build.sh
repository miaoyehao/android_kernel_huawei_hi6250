#!/bin/bash
# 设置脚本在发生错误时退出
set -e
# 安装依赖
echo "安装依赖..." sudo apt install -y glt-core gnupg flex bison gperf build-essential zip curl zlib1g-dev \ 
  gcc-multilib g++-multilib libc6-dev-i386 lib32ncurses5-dev x11proto-core-dev libx11-dev \ lib32z1-dev 
  libgl1-mesa-dev libxml2-utils xsltproc unzip bc
# 安装 Python 2
echo "安装 Python 2..." sudo apt install -y python2
# 创建 Python 2 的符号链接
echo "创建 Python 2 的符号链接..." sudo ln -sf /usr/bin/python2.7 /usr/bin/python
# 设置环境变量
echo "设置环境变量..." 
export ARCH=arm64 
export PATH=$PATH:/home/miao/android_kernel_huawei_hi6250
/linux-x86_64/bin 
export CROSS_COMPILE=aarch64-linux-android-
# 编译内核
echo "开始编译内核..." make ARCH=arm64 O=out merge_hi6250_defconfig make ARCH=arm64 O=out -j$(nproc)
echo "内核编译完成！"
