#
#	@(#)Makefile (bchan: BTRON BBS board viewer)
#		GNU make 用
#
#	make の方法
#	・make
#		リリース用の正式版を作成
#	・make mode=debug
#		デバッグ用に作成
#	・make clean
#		make で作成した全てのファイルを削除
#	・make install
#		規定の場所にインストール (実機用のみ)
#

# バージョン
version = 0x0300

# リリース用をデフォルトにする
mode =

# ソースの依存関係ファイル (自動生成)
DEPS = Dependencies
DEPENDENCIES_OUTPUT := $(DEPS)

# アプリケーション標準ルール
include ../../etc/makerules

#----------------------------------------------------------------------
# 作成対象
TARGET = bchanl

# ソースファイルのサーチパス
S = ../src
VPATH = $(S)

# ヘッダファイルのディレクトリ追加
HEADER := $(S) $(HEADER) ../src/bchanf/src

# ソースファイル
SRC =	main.c subjectcache.c subjectparser.c subjectlayout.c cache.c bbsmenucache.c bbsmenuparser.c bbsmenulayout.c bchanl_subject.c retriever.c subjectretriever.c tadlib.c bbsmenuretriever.c bbsmenufilter.c bchanl_hmi.c bchanl_menus.c subjectlist.c extbbslist.c bchanl_panels.c util.c

# データボックスソースファイル
DBSRC =

SRC += $(DBSRC)

WC_SRC = $(filter %.C, $(SRC))

# オブジェクトファイル
OBJ = $(addsuffix .o, $(basename $(SRC)))

# コンパイルオプション
ifneq ($(mode), debug)
  CFLAGS += -Wall
endif

CFLAGS += $(BUILDOPTIONS)

LOADLIBES += -limg

#----------------------------------------------------------------------
.PHONY: all clean install

INST = $(addprefix $(TARGET), .bz .map)
NOINST = $(addprefix $(TARGET), .out .fsn .dbx)

all: codegen library $(INST)

codegen:
	@$(MAKE) -f makefile_codegen -C ../src

library:
	@$(MAKE) -f Makefile.lib -C ./bchanf

$(TARGET).out: $(OBJ) bchanf/libbchanf.a
	$(LINK.o) $(LDOBJS) $^ $(LOADLIBES) $(LDLIBS) $(OUTPUT_OPTION)

clean:
	@$(MAKE) clean -f Makefile.lib -C ./bchanf
	$(RM) $(OBJ)
	$(RM) $(WC_SRC:%.C=%.c) $(DBSRC)
	$(RM) $(INST) $(NOINST) *.lst $(DEPS)
	$(RM) *.btf

install: $(INST:%=$(EXE_INSTALLDIR)/%)

$(EXE_INSTALLDIR)/$(TARGET).out: $(EXE_INSTALLDIR)/%: %
	$(EXE_INSTALL_STRIP)

# 圧縮アーカイブ
# -f0x40 は、小物登録用にアプリケーション属性 1 を設定する指定

$(TARGET).bz: $(TARGET).btf _VerUpCmd.btf
	$(MKBTF) -o$@ -c -tbchanl \
		@0.0.0.0.0._VerUpCmd.btf \
		@0.0.0.0.0.$(TARGET).btf

$(TARGET).btf: $(NOINST)
	$(CP) $(TARGET).out _t
	$(STRIP) _t
	$(MKBTF) -o$@ -a0x8002 -tbchanl -f0x40\
		9.0x163._t \
		10.0.$(TARGET).dbx \
		7.$(version).$(TARGET).fsn
	$(RM) _t

_VerUpCmd.btf: _VerUpCmd
	$(CP) $(S)/_VerUpCmd _t
	$(MKBTF) -o_VerUpCmd.btf -t_VerUpCmd 31.0._t
	$(RM) _t

# ソースの依存関係

$(WC_SRC:%.C=%.c):

$(DBSRC):

ifdef DEPENDENCIES_OUTPUT
$(DEPS):	; touch $(DEPS)
else
$(DEPS): $(SRC)	; $(MAKEDEPS) $@ $?
endif

include $(DEPS)
