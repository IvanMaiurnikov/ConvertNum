#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <ConvertNum.h>
#include <windows.h>
#ifdef _UNIT_TEST
#include "CUnit/Basic.h"
#include "UnitTest.h"
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

/**
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

/** *********************************************************************************
* @NAME  void conversion(sds in, sds* out)
* @DESCR decides how many parts to cut the sds in string to and sends them to trio_translate.
*
* @ARGS sds in - gathered from main str, contains numbers in char format.
*       sds * out - acts as a buffer that holds the translated sds in.
* @RET  void
*
********************************************************************************* */

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
int input_and_validate(int *input, char *in_str) {
	int       rc = 0,
		      i;
	long long ll_input;

	if (strlen(in_str) > 12) {
		rc = -3;
		return rc;
	}
	for (i = 0; i < strlen(in_str); i++) {
		if (in_str[i] != '-' && (in_str[i] < '0' || in_str[i] > '9')) {
			rc = -1;
			break;
		}
	}

	if (i == strlen(in_str)) {
		ll_input = atoll(in_str);
		if ((ll_input < LONG_MIN) || (ll_input > LONG_MAX)) {
			rc = -2;
		}
		else {
			*input = (int)ll_input;
			rc = 0;
		}
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
			rc = scanf("%11s", buff);
			rc = input_and_validate(&input, buff);
			if (rc == 0) {
				sprintf(buff, "%d", input);
				sds str = sdsnewlen(buff, strlen(buff));
				sds proc = sdsnew(""); // sending empty string
				conversion(str, &proc);
				cls(hStdout);
				printf("%d -> %s\n", input, proc);
				sdsfree(str);
				sdsfree(proc);
				fflush(stdin);
			}
			else {
				printf("Wrong value entered");
				break;
			}
		} while (1);
		cls(stdout);
}
#else
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
		|| (NULL == CU_add_test(pSuite, "test of input_and_validate() with typo in the input ( 23a14 )", test_input_and_validate_typo))
		|| (NULL == CU_add_test(pSuite, "test of input_and_validate() with minimal acceptable value ( -2147483648 )", test_input_and_validate_min))
		|| (NULL == CU_add_test(pSuite, "test of input_and_validate() with maximum acceptable value ( 2147483647 )", test_input_and_validate_max))
		|| (NULL == CU_add_test(pSuite, "test of input_and_validate() with value > max ( 21474836470  )", test_input_and_validate_overmax))
		|| (NULL == CU_add_test(pSuite, "test of input_and_validate() with value < min ( -21474836480  )", test_input_and_validate_overmin))
		|| (NULL == CU_add_test(pSuite, "test of input_and_validate() with long string", test_input_and_validate_longstr))
		
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

