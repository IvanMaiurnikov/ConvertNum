#include <stdio.h>
#include <stdlib.h>
#include <sds.h>

#define MAX_LEN (13)

char* SINGLE_DIGIT[] = {"", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"};

char* TENTH_DIGIT[] = {"ten", "eleven", "twelth", "thirteen", "fourteen", "fifteen", "sixteen", "seventeen", "eighteen", "nineteen"};

char* DOUBLE_DIGIT[] = {"", "", "twenty", "thirty", "fourty", "fifty", "sixty", "seventy", "eighty", "ninety"};

char* MULT[] = {"", "thousand", "million", "billion"};

char* SIGN[] = {"", "Minus "};

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
				*processed = sdscat(*processed, SINGLE_DIGIT[adr]);
				*processed = sdscat(*processed, " Hundred ");
			}
			adr = raw[1] - '0';
				if (raw[1] == '1') {
					adr = raw[2] - '0';
					*processed = sdscat(*processed, TENTH_DIGIT[adr]);
				}
				if (raw[1] != 1) {
					*processed = sdscat(*processed, DOUBLE_DIGIT[adr]);
					adr = raw[2] - '0';
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
		return rc;
}

void conversion(sds in, sds* out) {
	int i,
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

			trio_translate(clone, out);
			*out = sdscat(*out, " ");
			*out = sdscat(*out, MULT[weight]);
			//обробка множини
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
	sprintf(buff, "%d\n", input);
	sds str = sdsnew(buff, strlen(buff));
	sds proc = sdsnew(""); // sending empty string
	//conversion(str, &proc);
	printf("%s\n", proc);
	sdsfree(str);
	sdsfree(proc);
}

