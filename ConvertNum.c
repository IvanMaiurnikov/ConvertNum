#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <ConvertNum.h>

#ifdef _UNIT_TEST
#include "CUnit/Basic.h"
#endif

#define MAX_LEN (13)


char* SINGLE_DIGIT[] = {"", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"};

char* TENTH_DIGIT[] = {"ten", "eleven", "twelve", "thirteen", "fourteen", "fifteen", "sixteen", "seventeen", "eighteen", "nineteen"};

char* DOUBLE_DIGIT[] = {"", "", "twenty", "thirty", "fourty", "fifty", "sixty", "seventy", "eighty", "ninety"};

char* MULT[] = {"", "thousand", "million", "billion"};

char* SIGN[] = {"", "minus "};

void cls(HANDLE hConsole)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	SMALL_RECT scrollRect;
	COORD scrollTarget;
	CHAR_INFO fill;
	// Get the number of character cells in the current buffer.
	if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
	{
		return;
	}
	// Scroll the rectangle of the entire buffer.
	scrollRect.Left = 0;
	scrollRect.Top = 0;
	scrollRect.Right = csbi.dwSize.X;
	scrollRect.Bottom = csbi.dwSize.Y;
	// Scroll it upwards off the top of the buffer with a magnitude of the entire height.
	scrollTarget.X = 0;
	scrollTarget.Y = (SHORT)(0 - csbi.dwSize.Y);
	// Fill with empty spaces with the buffer's default text attribute.
	fill.Char.UnicodeChar = TEXT(' ');
	fill.Attributes = csbi.wAttributes;
	// Do the scroll
	ScrollConsoleScreenBuffer(hConsole, &scrollRect, NULL, scrollTarget, &fill);
	// Move the cursor to the top left corner too.
	csbi.dwCursorPosition.X = 0;
	csbi.dwCursorPosition.Y = 0;
	SetConsoleCursorPosition(hConsole, csbi.dwCursorPosition);
}

/*
* @NAME int trio_translate(sds raw, sds* processed)
* @DESCR Recieves a cut of sds string and assigns strings judging on  
*        symbol address.
* 
* @ARGS sds raw - gathered from conversion, contains numbers in char format.
*       sds * processed - on IN - empty string; on OUT - converted chars into string.
* @RET rc = 0 if something goes wrong
*      rc = 1 the function has completed conversion, added strings to processed,
*             weight is applied to add MULT.
*/
int trio_translate(sds raw, sds* processed) {
	int i,
		check,   // check - counter for 0s in raw, everytime 0 appear -> +1, if len == check -> 
                //-> rc = 2.
		rc = 0, // rc = 2 the function has completed conversion and found out there is no need,
	           //to add any tring or to apply the weight for MULT.
		len,    // len - represents the length of raw.
		adr;  //  adr - address gathered from converting raw into int, used for guiding 
	          //function which string should be attached.
	len = sdslen(raw);

		if (len > 2) {
			adr = raw[0] - '0';
			if (raw[0] != '0') {
				*processed = sdscat(*processed, SINGLE_DIGIT[adr]);
				*processed = sdscat(*processed, " hundred ");
			}
			adr = raw[1] - '0';
				if (raw[1] == '1') {
					adr = raw[2] - '0';
					*processed = sdscat(*processed, TENTH_DIGIT[adr]);
				}
				if (raw[1] != '1') {
					*processed = sdscat(*processed, DOUBLE_DIGIT[adr]);
					adr = raw[2] - '0';
					*processed = sdscat(*processed, " ");
					*processed = sdscat(*processed, SINGLE_DIGIT[adr]);
				}
				rc = 1;
		}
		if (len < 3) {
			if (len == 2) {
				adr = raw[0] - '0';
				if (raw[0] == '1') {
					adr = raw[1] - '0';
					*processed = sdscat(*processed, TENTH_DIGIT[adr]);
				}
				if (raw[0] != '1') {
					*processed = sdscat(*processed, DOUBLE_DIGIT[adr]);
					*processed = sdscat(*processed, " ");
					adr = raw[1] - '0';
					*processed = sdscat(*processed, SINGLE_DIGIT[adr]);
				}
			}
			if (len == 1) {
				adr = raw[0] - '0';
				if (raw[0] == '0') {
					*processed = sdscat(*processed, "zero");
				}
				else {
					*processed = sdscat(*processed, SINGLE_DIGIT[adr]);
				}
			}
			rc = 1;
		}
		check = 0;
		for (i = 0; i < len; i++) {
			check += raw[i] - '0';
		}
		if (check == 0) {
			rc = 2;
		}
		return rc;
}

/*
* @NAME  void conversion(sds in, sds* out)
* @DESCR decides how many parts to cut the sds in string to and sends them to trio_translate.
*
* @ARGS sds in - gathered from main str, contains numbers in char format.
*       sds * out - acts as a buffer that holds the translated sds in.
* @RET  void
*
*/

void conversion(sds in, sds* out) {
	int i,         
		rc,          // return code
		rem,         // holds address of end char.
		start = 0,   // address beginning.
		weight,      // length of sds in which decreases after each iteration, when reaches 0 - function ends.
		finish = 2;  // end of adress.
	sds clone;

	sdstrim(in," \n");
	if (in[0] == '-') { // check if the number is <0
		sdstrim(in, "-");
		*out = sdscat(*out, SIGN[1]);
	}

	weight = (((strlen(in) / 3) + (strlen(in) % 3 ? 1 : 0))) - 1;
	rem = strlen(in) % 3;
	do{

		if (rem) {
			finish = rem - 1;
		}

		clone = sdsnewlen(in, sdslen(in));
		sdsrange(clone , start, finish);
		rc = trio_translate(clone, out);
		//sdsfree(clone);
		if (rc != 2) {
			*out = sdscat(*out, " ");
			*out = sdscat(*out, MULT[weight]);
		}
		//('S)
		*out = sdscat(*out, " ");
		if (rem) {
			start = rem;
			finish = start + 2;
			rem = 0;
		}
		else
		{
			start += 3;
			finish += 3;
		}
		weight--;
		sdsfree(clone);
	} while (weight >= 0); 
	sdstrim(*out, " ");
}
/* ********************************************************************
 * @NAME  int input_and_validate(int *input)
 * @DESCR input value from a keyboard into pointer to "input" variable.
 *        Validate entered number to get 32-bit value
 * @ARGS  int *input - pointer to intever value to be entered;
 * @RET   int rc - 0 if error value entered, 1 if OK
 ******************************************************************** */
int input_and_validate(int *input) {
	int       rc = 0;
	long long ll_input;
	rc = scanf("%lld", &ll_input);
	if (rc) {
		if ((ll_input < LONG_MIN) || (ll_input > LONG_MAX)) {
			printf("Input exceeds 32 bit number. Exitting programm...");
			rc = 0;
		}
		else {
			*input = (int)ll_input;
		}
	}
	else {
		printf("Unrecognized input found. Exitting programm...");
	}
	return rc;
}

/*
* @NAME  int main()
* @DESCR scanf &input gets the int number which then is checked for illegal
*        characters and overflow (if one of these accure -> break the cycle).
*        if check is succesful -> convert input to sds and proceed to 
*        conversion.
*/

#ifndef _UNIT_TEST

int main()
{
	HANDLE hStdout;
	int       input,
	          rc   = 0;
	char      buff[MAX_LEN] = { 0 };
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	printf("This program translates decimal numbers into a string.\n"\
		"To exit the program input illegal character or number that exceeds 32 bit range.\n"\
		"Input the number in range of 32 bits...\n");
		do {
			printf("Awaiting input...\n");
			rc = input_and_validate(&input);
			if (rc == 1) {
				sprintf(buff, "%d", input);
				sds str = sdsnewlen(buff, strlen(buff));
				sds proc = sdsnew(""); // sending empty string
				conversion(str, &proc);
				cls(hStdout);
				printf("%lld -> %s\n", input, proc);
				sdsfree(str);
				sdsfree(proc);
				fflush(stdin);
			}
			else {
				break;
			}
		} while (1);
		cls(stdout);
}
#else
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

void test_input_and_validate_typo(void) {

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

int main()
{
	CU_pSuite pSuite = NULL;

	/* initialize the CUnit test registry */
	if (CUE_SUCCESS != CU_initialize_registry())
		return CU_get_error();
	/* add a suite to the registry */
	pSuite = CU_add_suite("ConvertNum application test suite", init_suite1, clean_suite1);
	if (NULL == pSuite) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	if (   (NULL == CU_add_test(pSuite, "test of trio_translate() with extra symbols in input string \"1234\"", test_trio_translate_extra))
		|| (NULL == CU_add_test(pSuite, "test of trio_translate() with zero input string", test_trio_translate_zero))
		|| (NULL == CU_add_test(pSuite, "test of convert() with negative input", test_convert_negative))
		|| (NULL == CU_add_test(pSuite, "test of convert() with minimal acceptable value ( -2147483648 )", test_convert_min))
		|| (NULL == CU_add_test(pSuite, "test of convert() with maximum acceptable value ( 2147483647 )", test_convert_max))
		|| (NULL == CU_add_test(pSuite, "test of convert() with zero input value ( 0 )", test_convert_zero))
		//|| (NULL == CU_add_test(pSuite, "test of convert() with value > max ( 21474836470  )", test_convert_overmax))
		

	   )
	{
		CU_cleanup_registry();
		return CU_get_error();
	}


	/* Run all tests using the CUnit Basic interface */
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();
	return CU_get_error();
}
#endif

