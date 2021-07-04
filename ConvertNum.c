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
			if (raw[i] == '0') {
				check +=1;
			}
		}
		if (check == len) {
			rc = 2;
		}
		return rc;
}

void conversion(sds in, sds* out) {
	int i,
		rc,
		rem,
		start = 0,
		weight,
		finish = 2;
	sds clone;
	sdstrim(in," \n");
	if (in[0] == '-') { // check if the number is <0
		sdstrim(in, "-");
		*out = sdscat(*out, SIGN[1]);
	}

		weight = (((strlen(in) / 3) + (strlen(in) % 3 ? 1 : 0))) - 1;
		rem = strlen(in) % 3;
		finish = 2;
		do{

			if (rem) {
				finish = rem - 1;
				
			}
			clone = sdsnew(in, sdslen(in));
			sdsrange(clone , start, finish);

			rc = trio_translate(clone, out);
			*out = sdscat(*out, " ");
			if (rc != 2) {
				*out = sdscat(*out, MULT[weight]);
			}
			//('S)
			*out = sdscat(*out, " ");
			if (rem) {
				start = rem;
				finish = start + 3;
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

	
int main()
{
	HANDLE hStdout;
	long input,
	   rc = 0;
	char buff[MAX_LEN] = { 0 };
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	printf("This program translates decimal numbers into a string."\
		"Input the number in range of 32 bits...\n");
		do {
			printf("Awaiting input...\n");
			rc = scanf_s("%d", &input);
			if (input < -2147483646 || input > 2147483647) {
				printf("Input exceeds 32 bit number.");
				rc = 0;
				break;
			}
			if (rc == 1) {

				sprintf(buff, "%d", input);
				sds str = sdsnew(buff, strlen(buff));
				sds proc = sdsnew(""); // sending empty string
				conversion(str, &proc);
				cls(hStdout);
				printf("%d -> %s\n", input, proc);
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

