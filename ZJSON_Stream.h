#pragma once

#define ZJSON_Disable 0
#define DefaultBufferSize 2
#include <iostream>
#include <fstream>
#include <sstream>
#include <assert.h>
#include <encoding.h>
#include <cstdio>

void ZJSON_Test()
{
	printf("T\n");
}
//stl的istream的包装流
template <typename Encoding>
class InStream1 {
public:
	typedef Encoding Ch;
	InStream1(std::istream& is) : is_(is) {
	}

	Ch Take() {
		int c = is_.get();
		return c == std::char_traits<char>::eof() ?  '\0' : (Ch)c;
	}
	Ch Peek() {
		int c = is_.peek();
		return c == std::char_traits<char>::eof() ?  '\0' : (Ch)c;
	}
	size_t Tell() {
		return (size_t)is_.tellg();
	}
	bool Put() { assert(ZJSON_Disable); }
	void Flash() { assert(ZJSON_Disable); }
private:
	std::istream& is_;
};
//从内存中读取JSON
//参数为：流读入指针
//编码类型待处理
template <typename StreamPointer,typename Encoding>
class InStream{
public:
	//typedef typename Encoding::CharType Ch;
	typedef Encoding Ch;
	InStream(char* src) :src_(src), head_(src) {}
	Ch Take() { return *src_++; }
	Ch Peek() { return *src_; }
	size_t Tell() { return static_cast<size_t>(src_ - head_); }
private:
	Ch* src_;
	Ch* head_;
};
template <typename Encoding>
class FileReadStream {
public:
	typedef typename Encoding Ch;
	FileReadStream(FILE *fp):fp_(fp),current(buffer),tail(buffer),buffer_size(DefaultBufferSize),read_count(0),count(0),eof(false){
		Read();
	}
	Ch Take() {
		Ch temp = *current;
		Read();
		return temp;
	}
	Ch Peek() {
		return *current;
	}
	size_t Tell() {
		//std::cout << current << buffer << std::endl;
		return count + static_cast<size_t>(current - buffer);
	}
	void Read()
	{
		if (current < tail) {
			current++;
		}
		else if(!eof){
			//std::cout << "Read" << std::endl;
			count += read_count;
			read_count = std::fread(buffer, 1,buffer_size, fp_);
			tail = buffer + read_count - 1;
			current = buffer;
			if (read_count < buffer_size)
			{
				++tail;
				*tail = '\0';
				//++tail;
				eof = true;
			}
			//std::cout << "T" << Tell() << std::endl;
		}
	}
private:
	FILE* fp_;
	//char* buffer;
	char buffer[DefaultBufferSize];
	char* current;
	char* tail;
	size_t buffer_size;
	size_t read_count;
	size_t count;
	bool eof;
};
