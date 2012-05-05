/*
 * test_util.c
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

#include    "util.h"

#include    <btron/btron.h>
#include	<tcode.h>
#include    <bstdio.h>
#include    <bstring.h>
#include    <tstring.h>

#include    <unittest_driver.h>

LOCAL UNITTEST_RESULT test_util_urlcheck_common(TC *url, W url_len, TCURL_CHECK_VALID_BBSURL expected)
{
	TCURL_CHECK_VALID_BBSURL ret;
	ret = tcurl_check_valid_bbsurl(url, url_len);
	if (ret != expected) {
		return UNITTEST_RESULT_FAIL;
	}
	return UNITTEST_RESULT_PASS;
}

LOCAL UNITTEST_RESULT test_util_urlcheck_1()
{
	TC url[] = {TK_h, TK_t, TK_t, TK_p, TK_COLN, TK_SLSH, TK_SLSH, TK_a, TK_a, TK_b, TK_SLSH, TK_b, TK_b, TK_b, TK_SLSH, TNULL};
	W url_len = tc_strlen(url);
	TCURL_CHECK_VALID_BBSURL expected = TCURL_CHECK_VALID_BBSURL_OK;
	return test_util_urlcheck_common(url, url_len, expected);
}

LOCAL UNITTEST_RESULT test_util_urlcheck_2()
{
	TC url[] = {TK_h, TK_t, TK_t, TK_p, TK_s, TK_COLN, TK_SLSH, TK_SLSH, TK_a, TK_a, TK_b, TK_SLSH, TK_b, TK_b, TK_b, TK_SLSH, TNULL};
	W url_len = tc_strlen(url);
	TCURL_CHECK_VALID_BBSURL expected = TCURL_CHECK_VALID_BBSURL_INVALID_SCHEME;
	return test_util_urlcheck_common(url, url_len, expected);
}

LOCAL UNITTEST_RESULT test_util_urlcheck_3()
{
	TC url[] = {TK_h, TK_t, TK_t, TK_p, TK_COLN, TK_SLSH, TNULL};
	W url_len = tc_strlen(url);
	TCURL_CHECK_VALID_BBSURL expected = TCURL_CHECK_VALID_BBSURL_INVALID_SCHEME;
	return test_util_urlcheck_common(url, url_len, expected);
}

LOCAL UNITTEST_RESULT test_util_urlcheck_4()
{
	TC url[] = {TK_h, TK_t, TK_t, TK_p, TK_COLN, TK_SLSH, TNULL};
	W url_len = tc_strlen(url);
	TCURL_CHECK_VALID_BBSURL expected = TCURL_CHECK_VALID_BBSURL_INVALID_SCHEME;
	return test_util_urlcheck_common(url, url_len, expected);
}

LOCAL UNITTEST_RESULT test_util_urlcheck_5()
{
	TC url[] = {TK_h, TK_t, TK_t, TK_p, TK_COLN, TNULL};
	W url_len = tc_strlen(url);
	TCURL_CHECK_VALID_BBSURL expected = TCURL_CHECK_VALID_BBSURL_INVALID_SCHEME;
	return test_util_urlcheck_common(url, url_len, expected);
}

LOCAL UNITTEST_RESULT test_util_urlcheck_6()
{
	TC url[] = {TK_h, TK_t, TK_t, TK_p, TK_COLN, TK_SLSH, TK_SLSH, TK_SLSH, TK_b, TK_b, TK_b, TK_SLSH, TNULL};
	W url_len = tc_strlen(url);
	TCURL_CHECK_VALID_BBSURL expected = TCURL_CHECK_VALID_BBSURL_INVALID_HOST;
	return test_util_urlcheck_common(url, url_len, expected);
}

LOCAL UNITTEST_RESULT test_util_urlcheck_7()
{
	TC url[] = {TK_h, TK_t, TK_t, TK_p, TK_COLN, TK_SLSH, TK_SLSH, TK_a, TK_a, TK_b, TNULL};
	W url_len = tc_strlen(url);
	TCURL_CHECK_VALID_BBSURL expected = TCURL_CHECK_VALID_BBSURL_INVALID_PATH;
	return test_util_urlcheck_common(url, url_len, expected);
}

LOCAL UNITTEST_RESULT test_util_urlcheck_8()
{
	TC url[] = {TK_h, TK_t, TK_t, TK_p, TK_COLN, TK_SLSH, TK_SLSH, TK_a, TK_a, TK_b, TK_SLSH, TNULL};
	W url_len = tc_strlen(url);
	TCURL_CHECK_VALID_BBSURL expected = TCURL_CHECK_VALID_BBSURL_INVALID_PATH;
	return test_util_urlcheck_common(url, url_len, expected);
}

LOCAL UNITTEST_RESULT test_util_urlcheck_9()
{
	TC url[] = {TK_h, TK_t, TK_t, TK_p, TK_COLN, TK_SLSH, TK_SLSH, TK_a, TK_a, TK_b, TK_SLSH, TK_b, TK_b, TK_b, TNULL};
	W url_len = tc_strlen(url);
	TCURL_CHECK_VALID_BBSURL expected = TCURL_CHECK_VALID_BBSURL_NO_LAST_SLSH;
	return test_util_urlcheck_common(url, url_len, expected);
}

LOCAL UNITTEST_RESULT test_util_urlcheck_10()
{
	TC url[] = {TK_h, TK_t, TK_t, TK_p, TK_COLN, TK_SLSH, TK_SLSH, TK_a, TK_a, TK_b, TK_SLSH, TK_b, TK_b, TK_b, TK_SLSH, TK_c, TK_c, TK_c, TNULL};
	W url_len = tc_strlen(url);
	TCURL_CHECK_VALID_BBSURL expected = TCURL_CHECK_VALID_BBSURL_INVALID_PATH;
	return test_util_urlcheck_common(url, url_len, expected);
}

LOCAL UNITTEST_RESULT test_util_urlcheck_11()
{
	TC url[] = {TK_h, TK_t, TK_t, TK_p, TK_COLN, TK_SLSH, TK_SLSH, TK_a, TK_a, TK_b, TK_SLSH, TK_b, TK_b, TK_b, TK_SLSH, TK_c, TK_c, TK_c, TK_SLSH, TNULL};
	W url_len = tc_strlen(url);
	TCURL_CHECK_VALID_BBSURL expected = TCURL_CHECK_VALID_BBSURL_INVALID_PATH;
	return test_util_urlcheck_common(url, url_len, expected);
}

LOCAL UNITTEST_RESULT test_util_urlcheck_12()
{
	TC url[] = {TK_h, TK_t, TK_t, TK_p, TK_COLN, TK_SLSH, TK_SLSH, TNULL};
	W url_len = tc_strlen(url);
	TCURL_CHECK_VALID_BBSURL expected = TCURL_CHECK_VALID_BBSURL_INVALID_HOST;
	return test_util_urlcheck_common(url, url_len, expected);
}

EXPORT VOID test_util_main(unittest_driver_t *driver)
{
	UNITTEST_DRIVER_REGIST(driver, test_util_urlcheck_1);
	UNITTEST_DRIVER_REGIST(driver, test_util_urlcheck_2);
	UNITTEST_DRIVER_REGIST(driver, test_util_urlcheck_3);
	UNITTEST_DRIVER_REGIST(driver, test_util_urlcheck_4);
	UNITTEST_DRIVER_REGIST(driver, test_util_urlcheck_5);
	UNITTEST_DRIVER_REGIST(driver, test_util_urlcheck_6);
	UNITTEST_DRIVER_REGIST(driver, test_util_urlcheck_7);
	UNITTEST_DRIVER_REGIST(driver, test_util_urlcheck_8);
	UNITTEST_DRIVER_REGIST(driver, test_util_urlcheck_9);
	UNITTEST_DRIVER_REGIST(driver, test_util_urlcheck_10);
	UNITTEST_DRIVER_REGIST(driver, test_util_urlcheck_11);
	UNITTEST_DRIVER_REGIST(driver, test_util_urlcheck_12);
}
