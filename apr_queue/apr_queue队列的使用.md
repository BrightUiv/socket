### apr-queue队列的使用

#### 一、安装apr队列

- sudo apt update
- sudo apt install libapr1 libapr1-dev

  - `libapr1`：是Apache Portable Runtime (APR) 库的==运行时包==，提供一套API，这套API旨在提供一种在各种操作系统平台上运行的方式，以实现高级编程任务，如文件系统操作、网络通信和线程管理。

  - `libapr1-dev`：是`libapr1`的==开发包==，包含了开发使用APR库所需的头文件和开发工具。这对于编译或开发需要使用APR库的应用程序是必需的。

  - libapr1、libapr1-dev是==软件包==，里面包含着库文件
- 关于库的知识点：
  - 库文件主要以三种形式存在：静态库（`.a`），共享库（`.so`），和链接器脚本（`.la`）	

- libapr1-dev中没有包含apr_queue.h的头文件
  - dpkg -L libapr1-dev | grep apr_queue.h查找，发现没有apr_queue.h头文件



### 二、apr-util-1.6.3.tar.gz的安装

- sudo mv /home/mao/apr-util-1.6.3.tar.gz /usr/local/src/安装apr-util文件的安装位置
- sudo tar -xvzf apr-util-1.6.3.tar.gz解压apr-util文件夹
- libapr-1和libapr1区别
  - **`libapr-1`** **`libapr-1`** 通常指的是APR库的实际动态链接库（.so或.dll文件）或静态库（.a或.lib文件）。这些库文件是编译时链接和运行时加载的实际二进制文件。`libapr-1` 是 Apache Portable Runtime (APR) 库的一部分
  - **`libapr1`** **`libapr1`** 通常指的是APR库的软件包名称，在基于Debian（如Ubuntu）的系统中使用。软件包包括了库文件（如`libapr-1.so`），可能还包括其他相关文件，如配置文件或文档。
- sudo ./configure --with-apr=/usr/lib/x86_64-linux-gnu/<出现错误，with-apr=没有正确指定位置>
- sudo ./configure --with-apr=/usr/bin/apr-1-config
- sudo make(未安装库)
- sudo apt-get update
- sudo apt-get install libexpat1-dev
- sudo make install
  - 出现结果：
  - /usr/bin/install -c -m 644 aprutil.exp /usr/local/apr/lib
    /usr/bin/install -c -m 755 apu-config.out /usr/local/apr/bin/apu-1-config
- gcc -o apr_queue_test  apr_queue_test.c -I/usr/include/apr-1.0 -I/usr/local/apr/include/apr-1 -L/usr/local/apr/lib -lapr-1 -laprutil-1，发现报错
-  cd /etc/ld.so.conf.d <发现缺少apr.conf文件>
- sudo vim apr.conf《添加路径：/usr/local/apr/lib，“:wq”保存退出》
- sudo ldconfig      <更新动态链接器的缓存>
- ldconfig -p | grep libname    <验证更改>
- gcc -o apr_queue_test  apr_queue_test.c -I/usr/include/apr-1.0 -I/usr/local/apr/include/apr-1 -L/usr/local/apr/lib -lapr-1 -laprutil-1 -lpthread<执行成功>
  - -I/usr/include/apr-1.0 -I/usr/local/apr/include/apr-1<是为了链接头文件>
  - -lapr-1 -laprutil-1<`-lapr-1`和`-laprutil-1`这两条命令的作用是在编译过程中将APR库和APR-util库链接到你的程序>



### 三、重新编译apr-util库

- 将apr-util-1.6.3/include/misc/apr_queue.c文件中删去
- 将apr-util-1.6.3/include/apr_queue.h文件删去
- sudo cp apr_queue.c  /usr/local/src/apr-util-1.6.3/misc
- sudo cp apr_queue.h  /usr/local/src/apr-util-1.6.3/include
- make clean
- sudo ./configure --with-apr=/usr/bin/apr-1-config
- sudo make
- sudo make install
- 编译成功

