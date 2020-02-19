#pragma once

#define ZJSON_Disable 0
#define DefaultBufferSize 2
#include <iostream>
#include <fstream>
#include <sstream>
#include <assert.h>
#include <cstdio>
#include <assert.h>
void ZJSON_Test()
{
	printf("T\n");
}
//stl��istream�İ�װ��
template <typename Encoding>
class InStreamWrapper {
public:
	typedef typename Encoding::Ch Ch;
	InStreamWrapper(std::istream& is) : is_(is) {}

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
//���ڴ��ж�ȡJSON
//����Ϊ��������ָ��
//�������ʹ�����
template <typename StreamPointer,typename Encoding>
class InStream{
public:
	//typedef typename Encoding::CharType Ch;
	typedef typename Encoding::Ch Ch;
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
	typedef typename Encoding::Ch Ch;
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


template <typename Encoding>
class OutStreamWrapper {
public:
	typedef typename Encoding::Ch Ch;
	OutStreamWrapper(std::istream& is) : is_(is) {}

	void Put(Ch c) {
		is_.put(c);
	}

	void Flush() {
		is_.Flush();
	}
	
	char Peek() const { assert(ZJSON_Disable); }
	char Take() { assert(ZJSON_Disable); }
	size_t Tell() const { assert(ZJSON_Disable); }
	char* PutBegin() { assert(ZJSON_Disable); }
	size_t PutEnd(char*) { assert(ZJSON_Disable); }

private:
	std::istream& is_;
};



template <typename StreamPointer, typename Encoding>
class OutStream {
public:
	typedef typename Encoding::Ch Ch;
	OutStream(char* src) :src_(src), head_(src) {}
	Ch* PutBegin() { return dst_ = src_; }
	void Put(Ch c) { *dst_++ = c; }
	void Flush() {}
	size_t PutEnd(Ch* begin) { return static_cast<size_t>(dst_ - begin); }
	Ch* Push(size_t count) { Ch* begin = dst_; dst_ += count; return begin; }
	void Pop(size_t count) { dst_ -= count; }
private:
	Ch* src_;
	Ch* dst_;
	Ch* head_;
};


template <typename Encoding>
class FileWriteStream
{
public:
	//typedef typename Encoding::Ch Ch;
	FileWriteStream(std::FILE* fp, char* buffer, size_t bufferSize) : fp_(fp), buffer_(buffer), bufferEnd_(buffer + bufferSize), current_(buffer_)
	{
		assert(fp_ != 0);
	}

	void Put(char c) {
		if (current_ >= bufferEnd_)
			Flush();

		*current_++ = c;
	}

	void PutN(char c, size_t n)
	{
		size_t avail = static_cast<size_t>(bufferEnd_ - current_);//�ɷ����ַ���
		while (n > avail) {
			std::memset(current_, c, avail);
			//void *memset(void *s, int ch, size_t n);��s�е�ǰλ�ú����n���ֽ�,�� ch �滻������ s ��
			current_ += avail;
			Flush();
			n -= avail;
			avail = static_cast<size_t>(bufferEnd_ - current_);
		}
		if (n > 0) {
			std::memset(current_, c, n);
			current_ += n;
		}
	}

	void Flush()
	{
		if (current_ != buffer_)//��Ϊ��
		{
			//size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE * stream)
			//(ָ��Ҫ��д���Ԫ�������ָ��,��д���ÿ��Ԫ�صĴ�С,Ԫ�صĸ���,ָ�� FILE �����ָ��)
			size_t result = std::fwrite(buffer_, 1, static_cast<size_t>(current_ - buffer_), fp_);
			current_ = buffer_;
		}
	}

private:
	FILE* fp_;
	char* buffer_;
	char* bufferEnd_;
	char* current_;

};