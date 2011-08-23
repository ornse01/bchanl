/*
 * test_subjectlist.c
 *
 * Copyright (c) 2011 project bchan
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
#include	<btron/dp.h>
#include	<tcode.h>
#include    <bstdio.h>
#include    <bstring.h>
#include    <bstdlib.h>

#include    "test.h"

#include    "subjectlist.h"
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

LOCAL TEST_RESULT test_sbjtlist_1()
{
	W err, num, len;
	Bool next;
	TC *title;
	sbjtlist_t *list;
	sbjtlist_iterator_t *list_iter;
	sbjtlist_tuple_t *tuple;
	sbjtcache_t *cache;
	sbjtparser_t *parser;
	sbjtparser_thread_t *thread = NULL;
	TEST_RESULT result = TEST_RESULT_PASS;

	cache = sbjtcache_new();
	sbjtcache_appenddata(cache, test_sbjtlist_testdata_01, strlen(test_sbjtlist_testdata_01));

	parser = sbjtparser_new(cache);
	list = sbjtlist_new();

	for (;;) {
		err = sbjtparser_getnextthread(parser, &thread);
		if (err != 1) {
			break;
		}
		if (thread != NULL) {
			sbjtlist_appendthread(list, thread);
		} else {
			break;
		}
	}

	sbjtlist_sort(list, SBJTLIST_SORTBY_NUMBER, NULL, 0);

	list_iter = sbjtlist_startread(list, False);
	for (;;) {
		next = sbjtlist_iterator_next(list_iter, &tuple);
		if (next == False) {
			break;
		}
		sbjtlist_tuple_gettitle(tuple, &title, &len);
		sbjtlist_tuple_getnumber(tuple, &num);
		printf("%d: %S\n", num, title);
	}
	sbjtlist_endread(list, list_iter);

	sbjtlist_delete(list);
	sbjtparser_delete(parser);

	sbjtcache_delete(cache);

	return result;
}

LOCAL VOID test_sbjtlist_printresult(TEST_RESULT (*proc)(), B *test_name)
{
	TEST_RESULT result;

	printf("test_sbjtlist: %s\n", test_name);
	printf("---------------------------------------------\n");
	result = proc();
	if (result == TEST_RESULT_PASS) {
		printf("--pass---------------------------------------\n");
	} else {
		printf("--fail---------------------------------------\n");
	}
	printf("---------------------------------------------\n");
}

EXPORT VOID test_sbjtlist_main()
{
	test_sbjtlist_printresult(test_sbjtlist_1, "test_sbjtlist_1");
}
