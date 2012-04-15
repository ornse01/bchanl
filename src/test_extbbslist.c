/*
 * test_extbbslist.c
 *
 * Copyright (c) 2012 project bchan
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 *
 */

#include    "test.h"

#include    "extbbslist.h"

#include    <btron/btron.h>
#include	<tcode.h>
#include    <bstdio.h>
#include    <bstring.h>
#include    <tstring.h>

#include    <unittest_driver.h>

LOCAL TC test_extbbslist_testtitle001[] = {TK_A, TK_A, TK_A, TNULL};
LOCAL TC test_extbbslist_testtitle002[] = {TK_B, TK_B, TK_B, TNULL};
LOCAL TC test_extbbslist_testtitle003[] = {TK_C, TK_C, TK_C, TNULL};
LOCAL TC test_extbbslist_testtitle004[] = {TK_D, TK_D, TK_D, TNULL};
LOCAL UB test_extbbslist_testurl001[] = "http://aaa.bbb.ccc/abcdef/";
LOCAL UB test_extbbslist_testurl002[] = "http://ddd.eee.fff/abcdef/";
LOCAL UB test_extbbslist_testurl003[] = "http://aaa.bbb.ccc/ghijkl/";
LOCAL UB test_extbbslist_testurl004[] = "http://aaa.bbb.ccc/mnopqr/";

LOCAL UNITTEST_RESULT test_extbbslist_1()
{
	extbbslist_t *list;
	extbbslist_readcontext_t *ctx;
	UNITTEST_RESULT result = UNITTEST_RESULT_PASS;
	TC *title;
	UB *url;
	W err, title_len, url_len;
	Bool cont;

	list = extbbslist_new(NULL, 0, 0);

	title = test_extbbslist_testtitle001;
	title_len = tc_strlen(title);
	url = test_extbbslist_testurl001;
	url_len = strlen(url);
	err = extbbslist_appenditem(list, title, title_len, url, url_len);
	if (err < 0) {
		printf("extbbslist_appenditem 001\n");
		result = UNITTEST_RESULT_FAIL;
	}
	title = test_extbbslist_testtitle002;
	title_len = tc_strlen(title);
	url = test_extbbslist_testurl002;
	url_len = strlen(url);
	err = extbbslist_appenditem(list, title, title_len, url, url_len);
	if (err < 0) {
		printf("extbbslist_appenditem 002\n");
		result = UNITTEST_RESULT_FAIL;
	}
	title = test_extbbslist_testtitle003;
	title_len = tc_strlen(title);
	url = test_extbbslist_testurl003;
	url_len = strlen(url);
	err = extbbslist_appenditem(list, title, title_len, url, url_len);
	if (err < 0) {
		printf("extbbslist_appenditem 003\n");
		result = UNITTEST_RESULT_FAIL;
	}
	title = test_extbbslist_testtitle004;
	title_len = tc_strlen(title);
	url = test_extbbslist_testurl004;
	url_len = strlen(url);
	err = extbbslist_appenditem(list, title, title_len, url, url_len);
	if (err < 0) {
		printf("extbbslist_appenditem 004\n");
		result = UNITTEST_RESULT_FAIL;
	}

	ctx = extbbslist_startread(list);
	if (ctx == NULL) {
		printf("extbbslist_startread\n");
		extbbslist_delete(list);
		return UNITTEST_RESULT_FAIL;
	}

	cont = extbbslist_readcontext_getnext(ctx, &title, &title_len, &url, &url_len);
	if (cont == False) {
		printf("extbbslist_readcontext_getnext return value 001\n");
		result = UNITTEST_RESULT_FAIL;
	}
	if (tc_strncmp(title, test_extbbslist_testtitle001, title_len) != 0) {
		printf("extbbslist_readcontext_getnext title 001\n");
		result = UNITTEST_RESULT_FAIL;
	}
	if (strncmp(url, test_extbbslist_testurl001, url_len) != 0) {
		printf("extbbslist_readcontext_getnext url 001\n");
		printf("                               expected = %s\n", test_extbbslist_testurl001);
		printf("                               result = %s\n", url);
		result = UNITTEST_RESULT_FAIL;
	}
	cont = extbbslist_readcontext_getnext(ctx, &title, &title_len, &url, &url_len);
	if (cont == False) {
		printf("extbbslist_readcontext_getnext return value 002\n");
		result = UNITTEST_RESULT_FAIL;
	}
	if (tc_strncmp(title, test_extbbslist_testtitle002, title_len) != 0) {
		printf("extbbslist_readcontext_getnext title 002\n");
		result = UNITTEST_RESULT_FAIL;
	}
	if (strncmp(url, test_extbbslist_testurl002, url_len) != 0) {
		printf("extbbslist_readcontext_getnext url 002\n");
		result = UNITTEST_RESULT_FAIL;
	}
	cont = extbbslist_readcontext_getnext(ctx, &title, &title_len, &url, &url_len);
	if (cont == False) {
		printf("extbbslist_readcontext_getnext return value 003\n");
		result = UNITTEST_RESULT_FAIL;
	}
	if (tc_strncmp(title, test_extbbslist_testtitle003, title_len) != 0) {
		printf("extbbslist_readcontext_getnext title 003\n");
		result = UNITTEST_RESULT_FAIL;
	}
	if (strncmp(url, test_extbbslist_testurl003, url_len) != 0) {
		printf("extbbslist_readcontext_getnext url 003\n");
		result = UNITTEST_RESULT_FAIL;
	}
	cont = extbbslist_readcontext_getnext(ctx, &title, &title_len, &url, &url_len);
	if (cont == False) {
		printf("extbbslist_readcontext_getnext return value 004\n");
		result = UNITTEST_RESULT_FAIL;
	}
	if (tc_strncmp(title, test_extbbslist_testtitle004, title_len) != 0) {
		printf("extbbslist_readcontext_getnext title 004\n");
		result = UNITTEST_RESULT_FAIL;
	}
	if (strncmp(url, test_extbbslist_testurl004, url_len) != 0) {
		printf("extbbslist_readcontext_getnext url 004\n");
		result = UNITTEST_RESULT_FAIL;
	}
	cont = extbbslist_readcontext_getnext(ctx, &title, &title_len, &url, &url_len);
	if (cont != False) {
		printf("extbbslist_readcontext_getnext return value 005\n");
		result = UNITTEST_RESULT_FAIL;
	}

	extbbslist_endread(list, ctx);

	extbbslist_delete(list);

	return result;
}

EXPORT VOID test_extbbslist_main(unittest_driver_t *driver)
{
	UNITTEST_DRIVER_REGIST(driver, test_extbbslist_1);
}
