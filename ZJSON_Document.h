#pragma once
#include <ZJSON.h>
#include <ZJSON_Reader.h>


template <typename Encoding>
class BasicValue {
public:
	typedef typename Encoding Ch;
	BasicValue() :type_(JSON_NULL){}
	class Int {
	public:
		int Number;
		char s[8];
	};
	class UnsignedInt {
	public:
		unsigned Number;
		char s[8];
	};
	class Int64 {
	public:
		__int64 Number;
		char s[4];
	};
	class UnsignedInt64 {
	public:
		unsigned __int64 Number;
		char s[4];
	};
	class Double {
	public:
		double Number;
		char s[4];
	};
	class ArrayData {
	public:
		size_t ArraySize;
		size_t Capacity;
		BasicValue<Encoding>* Element;
	};
	class ObjectData {
	public:
		size_t ObjectSize;
		size_t Capacity;
		BasicValue<Encoding>* Member;
	};
	class String {
	public:
		size_t Length;
		Ch* str;
		char s[4];
	};
	union Data
	{
		Int _Int;
		UnsignedInt _UnsignedInt;
		Int64 _Int64;
		UnsignedInt64 _UnsignedInt64;
		Double _Double;
		ArrayData _ArrayData;
		ObjectData _ObjectData;
		String _String;
	};
	void SetType(DataType t){type_ = t;	}
	DataType GetType(){return type_;}
	void SetDataInt(int x) { /*std::cout << "int" <<  x << std::endl;*/ data_._Int.Number = x; }
	void SetDataUnsignedInt(unsigned x){ /*std::cout <<"unsigned" << x << std::endl;*/  data_._UnsignedInt.Number = x;}
	void SetDataInt64(__int64 x){ /*std::cout << "__int64"<<x << std::endl;*/  data_._Int64.Number = x;}
	void SetDataUnsignedInt64(unsigned __int64 x){ /*std::cout << "unsigned __int64"<< x << std::endl;*/  data_._UnsignedInt64.Number = x;}
	void SetDataDouble(double x){ /*std::cout << "double" << x << std::endl;*/  data_._Double.Number = x;}
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
	void TT()
	{
		root.TT();
	}
	void TypePut() {
		switch (root.GetType()) {
		case 0:std::cout << "JSON_NULL" << std::endl; break;
		case 1:std::cout << "JSON_TRUE" << std::endl; break;
		case 2:std::cout << "JSON_FALSE" << std::endl; break;
		case 3:std::cout << "JSON_NUMBER" << std::endl; break;
		case 4:std::cout << "JSON_STRING" << std::endl; break;
		case 5:std::cout << "JSON_ARRAY" << std::endl; break;
		case 6:std::cout << "JSON_OBJECT" << std::endl; break;
		default:std::cout << "UNKNOWN_TYPE" << std::endl;
		}
	}
private:
	InputStream is_;
	BasicValue<Encoding> root;
};