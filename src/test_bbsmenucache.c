/*
 * test_bbsmenucache.c
 *
 * Copyright (c) 2009 project bchan
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

#include    <btron/btron.h>
#include	<tcode.h>
#include    <bstdio.h>
#include    <bstring.h>
#include    <bstdlib.h>
#include	<errcode.h>

#include    "test.h"

#include    "bbsmenucache.h"

LOCAL UB test_bbsmncache_testdata_01[] = {"aaaaabbbbbcccccddddd"};
LOCAL UB test_bbsmncache_testdata_01_1[] = {"aaaaa"};
LOCAL UB test_bbsmncache_testdata_01_2[] = {"bbbbb"};
LOCAL UB test_bbsmncache_testdata_01_3[] = {"ccccc"};
LOCAL UB test_bbsmncache_testdata_01_4[] = {"ddddd"};
LOCAL UB test_bbsmncache_testdata_02[] = {"XXX abcdef\r\nAAAA: valueA\r\nBBBB: valueB\r\n\r\n"};
LOCAL UB test_bbsmncache_testdata_07[] = {"XXX abcdef\r\nAAAA: valueC\r\nBBBB: valueC\r\n\r\n"};

LOCAL Bool test_bbsmncache_util_cmp_ctx_str(bbsmncache_datareadcontext_t *context, UB *data, W len)
{
	Bool cont;
	UB *bin_cache;
	W i = 0, len_cache, cmp;

	for (;;) {
		cont = bbsmncache_datareadcontext_nextdata(context, &bin_cache, &len_cache);
		if (cont == False) {
			break;
		}
		cmp = memcmp(data + i, bin_cache, len_cache);
		if (cmp != 0) {
			return False;
		}
		i += len_cache;
		if (i > len) {
			return False;
		}
	}

	if (i != len) {
		return False;
	}

	return True;
}

/* test_bbsmncache_1 */

LOCAL TEST_RESULT test_bbsmncache_1_testseq()
{
	bbsmncache_t *cache;
	TEST_RESULT result = TEST_RESULT_PASS;
	UB *header;
	W err, header_len, cmp;

	cache = bbsmncache_new();

	err = bbsmncache_updatelatestheader(cache, test_bbsmncache_testdata_07, strlen(test_bbsmncache_testdata_07));
	if (err < 0) {
		printf("bbsmncache_updatelataestheade error\n");
		bbsmncache_delete(cache);
		return TEST_RESULT_FAIL;
	}

	bbsmncache_getlatestheader(cache, &header, &header_len);
	if (header_len != strlen(test_bbsmncache_testdata_07)) {
		printf("bbsmncache_getlatestheader: length error\n");
		result = TEST_RESULT_FAIL;
	}
	cmp = memcmp(header, test_bbsmncache_testdata_07, header_len);
	if (cmp != 0) {
		printf("bbsmncache_getlatestheader: data error\n");
		result = TEST_RESULT_FAIL;
	}

	bbsmncache_delete(cache);

	return result;
}

LOCAL TEST_RESULT test_bbsmncache_1()
{
	return test_bbsmncache_1_testseq();
}

/* test_bbsmncache_2 */

LOCAL TEST_RESULT test_bbsmncache_2_testseq()
{
	bbsmncache_t *cache;
	TEST_RESULT result = TEST_RESULT_PASS;
	UB *header;
	W err, header_len, cmp;

	cache = bbsmncache_new();

	err = bbsmncache_updatelatestheader(cache, test_bbsmncache_testdata_07, strlen(test_bbsmncache_testdata_07));
	if (err < 0) {
		printf("bbsmncache_updatelataestheade error\n");
		bbsmncache_delete(cache);
		return TEST_RESULT_FAIL;
	}
	err = bbsmncache_updatelatestheader(cache, test_bbsmncache_testdata_02, strlen(test_bbsmncache_testdata_02));
	if (err < 0) {
		printf("bbsmncache_updatelataestheade error\n");
		bbsmncache_delete(cache);
		return TEST_RESULT_FAIL;
	}

	bbsmncache_getlatestheader(cache, &header, &header_len);
	if (header_len != strlen(test_bbsmncache_testdata_02)) {
		printf("bbsmncache_getlatestheader: length error\n");
		result = TEST_RESULT_FAIL;
	}
	cmp = memcmp(header, test_bbsmncache_testdata_02, header_len);
	if (cmp != 0) {
		printf("bbsmncache_getlatestheader: data error\n");
		result = TEST_RESULT_FAIL;
	}

	bbsmncache_delete(cache);

	return result;
}

LOCAL TEST_RESULT test_bbsmncache_2()
{
	return test_bbsmncache_2_testseq();
}

/* test_bbsmncache_3 */

LOCAL TEST_RESULT test_bbsmncache_3_testseq()
{
	bbsmncache_t *cache;
	TEST_RESULT result = TEST_RESULT_PASS;
	UB *header;
	W header_len;

	cache = bbsmncache_new();

	bbsmncache_getlatestheader(cache, &header, &header_len);
	if (header != NULL) {
		printf("bbsmncache_getlatestheader: data error\n");
		result = TEST_RESULT_FAIL;
	}
	if (header_len != 0) {
		printf("bbsmncache_getlatestheader: length error\n");
		result = TEST_RESULT_FAIL;
	}

	bbsmncache_delete(cache);

	return result;
}

LOCAL TEST_RESULT test_bbsmncache_3()
{
	return test_bbsmncache_3_testseq();
}

/* test_bbsmncache_4 */

LOCAL TEST_RESULT test_bbsmncache_4_testseq()
{
	W err;
	bbsmncache_t *cache;
	bbsmncache_datareadcontext_t *context;
	TEST_RESULT result = TEST_RESULT_PASS;
	Bool ok;

	cache = bbsmncache_new();

	err = bbsmncache_appenddata(cache, test_bbsmncache_testdata_01_1, strlen(test_bbsmncache_testdata_01_1));
	if (err < 0) {
		printf("bbsmncache_appenddata error 1\n");
		bbsmncache_delete(cache);
		return TEST_RESULT_FAIL;
	}
	err = bbsmncache_appenddata(cache, test_bbsmncache_testdata_01_2, strlen(test_bbsmncache_testdata_01_2));
	if (err < 0) {
		printf("bbsmncache_appenddata error 2\n");
		bbsmncache_delete(cache);
		return TEST_RESULT_FAIL;
	}
	err = bbsmncache_appenddata(cache, test_bbsmncache_testdata_01_3, strlen(test_bbsmncache_testdata_01_3));
	if (err < 0) {
		printf("bbsmncache_appenddata error 3\n");
		bbsmncache_delete(cache);
		return TEST_RESULT_FAIL;
	}
	err = bbsmncache_appenddata(cache, test_bbsmncache_testdata_01_4, strlen(test_bbsmncache_testdata_01_4));
	if (err < 0) {
		printf("bbsmncache_appenddata error 4\n");
		bbsmncache_delete(cache);
		return TEST_RESULT_FAIL;
	}

	context = bbsmncache_startdataread(cache, 0);
	if (context == NULL) {
		printf("bbsmncache_startdataread error\n");
		bbsmncache_delete(cache);
		return TEST_RESULT_FAIL;
	}
	ok = test_bbsmncache_util_cmp_ctx_str(context, test_bbsmncache_testdata_01, strlen(test_bbsmncache_testdata_01));
	if (ok != True) {
		result = TEST_RESULT_FAIL;
	}
	bbsmncache_enddataread(cache, context);

	bbsmncache_delete(cache);

	return result;
}

LOCAL TEST_RESULT test_bbsmncache_4()
{
	return test_bbsmncache_4_testseq();
}

/* test_bbsmncache_5 */

LOCAL TEST_RESULT test_bbsmncache_5_testseq()
{
	W err;
	bbsmncache_t *cache;
	bbsmncache_datareadcontext_t *context;
	TEST_RESULT result = TEST_RESULT_PASS;
	UB *bin_cache;
	W len_cache;
	Bool ok;

	cache = bbsmncache_new();

	err = bbsmncache_appenddata(cache, test_bbsmncache_testdata_01_1, strlen(test_bbsmncache_testdata_01_1));
	if (err < 0) {
		printf("bbsmncache_appenddata error 1\n");
		bbsmncache_delete(cache);
		return TEST_RESULT_FAIL;
	}
	err = bbsmncache_appenddata(cache, test_bbsmncache_testdata_01_2, strlen(test_bbsmncache_testdata_01_2));
	if (err < 0) {
		printf("bbsmncache_appenddata error 2\n");
		bbsmncache_delete(cache);
		return TEST_RESULT_FAIL;
	}
	err = bbsmncache_appenddata(cache, test_bbsmncache_testdata_01_3, strlen(test_bbsmncache_testdata_01_3));
	if (err < 0) {
		printf("bbsmncache_appenddata error 3\n");
		bbsmncache_delete(cache);
		return TEST_RESULT_FAIL;
	}
	err = bbsmncache_appenddata(cache, test_bbsmncache_testdata_01_4, strlen(test_bbsmncache_testdata_01_4));
	if (err < 0) {
		printf("bbsmncache_appenddata error 4\n");
		bbsmncache_delete(cache);
		return TEST_RESULT_FAIL;
	}

	context = bbsmncache_startdataread(cache, strlen(test_bbsmncache_testdata_01)+5);
	if (context == NULL) {
		bbsmncache_delete(cache);
		return TEST_RESULT_FAIL;
	}
	ok = bbsmncache_datareadcontext_nextdata(context, &bin_cache, &len_cache);
	if (ok == True) {
		result = TEST_RESULT_FAIL;
	}
	bbsmncache_enddataread(cache, context);

	bbsmncache_delete(cache);

	return result;
}

LOCAL TEST_RESULT test_bbsmncache_5()
{
	return test_bbsmncache_5_testseq();
}

LOCAL VOID test_bbsmncache_printresult(TEST_RESULT (*proc)(), B *test_name)
{
	TEST_RESULT result;

	printf("test_bbsmncache: %s\n", test_name);
	printf("---------------------------------------------\n");
	result = proc();
	if (result == TEST_RESULT_PASS) {
		printf("--pass---------------------------------------\n");
	} else {
		printf("--fail---------------------------------------\n");
	}
	printf("---------------------------------------------\n");
}

EXPORT VOID test_bbsmncache_main()
{
	test_bbsmncache_printresult(test_bbsmncache_1, "test_bbsmncache_1");
	test_bbsmncache_printresult(test_bbsmncache_2, "test_bbsmncache_2");
	test_bbsmncache_printresult(test_bbsmncache_3, "test_bbsmncache_3");
	test_bbsmncache_printresult(test_bbsmncache_4, "test_bbsmncache_4");
	test_bbsmncache_printresult(test_bbsmncache_5, "test_bbsmncache_5");
}
