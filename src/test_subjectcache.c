/*
 * test_subjectcache.c
 *
 * Copyright (c) 2009-2015 project bchan
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

#include    "subjectcache.h"

#include    <btron/btron.h>
#include	<tcode.h>
#include    <bstdio.h>
#include    <bstring.h>
#include    <bstdlib.h>
#include	<errcode.h>

#include    <unittest_driver.h>

LOCAL UB test_sbjtcache_testdata_01[] = {"aaaaabbbbbcccccddddd"};
LOCAL UB test_sbjtcache_testdata_01_1[] = {"aaaaa"};
LOCAL UB test_sbjtcache_testdata_01_2[] = {"bbbbb"};
LOCAL UB test_sbjtcache_testdata_01_3[] = {"ccccc"};
LOCAL UB test_sbjtcache_testdata_01_4[] = {"ddddd"};
LOCAL UB test_sbjtcache_testdata_02[] = {"XXX abcdef\r\nAAAA: valueA\r\nBBBB: valueB\r\n\r\n"};
LOCAL UB test_sbjtcache_testdata_04[] = {"aaa.bbb.ccc.jp"};
LOCAL UB test_sbjtcache_testdata_05[] = {"thread"};
LOCAL UB test_sbjtcache_testdata_07[] = {"XXX abcdef\r\nAAAA: valueC\r\nBBBB: valueC\r\n\r\n"};

LOCAL Bool test_sbjtcache_util_cmp_ctx_str(sbjtcache_datareadcontext_t *context, UB *data, W len)
{
	Bool cont;
	UB *bin_cache;
	W i = 0, len_cache, cmp;

	for (;;) {
		cont = sbjtcache_datareadcontext_nextdata(context, &bin_cache, &len_cache);
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

/* test_sbjtcache_1 */

LOCAL UNITTEST_RESULT test_sbjtcache_1()
{
	sbjtcache_t *cache;
	UNITTEST_RESULT result = UNITTEST_RESULT_PASS;
	UB *host;
	W host_len;

	cache = sbjtcache_new();

	sbjtcache_gethost(cache, &host, &host_len);
	if ((host != NULL)||(host_len != 0)) {
		result = UNITTEST_RESULT_FAIL;
	}

	sbjtcache_delete(cache);

	return result;
}

/* test_sbjtcache_2 */

LOCAL UNITTEST_RESULT test_sbjtcache_2()
{
	sbjtcache_t *cache;
	UNITTEST_RESULT result = UNITTEST_RESULT_PASS;
	UB *host;
	W err, host_len, cmp;

	cache = sbjtcache_new();

	err = sbjtcache_updatehost(cache, test_sbjtcache_testdata_04, strlen(test_sbjtcache_testdata_04));
	if (err < 0) {
		result = UNITTEST_RESULT_FAIL;
	}

	sbjtcache_gethost(cache, &host, &host_len);
	cmp = memcmp(host, test_sbjtcache_testdata_04, host_len);
	if (cmp != 0) {
		printf("sbjtcache_gethost: data error\n");
		result = UNITTEST_RESULT_FAIL;
	}

	sbjtcache_delete(cache);

	return result;
}

/* test_sbjtcache_3 */

LOCAL UNITTEST_RESULT test_sbjtcache_3()
{
	sbjtcache_t *cache;
	UNITTEST_RESULT result = UNITTEST_RESULT_PASS;
	UB *board;
	W board_len;

	cache = sbjtcache_new();

	sbjtcache_getboard(cache, &board, &board_len);
	if ((board != NULL)||(board_len != 0)) {
		result = UNITTEST_RESULT_FAIL;
	}

	sbjtcache_delete(cache);

	return result;
}

/* test_sbjtcache_4 */

LOCAL UNITTEST_RESULT test_sbjtcache_4()
{
	sbjtcache_t *cache;
	UNITTEST_RESULT result = UNITTEST_RESULT_PASS;
	UB *board;
	W err, board_len, cmp;

	cache = sbjtcache_new();

	err = sbjtcache_updateboard(cache, test_sbjtcache_testdata_05, strlen(test_sbjtcache_testdata_05));
	if (err < 0) {
		result = UNITTEST_RESULT_FAIL;
	}

	sbjtcache_getboard(cache, &board, &board_len);
	cmp = memcmp(board, test_sbjtcache_testdata_05, board_len);
	if (cmp != 0) {
		printf("sbjtcache_getboard: data error\n");
		result = UNITTEST_RESULT_FAIL;
	}

	sbjtcache_delete(cache);

	return result;
}

/* test_sbjtcache_5 */

LOCAL UNITTEST_RESULT test_sbjtcache_5_testseq()
{
	sbjtcache_t *cache;
	UNITTEST_RESULT result = UNITTEST_RESULT_PASS;
	UB *header;
	W err, header_len, cmp;

	cache = sbjtcache_new();

	err = sbjtcache_updatelatestheader(cache, test_sbjtcache_testdata_07, strlen(test_sbjtcache_testdata_07));
	if (err < 0) {
		printf("sbjtcache_updatelataestheade error\n");
		sbjtcache_delete(cache);
		return UNITTEST_RESULT_FAIL;
	}

	sbjtcache_getlatestheader(cache, &header, &header_len);
	if (header_len != strlen(test_sbjtcache_testdata_07)) {
		printf("sbjtcache_getlatestheader: length error\n");
		result = UNITTEST_RESULT_FAIL;
	}
	cmp = memcmp(header, test_sbjtcache_testdata_07, header_len);
	if (cmp != 0) {
		printf("sbjtcache_getlatestheader: data error\n");
		result = UNITTEST_RESULT_FAIL;
	}

	sbjtcache_delete(cache);

	return result;
}

LOCAL UNITTEST_RESULT test_sbjtcache_5()
{
	return test_sbjtcache_5_testseq();
}

/* test_sbjtcache_6 */

LOCAL UNITTEST_RESULT test_sbjtcache_6_testseq()
{
	sbjtcache_t *cache;
	UNITTEST_RESULT result = UNITTEST_RESULT_PASS;
	UB *header;
	W err, header_len, cmp;

	cache = sbjtcache_new();

	err = sbjtcache_updatelatestheader(cache, test_sbjtcache_testdata_07, strlen(test_sbjtcache_testdata_07));
	if (err < 0) {
		printf("sbjtcache_updatelataestheade error\n");
		sbjtcache_delete(cache);
		return UNITTEST_RESULT_FAIL;
	}
	err = sbjtcache_updatelatestheader(cache, test_sbjtcache_testdata_02, strlen(test_sbjtcache_testdata_02));
	if (err < 0) {
		printf("sbjtcache_updatelataestheade error\n");
		sbjtcache_delete(cache);
		return UNITTEST_RESULT_FAIL;
	}

	sbjtcache_getlatestheader(cache, &header, &header_len);
	if (header_len != strlen(test_sbjtcache_testdata_02)) {
		printf("sbjtcache_getlatestheader: length error\n");
		result = UNITTEST_RESULT_FAIL;
	}
	cmp = memcmp(header, test_sbjtcache_testdata_02, header_len);
	if (cmp != 0) {
		printf("sbjtcache_getlatestheader: data error\n");
		result = UNITTEST_RESULT_FAIL;
	}

	sbjtcache_delete(cache);

	return result;
}

LOCAL UNITTEST_RESULT test_sbjtcache_6()
{
	return test_sbjtcache_6_testseq();
}

/* test_sbjtcache_7 */

LOCAL UNITTEST_RESULT test_sbjtcache_7_testseq()
{
	sbjtcache_t *cache;
	UNITTEST_RESULT result = UNITTEST_RESULT_PASS;
	UB *header;
	W header_len;

	cache = sbjtcache_new();

	sbjtcache_getlatestheader(cache, &header, &header_len);
	if (header != NULL) {
		printf("sbjtcache_getlatestheader: data error\n");
		result = UNITTEST_RESULT_FAIL;
	}
	if (header_len != 0) {
		printf("sbjtcache_getlatestheader: length error\n");
		result = UNITTEST_RESULT_FAIL;
	}

	sbjtcache_delete(cache);

	return result;
}

LOCAL UNITTEST_RESULT test_sbjtcache_7()
{
	return test_sbjtcache_7_testseq();
}

/* test_sbjtcache_8 */

LOCAL UNITTEST_RESULT test_sbjtcache_8_testseq()
{
	W err;
	sbjtcache_t *cache;
	sbjtcache_datareadcontext_t *context;
	UNITTEST_RESULT result = UNITTEST_RESULT_PASS;
	Bool ok;

	cache = sbjtcache_new();

	err = sbjtcache_appenddata(cache, test_sbjtcache_testdata_01_1, strlen(test_sbjtcache_testdata_01_1));
	if (err < 0) {
		printf("sbjtcache_appenddata error 1\n");
		sbjtcache_delete(cache);
		return UNITTEST_RESULT_FAIL;
	}
	err = sbjtcache_appenddata(cache, test_sbjtcache_testdata_01_2, strlen(test_sbjtcache_testdata_01_2));
	if (err < 0) {
		printf("sbjtcache_appenddata error 2\n");
		sbjtcache_delete(cache);
		return UNITTEST_RESULT_FAIL;
	}
	err = sbjtcache_appenddata(cache, test_sbjtcache_testdata_01_3, strlen(test_sbjtcache_testdata_01_3));
	if (err < 0) {
		printf("sbjtcache_appenddata error 3\n");
		sbjtcache_delete(cache);
		return UNITTEST_RESULT_FAIL;
	}
	err = sbjtcache_appenddata(cache, test_sbjtcache_testdata_01_4, strlen(test_sbjtcache_testdata_01_4));
	if (err < 0) {
		printf("sbjtcache_appenddata error 4\n");
		sbjtcache_delete(cache);
		return UNITTEST_RESULT_FAIL;
	}

	context = sbjtcache_startdataread(cache, 0);
	if (context == NULL) {
		printf("sbjtcache_startdataread error\n");
		sbjtcache_delete(cache);
		return UNITTEST_RESULT_FAIL;
	}
	ok = test_sbjtcache_util_cmp_ctx_str(context, test_sbjtcache_testdata_01, strlen(test_sbjtcache_testdata_01));
	if (ok != True) {
		result = UNITTEST_RESULT_FAIL;
	}
	sbjtcache_enddataread(cache, context);

	sbjtcache_delete(cache);

	return result;
}

LOCAL UNITTEST_RESULT test_sbjtcache_8()
{
	return test_sbjtcache_8_testseq();
}

/* test_sbjtcache_9 */

LOCAL UNITTEST_RESULT test_sbjtcache_9_testseq()
{
	W err;
	sbjtcache_t *cache;
	sbjtcache_datareadcontext_t *context;
	UNITTEST_RESULT result = UNITTEST_RESULT_PASS;
	UB *bin_cache;
	W len_cache;
	Bool ok;

	cache = sbjtcache_new();

	err = sbjtcache_appenddata(cache, test_sbjtcache_testdata_01_1, strlen(test_sbjtcache_testdata_01_1));
	if (err < 0) {
		printf("sbjtcache_appenddata error 1\n");
		sbjtcache_delete(cache);
		return UNITTEST_RESULT_FAIL;
	}
	err = sbjtcache_appenddata(cache, test_sbjtcache_testdata_01_2, strlen(test_sbjtcache_testdata_01_2));
	if (err < 0) {
		printf("sbjtcache_appenddata error 2\n");
		sbjtcache_delete(cache);
		return UNITTEST_RESULT_FAIL;
	}
	err = sbjtcache_appenddata(cache, test_sbjtcache_testdata_01_3, strlen(test_sbjtcache_testdata_01_3));
	if (err < 0) {
		printf("sbjtcache_appenddata error 3\n");
		sbjtcache_delete(cache);
		return UNITTEST_RESULT_FAIL;
	}
	err = sbjtcache_appenddata(cache, test_sbjtcache_testdata_01_4, strlen(test_sbjtcache_testdata_01_4));
	if (err < 0) {
		printf("sbjtcache_appenddata error 4\n");
		sbjtcache_delete(cache);
		return UNITTEST_RESULT_FAIL;
	}

	context = sbjtcache_startdataread(cache, strlen(test_sbjtcache_testdata_01)+5);
	if (context == NULL) {
		sbjtcache_delete(cache);
		return UNITTEST_RESULT_FAIL;
	}
	ok = sbjtcache_datareadcontext_nextdata(context, &bin_cache, &len_cache);
	if (ok == True) {
		result = UNITTEST_RESULT_FAIL;
	}
	sbjtcache_enddataread(cache, context);

	sbjtcache_delete(cache);

	return result;
}

LOCAL UNITTEST_RESULT test_sbjtcache_9()
{
	return test_sbjtcache_9_testseq();
}

/* test_sbjtcache_10 */

LOCAL UNITTEST_RESULT test_sbjtcache_10()
{
	sbjtcache_t *cache;
	UNITTEST_RESULT result = UNITTEST_RESULT_PASS;
	UH port;
	W err, board_len, cmp;

	cache = sbjtcache_new();

	sbjtcache_updateport(cache, 12345);
	sbjtcache_getport(cache, &port);
	if (port != 12345) {
		printf("sbjtcache_getport: data error\n");
		result = UNITTEST_RESULT_FAIL;
	}

	sbjtcache_delete(cache);

	return result;
}

EXPORT VOID test_sbjtcache_main(unittest_driver_t *driver)
{
	UNITTEST_DRIVER_REGIST(driver, test_sbjtcache_1);
	UNITTEST_DRIVER_REGIST(driver, test_sbjtcache_2);
	UNITTEST_DRIVER_REGIST(driver, test_sbjtcache_3);
	UNITTEST_DRIVER_REGIST(driver, test_sbjtcache_4);
	UNITTEST_DRIVER_REGIST(driver, test_sbjtcache_5);
	UNITTEST_DRIVER_REGIST(driver, test_sbjtcache_6);
	UNITTEST_DRIVER_REGIST(driver, test_sbjtcache_7);
	UNITTEST_DRIVER_REGIST(driver, test_sbjtcache_8);
	UNITTEST_DRIVER_REGIST(driver, test_sbjtcache_9);
	UNITTEST_DRIVER_REGIST(driver, test_sbjtcache_10);
}
