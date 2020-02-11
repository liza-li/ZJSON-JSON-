#pragma once
#include <ZJSON.h>

class GenericReader {
public:
	GenericReader(int x) :id(x) {};
	template <typename InputStream,typename ValueContainer>
	void static Parse_Literal(InputStream& is, ValueContainer* v) {
		std::cout << "Parse_Literal" << std::endl;
	}
	template <typename InputStream, typename ValueContainer>
	void static Parse_Number(InputStream& is, ValueContainer* v) {
		std::cout << "Parse_Number" << std::endl;
	}
	template <typename InputStream, typename ValueContainer>
	void static Parse_String(InputStream& is, ValueContainer* v) {
		std::cout << "Parse_String" << std::endl;
	}
	template <typename InputStream, typename ValueContainer>
	void static Parse_Array(InputStream& is, ValueContainer* v) {
		std::cout << "Parse_Array" << std::endl;
	}
	template <typename InputStream, typename ValueContainer>
	void static Parse_Object(InputStream& is, ValueContainer* v) {
		std::cout << "Parse_Object" << std::endl;
	}
	template <typename InputStream, typename ValueContainer>
	void static Parse(InputStream& is,ValueContainer * v) {
		switch (is.Peek())
		{
		case 'n':Parse_Literal<InputStream,ValueContainer>(is, v);
		case 't':Parse_Literal<InputStream, ValueContainer>(is, v);
		case 'f':Parse_Literal<InputStream, ValueContainer>(is, v);
		case '"':Parse_String<InputStream, ValueContainer>(is, v);
		case '{':Parse_Object<InputStream, ValueContainer>(is, v);
		case '[':Parse_Array<InputStream, ValueContainer>(is, v);
		default:Parse_Number<InputStream, ValueContainer>(is,v);
		}
	}
	template <typename InputStream>
	void static ParseTest() { std::cout << "Pares Start" << std::endl; }
private:
	int id;
};