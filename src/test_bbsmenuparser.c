/*
 * test_bbsmenuparser.c
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

#include    "bbsmenuparser.h"

#include    <btron/btron.h>
#include	<tcode.h>
#include    <bstdio.h>
#include    <bstring.h>

#include    <unittest_driver.h>

#include    "bbsmenucache.h"

LOCAL UB test_bbsmnparser_testdata_01[] = {
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

LOCAL UNITTEST_RESULT test_bbsmnparser_1()
{
	W err;
	bbsmncache_t *cache;
	bbsmnparser_t *parser;
	bbsmnparser_item_t *item = NULL;
	UNITTEST_RESULT result = UNITTEST_RESULT_PASS;

	cache = bbsmncache_new();
	bbsmncache_appenddata(cache, test_bbsmnparser_testdata_01, strlen(test_bbsmnparser_testdata_01));

	parser = bbsmnparser_new(cache);

	for (;;) {
		err = bbsmnparser_getnextitem(parser, &item);
		if (err != 1) {
			break;
		}
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
	}

	bbsmnparser_delete(parser);

	bbsmncache_delete(cache);

	return result;
}

LOCAL UNITTEST_RESULT test_bbsmnparser_2()
{
	bbsmnparser_item_t item;
	Bool check;

	item.category = NULL;
	item.category_len = 0;
	item.url = NULL;
	item.url_len = 0;
	item.title = NULL;
	item.title_len = 0;

	check = bbsmnparser_item_checkboradurl(&item);
	if (check == False) {
		return UNITTEST_RESULT_PASS;
	}

	return UNITTEST_RESULT_FAIL;
}

LOCAL UNITTEST_RESULT test_bbsmnparser_3()
{
	bbsmnparser_item_t item;
	Bool check;

	item.category = (TC*)1;
	item.category_len = 0;
	item.url = NULL;
	item.url_len = 0;
	item.title = NULL;
	item.title_len = 0;

	check = bbsmnparser_item_checkboradurl(&item);
	if (check == True) {
		return UNITTEST_RESULT_PASS;
	}

	return UNITTEST_RESULT_FAIL;
}

LOCAL UNITTEST_RESULT test_bbsmnparser_4()
{
	bbsmnparser_item_t item;
	Bool check;

	item.category = NULL;
	item.category_len = 0;
	item.url = "http://2ch.net/";
	item.url_len = strlen(item.url);
	item.title = NULL;
	item.title_len = 0;

	check = bbsmnparser_item_checkboradurl(&item);
	if (check == False) {
		return UNITTEST_RESULT_PASS;
	}

	return UNITTEST_RESULT_FAIL;
}

LOCAL UNITTEST_RESULT test_bbsmnparser_5()
{
	bbsmnparser_item_t item;
	Bool check;

	item.category = NULL;
	item.category_len = 0;
	item.url = "http://2ch.net/board/";
	item.url_len = strlen(item.url);
	item.title = NULL;
	item.title_len = 0;

	check = bbsmnparser_item_checkboradurl(&item);
	if (check == False) {
		return UNITTEST_RESULT_PASS;
	}

	return UNITTEST_RESULT_FAIL;
}

LOCAL UNITTEST_RESULT test_bbsmnparser_6()
{
	bbsmnparser_item_t item;
	Bool check;

	item.category = NULL;
	item.category_len = 0;
	item.url = "http://2ch.net/board/aaa.html";
	item.url_len = strlen(item.url);
	item.title = NULL;
	item.title_len = 0;

	check = bbsmnparser_item_checkboradurl(&item);
	if (check == False) {
		return UNITTEST_RESULT_PASS;
	}

	return UNITTEST_RESULT_FAIL;
}

LOCAL UNITTEST_RESULT test_bbsmnparser_7()
{
	bbsmnparser_item_t item;
	Bool check;

	item.category = NULL;
	item.category_len = 0;
	item.url = "http://aaa.2ch.net/";
	item.url_len = strlen(item.url);
	item.title = NULL;
	item.title_len = 0;

	check = bbsmnparser_item_checkboradurl(&item);
	if (check == False) {
		return UNITTEST_RESULT_PASS;
	}

	return UNITTEST_RESULT_FAIL;
}

LOCAL UNITTEST_RESULT test_bbsmnparser_8()
{
	bbsmnparser_item_t item;
	Bool check;

	item.category = NULL;
	item.category_len = 0;
	item.url = "http://aaa.2ch.net/board/";
	item.url_len = strlen(item.url);
	item.title = NULL;
	item.title_len = 0;

	check = bbsmnparser_item_checkboradurl(&item);
	if (check == True) {
		return UNITTEST_RESULT_PASS;
	}

	return UNITTEST_RESULT_FAIL;
}

LOCAL UNITTEST_RESULT test_bbsmnparser_9()
{
	bbsmnparser_item_t item;
	Bool check;

	item.category = NULL;
	item.category_len = 0;
	item.url = "http://aaa.2ch.net/board/aaa.html";
	item.url_len = strlen(item.url);
	item.title = NULL;
	item.title_len = 0;

	check = bbsmnparser_item_checkboradurl(&item);
	if (check == False) {
		return UNITTEST_RESULT_PASS;
	}

	return UNITTEST_RESULT_FAIL;
}

LOCAL UNITTEST_RESULT test_bbsmnparser_10()
{
	bbsmnparser_item_t item;
	Bool check;

	item.category = NULL;
	item.category_len = 0;
	item.url = "http://aaa.bbspink.com/";
	item.url_len = strlen(item.url);
	item.title = NULL;
	item.title_len = 0;

	check = bbsmnparser_item_checkboradurl(&item);
	if (check == False) {
		return UNITTEST_RESULT_PASS;
	}

	return UNITTEST_RESULT_FAIL;
}

LOCAL UNITTEST_RESULT test_bbsmnparser_11()
{
	bbsmnparser_item_t item;
	Bool check;

	item.category = NULL;
	item.category_len = 0;
	item.url = "http://aaa.bbspink.com/board/";
	item.url_len = strlen(item.url);
	item.title = NULL;
	item.title_len = 0;

	check = bbsmnparser_item_checkboradurl(&item);
	if (check == False) {
		return UNITTEST_RESULT_PASS;
	}

	return UNITTEST_RESULT_FAIL;
}

LOCAL UNITTEST_RESULT test_bbsmnparser_12()
{
	bbsmnparser_item_t item;
	Bool check;

	item.category = NULL;
	item.category_len = 0;
	item.url = "http://aaa.bbspink.com/board/aaa.html";
	item.url_len = strlen(item.url);
	item.title = NULL;
	item.title_len = 0;

	check = bbsmnparser_item_checkboradurl(&item);
	if (check == False) {
		return UNITTEST_RESULT_PASS;
	}

	return UNITTEST_RESULT_FAIL;
}

LOCAL UNITTEST_RESULT test_bbsmnparser_13()
{
	bbsmnparser_item_t item;
	Bool check;

	item.category = NULL;
	item.category_len = 0;
	item.url = "http://aaa.machi.to/";
	item.url_len = strlen(item.url);
	item.title = NULL;
	item.title_len = 0;

	check = bbsmnparser_item_checkboradurl(&item);
	if (check == False) {
		return UNITTEST_RESULT_PASS;
	}

	return UNITTEST_RESULT_FAIL;
}

LOCAL UNITTEST_RESULT test_bbsmnparser_14()
{
	bbsmnparser_item_t item;
	Bool check;

	item.category = NULL;
	item.category_len = 0;
	item.url = "http://aaa.machi.to/board/";
	item.url_len = strlen(item.url);
	item.title = NULL;
	item.title_len = 0;

	check = bbsmnparser_item_checkboradurl(&item);
	if (check == False) {
		return UNITTEST_RESULT_PASS;
	}

	return UNITTEST_RESULT_FAIL;
}

LOCAL UNITTEST_RESULT test_bbsmnparser_15()
{
	bbsmnparser_item_t item;
	Bool check;

	item.category = NULL;
	item.category_len = 0;
	item.url = "http://aaa.machi.to/board/aaa.html";
	item.url_len = strlen(item.url);
	item.title = NULL;
	item.title_len = 0;

	check = bbsmnparser_item_checkboradurl(&item);
	if (check == False) {
		return UNITTEST_RESULT_PASS;
	}

	return UNITTEST_RESULT_FAIL;
}

typedef struct {
	UB *url;
	W url_len;
	UB *expected_host;
	W expected_host_len;
	UH expected_port;
	UB *expected_board;
	W expected_board_len;
} test_bbsmnparser_item_url_t;

LOCAL UNITTEST_RESULT test_bbsmnparser_item_url_common(test_bbsmnparser_item_url_t *data)
{
	bbsmncache_t *cache;
	bbsmnparser_t *parser;
	bbsmnparser_item_t *item;
	UNITTEST_RESULT ret = UNITTEST_RESULT_PASS;
	UB *host, *board;
	W host_len, board_len;
	UH port;

	cache = bbsmncache_new();
	if (cache == NULL) {
		return UNITTEST_RESULT_FAIL;
	}
	parser = bbsmnparser_new(cache);
	if (parser == NULL) {
		bbsmncache_delete(cache);
		return UNITTEST_RESULT_FAIL;
	}

	item = bbsmnparser_newboarditem(parser, NULL, 0, data->url, data->url_len);
	if (item == NULL) {
		ret = UNITTEST_RESULT_FAIL;
	}
	bbsmnparser_item_gethostboard(item, &host, &host_len, &port, &board, &board_len);
	if (host_len != data->expected_host_len) {
		printf("host length fail: expected = %d, result = %d\n", data->expected_host_len, host_len);
		ret = UNITTEST_RESULT_FAIL;
	} else if (strncmp(host, data->expected_host, host_len) != 0) {
		printf("host fail\n");
		ret = UNITTEST_RESULT_FAIL;
	}
	if (port != data->expected_port) {
		printf("port fail: expected = %d, result = %d\n", data->expected_port, port);
		ret = UNITTEST_RESULT_FAIL;
	}
	if (board_len != data->expected_board_len) {
		printf("board length fail: expected = %d, result = %d\n", data->expected_board_len, board_len);
		ret = UNITTEST_RESULT_FAIL;
	} else if (strncmp(board, data->expected_board, board_len) != 0) {
		printf("board fail\n");
		ret = UNITTEST_RESULT_FAIL;
	}

	bbsmnparser_delete(parser);
	bbsmncache_delete(cache);

	return ret;
}

LOCAL UNITTEST_RESULT test_bbsmnparser_item_url_1(test_bbsmnparser_item_url_t *data)
{
	UB url[] = "http://aaa.bbb.ccc/test/";
	W url_len = strlen(url);
	UB host[] = "aaa.bbb.ccc";
	W host_len = strlen(host);
	UH port = 80;
	UB board[] = "test";
	W board_len = strlen(board);
	test_bbsmnparser_item_url_t testdata = {
		url, url_len, host, host_len, port, board, board_len
	};
	return test_bbsmnparser_item_url_common(&testdata);
}

LOCAL UNITTEST_RESULT test_bbsmnparser_item_url_2(test_bbsmnparser_item_url_t *data)
{
	UB url[] = "http://aaa.bbb.ccc:65000/test/";
	W url_len = strlen(url);
	UB host[] = "aaa.bbb.ccc";
	W host_len = strlen(host);
	UH port = 65000;
	UB board[] = "test";
	W board_len = strlen(board);
	test_bbsmnparser_item_url_t testdata = {
		url, url_len, host, host_len, port, board, board_len
	};
	return test_bbsmnparser_item_url_common(&testdata);
}

EXPORT VOID test_bbsmnparser_main(unittest_driver_t *driver)
{
	UNITTEST_DRIVER_REGIST(driver, test_bbsmnparser_1);
	UNITTEST_DRIVER_REGIST(driver, test_bbsmnparser_2);
	UNITTEST_DRIVER_REGIST(driver, test_bbsmnparser_3);
	UNITTEST_DRIVER_REGIST(driver, test_bbsmnparser_4);
	UNITTEST_DRIVER_REGIST(driver, test_bbsmnparser_5);
	UNITTEST_DRIVER_REGIST(driver, test_bbsmnparser_6);
	UNITTEST_DRIVER_REGIST(driver, test_bbsmnparser_7);
	UNITTEST_DRIVER_REGIST(driver, test_bbsmnparser_8);
	UNITTEST_DRIVER_REGIST(driver, test_bbsmnparser_9);
	UNITTEST_DRIVER_REGIST(driver, test_bbsmnparser_10);
	UNITTEST_DRIVER_REGIST(driver, test_bbsmnparser_11);
	UNITTEST_DRIVER_REGIST(driver, test_bbsmnparser_12);
	UNITTEST_DRIVER_REGIST(driver, test_bbsmnparser_13);
	UNITTEST_DRIVER_REGIST(driver, test_bbsmnparser_14);
	UNITTEST_DRIVER_REGIST(driver, test_bbsmnparser_15);
	UNITTEST_DRIVER_REGIST(driver, test_bbsmnparser_item_url_1);
	UNITTEST_DRIVER_REGIST(driver, test_bbsmnparser_item_url_2);
}
