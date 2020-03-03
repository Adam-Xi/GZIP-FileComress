---
title: README.md
author: Adam-Xi
date: 2020-01-11
---
# 前言
### 文件压缩概念
> 数据压缩是指在不丢失有用信息的前提下，缩减数据量以减少存储空间，提高其传输、存储和处理效率，或按照一定的算法对数据进行重新组织，减少数据的冗余和存储的空间的一种技术方法
### 必要性
+ 1. 紧缩数据存储容量，减少存储空间
+ 2. 可以提高数据传输的速度，减少带宽占用量，提高通讯效率
+ 3. 对数据的一种加密保护，增强数据在传输过程中的安全性
### 文件压缩分类
- 有损压缩
  > 有损压缩是利用了人类对图像或声波中的某些频率成分不敏感的特性，允许压缩过程中损失一定的息；虽然不能完全恢复原始数据，但是所损失的部分对理解原始图像的影响缩小，却换来了大得多的压缩比，即指使用压缩后的数据进行重构，重构后的数据与原来的数据有所不同，但不影响人对原始资料表达的信息造成误解。
- 无损压缩
  > 对文件中数据按照特定的编码格式进行重新组织，压缩后的压缩文件可以被还原成与源文件完全相同的格式，不会影响文件内容，对于数码图像而言，不会使图像细节有任何损失。
<!--
# GZIP压缩算法
GZIP压缩算法经历了两个阶段:
* 第一阶段，使用改进的LZ77压缩算法对上下文中的重复语句进行压缩
* 第二阶段，采用huffman编码思想对第一阶段压缩完成的数据进行字节上的压缩
从而实现对数据的高效压缩存储
### GZIP中的LZ77算法
### GZIP中的Huffman算法
-->
# Huffman压缩算法
Huffman算法脱胎于GZIP算法，是GZIP算法的第二个压缩算法，其思想主要为以下几个步骤：
+ 压缩
	- 在字节层面找待压缩文件中的重复字节，并统计各个字节重复的次数
	- 根据重复字节及重复次数构建Huffman树
	- 根据构建的Huffman树获取Huffman编码
	- 利用Huffman编码对源文件进行压缩
	- 对压缩文件格式进行记录(文件后缀，字符次数对的总行数、字符及字符出现的次数)
+ 解压缩
	- 从压缩文件中获取源文件的后缀
	- 获取字符的总行数
	- 获取每个字符出现的次数
	- 重建Huffman树
	- 解压缩

# Huffman算法性能测试
使用Beyond Compare4对文件压缩前后进行比较，格式、数据等完全一致<br />
1、对.txt、.doc、.docx、.pdf等文档格式文件压缩效率区间为55%~98%，其中，.txt文件压缩效果最好，普遍为55%~80%<br />
2、对.jgp、.png等图片格式文件压缩效率区间为80%~100%<br />
3、对.mp3、.flac等音频格式文件压缩效率区间为90%~100%<br />
4、对.avi、.flv等视频格式文件压缩效率为80%~100%<br />
5、对.exe等的可执行程序格式文件压缩效率为95%~100%<br />
经过简单的测试可以看出，单纯的Huffman算法的压缩效率对于大多数文件来说较低，并且耗时较长<br />

# 路遇问题及解决方法
+ 压缩数据的格式需要进行统一，确保压缩和解压缩的一致性
    - 在编写代码前需要约定将源文件的后缀、字符次数对的总行数、字符次数对以怎样的格式排布在压缩文件中
+ 对存在汉字文件的压缩
    - 在编写代码时给简单测试用例中加入一个汉字时，发现程序会崩溃，利用变量监视窗口进行排查问题时，发现用来表示Huffman中权值的结构体中表示具体字符的变量定义为char类型，到用char变量表示汉字时为负值，所以用unsigned char进行代替，即可表示汉字
+ 文件压缩和解压缩了一部分
    - 由于使用C接口对文件的打开方式为"w"或"r"，会发现有时候程序莫名其妙的压缩和解压缩了一部分，针对这个头疼的问题，在确认了自己的处理逻辑没有问题后，在网上查了很多资料，无果，最后将压缩的文件用二进制的方式查看时发现了解压了一半的程序都终止在了FF，意识到文本文件是以FF结尾的，于是更改了文件的打开方式，以二进制文件读写打开，完美地解决了这个问题
+ 保证解压缩后的正确性
    - 对于有些文件比如结尾是以很多歌换行或者空格结束的，等等，许多奇奇怪怪的格式的文件，肉眼是无法看出来的，所以在网上找到了Beyond Commpare这款软件，尽管是收费的，但是一个月体验时间足够了！！！
+ 在测试大文件的压缩时，对于动辄上百M的文件的压缩和解压缩是一个令人头疼的问题，因为过程太漫长了，有时候将近十几分钟的压缩周期不仅对开发人员（me）而且对机器都是一个煎熬，终究是因为Huffman算法的时间效率确实很低，导致程序运行时间过长，用户体验不是很好
+ 在测试文件压缩前后大小时，发现对于很多的文件的压缩效率并不能使人满意，如上一版块算法性能测试所示，这是因为Huffman压缩算法本身带来的问题，它是根据源文件中同一字节重复出现的次数而设计的算法，若是源文件中并没有存在很多的重复字节，又或者源文件中存在很多重复一次两次的字节，导致还要耗费很大的空间来存储该字符次数对，得不偿失
+ 在压缩前后文件大小比较时，发现并不是每次压缩压缩文件都比源文件小，对此的解释为：例如对存储着"ABBCCDD"的文件，压缩后首行存储文件的后缀，下面一行存储总行数，再下面几行存储字符次数对，形如"B:2"，再下面才是存储的数据，所以导致解压缩后的文件会比源文件大，以此类推，对于其他大的文件而言，同理
+ 由于在代码中的文件打开方式为二进制读写打开，所以对于任意文件类型的文件，该程序都可以进行压缩解压缩
  
# 待改进
+ 目前该算法的最大问题是压缩效率太低，所以后期可以将其他压缩算法也添加进来，几个压缩算法从不同层面按照不同方式一起进行压缩，这样极大地保证了压缩效率
+ 后期可以学习duilib或Qt等的UI库，为该程序做一个界面，方便与用户进行交互