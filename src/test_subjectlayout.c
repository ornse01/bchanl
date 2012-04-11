/*
 * test_subjectlayout.c
 *
 * Copyright (c) 2009-2012 project bchan
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

#include    "subjectlayout.h"

#include    <btron/btron.h>
#include	<btron/dp.h>
#include	<tcode.h>
#include    <bstdio.h>
#include    <bstring.h>
#include    <bstdlib.h>

#include    <unittest_driver.h>

#include    "subjectlist.h"
#include    "subjectparser.h"
#include    "subjectcache.h"

LOCAL UB test_sbjtlayout_testdata_01[] = {
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

LOCAL BMP* test_sbjtlayout_util_alloc_BMP()
{
	W size;
	BMP* dest;

	dest = malloc(sizeof(UW)+sizeof(UH)*2+sizeof(RECT)+sizeof(UB*)*1);
	dest->planes = 1;
	dest->pixbits = (32 << 8)|28;
	dest->rowbytes = 100*4;
	dest->bounds = (RECT){{0,0,100,100}};

	size = ((dest->bounds.c.right - dest->bounds.c.left) * (dest->pixbits >> 8) + 15) / 16 * 2 * (dest->bounds.c.bottom - dest->bounds.c.top);

	dest->baseaddr[0] = malloc(size);

	return dest;
}

LOCAL VOID test_sbjtlayout_util_free_BMP(BMP *bmp)
{
	free(bmp->baseaddr[0]);
	free(bmp);
}

LOCAL UNITTEST_RESULT test_sbjtlayout_1()
{
	BMP *bmp;
	GID gid;
	W err;
	Bool next;
	sbjtlayout_t *layout;
	sbjtlist_t *list;
	sbjtlist_iterator_t *list_iter;
	sbjtlist_tuple_t *tuple;
	sbjtcache_t *cache;
	sbjtparser_t *parser;
	sbjtparser_thread_t *thread = NULL;
	UNITTEST_RESULT result = UNITTEST_RESULT_PASS;

	bmp = test_sbjtlayout_util_alloc_BMP();
	if (bmp == NULL) {
		return UNITTEST_RESULT_FAIL;
	}
	gid = gopn_mem(NULL, bmp, NULL);
	if (gid < 0) {
		return UNITTEST_RESULT_FAIL;
	}

	cache = sbjtcache_new();
	sbjtcache_appenddata(cache, test_sbjtlayout_testdata_01, strlen(test_sbjtlayout_testdata_01));

	parser = sbjtparser_new(cache);
	list = sbjtlist_new();
	layout = sbjtlayout_new(gid);

	for (;;) {
		err = sbjtparser_getnextthread(parser, &thread);
		if (err != 1) {
			break;
		}
		if (thread != NULL) {
			sbjtlist_appendthread(list, thread, 0x3eec16c0);
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
		sbjtlayout_appendthread(layout, tuple);
	}
	sbjtlist_endread(list, list_iter);

	sbjtlayout_delete(layout);
	sbjtlist_delete(list);
	sbjtparser_delete(parser);

	sbjtcache_delete(cache);

	gcls_env(gid);
	test_sbjtlayout_util_free_BMP(bmp);

	return result;
}

EXPORT VOID test_sbjtlayout_main(unittest_driver_t *driver)
{
	UNITTEST_DRIVER_REGIST(driver, test_sbjtlayout_1);
}
