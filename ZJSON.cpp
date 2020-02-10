#include <ZJSON_Stream.h>

template <typename TestStream>
void Test(TestStream& stream)
{
	using namespace std;
	//cout << stream.Tell() << endl;
	cout << stream.Take() << endl;
	cout << stream.Peek() << endl;
	cout << stream.Peek() << endl;
	cout << stream.Tell() << endl;
	cout << "Test End" << endl;
}
void Test1(std::ifstream&& is)
{
	char s[10];
	is.read(s, std::streamsize(sizeof(s)));
}
void Test2(FileReadStream<char>& file_is)
{
	/*char s[10];
	char* s1;
	std::cin >> s;
	s1 = s;
	while (*s1 != 'x')
		s1++;
	std::cout << static_cast<size_t>(s1 - s) << std::endl;*/
	while (file_is.Peek() != '\0')
	{
		std::cout << file_is.Take() << std::endl;
		getchar();
	}
}
int main()
{
	char s[10];
	std::cin >> s;
	InStream<char*, char> test1(s);
	Test<InStream<char*,char>>(test1);
	std::stringstream is1(s);
	//std::ifstream is("json.txt");
	//Test1(is);
	//ifstream没法复制，只能移动构造，无法作为赋值构造
	//is.peek();
	std::ifstream is2("json.txt");
	InStreamWrapper<char> String_is(is1);
	Test<InStreamWrapper<char>>(String_is);
	//delete& is;
	InStreamWrapper<char> File_is(is2);
	Test<InStreamWrapper<char>>(File_is);
	FILE* fp;
	fp = fopen("JSON1.txt", "r");
	FileReadStream<char> file_is(fp);
	Test<FileReadStream<char>>(file_is);
	Test2(file_is);
}