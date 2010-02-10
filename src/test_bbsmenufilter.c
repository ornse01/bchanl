/*
 * test_bbsmenufilter.c
 *
 * Copyright (c) 2010 project bchan
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

#include    <bstdio.h>

#include    "test.h"

#include    "bbsmenufilter.h"
#include    "bbsmenuparser.h"
#include    "bbsmenucache.h"

LOCAL UB test_bbsmnfilter_testdata_01[] = {
"
<HTML>
<HEAD>
<TITLE>BBS MENU for test</TITLE>
</HEAD>

<BODY>
<BR>
<font size=2>

<A HREF=http://xxx.2ch.net/AAA/>AAA</A>

<BR><BR><B>Cat 1</B><BR>
<A HREF=http://xxx.2ch.net/BBB/>BBB</A><br>
<A HREF=http://yyy.2ch.net/CCC/>CCC</A><br>
<A HREF=http://yyy.2ch.net/DDD/>DDD</A><br>
<A HREF=http://yyy.2ch.net/EEE/>EEE</A>

<BR><BR><B>Cat 2</B><BR>
<A HREF=http://xxx.2ch.net/FFF/>FFF</A><br>
<A HREF=http://xxx.2ch.net/GGG/>GGG</A><br>
<A HREF=http://xxx.2ch.net/HHH/>HHH</A><br>
<A HREF=http://xxx.2ch.net/III/>III</A><br>

<BR><BR><B>Cat 3</B><BR>
<A HREF=http://xxx.2ch.net/>other</A><br>
<A HREF=http://yyy.2ch.net/>other2</A><br>
<A HREF=http://zzz.2ch.net/zzz/zzz.php>other3</A><br>

<BR><BR><B>Cat 4</B><BR>
<A HREF=http://www.chokanji.com/ TARGET=_blank>ck4</A><br>
<A HREF=http://www.parsonal-media.co.jp/ TARGET=_blank>ck4</A><br>

</BODY></HTML>
"};

LOCAL TEST_RESULT test_bbsmnfilter_1()
{
	W ret,err;
	bbsmncache_t *cache;
	bbsmnparser_t *parser;
	bbsmnparser_item_t *item = NULL;
	bbsmnfilter_t *filter = NULL;
	TEST_RESULT result = TEST_RESULT_PASS;

	cache = bbsmncache_new();
	bbsmncache_appenddata(cache, test_bbsmnfilter_testdata_01, strlen(test_bbsmnfilter_testdata_01));
	parser = bbsmnparser_new(cache);

	filter = bbsmnfilter_new();

	for (;;) {
		err = bbsmnparser_getnextitem(parser, &item);
		if (err != 1) {
			break;
		}
		bbsmnfilter_inputitem(filter, item);
		for (;;) {
			ret = bbsmnfilter_outputitem(filter, &item);
			if (item != NULL) {
				if (item->category != NULL) {
					printf("category:\n%S\n", item->category);
				}
				if (item->url != NULL) {
					printf("url:\n%s\n", item->url);
				}
				if (item->title != NULL) {
					printf("title:\n%S\n", item->title);
				}
				bbsmnparser_item_delete(item);
			} else {
				break;
			}
			if (ret != BBSMNFILTER_OUTPUTITEM_CONTINUE) {
				break;
			}
		}
	}

	bbsmnfilter_delete(filter);

	bbsmnparser_delete(parser);
	bbsmncache_delete(cache);

	return result;
}

LOCAL TEST_RESULT test_bbsmnfilter_2_checkitem_index(bbsmnparser_item_t *item, W i)
{
	switch(i) {
	case 0:
	case 5:
		if (item->category == NULL) {
			return TEST_RESULT_FAIL;
		}
		break;
	case 1:
	case 2:
	case 3:
	case 4:
	case 6:
	case 7:
	case 8:
	case 9:
		if ((item->category != NULL)||(item->url == NULL)||(item->title == NULL)) {
			return TEST_RESULT_FAIL;
		}
		break;
	default:
		return TEST_RESULT_FAIL;
		break;
	}
	return TEST_RESULT_PASS;
}

LOCAL TEST_RESULT test_bbsmnfilter_2()
{
	W i = 0, ret,err;
	bbsmncache_t *cache;
	bbsmnparser_t *parser;
	bbsmnparser_item_t *item = NULL;
	bbsmnfilter_t *filter = NULL;
	TEST_RESULT result = TEST_RESULT_PASS;

	cache = bbsmncache_new();
	bbsmncache_appenddata(cache, test_bbsmnfilter_testdata_01, strlen(test_bbsmnfilter_testdata_01));
	parser = bbsmnparser_new(cache);

	filter = bbsmnfilter_new();

	for (;;) {
		err = bbsmnparser_getnextitem(parser, &item);
		if (err != 1) {
			break;
		}
		bbsmnfilter_inputitem(filter, item);
		for (;;) {
			ret = bbsmnfilter_outputitem(filter, &item);
			if (item != NULL) {
				result = test_bbsmnfilter_2_checkitem_index(item, i);
				i++;
				bbsmnparser_item_delete(item);

				if (result == TEST_RESULT_FAIL) {
					printf("fail i = %d\n", i-1);
					break;
				}
			} else {
				if ((ret != BBSMNFILTER_OUTPUTITEM_WAITNEXT)&&(ret != BBSMNFILTER_OUTPUTITEM_END)) {
					printf("invalid return value\n");
					result = TEST_RESULT_FAIL;
				}
				break;
			}
			if (ret != BBSMNFILTER_OUTPUTITEM_CONTINUE) {
				break;
			}
		}
		if (result == TEST_RESULT_FAIL) {
			break;
		}
	}

	bbsmnfilter_inputitem(filter, NULL);
	for (;;) {
		ret = bbsmnfilter_outputitem(filter, &item);
		if (item != NULL) {
			result = test_bbsmnfilter_2_checkitem_index(item, i);
			i++;
			bbsmnparser_item_delete(item);

			if (result == TEST_RESULT_FAIL) {
				printf("fail i = %d\n", i-1);
				break;
			}
		}
		if (ret == BBSMNFILTER_OUTPUTITEM_CONTINUE) {
			continue;
		}
		if (ret == BBSMNFILTER_OUTPUTITEM_END) {
			break;
		}
		result = TEST_RESULT_FAIL;
		printf("invalid return value\n");
		break;
	}
	if (i != 10) {
		result = TEST_RESULT_FAIL;
		printf("fail: too meny items\n");
	}

	bbsmnfilter_delete(filter);

	bbsmnparser_delete(parser);
	bbsmncache_delete(cache);

	return result;
}

LOCAL VOID test_bbsmnfilter_printresult(TEST_RESULT (*proc)(), B *test_name)
{
	TEST_RESULT result;

	printf("test_bbsmnfilter: %s\n", test_name);
	printf("---------------------------------------------\n");
	result = proc();
	if (result == TEST_RESULT_PASS) {
		printf("--pass---------------------------------------\n");
	} else {
		printf("--fail---------------------------------------\n");
	}
	printf("---------------------------------------------\n");
}

EXPORT VOID test_bbsmnfilter_main()
{
	test_bbsmnfilter_printresult(test_bbsmnfilter_1, "test_bbsmnfilter_1");
	test_bbsmnfilter_printresult(test_bbsmnfilter_2, "test_bbsmnfilter_2");
}
