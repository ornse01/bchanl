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
LOCAL UB test_extbbslist_testurl005[] = "http://a/a/";
LOCAL UB test_extbbslist_testurl006[] = "http://b/a/";
LOCAL UB test_extbbslist_testurl007[] = "http://c/a/";
LOCAL UB test_extbbslist_testurl008[] = "http://d/a/";
LOCAL TC test_extbbslist_testTCurl005[] = {TK_h, TK_t, TK_t, TK_p, TK_COLN, TK_SLSH, TK_SLSH, TK_a, TK_SLSH, TK_a, TK_SLSH, TNULL};
LOCAL TC test_extbbslist_testTCurl006[] = {TK_h, TK_t, TK_t, TK_p, TK_COLN, TK_SLSH, TK_SLSH, TK_b, TK_SLSH, TK_a, TK_SLSH, TNULL};
LOCAL TC test_extbbslist_testTCurl007[] = {TK_h, TK_t, TK_t, TK_p, TK_COLN, TK_SLSH, TK_SLSH, TK_c, TK_SLSH, TK_a, TK_SLSH, TNULL};
LOCAL TC test_extbbslist_testTCurl008[] = {TK_h, TK_t, TK_t, TK_p, TK_COLN, TK_SLSH, TK_SLSH, TK_d, TK_SLSH, TK_a, TK_SLSH, TNULL};

struct testextbbslist_input_t_ {
	TC *title;
	W title_len;
	TC *url;
	W url_len;
};
typedef struct testextbbslist_input_t_ testextbbslist_input_t;

struct testextbbslist_expected_t_ {
	TC *title;
	W title_len;
	UB *url;
	W url_len;
};
typedef struct testextbbslist_expected_t_ testextbbslist_expected_t;

LOCAL W test_extbbslist_appenditem(extbbslist_editcontext_t *ctx, testextbbslist_input_t *input, W input_len)
{
	W i, err, title_len, url_len;
	TC *title, *url;

	for (i = 0; i < input_len; i++) {
		title = input[i].title;
		title_len = input[i].title_len;
		url = input[i].url;
		url_len = input[i].url_len;
		err = extbbslist_editcontext_append(ctx, title, title_len, url, url_len);
		if (err < 0) {
			printf("extbbslist_editcontext_appenditem %d\n", i);
			return err;
		}
	}

	return 0;
}

LOCAL W test_extbbslist_checkexpected(extbbslist_t *list, testextbbslist_expected_t *expected, W expected_len)
{
	W ret = 0, i, title_len, url_len;
	TC *title;
	UB *url;
	Bool cont;
	extbbslist_readcontext_t *ctx;

	ctx = extbbslist_startread(list);
	if (ctx == NULL) {
		printf("extbbslist_startread\n");
		return -1;
	}

	for (i = 0; i < expected_len; i++) {
		cont = extbbslist_readcontext_getnext(ctx, &title, &title_len, &url, &url_len);
		if (cont == False) {
			printf("extbbslist_readcontext_getnext return value %d\n", i);
			ret = -1;
		}
		if (tc_strncmp(title, expected[i].title, title_len) != 0) {
			printf("extbbslist_readcontext_getnext title %d\n", i);
			ret = -1;
		}
		if (strncmp(url, expected[i].url, url_len) != 0) {
			printf("extbbslist_readcontext_getnext url %d\n", i);
			printf("                               expected = %s\n", expected[i].url);
			printf("                               result = %s\n", url);
			ret = -1;
		}
	}
	cont = extbbslist_readcontext_getnext(ctx, &title, &title_len, &url, &url_len);
	if (cont != False) {
		printf("extbbslist_readcontext_getnext return value last\n");
		ret = -1;
	}

	extbbslist_endread(list, ctx);

	return ret;	
}

LOCAL UNITTEST_RESULT test_extbbslist_1()
{
	extbbslist_t *list;
	UNITTEST_RESULT result = UNITTEST_RESULT_PASS;
	TC *title;
	UB *url;
	W err, title_len, url_len;
	testextbbslist_expected_t expected[] = {
		{
			test_extbbslist_testtitle001,
			tc_strlen(test_extbbslist_testtitle001),
			test_extbbslist_testurl001,
			strlen(test_extbbslist_testurl001)
		},
		{
			test_extbbslist_testtitle002,
			tc_strlen(test_extbbslist_testtitle002),
			test_extbbslist_testurl002,
			strlen(test_extbbslist_testurl002)
		},
		{
			test_extbbslist_testtitle003,
			tc_strlen(test_extbbslist_testtitle003),
			test_extbbslist_testurl003,
			strlen(test_extbbslist_testurl003)
		},
		{
			test_extbbslist_testtitle004,
			tc_strlen(test_extbbslist_testtitle004),
			test_extbbslist_testurl004,
			strlen(test_extbbslist_testurl004)
		},
	};
	W expected_len = sizeof(expected)/sizeof(testextbbslist_expected_t);

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

	err = test_extbbslist_checkexpected(list, expected, expected_len);
	if (err < 0) {
		result = UNITTEST_RESULT_FAIL;
	}

	extbbslist_delete(list);

	return result;
}

LOCAL UNITTEST_RESULT test_extbbslist_2()
{
	extbbslist_t *list;
	UNITTEST_RESULT result = UNITTEST_RESULT_PASS;
	TC *title;
	UB *url;
	W err, title_len, url_len, num;

	list = extbbslist_new(NULL, 0, 0);

	num = extbbslist_number(list);
	if (num != 0) {
		printf("extbbslist_number 000\n");
		result = UNITTEST_RESULT_FAIL;
	}

	title = test_extbbslist_testtitle001;
	title_len = tc_strlen(title);
	url = test_extbbslist_testurl001;
	url_len = strlen(url);
	err = extbbslist_appenditem(list, title, title_len, url, url_len);
	if (err < 0) {
		printf("extbbslist_appenditem 001\n");
		result = UNITTEST_RESULT_FAIL;
	}
	num = extbbslist_number(list);
	if (num != 1) {
		printf("extbbslist_number 001\n");
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
	num = extbbslist_number(list);
	if (num != 2) {
		printf("extbbslist_number 002\n");
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
	num = extbbslist_number(list);
	if (num != 3) {
		printf("extbbslist_number 003\n");
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
	num = extbbslist_number(list);
	if (num != 4) {
		printf("extbbslist_number 004\n");
		result = UNITTEST_RESULT_FAIL;
	}

	title = test_extbbslist_testtitle002;
	title_len = tc_strlen(title);
	err = extbbslist_deleteitem(list, title, title_len);
	if (err < 0) {
		printf("extbbslist_deleteitem 002\n");
		result = UNITTEST_RESULT_FAIL;
	}
	num = extbbslist_number(list);
	if (num != 3) {
		printf("extbbslist_number 002\n");
		result = UNITTEST_RESULT_FAIL;
	}

	extbbslist_delete(list);

	return result;
}

LOCAL UNITTEST_RESULT test_extbbslist_3()
{
	extbbslist_t *list;
	extbbslist_editcontext_t *editctx;
	UNITTEST_RESULT result = UNITTEST_RESULT_PASS;
	W err;
	testextbbslist_input_t input[] = {
		{
			test_extbbslist_testtitle001,
			tc_strlen(test_extbbslist_testtitle001),
			test_extbbslist_testTCurl005,
			tc_strlen(test_extbbslist_testTCurl005),
		},
		{
			test_extbbslist_testtitle002,
			tc_strlen(test_extbbslist_testtitle002),
			test_extbbslist_testTCurl006,
			tc_strlen(test_extbbslist_testTCurl006),
		},
		{
			test_extbbslist_testtitle003,
			tc_strlen(test_extbbslist_testtitle003),
			test_extbbslist_testTCurl007,
			tc_strlen(test_extbbslist_testTCurl007),
		},
		{
			test_extbbslist_testtitle004,
			tc_strlen(test_extbbslist_testtitle004),
			test_extbbslist_testTCurl008,
			tc_strlen(test_extbbslist_testTCurl008),
		},
	};
	W input_len = sizeof(input)/sizeof(testextbbslist_input_t);
	testextbbslist_expected_t expected[] = {
		{
			test_extbbslist_testtitle001,
			tc_strlen(test_extbbslist_testtitle001),
			test_extbbslist_testurl005,
			strlen(test_extbbslist_testurl005)
		},
		{
			test_extbbslist_testtitle002,
			tc_strlen(test_extbbslist_testtitle002),
			test_extbbslist_testurl006,
			strlen(test_extbbslist_testurl006)
		},
		{
			test_extbbslist_testtitle003,
			tc_strlen(test_extbbslist_testtitle003),
			test_extbbslist_testurl007,
			strlen(test_extbbslist_testurl007)
		},
		{
			test_extbbslist_testtitle004,
			tc_strlen(test_extbbslist_testtitle004),
			test_extbbslist_testurl008,
			strlen(test_extbbslist_testurl008)
		},
	};
	W expected_len = sizeof(expected)/sizeof(testextbbslist_expected_t);

	list = extbbslist_new(NULL, 0, 0);

	editctx = extbbslist_startedit(list);
	if (editctx == NULL) {
		printf("extbbslist_startedit\n");
		extbbslist_delete(list);
		return UNITTEST_RESULT_FAIL;
	}
	err = test_extbbslist_appenditem(editctx, input, input_len);
	if (err < 0) {
		result = UNITTEST_RESULT_FAIL;
	}
	extbbslist_endedit(list, editctx, True);

	err = test_extbbslist_checkexpected(list, expected, expected_len);
	if (err < 0) {
		result = UNITTEST_RESULT_FAIL;
	}

	extbbslist_delete(list);

	return result;
}

LOCAL UNITTEST_RESULT test_extbbslist_4()
{
	extbbslist_t *list;
	extbbslist_editcontext_t *editctx;
	UNITTEST_RESULT result = UNITTEST_RESULT_PASS;
	W err;
	testextbbslist_input_t input[] = {
		{
			test_extbbslist_testtitle001,
			tc_strlen(test_extbbslist_testtitle001),
			test_extbbslist_testTCurl005,
			tc_strlen(test_extbbslist_testTCurl005),
		},
		{
			test_extbbslist_testtitle002,
			tc_strlen(test_extbbslist_testtitle002),
			test_extbbslist_testTCurl006,
			tc_strlen(test_extbbslist_testTCurl006),
		},
		{
			test_extbbslist_testtitle003,
			tc_strlen(test_extbbslist_testtitle003),
			test_extbbslist_testTCurl007,
			tc_strlen(test_extbbslist_testTCurl007),
		},
		{
			test_extbbslist_testtitle004,
			tc_strlen(test_extbbslist_testtitle004),
			test_extbbslist_testTCurl008,
			tc_strlen(test_extbbslist_testTCurl008),
		},
	};
	W input_len = sizeof(input)/sizeof(testextbbslist_input_t);

	list = extbbslist_new(NULL, 0, 0);

	editctx = extbbslist_startedit(list);
	if (editctx == NULL) {
		printf("extbbslist_startedit\n");
		extbbslist_delete(list);
		return UNITTEST_RESULT_FAIL;
	}
	err = test_extbbslist_appenditem(editctx, input, input_len);
	if (err < 0) {
		result = UNITTEST_RESULT_FAIL;
	}
	extbbslist_endedit(list, editctx, False);

	err = test_extbbslist_checkexpected(list, NULL, 0);
	if (err < 0) {
		result = UNITTEST_RESULT_FAIL;
	}

	extbbslist_delete(list);

	return result;
}

LOCAL UNITTEST_RESULT test_extbbslist_5()
{
	extbbslist_t *list;
	extbbslist_editcontext_t *editctx;
	UNITTEST_RESULT result = UNITTEST_RESULT_PASS;
	W err, num;
	testextbbslist_input_t input[] = {
		{
			test_extbbslist_testtitle001,
			tc_strlen(test_extbbslist_testtitle001),
			test_extbbslist_testTCurl005,
			tc_strlen(test_extbbslist_testTCurl005),
		},
		{
			test_extbbslist_testtitle002,
			tc_strlen(test_extbbslist_testtitle002),
			test_extbbslist_testTCurl006,
			tc_strlen(test_extbbslist_testTCurl006),
		},
		{
			test_extbbslist_testtitle003,
			tc_strlen(test_extbbslist_testtitle003),
			test_extbbslist_testTCurl007,
			tc_strlen(test_extbbslist_testTCurl007),
		},
		{
			test_extbbslist_testtitle004,
			tc_strlen(test_extbbslist_testtitle004),
			test_extbbslist_testTCurl008,
			tc_strlen(test_extbbslist_testTCurl008),
		},
	};
	W input_len = sizeof(input)/sizeof(testextbbslist_input_t);

	list = extbbslist_new(NULL, 0, 0);

	editctx = extbbslist_startedit(list);
	if (editctx == NULL) {
		printf("extbbslist_startedit\n");
		extbbslist_delete(list);
		return UNITTEST_RESULT_FAIL;
	}
	err = test_extbbslist_appenditem(editctx, input, input_len);
	if (err < 0) {
		result = UNITTEST_RESULT_FAIL;
	}
	extbbslist_endedit(list, editctx, True);

	num = extbbslist_number(list);
	if (num != 4) {
		printf("extbbslist_number 002\n");
		result = UNITTEST_RESULT_FAIL;
	}

	extbbslist_delete(list);

	return result;
}

LOCAL UNITTEST_RESULT test_extbbslist_6()
{
	extbbslist_t *list;
	extbbslist_editcontext_t *editctx;
	UNITTEST_RESULT result = UNITTEST_RESULT_PASS;
	W err, num;
	testextbbslist_input_t input[] = {
		{
			test_extbbslist_testtitle001,
			tc_strlen(test_extbbslist_testtitle001),
			test_extbbslist_testTCurl005,
			tc_strlen(test_extbbslist_testTCurl005),
		},
		{
			test_extbbslist_testtitle002,
			tc_strlen(test_extbbslist_testtitle002),
			test_extbbslist_testTCurl006,
			tc_strlen(test_extbbslist_testTCurl006),
		},
		{
			test_extbbslist_testtitle003,
			tc_strlen(test_extbbslist_testtitle003),
			test_extbbslist_testTCurl007,
			tc_strlen(test_extbbslist_testTCurl007),
		},
		{
			test_extbbslist_testtitle004,
			tc_strlen(test_extbbslist_testtitle004),
			test_extbbslist_testTCurl008,
			tc_strlen(test_extbbslist_testTCurl008),
		},
	};
	W input_len = sizeof(input)/sizeof(testextbbslist_input_t);

	list = extbbslist_new(NULL, 0, 0);


	editctx = extbbslist_startedit(list);
	if (editctx == NULL) {
		printf("extbbslist_startedit\n");
		extbbslist_delete(list);
		return UNITTEST_RESULT_FAIL;
	}
	err = test_extbbslist_appenditem(editctx, input, input_len);
	if (err < 0) {
		result = UNITTEST_RESULT_FAIL;
	}
	extbbslist_endedit(list, editctx, False);

	num = extbbslist_number(list);
	if (num != 0) {
		printf("extbbslist_number 002\n");
		result = UNITTEST_RESULT_FAIL;
	}

	extbbslist_delete(list);

	return result;
}

LOCAL UNITTEST_RESULT test_extbbslist_7()
{
	extbbslist_t *list;
	extbbslist_editcontext_t *editctx;
	UNITTEST_RESULT result = UNITTEST_RESULT_PASS;
	W err;
	testextbbslist_input_t input[] = {
		{
			test_extbbslist_testtitle001,
			tc_strlen(test_extbbslist_testtitle001),
			test_extbbslist_testTCurl005,
			tc_strlen(test_extbbslist_testTCurl005),
		},
		{
			test_extbbslist_testtitle002,
			tc_strlen(test_extbbslist_testtitle002),
			test_extbbslist_testTCurl006,
			tc_strlen(test_extbbslist_testTCurl006),
		},
		{
			test_extbbslist_testtitle003,
			tc_strlen(test_extbbslist_testtitle003),
			test_extbbslist_testTCurl007,
			tc_strlen(test_extbbslist_testTCurl007),
		},
		{
			test_extbbslist_testtitle004,
			tc_strlen(test_extbbslist_testtitle004),
			test_extbbslist_testTCurl008,
			tc_strlen(test_extbbslist_testTCurl008),
		},
	};
	W input_len = sizeof(input)/sizeof(testextbbslist_input_t);
	testextbbslist_expected_t expected[] = {
		{
			test_extbbslist_testtitle001,
			tc_strlen(test_extbbslist_testtitle001),
			test_extbbslist_testurl005,
			strlen(test_extbbslist_testurl005)
		},
		{
			test_extbbslist_testtitle002,
			tc_strlen(test_extbbslist_testtitle002),
			test_extbbslist_testurl006,
			strlen(test_extbbslist_testurl006)
		},
		{
			test_extbbslist_testtitle004,
			tc_strlen(test_extbbslist_testtitle004),
			test_extbbslist_testurl008,
			strlen(test_extbbslist_testurl008)
		},
	};
	W expected_len = sizeof(expected)/sizeof(testextbbslist_expected_t);

	list = extbbslist_new(NULL, 0, 0);

	editctx = extbbslist_startedit(list);
	if (editctx == NULL) {
		printf("extbbslist_startedit\n");
		extbbslist_delete(list);
		return UNITTEST_RESULT_FAIL;
	}
	err = test_extbbslist_appenditem(editctx, input, input_len);
	if (err < 0) {
		result = UNITTEST_RESULT_FAIL;
	}
	extbbslist_editcontext_deleteitem(editctx, 2);
	extbbslist_endedit(list, editctx, True);

	err = test_extbbslist_checkexpected(list, expected, expected_len);
	if (err < 0) {
		result = UNITTEST_RESULT_FAIL;
	}

	extbbslist_delete(list);

	return result;
}

LOCAL UNITTEST_RESULT test_extbbslist_8()
{
	extbbslist_t *list;
	extbbslist_editcontext_t *editctx;
	UNITTEST_RESULT result = UNITTEST_RESULT_PASS;
	W err, num;
	testextbbslist_input_t input[] = {
		{
			test_extbbslist_testtitle001,
			tc_strlen(test_extbbslist_testtitle001),
			test_extbbslist_testTCurl005,
			tc_strlen(test_extbbslist_testTCurl005),
		},
		{
			test_extbbslist_testtitle002,
			tc_strlen(test_extbbslist_testtitle002),
			test_extbbslist_testTCurl006,
			tc_strlen(test_extbbslist_testTCurl006),
		},
		{
			test_extbbslist_testtitle003,
			tc_strlen(test_extbbslist_testtitle003),
			test_extbbslist_testTCurl007,
			tc_strlen(test_extbbslist_testTCurl007),
		},
		{
			test_extbbslist_testtitle004,
			tc_strlen(test_extbbslist_testtitle004),
			test_extbbslist_testTCurl008,
			tc_strlen(test_extbbslist_testTCurl008),
		},
	};
	W input_len = sizeof(input)/sizeof(testextbbslist_input_t);

	list = extbbslist_new(NULL, 0, 0);

	editctx = extbbslist_startedit(list);
	if (editctx == NULL) {
		printf("extbbslist_startedit\n");
		extbbslist_delete(list);
		return UNITTEST_RESULT_FAIL;
	}
	err = test_extbbslist_appenditem(editctx, input, input_len);
	if (err < 0) {
		result = UNITTEST_RESULT_FAIL;
	}
	extbbslist_editcontext_deleteitem(editctx, 2);
	extbbslist_endedit(list, editctx, True);

	num = extbbslist_number(list);
	if (num != 3) {
		printf("extbbslist_number 002\n");
		result = UNITTEST_RESULT_FAIL;
	}

	extbbslist_delete(list);

	return result;
}

LOCAL UNITTEST_RESULT test_extbbslist_swap_common(testextbbslist_input_t *input, W input_len, W i0, W i1, testextbbslist_expected_t *expected, W expected_len)
{
	extbbslist_t *list;
	extbbslist_editcontext_t *editctx;
	UNITTEST_RESULT result = UNITTEST_RESULT_PASS;
	W err;

	list = extbbslist_new(NULL, 0, 0);

	editctx = extbbslist_startedit(list);
	if (editctx == NULL) {
		printf("extbbslist_startedit\n");
		extbbslist_delete(list);
		return UNITTEST_RESULT_FAIL;
	}
	err = test_extbbslist_appenditem(editctx, input, input_len);
	if (err < 0) {
		result = UNITTEST_RESULT_FAIL;
	}
	extbbslist_editcontext_swapitem(editctx, i0, i1);
	extbbslist_endedit(list, editctx, True);

	err = test_extbbslist_checkexpected(list, expected, expected_len);
	if (err < 0) {
		result = UNITTEST_RESULT_FAIL;
	}

	extbbslist_delete(list);

	return result;
}

LOCAL UNITTEST_RESULT test_extbbslist_9()
{
	testextbbslist_input_t input[] = {
		{
			test_extbbslist_testtitle001,
			tc_strlen(test_extbbslist_testtitle001),
			test_extbbslist_testTCurl005,
			tc_strlen(test_extbbslist_testTCurl005),
		},
		{
			test_extbbslist_testtitle002,
			tc_strlen(test_extbbslist_testtitle002),
			test_extbbslist_testTCurl006,
			tc_strlen(test_extbbslist_testTCurl006),
		},
		{
			test_extbbslist_testtitle003,
			tc_strlen(test_extbbslist_testtitle003),
			test_extbbslist_testTCurl007,
			tc_strlen(test_extbbslist_testTCurl007),
		},
		{
			test_extbbslist_testtitle004,
			tc_strlen(test_extbbslist_testtitle004),
			test_extbbslist_testTCurl008,
			tc_strlen(test_extbbslist_testTCurl008),
		},
	};
	W input_len = sizeof(input)/sizeof(testextbbslist_input_t);
	testextbbslist_expected_t expected[] = {
		{
			test_extbbslist_testtitle002,
			tc_strlen(test_extbbslist_testtitle002),
			test_extbbslist_testurl006,
			strlen(test_extbbslist_testurl006)
		},
		{
			test_extbbslist_testtitle001,
			tc_strlen(test_extbbslist_testtitle001),
			test_extbbslist_testurl005,
			strlen(test_extbbslist_testurl005)
		},
		{
			test_extbbslist_testtitle003,
			tc_strlen(test_extbbslist_testtitle003),
			test_extbbslist_testurl007,
			strlen(test_extbbslist_testurl007)
		},
		{
			test_extbbslist_testtitle004,
			tc_strlen(test_extbbslist_testtitle004),
			test_extbbslist_testurl008,
			strlen(test_extbbslist_testurl008)
		},
	};
	W expected_len = sizeof(expected)/sizeof(testextbbslist_expected_t);

	return test_extbbslist_swap_common(input, input_len, 0, 1, expected, expected_len);
}

LOCAL UNITTEST_RESULT test_extbbslist_10()
{
	testextbbslist_input_t input[] = {
		{
			test_extbbslist_testtitle001,
			tc_strlen(test_extbbslist_testtitle001),
			test_extbbslist_testTCurl005,
			tc_strlen(test_extbbslist_testTCurl005),
		},
		{
			test_extbbslist_testtitle002,
			tc_strlen(test_extbbslist_testtitle002),
			test_extbbslist_testTCurl006,
			tc_strlen(test_extbbslist_testTCurl006),
		},
		{
			test_extbbslist_testtitle003,
			tc_strlen(test_extbbslist_testtitle003),
			test_extbbslist_testTCurl007,
			tc_strlen(test_extbbslist_testTCurl007),
		},
		{
			test_extbbslist_testtitle004,
			tc_strlen(test_extbbslist_testtitle004),
			test_extbbslist_testTCurl008,
			tc_strlen(test_extbbslist_testTCurl008),
		},
	};
	W input_len = sizeof(input)/sizeof(testextbbslist_input_t);
	testextbbslist_expected_t expected[] = {
		{
			test_extbbslist_testtitle003,
			tc_strlen(test_extbbslist_testtitle003),
			test_extbbslist_testurl007,
			strlen(test_extbbslist_testurl007)
		},
		{
			test_extbbslist_testtitle002,
			tc_strlen(test_extbbslist_testtitle002),
			test_extbbslist_testurl006,
			strlen(test_extbbslist_testurl006)
		},
		{
			test_extbbslist_testtitle001,
			tc_strlen(test_extbbslist_testtitle001),
			test_extbbslist_testurl005,
			strlen(test_extbbslist_testurl005)
		},
		{
			test_extbbslist_testtitle004,
			tc_strlen(test_extbbslist_testtitle004),
			test_extbbslist_testurl008,
			strlen(test_extbbslist_testurl008)
		},
	};
	W expected_len = sizeof(expected)/sizeof(testextbbslist_expected_t);

	return test_extbbslist_swap_common(input, input_len, 0, 2, expected, expected_len);
}

LOCAL UNITTEST_RESULT test_extbbslist_11()
{
	testextbbslist_input_t input[] = {
		{
			test_extbbslist_testtitle001,
			tc_strlen(test_extbbslist_testtitle001),
			test_extbbslist_testTCurl005,
			tc_strlen(test_extbbslist_testTCurl005),
		},
		{
			test_extbbslist_testtitle002,
			tc_strlen(test_extbbslist_testtitle002),
			test_extbbslist_testTCurl006,
			tc_strlen(test_extbbslist_testTCurl006),
		},
		{
			test_extbbslist_testtitle003,
			tc_strlen(test_extbbslist_testtitle003),
			test_extbbslist_testTCurl007,
			tc_strlen(test_extbbslist_testTCurl007),
		},
		{
			test_extbbslist_testtitle004,
			tc_strlen(test_extbbslist_testtitle004),
			test_extbbslist_testTCurl008,
			tc_strlen(test_extbbslist_testTCurl008),
		},
	};
	W input_len = sizeof(input)/sizeof(testextbbslist_input_t);
	testextbbslist_expected_t expected[] = {
		{
			test_extbbslist_testtitle001,
			tc_strlen(test_extbbslist_testtitle001),
			test_extbbslist_testurl005,
			strlen(test_extbbslist_testurl005)
		},
		{
			test_extbbslist_testtitle004,
			tc_strlen(test_extbbslist_testtitle004),
			test_extbbslist_testurl008,
			strlen(test_extbbslist_testurl008)
		},
		{
			test_extbbslist_testtitle003,
			tc_strlen(test_extbbslist_testtitle003),
			test_extbbslist_testurl007,
			strlen(test_extbbslist_testurl007)
		},
		{
			test_extbbslist_testtitle002,
			tc_strlen(test_extbbslist_testtitle002),
			test_extbbslist_testurl006,
			strlen(test_extbbslist_testurl006)
		},
	};
	W expected_len = sizeof(expected)/sizeof(testextbbslist_expected_t);

	return test_extbbslist_swap_common(input, input_len, 1, 3, expected, expected_len);
}

LOCAL UNITTEST_RESULT test_extbbslist_12()
{
	testextbbslist_input_t input[] = {
		{
			test_extbbslist_testtitle001,
			tc_strlen(test_extbbslist_testtitle001),
			test_extbbslist_testTCurl005,
			tc_strlen(test_extbbslist_testTCurl005),
		},
		{
			test_extbbslist_testtitle002,
			tc_strlen(test_extbbslist_testtitle002),
			test_extbbslist_testTCurl006,
			tc_strlen(test_extbbslist_testTCurl006),
		},
		{
			test_extbbslist_testtitle003,
			tc_strlen(test_extbbslist_testtitle003),
			test_extbbslist_testTCurl007,
			tc_strlen(test_extbbslist_testTCurl007),
		},
		{
			test_extbbslist_testtitle004,
			tc_strlen(test_extbbslist_testtitle004),
			test_extbbslist_testTCurl008,
			tc_strlen(test_extbbslist_testTCurl008),
		},
	};
	W input_len = sizeof(input)/sizeof(testextbbslist_input_t);
	testextbbslist_expected_t expected[] = {
		{
			test_extbbslist_testtitle001,
			tc_strlen(test_extbbslist_testtitle001),
			test_extbbslist_testurl005,
			strlen(test_extbbslist_testurl005)
		},
		{
			test_extbbslist_testtitle002,
			tc_strlen(test_extbbslist_testtitle002),
			test_extbbslist_testurl006,
			strlen(test_extbbslist_testurl006)
		},
		{
			test_extbbslist_testtitle004,
			tc_strlen(test_extbbslist_testtitle004),
			test_extbbslist_testurl008,
			strlen(test_extbbslist_testurl008)
		},
		{
			test_extbbslist_testtitle003,
			tc_strlen(test_extbbslist_testtitle003),
			test_extbbslist_testurl007,
			strlen(test_extbbslist_testurl007)
		},
	};
	W expected_len = sizeof(expected)/sizeof(testextbbslist_expected_t);

	return test_extbbslist_swap_common(input, input_len, 2, 3, expected, expected_len);
}

LOCAL UNITTEST_RESULT test_extbbslist_13()
{
	testextbbslist_input_t input[] = {
		{
			test_extbbslist_testtitle001,
			tc_strlen(test_extbbslist_testtitle001),
			test_extbbslist_testTCurl005,
			tc_strlen(test_extbbslist_testTCurl005),
		},
		{
			test_extbbslist_testtitle002,
			tc_strlen(test_extbbslist_testtitle002),
			test_extbbslist_testTCurl006,
			tc_strlen(test_extbbslist_testTCurl006),
		},
		{
			test_extbbslist_testtitle003,
			tc_strlen(test_extbbslist_testtitle003),
			test_extbbslist_testTCurl007,
			tc_strlen(test_extbbslist_testTCurl007),
		},
		{
			test_extbbslist_testtitle004,
			tc_strlen(test_extbbslist_testtitle004),
			test_extbbslist_testTCurl008,
			tc_strlen(test_extbbslist_testTCurl008),
		},
	};
	W input_len = sizeof(input)/sizeof(testextbbslist_input_t);
	testextbbslist_expected_t expected[] = {
		{
			test_extbbslist_testtitle001,
			tc_strlen(test_extbbslist_testtitle001),
			test_extbbslist_testurl005,
			strlen(test_extbbslist_testurl005)
		},
		{
			test_extbbslist_testtitle002,
			tc_strlen(test_extbbslist_testtitle002),
			test_extbbslist_testurl006,
			strlen(test_extbbslist_testurl006)
		},
		{
			test_extbbslist_testtitle003,
			tc_strlen(test_extbbslist_testtitle003),
			test_extbbslist_testurl007,
			strlen(test_extbbslist_testurl007)
		},
		{
			test_extbbslist_testtitle004,
			tc_strlen(test_extbbslist_testtitle004),
			test_extbbslist_testurl008,
			strlen(test_extbbslist_testurl008)
		},
	};
	W expected_len = sizeof(expected)/sizeof(testextbbslist_expected_t);

	return test_extbbslist_swap_common(input, input_len, 0, 4, expected, expected_len);
}

LOCAL UNITTEST_RESULT test_extbbslist_14()
{
	testextbbslist_input_t input[] = {
		{
			test_extbbslist_testtitle001,
			tc_strlen(test_extbbslist_testtitle001),
			test_extbbslist_testTCurl005,
			tc_strlen(test_extbbslist_testTCurl005),
		},
		{
			test_extbbslist_testtitle002,
			tc_strlen(test_extbbslist_testtitle002),
			test_extbbslist_testTCurl006,
			tc_strlen(test_extbbslist_testTCurl006),
		},
		{
			test_extbbslist_testtitle003,
			tc_strlen(test_extbbslist_testtitle003),
			test_extbbslist_testTCurl007,
			tc_strlen(test_extbbslist_testTCurl007),
		},
		{
			test_extbbslist_testtitle004,
			tc_strlen(test_extbbslist_testtitle004),
			test_extbbslist_testTCurl008,
			tc_strlen(test_extbbslist_testTCurl008),
		},
	};
	W input_len = sizeof(input)/sizeof(testextbbslist_input_t);
	testextbbslist_expected_t expected[] = {
		{
			test_extbbslist_testtitle001,
			tc_strlen(test_extbbslist_testtitle001),
			test_extbbslist_testurl005,
			strlen(test_extbbslist_testurl005)
		},
		{
			test_extbbslist_testtitle002,
			tc_strlen(test_extbbslist_testtitle002),
			test_extbbslist_testurl006,
			strlen(test_extbbslist_testurl006)
		},
		{
			test_extbbslist_testtitle003,
			tc_strlen(test_extbbslist_testtitle003),
			test_extbbslist_testurl007,
			strlen(test_extbbslist_testurl007)
		},
		{
			test_extbbslist_testtitle004,
			tc_strlen(test_extbbslist_testtitle004),
			test_extbbslist_testurl008,
			strlen(test_extbbslist_testurl008)
		},
	};
	W expected_len = sizeof(expected)/sizeof(testextbbslist_expected_t);

	return test_extbbslist_swap_common(input, input_len, 4, 0, expected, expected_len);
}

LOCAL UNITTEST_RESULT test_extbbslist_15()
{
	testextbbslist_input_t input[] = {
		{
			test_extbbslist_testtitle001,
			tc_strlen(test_extbbslist_testtitle001),
			test_extbbslist_testTCurl005,
			tc_strlen(test_extbbslist_testTCurl005),
		},
		{
			test_extbbslist_testtitle002,
			tc_strlen(test_extbbslist_testtitle002),
			test_extbbslist_testTCurl006,
			tc_strlen(test_extbbslist_testTCurl006),
		},
		{
			test_extbbslist_testtitle003,
			tc_strlen(test_extbbslist_testtitle003),
			test_extbbslist_testTCurl007,
			tc_strlen(test_extbbslist_testTCurl007),
		},
		{
			test_extbbslist_testtitle004,
			tc_strlen(test_extbbslist_testtitle004),
			test_extbbslist_testTCurl008,
			tc_strlen(test_extbbslist_testTCurl008),
		},
	};
	W input_len = sizeof(input)/sizeof(testextbbslist_input_t);
	testextbbslist_expected_t expected[] = {
		{
			test_extbbslist_testtitle001,
			tc_strlen(test_extbbslist_testtitle001),
			test_extbbslist_testurl005,
			strlen(test_extbbslist_testurl005)
		},
		{
			test_extbbslist_testtitle002,
			tc_strlen(test_extbbslist_testtitle002),
			test_extbbslist_testurl006,
			strlen(test_extbbslist_testurl006)
		},
		{
			test_extbbslist_testtitle003,
			tc_strlen(test_extbbslist_testtitle003),
			test_extbbslist_testurl007,
			strlen(test_extbbslist_testurl007)
		},
		{
			test_extbbslist_testtitle004,
			tc_strlen(test_extbbslist_testtitle004),
			test_extbbslist_testurl008,
			strlen(test_extbbslist_testurl008)
		},
	};
	W expected_len = sizeof(expected)/sizeof(testextbbslist_expected_t);

	return test_extbbslist_swap_common(input, input_len, -1, 0, expected, expected_len);
}

LOCAL UNITTEST_RESULT test_extbbslist_16()
{
	testextbbslist_input_t input[] = {
		{
			test_extbbslist_testtitle001,
			tc_strlen(test_extbbslist_testtitle001),
			test_extbbslist_testTCurl005,
			tc_strlen(test_extbbslist_testTCurl005),
		},
		{
			test_extbbslist_testtitle002,
			tc_strlen(test_extbbslist_testtitle002),
			test_extbbslist_testTCurl006,
			tc_strlen(test_extbbslist_testTCurl006),
		},
		{
			test_extbbslist_testtitle003,
			tc_strlen(test_extbbslist_testtitle003),
			test_extbbslist_testTCurl007,
			tc_strlen(test_extbbslist_testTCurl007),
		},
		{
			test_extbbslist_testtitle004,
			tc_strlen(test_extbbslist_testtitle004),
			test_extbbslist_testTCurl008,
			tc_strlen(test_extbbslist_testTCurl008),
		},
	};
	W input_len = sizeof(input)/sizeof(testextbbslist_input_t);
	testextbbslist_expected_t expected[] = {
		{
			test_extbbslist_testtitle001,
			tc_strlen(test_extbbslist_testtitle001),
			test_extbbslist_testurl005,
			strlen(test_extbbslist_testurl005)
		},
		{
			test_extbbslist_testtitle002,
			tc_strlen(test_extbbslist_testtitle002),
			test_extbbslist_testurl006,
			strlen(test_extbbslist_testurl006)
		},
		{
			test_extbbslist_testtitle003,
			tc_strlen(test_extbbslist_testtitle003),
			test_extbbslist_testurl007,
			strlen(test_extbbslist_testurl007)
		},
		{
			test_extbbslist_testtitle004,
			tc_strlen(test_extbbslist_testtitle004),
			test_extbbslist_testurl008,
			strlen(test_extbbslist_testurl008)
		},
	};
	W expected_len = sizeof(expected)/sizeof(testextbbslist_expected_t);

	return test_extbbslist_swap_common(input, input_len, 0, -1, expected, expected_len);
}

LOCAL UNITTEST_RESULT test_extbbslist_swap_selected_common(testextbbslist_input_t *input, W input_len, W sel_before, W i0, W i1, W sel_expected)
{
	extbbslist_t *list;
	extbbslist_editcontext_t *editctx;
	UNITTEST_RESULT result = UNITTEST_RESULT_PASS;
	W sel, err;

	list = extbbslist_new(NULL, 0, 0);

	editctx = extbbslist_startedit(list);
	if (editctx == NULL) {
		printf("extbbslist_startedit\n");
		extbbslist_delete(list);
		return UNITTEST_RESULT_FAIL;
	}

	err = test_extbbslist_appenditem(editctx, input, input_len);
	if (err < 0) {
		result = UNITTEST_RESULT_FAIL;
	}
	extbbslist_editcontext_setselect(editctx, sel_before);
	extbbslist_editcontext_swapitem(editctx, i0, i1);

	sel = extbbslist_editcontext_getselect(editctx);
	if (sel != sel_expected) {
		printf("extbbslist_editcontext_getselected fail: expected = %d, result = %d\n", sel_expected, sel);
		result = UNITTEST_RESULT_FAIL;
	}

	extbbslist_endedit(list, editctx, True);

	extbbslist_delete(list);

	return result;
}

LOCAL UNITTEST_RESULT test_extbbslist_17()
{
	testextbbslist_input_t input[] = {
		{
			test_extbbslist_testtitle001,
			tc_strlen(test_extbbslist_testtitle001),
			test_extbbslist_testTCurl005,
			tc_strlen(test_extbbslist_testTCurl005),
		},
		{
			test_extbbslist_testtitle002,
			tc_strlen(test_extbbslist_testtitle002),
			test_extbbslist_testTCurl006,
			tc_strlen(test_extbbslist_testTCurl006),
		},
		{
			test_extbbslist_testtitle003,
			tc_strlen(test_extbbslist_testtitle003),
			test_extbbslist_testTCurl007,
			tc_strlen(test_extbbslist_testTCurl007),
		},
		{
			test_extbbslist_testtitle004,
			tc_strlen(test_extbbslist_testtitle004),
			test_extbbslist_testTCurl008,
			tc_strlen(test_extbbslist_testTCurl008),
		},
	};
	W input_len = sizeof(input)/sizeof(testextbbslist_input_t);

	return test_extbbslist_swap_selected_common(input, input_len, 0, 0, 1, 1);
}

LOCAL UNITTEST_RESULT test_extbbslist_18()
{
	testextbbslist_input_t input[] = {
		{
			test_extbbslist_testtitle001,
			tc_strlen(test_extbbslist_testtitle001),
			test_extbbslist_testTCurl005,
			tc_strlen(test_extbbslist_testTCurl005),
		},
		{
			test_extbbslist_testtitle002,
			tc_strlen(test_extbbslist_testtitle002),
			test_extbbslist_testTCurl006,
			tc_strlen(test_extbbslist_testTCurl006),
		},
		{
			test_extbbslist_testtitle003,
			tc_strlen(test_extbbslist_testtitle003),
			test_extbbslist_testTCurl007,
			tc_strlen(test_extbbslist_testTCurl007),
		},
		{
			test_extbbslist_testtitle004,
			tc_strlen(test_extbbslist_testtitle004),
			test_extbbslist_testTCurl008,
			tc_strlen(test_extbbslist_testTCurl008),
		},
	};
	W input_len = sizeof(input)/sizeof(testextbbslist_input_t);

	return test_extbbslist_swap_selected_common(input, input_len, 1, 0, 1, 0);
}

LOCAL UNITTEST_RESULT test_extbbslist_19()
{
	testextbbslist_input_t input[] = {
		{
			test_extbbslist_testtitle001,
			tc_strlen(test_extbbslist_testtitle001),
			test_extbbslist_testTCurl005,
			tc_strlen(test_extbbslist_testTCurl005),
		},
		{
			test_extbbslist_testtitle002,
			tc_strlen(test_extbbslist_testtitle002),
			test_extbbslist_testTCurl006,
			tc_strlen(test_extbbslist_testTCurl006),
		},
		{
			test_extbbslist_testtitle003,
			tc_strlen(test_extbbslist_testtitle003),
			test_extbbslist_testTCurl007,
			tc_strlen(test_extbbslist_testTCurl007),
		},
		{
			test_extbbslist_testtitle004,
			tc_strlen(test_extbbslist_testtitle004),
			test_extbbslist_testTCurl008,
			tc_strlen(test_extbbslist_testTCurl008),
		},
	};
	W input_len = sizeof(input)/sizeof(testextbbslist_input_t);

	return test_extbbslist_swap_selected_common(input, input_len, 0, 0, 2, 2);
}

LOCAL UNITTEST_RESULT test_extbbslist_20()
{
	testextbbslist_input_t input[] = {
		{
			test_extbbslist_testtitle001,
			tc_strlen(test_extbbslist_testtitle001),
			test_extbbslist_testTCurl005,
			tc_strlen(test_extbbslist_testTCurl005),
		},
		{
			test_extbbslist_testtitle002,
			tc_strlen(test_extbbslist_testtitle002),
			test_extbbslist_testTCurl006,
			tc_strlen(test_extbbslist_testTCurl006),
		},
		{
			test_extbbslist_testtitle003,
			tc_strlen(test_extbbslist_testtitle003),
			test_extbbslist_testTCurl007,
			tc_strlen(test_extbbslist_testTCurl007),
		},
		{
			test_extbbslist_testtitle004,
			tc_strlen(test_extbbslist_testtitle004),
			test_extbbslist_testTCurl008,
			tc_strlen(test_extbbslist_testTCurl008),
		},
	};
	W input_len = sizeof(input)/sizeof(testextbbslist_input_t);

	return test_extbbslist_swap_selected_common(input, input_len, 0, 0, 4, 0);
}

EXPORT VOID test_extbbslist_main(unittest_driver_t *driver)
{
	UNITTEST_DRIVER_REGIST(driver, test_extbbslist_1);
	UNITTEST_DRIVER_REGIST(driver, test_extbbslist_2);
	UNITTEST_DRIVER_REGIST(driver, test_extbbslist_3);
	UNITTEST_DRIVER_REGIST(driver, test_extbbslist_4);
	UNITTEST_DRIVER_REGIST(driver, test_extbbslist_5);
	UNITTEST_DRIVER_REGIST(driver, test_extbbslist_6);
	UNITTEST_DRIVER_REGIST(driver, test_extbbslist_7);
	UNITTEST_DRIVER_REGIST(driver, test_extbbslist_8);
	UNITTEST_DRIVER_REGIST(driver, test_extbbslist_9);
	UNITTEST_DRIVER_REGIST(driver, test_extbbslist_10);
	UNITTEST_DRIVER_REGIST(driver, test_extbbslist_11);
	UNITTEST_DRIVER_REGIST(driver, test_extbbslist_12);
	UNITTEST_DRIVER_REGIST(driver, test_extbbslist_13);
	UNITTEST_DRIVER_REGIST(driver, test_extbbslist_14);
	UNITTEST_DRIVER_REGIST(driver, test_extbbslist_15);
	UNITTEST_DRIVER_REGIST(driver, test_extbbslist_16);
	UNITTEST_DRIVER_REGIST(driver, test_extbbslist_17);
	UNITTEST_DRIVER_REGIST(driver, test_extbbslist_18);
	UNITTEST_DRIVER_REGIST(driver, test_extbbslist_19);
	UNITTEST_DRIVER_REGIST(driver, test_extbbslist_20);
}
