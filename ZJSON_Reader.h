#pragma once
#include <ZJSON.h>
#define ISDIGIT(c) (c >= '0' && c <= '9')

class GenericReader {
public:
	GenericReader(int x) :id(x) {};
	template <typename InputStream,typename ValueContainer>
	void static Parse_Literal(InputStream& is, ValueContainer* v) {
		std::cout << "Parse_Literal" << std::endl;
		assert(is.Peek() == 'n' || is.Peek() == 't' || is.Peek() == 'f');
		if (is.Peek() == 'n')
		{
			is.Take();
			if (is.Take() == 'u' && is.Take() == 'l' && is.Take() == 'l')
			{
				(*v).SetType(JSON_NULL);
				return;
			}
			Error();
			assert(0);
		}
		if (is.Peek() == 't')
		{
			is.Take();
			if (is.Take() == 'r' && is.Take() == 'u' && is.Take() == 'e')
			{
				(*v).SetType(JSON_TRUE);
				return;
			}
			Error();
			assert(0);
		}
		if (is.Peek() == 'f')
		{
			is.Take();
			if (is.Take() == 'a' && is.Take() == 'l' && is.Take() == 's' && is.Take() == 'e')
			{
				(*v).SetType(JSON_FALSE);
				return;
			}
			Error();
			assert(0);
		}
	}
	template <typename InputStream, typename ValueContainer>
	void static Parse_Number(InputStream& is, ValueContainer* v) {
		std::cout << "Parse_Number" << std::endl;
		//char* end;
		/* \TODO validate number */
		//int now = 0;
		bool Sign = false;
		char pre;
		if (is.Peek() == '-')
		{
			pre = is.Take();
			Sign = true;
		}
		else
		{
			if (!ISDIGIT(is.Peek()))
				return ;
			else if (is.Peek() == '0')
			{
				pre = is.Take();
				if(is.Peek() != '.' && is.Peek() != 'e' && is.Peek() != 'E' && is.Peek() != '\0')
					return ;
			}
		}
		
		int NowInt = 0;
		unsigned NowUnsignedInt = 0;
		__int64 NowInt64 = 0;
		unsigned __int64 NowUnsignedInt64 = 0;
		double NowDouble = 0.0;

		int Digit = 0;//数位个数
		int VIS_E, VIS_DOT;
		VIS_E = 0;//e或E之后的数字个数
		VIS_DOT = 0;//小数点后的数字个数
		enum NumberType {
			IntFlag = 0,UnsignedIntFlag,Int64Flag,UnsignedInt64Flag,DoubleFlag
		};
		NumberType NowType = IntFlag;
		int ExpSign = 0;//e的次数的正负，1为负数，0为正数
		int ExpNumber = 0;
		while (is.Peek() != '\0')
		{
			if (!ISDIGIT(is.Peek()))
			{
				if ((is.Peek() == 'e' || is.Peek() == 'E'))
				{
					if (VIS_E == 0)
					{
						VIS_E = 1;
						switch (NowType)
						{
							case IntFlag:NowDouble = static_cast<double>(NowInt); break;
							case UnsignedIntFlag:NowDouble = static_cast<double>(NowUnsignedInt); break;
							case Int64Flag:NowDouble = static_cast<double>(NowInt64); break;
							case UnsignedInt64Flag:NowDouble = static_cast<double>(NowUnsignedInt64); break;
							default:break;
						}
						NowType = DoubleFlag;
					}
					else {
						assert(0);
						return;
					}
				}
				else if (is.Peek() == '.')
				{
					if (VIS_E)
					{
						assert(0);
						return ;
					}
					else if (VIS_DOT == 0)
					{
						VIS_DOT = 1;
						switch (NowType)
						{
						case IntFlag:NowDouble = static_cast<double>(NowInt); break;
						case UnsignedIntFlag:NowDouble = static_cast<double>(NowUnsignedInt); break;
						case Int64Flag:NowDouble = static_cast<double>(NowInt64); break;
						case UnsignedInt64Flag:NowDouble = static_cast<double>(NowUnsignedInt64); break;
						default:break;
						}
						NowType = DoubleFlag;
					}
					else
					{
						assert(0);
						return ;
					}
				}
				else if (is.Peek() == '+' || is.Peek() == '-')
				{
					if (!(pre == 'e' || pre == 'E'))
						return ;
					if (is.Peek() == '-')
						ExpSign = 1;
				}
				else
					return ;
			}
			else
			{
				if (VIS_E) {
					ExpNumber = ExpNumber * 10 + (is.Peek() - '0');
					VIS_E++;
				}
				else
				{
					if (VIS_DOT)
						VIS_DOT++;
					Digit++;
					if (Digit > 20)
					{
						assert(0);
						return ;
						//数据范围不够
					}
					//if（NowType == DoubleFlag && Digit > 16）
					//{
					//	assert(0);
					//	return ;
					//	//精度不够
					//}
					switch (NowType) {
					case IntFlag: {
						//std::cout <<"int: " << NowInt << std::endl;
						if (Sign)
						{
							if (NowInt > 214748364 || (NowInt == 214748364 && is.Peek() > '8'))
							{
								NowInt64 = static_cast<__int64>(NowInt) * 10 + (is.Peek() - '0');
								NowType = Int64Flag;
							}
							else
								NowInt = NowInt * 10 + (is.Peek() - '0');
						}
						else {
							//Int转化为UnsignedInt
							if (NowInt > 214748364 && NowInt < 429496729 || (NowInt == 214748364 && is.Peek() >= '8') || (NowInt == 429496729 && is.Peek() <= '5'))
							{
								NowUnsignedInt = static_cast<unsigned>(NowInt) * 10 + (is.Peek() - '0');
								NowType = UnsignedIntFlag;
							}
							//Int转化为_Int64
							else if (NowInt > 214748364)
							{
								NowInt64 = static_cast<__int64>(NowInt) * 10 + (is.Peek() - '0');
								NowType = Int64Flag;
							}
							else
								NowInt = NowInt * 10 + (is.Peek() - '0');
						}
						break;
					}
					case UnsignedIntFlag: {
						//std::cout << "unsigned: " << NowUnsignedInt << std::endl;
						if (Sign)
							assert(0);
						if (NowUnsignedInt > 429496729 || (NowUnsignedInt == 429496729 && is.Peek() >= '6')) {
							NowInt64 = static_cast<__int64>(NowUnsignedInt) * 10 + (is.Peek() - '0');
							NowType = Int64Flag;
						}
						else
							NowUnsignedInt = NowUnsignedInt * 10 + (is.Peek() - '0');
						break;
					}
					case Int64Flag: {
						//std::cout << "__int64: " <<NowInt64 << std::endl;
						if (Sign)
						{
							if (NowInt64 > 922337203685477580 || (NowInt64 == 922337203685477580 && is.Peek() > '8'))
							{
								NowDouble = static_cast<double>(NowInt64) * 10 + (is.Peek() - '0');
								NowType = DoubleFlag;
							}
							else
								NowInt64 = NowInt64 * 10 + (is.Peek() - '0');
						}
						else {
							//Int转化为UnsignedInt64
							if (NowInt64 > 922337203685477580 && NowInt64 < 1844674407370955161 || (NowInt64 == 922337203685477580 && is.Peek() >= '8') || (NowInt64 == 1844674407370955161 && is.Peek() <= '5'))
							{
								NowUnsignedInt64 = static_cast<unsigned __int64>(NowInt64) * 10 + (is.Peek() - '0');
								NowType = UnsignedInt64Flag;
							}
							//Int64转化为double
							else if (NowInt64 > 922337203685477580)
							{
								NowDouble = static_cast<double>(NowInt64) * 10 + (is.Peek() - '0');
								NowType = DoubleFlag;
							}
							else
								NowInt64 = NowInt64 * 10 + (is.Peek() - '0');
						}
						break;
					}
					case UnsignedInt64Flag: {
						//std::cout << "unsigned __int64: " << NowUnsignedInt64 << std::endl;
						if (Sign)
							assert(0);
						if (NowUnsignedInt64 > 1844674407370955161 || (NowUnsignedInt64 == 1844674407370955161 && is.Peek() >= '6')) {
							NowDouble = static_cast<double>(NowUnsignedInt64) * 10 + (is.Peek() - '0');
							NowType = DoubleFlag;
						}
						else
							NowUnsignedInt64 = NowUnsignedInt64 * 10 + (is.Peek() - '0');
						break;
					}
					case DoubleFlag:/*std::cout <<"double: "<<NowDouble <<std::endl;*/ NowDouble = NowDouble * 10.0 + (is.Peek() - '0'); break;
					}
				}
			}
			pre = is.Take();
		}
		//double 有效数字52位存储，指数十一位存储
		if (NowType == DoubleFlag)
		{
			if (ExpSign)
			{
				VIS_DOT += ExpNumber;
				VIS_DOT = 0;
			}
			for (int i = 1; i < VIS_DOT; i++)
				NowDouble = NowDouble / 10.0;
			//std::cout << VIS_E << std::endl;
			for (int i = 1; i <= ExpNumber; i++)
				NowDouble = NowDouble * 10.0;
		}
		if (VIS_DOT == 1 || VIS_E == 1)
			return ;
		switch (NowType)
		{
			case IntFlag:(*v).SetDataInt(NowInt); break;
			case UnsignedIntFlag:(*v).SetDataUnsignedInt(NowUnsignedInt); break;
			case Int64Flag:(*v).SetDataInt64(NowInt64); break;
			case UnsignedInt64Flag:(*v).SetDataUnsignedInt64(NowUnsignedInt64); break;
			case DoubleFlag:(*v).SetDataDouble(NowDouble); break;
			default:
				break;
		}
		/*
		if (!(*v).DataTest())
			//return LEPT_PARSE_NUMBER_TOO_BIG;
			return;*/
		//std::cout << "T" << std::endl;
		(*v).SetType(JSON_NUMBER);
		return ;
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
		case 'n':Parse_Literal<InputStream, ValueContainer>(is, v); break;
		case 't':Parse_Literal<InputStream, ValueContainer>(is, v); break;
		case 'f':Parse_Literal<InputStream, ValueContainer>(is, v); break;
		case '"':Parse_String<InputStream, ValueContainer>(is, v); break;
		case '{':Parse_Object<InputStream, ValueContainer>(is, v); break;
		case '[':Parse_Array<InputStream, ValueContainer>(is, v); break;
		default:Parse_Number<InputStream, ValueContainer>(is, v); break;
		}
	}
	template <typename InputStream>
	void static ParseTest() { std::cout << "Pares Start" << std::endl; }
private:
	int id;
};