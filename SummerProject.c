#include <stdio.h>
#include <stdlib.h>
#include <sds.h>

#define MAX_LEN (12)

char* SIGNLE_DIGIT[] = {"", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"};

char* DOUBLE_DIGIT[] = {"ten", "eleven", "twelth", "thirteen", "fourteen", "fifteen", "sixteen", "seventeen", "eighteen", "nineteen", "twenty"};

char* MULT[] = {"hundred", "thousand", "million", "billion"};

char* SIGN[] = {"", "minus"};

void conversion(sds in, sds* out, int val) {
	if (val < 0) {
		out = sdscat(*out, "minus");
	}
	if (val / 1000000000 > 0) {
		out = sdscat(*out, "billion");

	}
}

	int input_req() {
		int number;
		do {
			scanf("%d", &number);
			if (number < sizeof(int) || number > sizeof(int)) {
				printf("Input exceeds 32 bit number. Please try again.");
				continue;
			}
			else {
				break;
			}// make an rc, if int fails send error and retry
		} while (1);              // if int< or >int -> error(type of error) try again
								  // if correct -> send int to conversion
		return number;						  // user can quit from this point too
	}

int main()
{
	int input;
	char buff[MAX_LEN] = { 0 };
   /*TO DO
   * -interchangable memory variable
   * -convert int to char.
   * -if number goes over 32 bits send error in main
   * - UI has to be appealing 
   * -clear UI after operation
   */
	printf("This program converts int number"\
           "into sequence of words."\
           "Input number:");
	input = input_req(); // make an rc, if int fails send error and retry 
	sprintf(buff, "%11d", input);
	sds str = sdsnew(buff, strlen(buff));
	sds proc = sdsnew("", MAX_LEN); // sending empty string
	conversion(str, proc, input);
}

