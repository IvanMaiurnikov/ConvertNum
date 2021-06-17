#include <stdio.h>
#include <stdlib.h>
#include <sds.h>

#define MAX_LEN (12)

char* SINGLE_DIGIT[] = {"", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"};

char* TENTH_DIGIT[] = {"ten", "eleven", "twelth", "thirteen", "fourteen", "fifteen", "sixteen", "seventeen", "eighteen", "nineteen"};

char* DOUBLE_DIGIT[] = { "twenty", "thirty", "fourty", "fifty", "sixty", "seventy", "eighty", "ninety"};

char* MULT[] = {"", "ten", "hundred", "thousand", "million", "billion"};

char* SIGN[] = {"", "minus"};

int trio_translate(sds raw, sds* processed) {
	int i,
		rc = 0,
		len,
		adr;
	len = sdslen(raw);
	// test the values, change between sizes!!!!
		if (len > 2) {
			adr = raw[0] - '0';
			if (raw[0] != '0') {
				sdscat(*processed, SINGLE_DIGIT[adr]);
				sdscat(*processed, MULT[1]);
			}
			adr = raw[1] - '0';
				if (raw[1] == '1') {
					adr = raw[2] - '0';
					sdscat(*processed, TENTH_DIGIT[adr]);
				}
				if (raw[1] != 1) {
					sdscat(*processed, DOUBLE_DIGIT[adr]);
					adr = raw[2] - '0';
					sdscat(*processed, SINGLE_DIGIT[adr]);
				}
				rc = 1;
		}
		if (len < 3) {
			if (len == 2) {
				adr = raw[0] - '0';
				if (raw[0] == '1') {
					adr = raw[1] - '0';
					sdscat(*processed, TENTH_DIGIT[adr]);
				}
				if (raw[0] != '1') {
					sdscat(*processed, DOUBLE_DIGIT[adr]);
					adr = raw[1] - '0';
					sdscat(*processed, SINGLE_DIGIT[adr]);
				}
			}
			if (len == 1) {
				adr = raw[0] - '0';
				sdscat(*processed, SINGLE_DIGIT[adr]);
			}
			rc = 1;
		}
		return rc;
}

void conversion(sds in, sds* out) {
	int i,
		count,
		subcount,
		start = 0,
		weight,
		lose,
		finish = 0;
	sds* clone,
		sign_tokens,
		subtoken;
	sdstrim(in," \n");
	if (in[0] == '-') { // check if the number is <0
		sdstrim(in, "-");
		sdscat(*out, SIGN[1]);
	}

		weight = (((strlen(in) / 3) + (strlen(in) % 3 ? 1 : 0))) - 1;
		do {
			sds clone = sdsnew(in, sdslen(in));
			if (strlen(in) % 3 > 0) {
				lose = (strlen(in)) % 3;
				finish = (strlen(in) % 3) - 1;
				sdsrange(in, start, finish);
				trio_translate(in, &out);
				start = finish + 1;
				finish += finish + 3;
				weight -= weight - lose;
			}
			else {
				finish += 2;
			}
			
			weight = weight - 3;
		} while (weight); 
	
}

	int input_req() {
		int number;
		do {
			scanf("%d", &number);
			if (number < INT_MIN || number > INT_MAX) {
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
	sds proc = sdsnew("", 20); // sending empty string
	conversion(str, &proc);
}

