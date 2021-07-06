#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <ConvertNum.h>
#include <windows.h>
#ifdef _UNIT_TEST

#include "CUnit/Basic.h"
#include "UnitTest.h"

/* Pointer to the file used by the tests. */
static FILE* temp_file = NULL;

int init_suite1(void)
{
	if (NULL == (temp_file = fopen("temp.txt", "w+"))) {
		return -1;
	}
	else {
		return 0;
	}
}

int clean_suite1(void)
{
	if (0 != fclose(temp_file)) {
		return -1;
	}
	else {
		temp_file = NULL;
		return 0;
	}
}

void test_trio_translate_zero(void) {
	sds in = sdsnew("0");
	sds out = sdsnew("");
	if (NULL != temp_file) {
		CU_ASSERT(2 == trio_translate(in, &out));
	}
	sdsfree(in);
	sdsfree(out);
}

void test_trio_translate_extra(void)
{
	sds in = sdsnew("12345");
	sds out = sdsnew("");
	if (NULL != temp_file) {
		CU_ASSERT(0 < trio_translate(in, &out));
		CU_ASSERT(0==strcmp(out, "one hundred twenty three"));
	}
	sdsfree(in);
	sdsfree(out);
}

void test_convert_negative(void) {
	sds in = sdsnew("-12345");
	sds out = sdsnew("");
	if (NULL != temp_file) {
		conversion(in, &out);
		CU_ASSERT(0==strcmp(out, "minus twelve thousand three hundred fourty five"));
	}
	sdsfree(in);
	sdsfree(out);
}

void test_convert_min(void) {
	sds in = sdsnew("-2147483648");
	sds out = sdsnew("");
	if (NULL != temp_file) {
		conversion(in, &out);
		CU_ASSERT(0==strcmp(out, "minus two billion one hundred fourty seven million four hundred eighty three thousand six hundred fourty eight"));
	}
	sdsfree(in);
	sdsfree(out);
}

void test_convert_max(void) {
	sds in = sdsnew("2147483647");
	sds out = sdsnew("");
	if (NULL != temp_file) {
		conversion(in, &out);
		CU_ASSERT(0 == strcmp(out, "two billion one hundred fourty seven million four hundred eighty three thousand six hundred fourty seven"));
	}
	sdsfree(in);
	sdsfree(out);
}

void test_convert_overmax(void) {
	sds in = sdsnew("2147483647000");
	sds out = sdsnew("");
	if (NULL != temp_file) {
		conversion(in, &out);
		CU_ASSERT(0 == strcmp(out, "error"));
	}
	sdsfree(in);
	sdsfree(out);
}

void test_convert_zero(void) {
	sds in = sdsnew("0");
	sds out = sdsnew("");
	if (NULL != temp_file) {
		conversion(in, &out);
		CU_ASSERT(0 == strcmp(out, "zero"));
	}
	sdsfree(in);
	sdsfree(out);
}

void test_input_and_validate_typo(void) {
	char buf[8];
	int input,
		rc;
	sprintf(buf, "23a14");
	rc = input_and_validate(&input, buf);
	if (NULL != temp_file) {
		CU_ASSERT(-1 == rc);
	}
}

void test_input_and_validate_min(void) {
	char buf[32];
	int input,
		rc;
	sprintf(buf, "-2147483648");
	rc = input_and_validate(&input, buf);
	if (NULL != temp_file) {
		CU_ASSERT(0 == rc);
	}
}

void test_input_and_validate_max(void) {
	char buf[32];
	int input,
		rc;
	sprintf(buf, "2147483647");
	rc = input_and_validate(&input, buf);
	if (NULL != temp_file) {
		CU_ASSERT(0 == rc);
	}
}
void test_input_and_validate_overmax(void) {
	char buf[32];
	int input,
		rc;
	sprintf(buf, "21474836470");
	rc = input_and_validate(&input, buf);
	if (NULL != temp_file) {
		CU_ASSERT(-2 == rc);
	}
}

void test_input_and_validate_overmin(void) {
	char buf[32];
	int input,
		rc;
	sprintf(buf, "-21474836480");
	rc = input_and_validate(&input, buf);
	if (NULL != temp_file) {
		CU_ASSERT(-2 == rc);
	}
}

void test_input_and_validate_longstr(void) {
	char buf[32];
	int input,
		rc;
	sprintf(buf, "123412341234123455");

	rc = input_and_validate(&input, buf);
	if (NULL != temp_file) {
		printf("( %s ) ", buf);
		CU_ASSERT(-3 == rc);
	}
}

#endif