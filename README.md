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

ZJSON_Stack.h利用C++的模板类，每次压入栈的时候传入类型，从而使得栈顶指针根据压入的内容大小进行移动，在栈空间不足时通过栈对应分配器的Realloc函数将栈的大小扩展当前的1.5倍，从而实现了一个混合类型堆栈。
	
	class Stack {
   	 Stack(Allocator* allocator, size_t stackCapacity);
    	~Stack();
    
   	 void Clear();
   	 void ShrinkToFit();
    
   	 template<typename T> T* Push(size_t count = 1);
   	 template<typename T> T* Pop(size_t count);
  	 template<typename T> T* Top(); //返回最后一个入栈元素地址
   	 template<typename T> T* Bottom(); //返回第一个入栈元素地址

  	  Allocator& GetAllocator();
  	  bool Empty() const;
  	  size_t GetSize();
  	  size_t GetCapacity();
	};
	
注意：为了完全防止拷贝构造函数调用的可能性，这些函数都是返回指针。

同时考虑到多次调用需要大量的申请一块类似的内存空间，然后又释放掉，耗费时间。如果当前系统中有大量的内存碎片，并且我们申请的空间很大，甚至有可能失败。对此我们采用placement new（定位放置new），即在用户指定的内存位置上构建新的对象，这个构建过程不需要额外分配内存，只需要调用对象的构造函数即可。
因为此函数相当于在一块已经分配好的内存上通过调用构造函数进行操作，所以结束后需要调用析构函数并释放内存，并不需要调用delete函数。

## 四、ZJSON_Allocator.h

Allocator（分配器）作用为申请内存以及内存的扩容，主要用在多类型堆栈的建立与扩容以及JSON值赋值时。
一个基本的分配器需要实现三个功能：内存分配Malloc(),内存块重新划分Realloc(),以及释放内存快Free().以下为基本定义：
	
	class CrtAllocator {
	public:
		static const bool kNeedFree = true;//声明需要释放内存
		
	// 申请内存块。
  	// size为内存块的大小，以字节记。returns 指向内存块的指针。
		void* Malloc(size_t size) {
			if (size) {
				return std::malloc(size);//大小不为0，申请空间
			}
			else
				return NULL;
		}
		
	// 调整内存块的大小。
	//  oPtr为当前内存块的指针，允许空指针，osize为当前大小，以字节记。 newsize为新大小，以字节记。
		void* Realloc(void* oPtr, size_t osize, size_t newsize)
		{
			//(void)osize;
			if (newsize==0)//若新大小为0，释放内存
			{
				free(oPtr);
				return NULL;
			}
			return std::realloc(oPtr, newsize);
		}
		
	// 释放内存块。
	//ptr 指向内存块的指针，允许空指针。
		static void Free(void* ptr)
		{
		 	std::free(ptr);
		}
	};
	
而在构建DOM树时我们需要重新设计一个DOM 的默认内存分配器MemoryAllocator，需要注意的是：**它只申请内存而不释放内存。**
它将严格遵守以下步骤来申请内存：
1.如果可用，使用用户提供的缓冲区。
2.如果用户提供的缓冲区已满，使用当前内存块。
3.如果当前内存块已满，申请新的内存块。
	
除了基本的Malloc(),Realloc(),Free()外，我们还需要考虑计算分配内存块已用字节总数Size()来确认是否需要扩展内存,以及在Realloc()中需要考虑到：
1.重新分配进程时需要分配和复制内存并且不释放原始缓冲区。
2.如果新大小小于原始大小不收缩，返回原指针；
3.如果为最后一个内存块，只需要扩展即可，不需要重新划分；
	
具体实现请参考源代码ZJSON_Allocator.h

## 五、ZJSON_Reader.h

## 六、ZJSON_Writer.h

## 七、ZJSON_Document.h
简单实现了BasicValue类用来表示JSON的值，BasicDocument类用来保存JSON，从而生成DOM树。DOM中包含一个Parse函数，通过调用Reader中的Parse将值保存为BasicValue返回给DOM。
