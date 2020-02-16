#pragma once
#include <ZJSON.h>
#include <ZJSON_Stack.h>
#include <ZJSON_Encoding.h>
#include < nmmintrin.h >
#define ISDIGIT(c) (c >= '0' && c <= '9')
#define ISHEX(c) ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))

class GenericReader {
public:
	GenericReader(int x) :id(x) {};
	//解析NULL、TRUE、FALSE
	template <typename InputStream,typename ValueContainer>
	int static Parse_Literal(InputStream& is, ValueContainer* v) {
		std::cout << "Parse_Literal" << std::endl;
		assert(is.Peek() == 'n' || is.Peek() == 't' || is.Peek() == 'f');
		if (is.Peek() == 'n')
		{
			is.Take();
			if (is.Take() == 'u' && is.Take() == 'l' && is.Take() == 'l')
			{
				(*v).SetType(JSON_NULL);
				return PARSE_SUCCESSFUL;
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
				return PARSE_SUCCESSFUL;
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
				return PARSE_SUCCESSFUL;
			}
			Error();
			assert(0);
		}
		return UNKNOWN_DATA_ERROR;
	}
	//解析数字
	template <typename InputStream, typename ValueContainer>
	int static Parse_Number(InputStream& is, ValueContainer* v) {
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
				return UNKNOWN_DATA_ERROR;
			else if (is.Peek() == '0')
			{
				pre = is.Take();
				if(is.Peek() != '.' && is.Peek() != 'e' && is.Peek() != 'E' && is.Peek() != '\0')
					return NUMBER_FORMAT_ERROR;
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
						return UNKNOWN_DATA_ERROR;
					}
				}
				else if (is.Peek() == '.')
				{
					if (VIS_E)
					{
						assert(0);
						return NUMBER_FORMAT_ERROR;
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
						return UNKNOWN_DATA_ERROR;
					}
				}
				else if (is.Peek() == '+' || is.Peek() == '-')
				{
					if (!(pre == 'e' || pre == 'E'))
						return UNKNOWN_DATA_ERROR;
					if (is.Peek() == '-')
						ExpSign = 1;
				}
				else
					return UNKNOWN_DATA_ERROR;
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
						return NUMBER_RANGE_ERROR;
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
			if (Digit > 16)
				return NUMBER_RANGE_ERROR;
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
			return NUMBER_FORMAT_ERROR;
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
		return PARSE_SUCCESSFUL;
	}
	template <typename InputStream>
	bool static ParseHex4(InputStream& is, unsigned *u)
	{
		unsigned cnt = 0;
		unsigned u1 = 0;
		while (is.Peek() != '\0')
		{
			if (is.Peek() == '\\')
				break;
			if (!ISHEX(is.Peek()))
				break;
			if (is.Peek() >= 'a')
				u1 = u1 * 16 + (is.Peek()) - 'a' + 10;
			else if (is.Peek() >= 'A')
				u1 = u1 * 16 + (is.Peek()) - 'A' + 10;
			else
				u1 = u1 * 16 + (is.Peek()) - '0';
			//printf("%d\n", u1);
			is.Take();
			cnt++;
		}
		if (cnt < 4)
			return false;
		*u = u1;
		return true;
	}
	//解析字符串
	template <typename InputStream, typename ValueContainer,typename StackType>
	int static Parse_String(InputStream& is, ValueContainer* v,StackType &stack_) {
		std::cout << "Parse_String" << std::endl;
		int head = stack_.GetSize();
		size_t  len;
		unsigned u, H, L;
		H = 0;
		assert(is.Take() == '\"');
		//cout << head << endl;
		//std::cout << "T" << std::endl;
		for (;;) {
			char ch = is.Take();
			switch (ch) {
			case '\"':
				//cout << "T" << endl;
				len = static_cast<size_t>((stack_.GetSize() - head));
				std::cout << len << std::endl;
				(*v).SetType(JSON_STRING);
				(*v).SetString<CrtAllocator>(stack_.template Pop<char>(len),len,stack_.GetAllocator());
				return PARSE_SUCCESSFUL;
			case '\\':
				switch (is.Take()) {
				case '\"': *stack_.template Push<char>() = '\"'; break;
				case '\\': *stack_.template Push<char>() = '\\'; break;
				case '/':  *stack_.template Push<char>() = '/'; break;
				case 'b':  *stack_.template Push<char>() = '\b'; break;
				case 'f':  *stack_.template Push<char>() = '\f'; break;
				case 'n':  *stack_.template Push<char>() = '\n'; break;
				case 'r':  *stack_.template Push<char>() = '\r'; break;
				case 't':  *stack_.template Push<char>() = '\t'; break;
				case 'u':
					if (!(ParseHex4<InputStream>(is,&u)))
						//STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
						return UNICODE_HEX_ERROR;
					if (u >= 0xD800 && u <= 0xDBFF)
					{
						//printf("TT\n");
						H = u;
						if (is.Take() == '\\' && is.Take() == 'u')
						{
							if (!(ParseHex4<InputStream>(is, &L)))
								return UNICODE_HEX_ERROR;
							if (L < 0xDC00 || L > 0xDFFF)
								return UNICODE_SURROGATE_ERROR;
							u = 0x10000 + (H - 0xD800) * 0x400 + (L - 0xDC00);
							H = 0;
							UTF8<>::Encode<StackType>(u,stack_);
								break;
						}
						else
							return UNICODE_SURROGATE_ERROR;
					}
					UTF8<>::Encode<StackType>(u,stack_);
					break;
				default:
					return STRING_ESCAPE_ERROR;
				}
				break;
			case '\0':
				return MISS_QUOTATION_MARK;
			default:
				if ((unsigned char)ch < 0x20)
					return UNKNOWN_CHAR_ERROR;
				*stack_.template Push<char>() = ch;
			}
		}
	}
	template <typename InputStream, typename ValueContainer, typename StackType>
	int static Parse_Array(InputStream& is, ValueContainer* v,StackType &stack_) {
		std::cout << "Parse_Array" << std::endl;
		assert(is.Take() == '[');
		SkipWhiteSpace<InputStream>(is);
		while (1)
		{
			new (stack_.template Push<ValueContainer> ()) ValueContainer();
			Parse<InputStream, ValueContainer, StackType>(is, stack_.template Top<ValueContainer>(), stack_);
			(*v).AddElement<CrtAllocator>(stack_.template Pop<ValueContainer>(1), stack_.GetAllocator());
			SkipWhiteSpace<InputStream>(is);
			if (is.Peek() == ',')
				SkipWhiteSpace<InputStream>(is);
			else if (is.Peek() == ']')
			{
				//(*v).SetArrayEnd();
				return PARSE_SUCCESSFUL;
			}
			else
				return PARESE_ARRAY_ESCAPE_ERROR;
		}
	}
	template <typename InputStream, typename ValueContainer,typename StackType>
	int static Parse_Object(InputStream& is, ValueContainer* v,StackType &stack_) {
		std::cout << "Parse_Object" << std::endl;
		assert(is.Take() == '{');
		SkipWhiteSpace<InputStream>(is);
		while (1)
		{
			new (stack_.template Push<ValueContainer>()) ValueContainer();
			Parse<InputStream, ValueContainer, StackType>(is, stack_.template Top<ValueContainer>(), stack_);
			(*v).AddObjectName<CrtAllocator>(stack_.template Pop<ValueContainer>(1), stack_.GetAllocator());
			SkipWhiteSpace<InputStream>(is);
			if (is.Peek() == ':')
				SkipWhiteSpace<InputStream>(is);
			else
				return UNKNOWN_DATA_ERROR;
			new (stack_.template Push<ValueContainer>()) ValueContainer();
			Parse<InputStream, ValueContainer, StackType>(is, stack_.template Top<ValueContainer>(), stack_);
			(*v).AddObjectValue<CrtAllocator>(stack_.template Pop<ValueContainer>(1), stack_.GetAllocator());
			SkipWhiteSpace<InputStream>(is);
			if (is.Peek() == ',')
				SkipWhiteSpace<InputStream>(is);
			else if (is.Peek() == ']')
			{
				//(*v).SetArrayEnd();
				return PARSE_SUCCESSFUL;
			}
			else
				return PARESE_ARRAY_ESCAPE_ERROR;
		}
	}

	template <typename InputStream>
	void static SkipWhiteSpace(InputStream &is)
	{
		while (is.Peek() == ' ' || is.Peek() == '\t' || is.Peek() == '\n' || is.Peek() == '\r')
			is.Take();
	}
	//用来处理中间部分字符串片段全为空白字符情况
	inline const char* SkipWhitespace(const char* p, const char* end) {
		while (p != end && (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t'))
			++p;
		return p;
	}
	/*----------------SSE4.2-------------------------------*/
	inline const char* SkipWhitespace_SIMD(const char* p)
	{
		// 快速返回（第一个即为非空白字符，）
		if (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t')
			++p;
		else
			return p;
		// 对齐处理（16字节）,c
		const char* nextAligned = reinterpret_cast<const char*>((reinterpret_cast<size_t>(p) + 15)& static_cast<size_t>(~15));
		while (p != nextAligned) {
			if (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t')
				++p;
			else
				return p;
		}
		static const char whitespace[16] = " \n\r\t";
		const __m128i w = _mm_load_si128((const __m128i*) & whitespace[0]);// _mm_load_si128（）加载128位值（16字节）
		for (;; p += 16) {
			const __m128i s = _mm_load_si128((const __m128i*)p);
			/*
				利用_mm_cmpistrm(a,b,c)将w（a）与s（b）进行对比，c为一个常量，包含以下：指示字符是字节还是单词，要做的比较的类型，
			以及返回值的格式。
				名词解释：_SIDD_UBYTE_OPS: 操作单位是无号字节，即16个 unsigned char。
						  _SIDD_CMP_EQUAL_ANY: 每次比较 s 里的字符，是否和 w 中的任意字符相等。
						  _SIDD_LEAST_SIGNIFICANT返回设置为1的最右边位的索引_
						  _SIDD_NEGATIVE_POLARITY: 把结果反转。这里指返回值的1代表非空白字符。
				即将16字节字符串与空白字符进行对比，非空白字符为1，空白字符为0。返回最右边位（第一位）非空白字符位置

			*/
			const int r = _mm_cmpistri(w, s, _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_ANY | _SIDD_LEAST_SIGNIFICANT | _SIDD_NEGATIVE_POLARITY);
			//如果r!=16，意味着存在非0字符，将指针移到相应位置返回。
			if (r != 16) {
				return p + r;
			}
		}
	}
	//处理中间字符串片段（含end结尾）
	inline const char* SkipWhitespace_SIMD(const char* p, const char* end)
	{
		// 快速返回（第一个即为非空白字符，）
		if (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t')
			++p;
		else
			return p;
		//中间片段处理因为有界所以无需进行对齐处理	
		//处理同上
		static const char whitespace[16] = " \n\r\t";
		const __m128i w = _mm_load_si128((const __m128i*) & whitespace[0]);
		for (; p <= end - 16; p += 16)
		{
			const __m128i s = _mm_loadu_si128(reinterpret_cast<const __m128i*>(p));
			const int r = _mm_cmpistri(w, s, _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_ANY | _SIDD_LEAST_SIGNIFICANT | _SIDD_NEGATIVE_POLARITY);
			//含有非空白字符
			if (r != 16)
				return p + r;
		}
		//若循环完毕并未返回，则说明全为空白字符，p指针不动，需要调用SkipWhitespace函数移动指针
		return SkipWhitespace(p, end);
	}
	template <typename InputStream, typename ValueContainer,typename StackType>
	int static ParseValue(InputStream& is,ValueContainer * v,StackType &stack_) {
		switch (is.Peek())
		{
		case 'n':return Parse_Literal<InputStream, ValueContainer>(is, v);
		case 't':return Parse_Literal<InputStream, ValueContainer>(is, v);
		case 'f':return Parse_Literal<InputStream, ValueContainer>(is, v);
		case '"':return Parse_String<InputStream, ValueContainer,StackType>(is, v,stack_);
		case '{':return Parse_Object<InputStream, ValueContainer>(is, v,stack_);
		case '[':return Parse_Array<InputStream, ValueContainer>(is, v,stack_);
		default:return Parse_Number<InputStream, ValueContainer>(is, v);
		}
	}
	template <typename InputStream, typename ValueContainer, typename StackType>
	int static Parse(InputStream& is, ValueContainer* v, StackType& stack_) {
		SkipWhiteSpace(is);
		//if (ParseValue<InputStream, ValueContainer, StackType>(is, v, stack_) == PARSE_SUCCESSFUL) {
		//	SkipWhiteSpace<InputStream>(is);
		//	if (is.Peek() != '\0')
		//	{
		//		(*v).SetType(JSON_NULL);
		//		return 
		//	}
		//}
		return ParseValue<InputStream, ValueContainer, StackType>(is, v, stack_);
	}
	template <typename InputStream>
	void static ParseTest() { std::cout << "Pares Start" << std::endl; }
private:
	int id;
};