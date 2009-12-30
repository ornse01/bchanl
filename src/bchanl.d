----
--  bchanl.d
--
-- Copyright (c) 2009 project bchan
--
-- This software is provided 'as-is', without any express or implied
-- warranty. In no event will the authors be held liable for any damages
-- arising from the use of this software.
--
-- Permission is granted to anyone to use this software for any purpose,
-- including commercial applications, and to alter it and redistribute it
-- freely, subject to the following restrictions:
--
-- 1. The origin of this software must not be misrepresented; you must not
--    claim that you wrote the original software. If you use this software
--    in a product, an acknowledgment in the product documentation would be
--    appreciated but is not required.
--
-- 2. Altered source versions must be plainly marked as such, and must not be
--    misrepresented as being the original software.
--
-- 3. This notice may not be removed or altered from any source
--    distribution.
--
----

.BASE = 0L

#include	<stddef.d>

.BASE = 0H

.MENU_TEST	=	20
.TEXT_MLIST0	=	21
.TEXT_MLIST1	=	22
.TEXT_MLIST2	=	23
.VIEWER_FUSEN	=	24
.TEXT_WTIT_BBSMENU	= 25
.TEXT_WTIT_SUBJECT	= 26
.TEXT_MSG_RETRBBSMENU	= 27
.TEXT_MSG_RETRSUBJECT	= 28
.TEXT_MSG_ERRRETR	= 29

---------
-- data type = PARTS_DATA
---------

---------
-- data type = TEXT_DATA
---------
	{% 6 0}		-- datatype TEXT_DATA
	{# TEXT_MLIST0 0 0}	-- data number
	MC_STR+MC_STRKEY1 "Ｅ終了\0"

	{# TEXT_MLIST1 0 0}	-- data number
	MC_STR "表示" MC_STR "再表示\0"

	{# TEXT_MLIST2 0 0}	-- data number
	MC_STR "操作"
	MC_STR "板一覧再取得\0"

	{# TEXT_WTIT_BBSMENU 0 0}	-- data number
	"２ちゃんねる板一覧\0"

	{# TEXT_WTIT_SUBJECT 0 0}	-- data number
	"スレ一覧\0"

	{# TEXT_MSG_RETRBBSMENU 0 0}	-- data number
	"板一覧取得中\0"

	{# TEXT_MSG_RETRSUBJECT 0 0}	-- data number
	"スレ一覧取得中\0"

	{# TEXT_MSG_ERRRETR 0 0}	-- data number
	"取得に失敗しました\0"

---------
-- data type = MENU_DATA
---------
	{% 8 0}			-- datatype MENU_DATA
	{# MENU_TEST 0 0} 	-- data number
	0L 0L 0L TEXT_MLIST0:L 0L	-- mlist0
	0L 0L 0L TEXT_MLIST1:L 0L	-- mlist1
	0L 0L 0L TEXT_MLIST2:L 0L	-- mlist2
	0L 0L 0L 0L 0L	-- [ウィンドウ]
	0L 0L 0L 0L 0L	-- [小物]

---------
-- data type = USER_DATA
---------
	{% 64 0}		-- datatype USER_DATA
	{# VIEWER_FUSEN 0 0} 	-- data number
	0 0 0 0					-- r
	16					-- chsz
	0x10000000L 0x10000000L 0x10FFFFFFL	-- colors
	4					-- pict
	0x8000	0xC053	0x8000			-- apl-id
	"ｂｃｈａｎ"16				-- name
	"２ｃｈスレ"16				-- type
	0					-- dlen
