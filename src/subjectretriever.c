/*
 * subjectretriever.c
 *
 * Copyright (c) 2009-2014 project bchan
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

#include    "subjectretriever.h"

#include    "subjectcache.h"
#include	<http/http_typedef.h>
#include	<http/http_connector.h>

#ifdef BCHANL_CONFIG_DEBUG
# define DP(arg) printf arg
# define DP_ER(msg, err) printf("%s (%d/%x)\n", msg, err>>16, err)
#else
# define DP(arg) /**/
# define DP_ER(msg, err) /**/
#endif

struct sbjtretriever_t_ {
	http_connector_t *connector;
	ID endpoint;
	HTTP_STATUSCODE status;
	UB *useragent;
	W useragent_len;
};

EXPORT sbjtretriever_t* sbjtretriever_new(http_connector_t *connector, UB *useragent, W useragent_len)
{
	sbjtretriever_t *retriever;

	retriever = (sbjtretriever_t*)malloc(sizeof(sbjtretriever_t));
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

EXPORT VOID sbjtretriever_delete(sbjtretriever_t *retriever)
{
	if (retriever->endpoint > 0) {
		http_connector_deleteendpoint(retriever->connector, retriever->endpoint);
	}
	free(retriever);
}

/* from http://www.monazilla.org/index.php?e=197 */
#if 0
"GET /[板名]/subject.txt HTTP/1.1
Accept-Encoding: gzip
Host: [サーバー]
Accept: */*
Referer: http://[サーバー]/[板名]/
Accept-Language: ja
If-Modified-Since: Tue, 23 Dec 2008 14:08:21 GMT
If-None-Match: \"3462-1b80-45eb74ece1f40\"
User-Agent: Monazilla/1.00 (monaweb/1.00)
Connection: close
"
#endif

EXPORT W sbjtretriever_sendrequest(sbjtretriever_t *retriever, sbjtcache_t *cache)
{
	W host_len;
	UB *host;

	if (retriever->endpoint > 0) {
		DP(("sbjtretriever_sendrequest: requesting\n"));
		return -1;
	}

	sbjtcache_gethost(cache, &host, &host_len);

	retriever->endpoint = http_connector_createendpoint(retriever->connector, host, host_len, 80, HTTP_METHOD_GET);
	if (retriever->endpoint < 0) {
		DP_ER("http_connector_createendpoint error", retriever->endpoint);
		return -1;
	}

	return 0;
}

EXPORT Bool sbjtretriever_iswaitingendpoint(sbjtretriever_t *retriever, ID endpoint)
{
	if (retriever->endpoint == endpoint) {
		return True;
	}
	return False;
}

LOCAL UB header1[] =
"Accept: */*\r\n"
"Referer: http://";
LOCAL UB header2[] =
"/\r\n"
"Accept-Language: ja\r\n"
"User-Agent: ";
LOCAL UB header_default_ua[] = "Monazilla/1.00";
LOCAL UB header_crlf[] = "\r\n";

EXPORT W sbjtretriever_recievehttpevent(sbjtretriever_t *retriever, sbjtcache_t *cache, http_connector_event *hevent)
{
	http_connector_t *connector = retriever->connector;
	W host_len, board_len, path_len;
	UB *host, *board, *path;

	if (retriever->endpoint <= 0) {
		return -1;
	}

	if (hevent->type == HTTP_CONNECTOR_EVENTTYPE_SEND) {
		sbjtcache_getboard(cache, &host, &host_len);
		sbjtcache_getboard(cache, &board, &board_len);

		path_len = 1 + board_len + 12;
		path = malloc(sizeof(UB)*(path_len+1));
		if (path == NULL) {
			return -1;
		}
		path[0] = '/';
		strncpy(path+1, board, board_len);
		strncpy(path+1+board_len, "/subject.txt", 12);
		path[1+board_len+12] = '\0';
		http_connector_sendrequestline(connector, hevent->endpoint, path, path_len);
		free(path);

		http_connector_sendheader(connector, hevent->endpoint, header1, strlen(header1));
		http_connector_sendheader(connector, hevent->endpoint, host, host_len);
		http_connector_sendheader(connector, hevent->endpoint, "/", 1);
		http_connector_sendheader(connector, hevent->endpoint, board, board_len);
		http_connector_sendheader(connector, hevent->endpoint, header2, strlen(header2));
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
			sbjtcache_cleardata(cache);
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
			sbjtcache_appenddata(cache, hevent->data.receive_messagebody.bin, hevent->data.receive_messagebody.len);
		}
	} else if (hevent->type == HTTP_CONNECTOR_EVENTTYPE_RECEIVE_MESSAGEBODY_END) {
		http_connector_deleteendpoint(connector, hevent->endpoint);
		retriever->endpoint = -1;
		return SBJTRETRIEVER_REQUEST_ALLRELOAD;
	} else if (hevent->type == HTTP_CONNECTOR_EVENTTYPE_ERROR) {
		http_connector_deleteendpoint(connector, hevent->endpoint);
		retriever->endpoint = -1;
		return SBJTRETRIEVER_REQUEST_ERROR;
	} else {
		/* error */
		return -1;
	}

	return SBJTRETRIEVER_REQUEST_WAITNEXT;
}
