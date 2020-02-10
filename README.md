# ZJSON简单高效的JSON库的实现
大学已经过了两年半的时间，一直在搞竞赛，但是项目经历也是不可缺少的一环，于是和同学从寒假开始着手自己的项目经历，主要是学习腾讯的开源JSON框架RapidJson，希望把自己的学习的内容写下来，能够帮助到一些和我一样从竞赛学习转项目学习的人。

学习资料：

Rapid中文文档：[http://rapidjson.org/zh-cn](http://rapidjson.org/zh-cn)

从零开始的 JSON 库教程 :[https://github.com/miloyip/json-tutorial](https://github.com/miloyip/json-tutorial)
# 概述：
## 一、目录：
1、JSON库的基本框架

2、JSON输入输出流的实现

3、JSON解析的实现（一）

4、多类型栈缓冲区与内存分配器的实现

5、JSON解析的实现（二）

6、JSON生成器的实现 

## 二、实现功能：

本项目主要参照学习资料中的两个项目，目的为完成一个轻量且高效的JSON解析器，具体功能如下：

1、基于DOM方式的JSON存储以及修改

2、基于流输入的JSON解析，支持UTF-8,UTF-16大端小端编码，UTF-32大端小端编码

3、基于流输出的JSON生成

# 基本框架：

从功能出发，第一步我们需要实现基于流的输入输出，从而可以读取和输出JSON串，其次在完成基本的读取输入功能之后，我们将各种编码方式统一转化为Unicode编码以及Unicode编码向各种编码方式的转化。在完成之前的功能之后我们获得了一个JSON串，接下来要实现JSON串的解析功能。在JSON解析过程中，我们使用一个DOM（类似于容器）来保存JSON以及调用解析函数，在完成解析之后，可以调用DOM的查询功能去查找修改JSON串。其中所有的缓冲区我们使用一个多类型的堆栈来实现，而内存的分配使用Allocator来申请内存。

为了完成以上功能我们有以下几个头文件要实现：

## 一、ZJSON_Stream.h

![在这里插入图片描述](https://img-blog.csdnimg.cn/2020012819521817.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3o0NzI0MjE1MTk=,size_16,color_FFFFFF,t_70)

关于流的基本要求，这里参照RapidJSON库的实现方式，具体如上图，该头文件包含的内容如下：

![image-20200210173136881](https://github.com/z472421519/ZJSON-JSON-/blob/master/Picture/ZJSON_Encoding.PNG)

## 二、ZJSON_Encoding

关于编码主要实现解码与编码这两个逆过程，基本要求也是参照RapidJSON库，具体如下：

![在这里插入图片描述](https://img-blog.csdnimg.cn/20200129161929489.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3o0NzI0MjE1MTk=,size_16,color_FFFFFF,t_70)

我们要是实现的库包含多种编码，具体如下：每种编码都包含上述函数（如果不需要调用也会有空实现）

![image-20200210174126543](https://github.com/z472421519/ZJSON-JSON-/blob/master/Picture/ZJSON_Encoding.PNG)

## 三、ZJSON_Stack.h

## 四、ZJSON_Allocator.h

## 五、ZJSON_Reader.h

## 六、ZJSON_Writer.h

## 七、ZJSON_Document.h
