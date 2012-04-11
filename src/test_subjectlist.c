/*
 * test_subjectlist.c
 *
 * Copyright (c) 2011-2012 project bchan
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

#include    "subjectlist.h"

#include    <btron/btron.h>
#include	<btron/dp.h>
#include	<tcode.h>
#include    <bstdio.h>
#include    <bstring.h>
#include    <bstdlib.h>
#include    <tstring.h>

#include    <unittest_driver.h>

#include    "subjectparser.h"
#include    "subjectcache.h"

LOCAL UB test_sbjtlist_testdata_01[] = {
	0x39, 0x32, 0x34, 0x30, 0x39, 0x31, 0x30, 0x30,
	0x31, 0x31, 0x2e, 0x64, 0x61, 0x74, 0x3c, 0x3e,
	0x81, 0x9a, 0x82, 0x51, 0x82, 0xbf, 0x82, 0xe1,
	0x82, 0xf1, 0x82, 0xcb, 0x82, 0xe9, 0x82, 0x50,
	0x82, 0x4f, 0x8e, 0xfc, 0x94, 0x4e, 0x8b, 0x4c,
	0x94, 0x4f, 0x81, 0x9a, 0x81, 0x40, 0x93, 0xfa,
	0x96, 0x7b, 0x83, 0x56, 0x83, 0x8a, 0x81, 0x5b,
	0x83, 0x59, 0x82, 0xf0, 0x97, 0x5c, 0x91, 0x7a,
	0x82, 0xb5, 0x82, 0xc4, 0x83, 0x76, 0x83, 0x8c,
	0x83, 0x5b, 0x83, 0x93, 0x83, 0x67, 0x82, 0xf0,
	0x83, 0x51, 0x83, 0x62, 0x83, 0x67, 0x21, 0x20,
	0x28, 0x31, 0x29, 0x0d, 0x0a, 0x31, 0x32, 0x32,
	0x39, 0x33, 0x34, 0x34, 0x30, 0x38, 0x39, 0x2e,
	0x64, 0x61, 0x74, 0x3c, 0x3e, 0x61, 0x61, 0x61,
	0x20, 0x62, 0x62, 0x62, 0x62, 0x20, 0x28, 0x34,
	0x38, 0x32, 0x29, 0x0d, 0x0a, 0x31, 0x32, 0x33,
	0x33, 0x38, 0x33, 0x33, 0x30, 0x32, 0x39, 0x2e,
	0x64, 0x61, 0x74, 0x3c, 0x3e, 0x81, 0x79, 0x96,
	0x88, 0x8c, 0x8e, 0x32, 0x93, 0xfa, 0x94, 0xad,
	0x94, 0x84, 0x81, 0x7a, 0x8c, 0xea, 0x82, 0xe9,
	0x83, 0x58, 0x83, 0x8c, 0x33, 0x20, 0x28, 0x39,
	0x32, 0x34, 0x29, 0x0d, 0x0a, 0x31, 0x32, 0x35,
	0x31, 0x33, 0x35, 0x35, 0x38, 0x36, 0x31, 0x2e,
	0x64, 0x61, 0x74, 0x3c, 0x3e, 0x83, 0x58, 0x83,
	0x8c, 0x20, 0x28, 0x31, 0x37, 0x35, 0x29, 0x0d,
	0x0a, 0x31, 0x32, 0x30, 0x31, 0x30, 0x37, 0x36,
	0x38, 0x34, 0x31, 0x2e, 0x64, 0x61, 0x74, 0x3c,
	0x3e, 0x82, 0xa0, 0x82, 0xe8, 0x82, 0xaa, 0x82,
	0xbf, 0x82, 0xc8, 0x82, 0xb1, 0x82, 0xc6, 0x20,
	0x28, 0x35, 0x32, 0x33, 0x29
};

LOCAL TC test_sbjtlist_title_01[] = {0x217a, 0x2332, 0x2441, 0x2463, 0x2473, 0x244d, 0x246b, 0x2331, 0x2330, 0x3c7e, 0x472f, 0x352d, 0x4730, 0x217a, 0x2121, 0x467c, 0x4b5c, 0x2537, 0x256a, 0x213c, 0x253a, 0x2472, 0x4d3d, 0x415b, 0x2437, 0x2446, 0x2557, 0x256c, 0x253c, 0x2573, 0x2548, 0x2472, 0x2532, 0x2543, 0x2548, 0x212a, TNULL};
LOCAL TC test_sbjtlist_title_02[] = {0x2361, 0x2361, 0x2361, 0x2121, 0x2362, 0x2362, 0x2362, 0x2362, TNULL};
LOCAL TC test_sbjtlist_title_03[] = {0x215a, 0x4b68, 0x376e, 0x2332, 0x467c, 0x482f, 0x4764, 0x215b, 0x386c, 0x246b, 0x2539, 0x256c, 0x2333, TNULL};
LOCAL TC test_sbjtlist_title_04[] = {0x2539, 0x256c, TNULL};

struct testsbjtlist_expected_t_ {
	W num;
	TC *title;
	W title_len;
};
typedef struct testsbjtlist_expected_t_ testsbjtlist_expected_t;

LOCAL UNITTEST_RESULT test_sbjtlist_checksort(UB *testdata, W testdata_len, W sortby, Bool descendant, TC *filterword, W filterword_len, testsbjtlist_expected_t *expected, W expected_len)
{
	W err, num, len, i;
	Bool next;
	TC *title;
	sbjtlist_t *list;
	sbjtlist_iterator_t *list_iter;
	sbjtlist_tuple_t *tuple;
	sbjtcache_t *cache;
	sbjtparser_t *parser;
	sbjtparser_thread_t *thread = NULL;
	UNITTEST_RESULT result = UNITTEST_RESULT_PASS;

	cache = sbjtcache_new();
	sbjtcache_appenddata(cache, testdata, testdata_len);

	parser = sbjtparser_new(cache);
	list = sbjtlist_new();

	for (;;) {
		err = sbjtparser_getnextthread(parser, &thread);
		if (err != 1) {
			break;
		}
		if (thread != NULL) {
			sbjtlist_appendthread(list, thread, 1300000000);
		} else {
			break;
		}
	}

	sbjtlist_sort(list, sortby, filterword, filterword_len);

	list_iter = sbjtlist_startread(list, descendant);
	for (i = 0;; i++) {
		next = sbjtlist_iterator_next(list_iter, &tuple);
		if (next == False) {
			break;
		}
		if (i >= expected_len) {
			result = UNITTEST_RESULT_FAIL;
			break;
		}
		sbjtlist_tuple_gettitle(tuple, &title, &len);
		sbjtlist_tuple_getnumber(tuple, &num);
		if (expected[i].num != num) {
			result = UNITTEST_RESULT_FAIL;
			break;
		}
		if (expected[i].title_len != len) {
			result = UNITTEST_RESULT_FAIL;
			break;
		}
		if (tc_strncmp(title, expected[i].title, len) != 0) {
			result = UNITTEST_RESULT_FAIL;
			break;
		}
	}
	sbjtlist_endread(list, list_iter);

	if ((result = UNITTEST_RESULT_PASS) && (i != expected_len)) {
		result = UNITTEST_RESULT_FAIL;
	}

	sbjtlist_delete(list);
	sbjtparser_delete(parser);

	sbjtcache_delete(cache);

	return result;
}

LOCAL UNITTEST_RESULT test_sbjtlist_1()
{
	testsbjtlist_expected_t expected[] = {
		{
			1,
			test_sbjtlist_title_01,
			tc_strlen(test_sbjtlist_title_01)
		},
		{
			2,
			test_sbjtlist_title_02,
			tc_strlen(test_sbjtlist_title_02)
		},
		{
			3,
			test_sbjtlist_title_03,
			tc_strlen(test_sbjtlist_title_03)
		},
		{
			4,
			test_sbjtlist_title_04,
			tc_strlen(test_sbjtlist_title_04)
		},
	};
	return test_sbjtlist_checksort(test_sbjtlist_testdata_01, strlen(test_sbjtlist_testdata_01), SBJTLIST_SORTBY_NUMBER, False, NULL, 0, expected, sizeof(expected));
}

LOCAL UNITTEST_RESULT test_sbjtlist_2()
{
	testsbjtlist_expected_t expected[] = {
		{
			1,
			test_sbjtlist_title_01,
			tc_strlen(test_sbjtlist_title_01)
		},
		{
			4,
			test_sbjtlist_title_04,
			tc_strlen(test_sbjtlist_title_04)
		},
		{
			2,
			test_sbjtlist_title_02,
			tc_strlen(test_sbjtlist_title_02)
		},
		{
			3,
			test_sbjtlist_title_03,
			tc_strlen(test_sbjtlist_title_03)
		},
	};
	return test_sbjtlist_checksort(test_sbjtlist_testdata_01, strlen(test_sbjtlist_testdata_01), SBJTLIST_SORTBY_RES, False, NULL, 0, expected, sizeof(expected));
}

LOCAL UNITTEST_RESULT test_sbjtlist_3()
{
	testsbjtlist_expected_t expected[] = {
		{
			2,
			test_sbjtlist_title_02,
			tc_strlen(test_sbjtlist_title_02)
		},
		{
			3,
			test_sbjtlist_title_03,
			tc_strlen(test_sbjtlist_title_03)
		},
		{
			4,
			test_sbjtlist_title_04,
			tc_strlen(test_sbjtlist_title_04)
		},
		{
			1,
			test_sbjtlist_title_01,
			tc_strlen(test_sbjtlist_title_01)
		},
	};
	return test_sbjtlist_checksort(test_sbjtlist_testdata_01, strlen(test_sbjtlist_testdata_01), SBJTLIST_SORTBY_SINCE, False, NULL, 0, expected, sizeof(expected));
}

LOCAL UNITTEST_RESULT test_sbjtlist_4()
{
	testsbjtlist_expected_t expected[] = {
		{
			1,
			test_sbjtlist_title_01,
			tc_strlen(test_sbjtlist_title_01)
		}, /* vigor = 0.00 */
		{
			4,
			test_sbjtlist_title_04,
			tc_strlen(test_sbjtlist_title_04)
		}, /* vigor = 0.07 */
		{
			2,
			test_sbjtlist_title_02,
			tc_strlen(test_sbjtlist_title_02)
		}, /* vigor = 0.14 */
		{
			3,
			test_sbjtlist_title_03,
			tc_strlen(test_sbjtlist_title_03)
		}, /* vigor = 0.02 */
	};
	return test_sbjtlist_checksort(test_sbjtlist_testdata_01, strlen(test_sbjtlist_testdata_01), SBJTLIST_SORTBY_VIGOR, False, NULL, 0, expected, sizeof(expected));
}

LOCAL UNITTEST_RESULT test_sbjtlist_5()
{
	testsbjtlist_expected_t expected[] = {
		{
			4,
			test_sbjtlist_title_04,
			tc_strlen(test_sbjtlist_title_04)
		},
		{
			3,
			test_sbjtlist_title_03,
			tc_strlen(test_sbjtlist_title_03)
		},
		{
			2,
			test_sbjtlist_title_02,
			tc_strlen(test_sbjtlist_title_02)
		},
		{
			1,
			test_sbjtlist_title_01,
			tc_strlen(test_sbjtlist_title_01)
		},
	};
	return test_sbjtlist_checksort(test_sbjtlist_testdata_01, strlen(test_sbjtlist_testdata_01), SBJTLIST_SORTBY_NUMBER, True, NULL, 0, expected, sizeof(expected));
}

LOCAL UNITTEST_RESULT test_sbjtlist_6()
{
	testsbjtlist_expected_t expected[] = {
		{
			3,
			test_sbjtlist_title_03,
			tc_strlen(test_sbjtlist_title_03)
		},
		{
			2,
			test_sbjtlist_title_02,
			tc_strlen(test_sbjtlist_title_02)
		},
		{
			4,
			test_sbjtlist_title_04,
			tc_strlen(test_sbjtlist_title_04)
		},
		{
			1,
			test_sbjtlist_title_01,
			tc_strlen(test_sbjtlist_title_01)
		},
	};
	return test_sbjtlist_checksort(test_sbjtlist_testdata_01, strlen(test_sbjtlist_testdata_01), SBJTLIST_SORTBY_RES, True, NULL, 0, expected, sizeof(expected));
}

LOCAL UNITTEST_RESULT test_sbjtlist_7()
{
	testsbjtlist_expected_t expected[] = {
		{
			1,
			test_sbjtlist_title_01,
			tc_strlen(test_sbjtlist_title_01)
		},
		{
			4,
			test_sbjtlist_title_04,
			tc_strlen(test_sbjtlist_title_04)
		},
		{
			3,
			test_sbjtlist_title_03,
			tc_strlen(test_sbjtlist_title_03)
		},
		{
			2,
			test_sbjtlist_title_02,
			tc_strlen(test_sbjtlist_title_02)
		},
	};
	return test_sbjtlist_checksort(test_sbjtlist_testdata_01, strlen(test_sbjtlist_testdata_01), SBJTLIST_SORTBY_SINCE, True, NULL, 0, expected, sizeof(expected));
}

LOCAL UNITTEST_RESULT test_sbjtlist_8()
{
	testsbjtlist_expected_t expected[] = {
		{
			3,
			test_sbjtlist_title_03,
			tc_strlen(test_sbjtlist_title_03)
		}, /* vigor = 0.02 */
		{
			2,
			test_sbjtlist_title_02,
			tc_strlen(test_sbjtlist_title_02)
		}, /* vigor = 0.14 */
		{
			4,
			test_sbjtlist_title_04,
			tc_strlen(test_sbjtlist_title_04)
		}, /* vigor = 0.07 */
		{
			1,
			test_sbjtlist_title_01,
			tc_strlen(test_sbjtlist_title_01)
		}, /* vigor = 0.00 */
	};
	return test_sbjtlist_checksort(test_sbjtlist_testdata_01, strlen(test_sbjtlist_testdata_01), SBJTLIST_SORTBY_VIGOR, True, NULL, 0, expected, sizeof(expected));
}

LOCAL UNITTEST_RESULT test_sbjtlist_9()
{
	testsbjtlist_expected_t expected[] = {
		{
			1,
			test_sbjtlist_title_01,
			tc_strlen(test_sbjtlist_title_01)
		},
	};
	TC filter[] = {0x2332, 0x2441, 0x2463, 0x2473, 0x244d, 0x246b, TNULL};
	return test_sbjtlist_checksort(test_sbjtlist_testdata_01, strlen(test_sbjtlist_testdata_01), SBJTLIST_SORTBY_NUMBER, False, filter, tc_strlen(filter), expected, sizeof(expected));
}

LOCAL UNITTEST_RESULT test_sbjtlist_10()
{
	testsbjtlist_expected_t expected[] = {
		{
			2,
			test_sbjtlist_title_02,
			tc_strlen(test_sbjtlist_title_02)
		},
	};
	TC filter[] = {0x2361, 0x2361, TNULL};
	return test_sbjtlist_checksort(test_sbjtlist_testdata_01, strlen(test_sbjtlist_testdata_01), SBJTLIST_SORTBY_NUMBER, False, filter, tc_strlen(filter), expected, sizeof(expected));
}

LOCAL UNITTEST_RESULT test_sbjtlist_11()
{
	testsbjtlist_expected_t expected[] = {
		{
			3,
			test_sbjtlist_title_03,
			tc_strlen(test_sbjtlist_title_03)
		},
		{
			4,
			test_sbjtlist_title_04,
			tc_strlen(test_sbjtlist_title_04)
		},
	};
	TC filter[] = {0x2539, 0x256c, TNULL};
	return test_sbjtlist_checksort(test_sbjtlist_testdata_01, strlen(test_sbjtlist_testdata_01), SBJTLIST_SORTBY_NUMBER, False, filter, tc_strlen(filter), expected, sizeof(expected));
}

LOCAL UNITTEST_RESULT test_sbjtlist_12()
{
	testsbjtlist_expected_t expected[] = {
		{
			1,
			test_sbjtlist_title_01,
			tc_strlen(test_sbjtlist_title_01)
		},
	};
	TC filter[] = {0x2532, 0x2543, 0x2548, 0x212a, TNULL};
	return test_sbjtlist_checksort(test_sbjtlist_testdata_01, strlen(test_sbjtlist_testdata_01), SBJTLIST_SORTBY_NUMBER, False, filter, tc_strlen(filter), expected, sizeof(expected));
}

LOCAL UNITTEST_RESULT test_sbjtlist_13()
{
	testsbjtlist_expected_t expected[] = {
	};
	TC filter[] = {0x2422, TNULL};
	return test_sbjtlist_checksort(test_sbjtlist_testdata_01, strlen(test_sbjtlist_testdata_01), SBJTLIST_SORTBY_NUMBER, False, filter, tc_strlen(filter), expected, sizeof(expected));
}

LOCAL UNITTEST_RESULT test_sbjtlist_14()
{
	testsbjtlist_expected_t expected[] = {
	};
	TC filter[] = {0x2332, 0x2441, 0x2463, 0x2473, 0x244d, 0x2422, TNULL};
	return test_sbjtlist_checksort(test_sbjtlist_testdata_01, strlen(test_sbjtlist_testdata_01), SBJTLIST_SORTBY_NUMBER, False, filter, tc_strlen(filter), expected, sizeof(expected));
}

EXPORT VOID test_sbjtlist_main(unittest_driver_t *driver)
{
	UNITTEST_DRIVER_REGIST(driver, test_sbjtlist_1);
	UNITTEST_DRIVER_REGIST(driver, test_sbjtlist_2);
	UNITTEST_DRIVER_REGIST(driver, test_sbjtlist_3);
	UNITTEST_DRIVER_REGIST(driver, test_sbjtlist_4);
	UNITTEST_DRIVER_REGIST(driver, test_sbjtlist_5);
	UNITTEST_DRIVER_REGIST(driver, test_sbjtlist_6);
	UNITTEST_DRIVER_REGIST(driver, test_sbjtlist_7);
	UNITTEST_DRIVER_REGIST(driver, test_sbjtlist_8);
	UNITTEST_DRIVER_REGIST(driver, test_sbjtlist_9);
	UNITTEST_DRIVER_REGIST(driver, test_sbjtlist_10);
	UNITTEST_DRIVER_REGIST(driver, test_sbjtlist_11);
	UNITTEST_DRIVER_REGIST(driver, test_sbjtlist_12);
	UNITTEST_DRIVER_REGIST(driver, test_sbjtlist_13);
	UNITTEST_DRIVER_REGIST(driver, test_sbjtlist_14);
}
