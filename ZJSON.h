#pragma once
#include <cstdio>
#define	Stream_Test 0
#define Parse_Test 1
#define  Error()\
	do {\
		fprintf(stderr, "%s:%d: \n", __FILE__, __LINE__); \
	} while (0);
enum DataType {
	JSON_NULL, JSON_TRUE, JSON_FALSE, JSON_NUMBER, JSON_STRING, JSON_ARRAY, JSON_OBJECT,UNKNOWN_TYPE
};
