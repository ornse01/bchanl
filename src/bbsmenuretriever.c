/*
 * bbsmenuretriever.c
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

#include	<basic.h>
#include	<bstdlib.h>
#include	<bstdio.h>
#include	<bstring.h>
#include	<errcode.h>
#include	<btron/btron.h>
#include	<btron/bsocket.h>

#include    "bbsmenuretriever.h"

#include    "bbsmenucache.h"
#include	<http/http_typedef.h>
#include	<http/http_connector.h>

#ifdef BCHANL_CONFIG_DEBUG
# define DP(arg) printf arg
# define DP_ER(msg, err) printf("%s (%d/%x)\n", msg, err>>16, err)
#else
# define DP(arg) /**/
# define DP_ER(msg, err) /**/
#endif

struct bbsmnretriever_t_ {
	http_connector_t *connector;
	ID endpoint;
	HTTP_STATUSCODE status;
	UB *useragent;
	W useragent_len;
};

EXPORT bbsmnretriever_t* bbsmnretriever_new(http_connector_t *connector, UB *useragent, W useragent_len)
{
	bbsmnretriever_t *retriever;

	retriever = (bbsmnretriever_t*)malloc(sizeof(bbsmnretriever_t));
	if (retriever == NULL) {
		return NULL;
	}
	retriever->connector = connector;
	retriever->endpoint = -1;
	retriever->status = 0;
	retriever->useragent = useragent;
	retriever->useragent_len = useragent_len;

	return retriever;
}

EXPORT VOID bbsmnretriever_delete(bbsmnretriever_t *retriever)
{
	if (retriever->endpoint > 0) {
		http_connector_deleteendpoint(retriever->connector, retriever->endpoint);
	}
	free(retriever);
}

/* from http://www.monazilla.org/index.php?e=196 */
#if 0
"
GET /bbsmenu.html HTTP/1.1
Accept-Encoding: gzip
Host: menu.2ch.net
Accept: */*
Referer: http://menu.2ch.net/
Accept-Language: ja
User-Agent: Monazilla/1.00
Connection: close
"
#endif

EXPORT W bbsmnretriever_sendrequest(bbsmnretriever_t *retriever, bbsmncache_t *cache)
{
	UB host[] = "menu.2ch.net";

	if (retriever->endpoint > 0) {
		DP(("bbsmnretriever_sendrequest: requesting\n"));
		return -1;
	}

	retriever->endpoint = http_connector_createendpoint(retriever->connector, host, strlen(host), 80, HTTP_METHOD_GET);
	if (retriever->endpoint < 0) {
		DP_ER("http_connector_createendpoint error", retriever->endpoint);
		return -1;
	}

	return 0;
}

EXPORT Bool bbsmnretriever_iswaitingendpoint(bbsmnretriever_t *retriever, ID endpoint)
{
	if (retriever->endpoint == endpoint) {
		return True;
	}
	return False;
}

LOCAL UB path[] = "/bbsmenu.html";
LOCAL UB header[] =
"Accept: */*\r\n"
"Referer: http://menu.2ch.net/\r\n"
"Accept-Language: ja\r\n"
"User-Agent: ";
LOCAL UB header_default_ua[] = "Monazilla/1.00";
LOCAL UB header_crlf[] = "\r\n";

EXPORT W bbsmnretriever_recievehttpevent(bbsmnretriever_t *retriever, bbsmncache_t *cache, http_connector_event *hevent)
{
	http_connector_t *connector = retriever->connector;

	if (retriever->endpoint <= 0) {
		return -1;
	}

	if (hevent->type == HTTP_CONNECTOR_EVENTTYPE_SEND) {
		http_connector_sendrequestline(connector, hevent->endpoint, path, strlen(path));
		http_connector_sendheader(connector, hevent->endpoint, header, strlen(header));
		if (retriever->useragent != NULL) {
			http_connector_sendheader(connector, hevent->endpoint, retriever->useragent, retriever->useragent_len);
		} else {
			http_connector_sendheader(connector, hevent->endpoint, header_default_ua, strlen(header_default_ua));
		}
		http_connector_sendheader(connector, hevent->endpoint, header_crlf, strlen(header_crlf));
		http_connector_sendheaderend(connector, hevent->endpoint);
		http_connector_sendmessagebody(connector, hevent->endpoint, NULL, 0);
		http_connector_sendmessagebodyend(connector, hevent->endpoint);
	} else if (hevent->type == HTTP_CONNECTOR_EVENTTYPE_RECEIVE_STATUSLINE) {
		DP(("HTTP_CONNECTOR_EVENTTYPE_RECEIVE_STATUSLINE\n"));
		DP(("    status = %d\n", hevent->data.receive_statusline.statuscode));
		retriever->status = hevent->data.receive_statusline.statuscode;
		if (retriever->status == HTTP_STATUSCODE_200_OK) {
			bbsmncache_cleardata(cache);
		}
	} else if (hevent->type == HTTP_CONNECTOR_EVENTTYPE_RECEIVE_HEADER) {
#ifdef BCHANL_CONFIG_DEBUG
		{
			W i = 0;
			for (i = 0; i < hevent->data.receive_header.len; i++) {
				printf("%c", hevent->data.receive_header.bin[i]);
			}
		}
#endif
	} else if (hevent->type == HTTP_CONNECTOR_EVENTTYPE_RECEIVE_HEADER_END) {
	} else if (hevent->type == HTTP_CONNECTOR_EVENTTYPE_RECEIVE_MESSAGEBODY) {
		if (retriever->status == HTTP_STATUSCODE_200_OK) {
			bbsmncache_appenddata(cache, hevent->data.receive_messagebody.bin, hevent->data.receive_messagebody.len);
		}
	} else if (hevent->type == HTTP_CONNECTOR_EVENTTYPE_RECEIVE_MESSAGEBODY_END) {
		http_connector_deleteendpoint(connector, hevent->endpoint);
		retriever->endpoint = -1;
		return BBSMNRETRIEVER_REQUEST_ALLRELOAD;
	} else if (hevent->type == HTTP_CONNECTOR_EVENTTYPE_ERROR) {
		http_connector_deleteendpoint(connector, hevent->endpoint);
		retriever->endpoint = -1;
	} else {
		/* error */
		return -1;
	}

	return BBSMNRETRIEVER_REQUEST_WAITNEXT;
}
