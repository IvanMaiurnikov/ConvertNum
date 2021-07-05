#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <ConvertNum.h>

#define MAX_LEN (13)


char* SINGLE_DIGIT[] = {"", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"};

char* TENTH_DIGIT[] = {"ten", "eleven", "twelth", "thirteen", "fourteen", "fifteen", "sixteen", "seventeen", "eighteen", "nineteen"};

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
*       len - represents the length of raw.
*       check - counter for 0s in raw, everytime 0 appear -> +1, if len == check -> 
*       -> rc = 2.
*       rc - return code.
*       adr - address gathered from converting raw into int, used for guiding 
*       function which string should be attached.
* 
* @RET rc = 0 if something goes wrong
*      rc = 1 the function has completed conversion, added strings to processed,
*      weight is applied to add MULT.
*      rc = 2 the function has completed conversion and found out there is no need,
*      to add any tring or to apply the weight for MULT.
*/
int trio_translate(sds raw, sds* processed) {
	int i,
		check,
		rc = 0,
		len,
		adr;
	len = sdslen(raw);
	// test the values, change between sizes!!!!
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
				*processed = sdscat(*processed, SINGLE_DIGIT[adr]);
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

		clone = sdsnew(in, sdslen(in));
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
}

/*
* @NAME  int main()
* @DESCR scanf &input gets the int number which then is checked for illegal
*        characters and overflow (if one of these accure -> break the cycle).
*        if check is succesful -> convert input to sds and proceed to 
*        conversion.
*/

int main()
{
	HANDLE hStdout;
	long long input,
	   rc = 0;
	char buff[MAX_LEN] = { 0 };
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	printf("This program translates decimal numbers into a string.\n"\
		"To exit the program input illegal character or number that exceeds 32 bit range.\n"\
		"Input the number in range of 32 bits...\n");
		do {
			printf("Awaiting input...\n");
			rc = scanf("%lld", &input);
			if (input < LONG_MIN || input > LONG_MAX) {
				printf("Input exceeds 32 bit number.");
				rc = 0;
				break;
			}
			if (rc == 1) {

				sprintf(buff, "%lld", input);
				sds str = sdsnew(buff, strlen(buff));
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
		printf("Unrecognized input found. Exitting programm...");
}

