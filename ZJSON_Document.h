#pragma once
#include <ZJSON_Reader.h>

enum DataType {
	JSON_NULL,JSON_TRUE,JSON_FALSE,JSON_NUMBER,JSON_STRING,JSON_ARRAY,JSON_OBJECT
};
template <typename Encoding>
class BasicValue {
public:
	typedef typename Encoding Ch;
	BasicValue() :type_(JSON_NULL){}
	union Data {
		class _Int {
			int Number;
			char s[8];
		};
		class _Int64 {
			_Int64 Number;
			char s[4];
		};
		class _Double {
			double Number;
			char s[4];
		};
		class ArrayData {
			size_t ArraySize;
			size_t Capacity;
			BasicValue<Encoding>* Element;
		};
		class ObjectData {
			size_t ObjectSize;
			size_t Capacity;
			BasicValue<Encoding>* Member;
		};
		class String {
			size_t Length;
			Ch* str;
			char s[4];
		};
	};
private:
	Data data_;
	DataType type_;
};
template <typename InputStream,typename Encoding>
class BasicDocument
{
public:
	BasicDocument(InputStream& is) : is_(is){};
	template <typename Reader = GenericReader>
	void Parse() {
		typename Reader::template Parse<InputStream,BasicValue<Encoding> >(is_,&root);
	}
private:
	InputStream is_;
	BasicValue<Encoding> root;
};