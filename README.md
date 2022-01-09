# Nice_Cmdline
&emsp;&emsp;这是一个基于Linux由C语言编写的交互式命令行库。
</br>&emsp;&emsp;导入它后，你可以向其中添加属于自己的命令组。而在此基础之上，它也可以完成若干Shell操作：比如说历史记录、命令补全、若干快捷键操作等。


## 项目安装

### 1.安装
```shell
$ git clone https://github.com/zgg2001/NiceCmdline
$ cd NiceCmdline/
$ sudo make
$ sudo make install
```

### 2.编译执行样例程序
```shell
$ cd example/
$ make
$ ./test
```


## 实现功能
* 自定义命令组
* 对自定义命令的补全
</br>[tab]
* 对自定义命令的匹配执行
</br>[回车]
* 对已执行的命令进行历史记录
</br>[up / ctrl p / down / ctrl n]
* Shell中常见的操作
</br>[光标向左: left / ctrl b]
</br>[光标向右: right / ctrl f]
</br>[退格]
</br>[移动光标至最左: ctrl a]
</br>[移动光标至最右: ctrl e]
</br>[剪切光标右侧的内容: ctrl k]
</br>[粘贴剪切的内容: ctrl y]
</br>[重置命令行: ctrl c]
</br>[删除光标右侧的第一个字符: delete / ctrl d]
</br>[剪切光标左侧的第一个词: alt 退格 / ctrl w]
</br>[剪切光标右侧的第一个词: alt d]
</br>[光标向左移动到当前单词最前端: alt b]
</br>[光标向右移动到当前单词最后端: alt f]


## 快速了解
*基于(2022/1/9)版本</br>

本项目中源码文件共有8个部分，简单进行介绍：
##### 1. cmdline
&emsp;&emsp;命令行的主体部分，命令行的启动、交互、停止等操作均由其控制。
##### 2. receiver
&emsp;&emsp;cmdline中的接收器部分，输入内容由其接收并存入inputbuf缓冲区。</br>
&emsp;&emsp;接受器不止进行输入的接收，也会对输入进行初步解析，根据输入来触发回车、删除、历史查询等操作。
##### 3. parser_vt102
&emsp;&emsp;receiver中的控制码解析器部分，对用户输入的控制码进行解析匹配。</br>
&emsp;&emsp;由于receiver中是以字符为单位进行解析的，为了可以识别多字符控制码(例如ctrl a、ctrl e等)，需要一个独立的二级缓冲区进行控制码的匹配，由此这部分就是干这个工作的。
##### 4. inputbuf
&emsp;&emsp;receiver中的输入缓冲区部分。receiver中存在三个缓冲区，其中左右缓冲区分别储存光标左、右的命令内容，而命令缓冲区储存左右缓冲区整合后的完整命令内容。</br>
&emsp;&emsp;inputbuf使用连续内存+记录首尾下标的思路，使得连续内存下首尾添加字符的时间复杂度为O(1)，而且实现起来也非常简单。
##### 5. history
&emsp;&emsp;receiver中的历史记录部分。使用双向链表实现，其每个节点中储存一条历史命令。</br>
&emsp;&emsp;由于节点中储存命令的空间以及节点本身是malloc出来的，所以需要注意释放(已有实现，调用即可)。这样的实现使得用户可以完全自定义历史记录数量(空间允许的情况下)。
##### 6. parse
&emsp;&emsp;此部分定义了"命令"的数据结构以及所属于它的结构"令牌"。另外此部分也定义了命令解析逻辑以及命令补全逻辑(均基于对"命令"数据结构的比对)。</br>
&emsp;&emsp;"命令"中主要包含一个回调函数和若干"令牌"。回调函数规定了此命令触发后执行的内容，而"令牌"则固定了命令的格式内容。</br>
&emsp;&emsp;例如命令"set mode 1"可以被看做"字符串 字符串 数字"的格式，则对应到数据结构"命令"中就是三个"令牌"：两个字符串令牌和一个数字令牌。由此只要添加对应的令牌并规定令牌的正确匹配内容，便可以设计出想要的命令。
##### 7. parse_string
&emsp;&emsp;字符串类型令牌。</br>
&emsp;&emsp;支持一个令牌匹配多个字符串。
##### 8. parse_num
&emsp;&emsp;数字类型令牌。</br>
&emsp;&emsp;支持8、16、32位的有、无符号整数 以及 32位单精度浮点数。


## 作者
zgg2001


## 版权说明
该项目签署了 Apache-2.0 授权许可，详情请参阅 [LICENSE](https://github.com/zgg2001/NiceCmdline/blob/main/LICENSE)


## 参考开源项目
1. aversive的parse部分</br>https://www.droids-corp.org/~zer0/doxygen_aversive/html/files.html
2. dpdk的cmdline部分</br>http://doc.dpdk.org/api-16.04/cmdline_8h.html
