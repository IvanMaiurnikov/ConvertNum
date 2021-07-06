#ifndef _UNIT_TEST_H
#define _UNIT_TEST_H
int init_suite1(void);
int clean_suite1(void);
void test_trio_translate_zero(void);
void test_trio_translate_extra(void);
void test_convert_negative(void);
void test_convert_min(void);
void test_convert_max(void);
void test_convert_overmax(void);
void test_convert_zero(void);
void test_input_and_validate_typo(void);
void test_input_and_validate_min(void);
void test_input_and_validate_max(void);
void test_input_and_validate_overmax(void);
void test_input_and_validate_overmin(void);
void test_input_and_validate_longstr(void);
#endif
