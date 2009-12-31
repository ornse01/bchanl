/*
 * bbsmenuretriever.c
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

#include	<basic.h>
#include	<bstdlib.h>
#include	<bstdio.h>
#include	<bstring.h>
#include	<errcode.h>
#include	<btron/btron.h>
#include	<btron/bsocket.h>

#include    "bbsmenuretriever.h"

#include    "retriever.h"
#include    "bbsmenucache.h"

#ifdef BCHANL_CONFIG_DEBUG
# define DP(arg) printf arg
# define DP_ER(msg, err) printf("%s (%d/%x)\n", msg, err>>16, err)
#else
# define DP(arg) /**/
# define DP_ER(msg, err) /**/
#endif

struct bbsmnretriever_t_ {
	retriever_t *retr;
};

EXPORT bbsmnretriever_t* bbsmnretriever_new()
{
	bbsmnretriever_t *retriever;

	retriever = (bbsmnretriever_t*)malloc(sizeof(bbsmnretriever_t));
	if (retriever == NULL) {
		return NULL;
	}
	retriever->retr = retriever_new();
	if (retriever->retr == NULL) {
		free(retriever);
		return NULL;
	}

	return retriever;
}

EXPORT VOID bbsmnretriever_delete(bbsmnretriever_t *retriever)
{
	retriever_delete(retriever->retr);
	free(retriever);
}

#define SO_ERR_SEND_LEN(sockID, str, len) \
   err = so_send(sockID, (str), (len), 0); \
   if(err < 0){ \
     return err; \
   }

#define SO_ERR_SEND(sockID, str) SO_ERR_SEND_LEN(sockID, (str), strlen((str)))

LOCAL W bbsmnretriever_sendheader(W sock)
{
	W err;

	SO_ERR_SEND(sock, "GET /bbsmenu.html HTTP/1.1\r\n");
	SO_ERR_SEND(sock, "Accept-Encoding: gzip\r\n");
	SO_ERR_SEND(sock, "Host: menu.2ch.net\r\n");
	SO_ERR_SEND(sock, "Accept: */*\r\n");
	SO_ERR_SEND(sock, "Referer: http://menu.2ch.net/\r\n");
	SO_ERR_SEND(sock, "Accept-Language: ja\r\n");
	SO_ERR_SEND(sock, "User-Agent: Monazilla/1.00 (bchanl/0.01)\r\n");
	SO_ERR_SEND(sock, "Connection: close\r\n");
	SO_ERR_SEND(sock, "\r\n");

	return 0;
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
	W sock, err, ret = -1, len, status;
	UB *bin, *host = "menu.2ch.net";

	retriever_clearbuffer(retriever->retr);

	err = retriever_gethost(retriever->retr, host);
	if (err < 0) {
		return err;
	}
	sock = retriver_connectsocket(retriever->retr);
	if (sock < 0) {
		return sock;
	}

	err = bbsmnretriever_sendheader(sock);
	if (err < 0) {
		so_close(sock);
		return err;
	}

	err = retriever_recieve(retriever->retr, sock);
	if (err < 0) {
		so_close(sock);
		return err;
	}

	err = retriever_parsehttpresponse(retriever->retr);
	if (err < 0) {
		so_close(sock);
		return err;
	}
	retriever_dumpheader(retriever->retr);

	err = retriever_decompress(retriever->retr);
	if (err < 0) {
		so_close(sock);
		return err;
	}

	status = retriever_parse_response_status(retriever->retr);
	if (status == 200) {
		bin = retriever_getbody(retriever->retr);
		len = retriever_getbodylength(retriever->retr);
		bbsmncache_cleardata(cache);
		bbsmncache_appenddata(cache, bin, len);

		bin = retriever_getheader(retriever->retr);
		len = retriever_getheaderlength(retriever->retr);
		bbsmncache_updatelatestheader(cache, bin, len);

		ret = BBSMNRETRIEVER_REQUEST_ALLRELOAD;
	}

	so_close(sock);

	retriever_clearbuffer(retriever->retr);

	return ret;
}
