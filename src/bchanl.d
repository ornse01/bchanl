----
--  bchanl.d
--
-- Copyright (c) 2009-2012 project bchan
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
.TB_SBJTOPT_FLT	=	30
.WS_SBJTOPT_ODR	=	31
.WS_SBJTOPT_ODRBY	=	32

--- for subject option window layout values.

.SBJTOPT_MARGIN = 8
.SBJTOPT_FLTR_HEIGHT = 24
.SBJTOPT_FLTR_WIDTH = 360
.SBJTOPT_ODR_WIDTH = 80
.SBJTOPT_ODR_HEIGHT = 4+16+4+16+4
.SBJTOPT_ODRBY_WIDTH = 120
.SBJTOPT_ODRBY_HEIGHT = 4+16+3+16+3+16+3+16+4
.SBJTOPT_FLTR_L = SBJTOPT_MARGIN
.SBJTOPT_FLTR_T = SBJTOPT_MARGIN
.SBJTOPT_FLTR_R = SBJTOPT_FLTR_L+SBJTOPT_FLTR_WIDTH
.SBJTOPT_FLTR_B = SBJTOPT_FLTR_T+SBJTOPT_FLTR_HEIGHT
.SBJTOPT_ODR_L = SBJTOPT_MARGIN
.SBJTOPT_ODR_T = SBJTOPT_FLTR_B+SBJTOPT_MARGIN
.SBJTOPT_ODR_R = SBJTOPT_ODR_L+SBJTOPT_ODR_WIDTH
.SBJTOPT_ODR_B = SBJTOPT_ODR_T+SBJTOPT_ODR_HEIGHT
.SBJTOPT_ODRBY_L = SBJTOPT_ODR_R+SBJTOPT_MARGIN
.SBJTOPT_ODRBY_T = SBJTOPT_FLTR_B+SBJTOPT_MARGIN
.SBJTOPT_ODRBY_R = SBJTOPT_ODRBY_L+SBJTOPT_ODRBY_WIDTH
.SBJTOPT_ODRBY_B = SBJTOPT_ODRBY_T+SBJTOPT_ODRBY_HEIGHT

---------
-- data type = PARTS_DATA
---------
	{% 7 0}		-- datatype PARTS_DATA

	{# TB_SBJTOPT_FLT 0 0} 	-- data number
	TB_PARTS+P_DISP:L	-- type
	{SBJTOPT_FLTR_L:H SBJTOPT_FLTR_T:H SBJTOPT_FLTR_R:H SBJTOPT_FLTR_B:H}	-- r
	128L			-- txsize
	0L			-- text
	{0L 0L -1L 0L}		-- PARTDISP

	{# WS_SBJTOPT_ODR 0 0} 	-- data number
	WS_PARTS+P_DISP:L	-- type
	{SBJTOPT_ODR_L:H SBJTOPT_ODR_T:H SBJTOPT_ODR_R:H SBJTOPT_ODR_B:H}	-- r
	1L			-- cv
	OFFSET:L+20		-- name
	{0L 0L -1L 0L}		-- PARTDISP
	MC_STR "昇順"
	MC_STR "降順\0"

	{# WS_SBJTOPT_ODRBY 0 0} 	-- data number
	WS_PARTS+P_DISP:L	-- type
	{SBJTOPT_ODRBY_L:H SBJTOPT_ODRBY_T:H SBJTOPT_ODRBY_R:H SBJTOPT_ODRBY_B:H}	-- r
	1L			-- cv
	OFFSET:L+20		-- name
	{0L 0L -1L 0L}		-- PARTDISP
	MC_STR "順"
	MC_STR "レス"
	MC_STR "Ｓｉｎｃｅ"
	MC_STR "勢い\0"

---------
-- data type = TEXT_DATA
---------
	{% 6 0}		-- datatype TEXT_DATA
	{# TEXT_MLIST0 0 0}	-- data number
	MC_STRKEY1 "Ｅ終了\0"

	{# TEXT_MLIST1 0 0}	-- data number
	MC_STR "表示"
	MC_STR "再表示"
	MC_IND "スレ一覧設定\0"

	{# TEXT_MLIST2 0 0}	-- data number
	MC_STR "操作"
	MC_STR "板一覧再取得"
	MC_STR "外部板の追加\0"

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
