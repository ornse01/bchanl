#
#	@(#)Makefile (bchan: BTRON 2channel board viewer)
#		GNU make ��
#
#	make ����ˡ
#	��make
#		��꡼���Ѥ������Ǥ����
#	��make mode=debug
#		�ǥХå��Ѥ˺���
#	��make clean
#		make �Ǻ����������ƤΥե��������
#	��make install
#		����ξ��˥��󥹥ȡ��� (�µ��ѤΤ�)
#

# �С������
version = 0x0010

# ��꡼���Ѥ�ǥե���Ȥˤ���
mode =

# �������ΰ�¸�ط��ե����� (��ư����)
DEPS = Dependencies
DEPENDENCIES_OUTPUT := $(DEPS)

# ���ץꥱ�������ɸ��롼��
include ../../etc/makerules

#----------------------------------------------------------------------
# �����о�
TARGET = bchanl

# �������ե�����Υ������ѥ�
S = ../src
VPATH = $(S)

# �إå��ե�����Υǥ��쥯�ȥ��ɲ�
HEADER := $(S) $(HEADER)

# �������ե�����
SRC =	main.c subjectcache.c subjectparser.c subjectlayout.c window.c cache.c bbsmenucache.c bbsmenuparser.c bbsmenulayout.c bchanl_subject.c retriever.c subjectretriever.c tadlib.c bbsmenuretriever.c parselib.c bbsmenufilter.c

# �ǡ����ܥå����������ե�����
DBSRC =

SRC += $(DBSRC)

WC_SRC = $(filter %.C, $(SRC))

# ���֥������ȥե�����
OBJ = $(addsuffix .o, $(basename $(SRC)))

# ����ѥ��륪�ץ����
ifneq ($(mode), debug)
  CFLAGS += -Wall
endif

CFLAGS += $(BUILDOPTIONS)

LOADLIBES += -limg

#----------------------------------------------------------------------
.PHONY: all clean install

INST = $(addprefix $(TARGET), .bz .map)
NOINST = $(addprefix $(TARGET), .out .fsn .dbx)

all: $(INST)

$(TARGET).out: $(OBJ)
	$(LINK.o) $(LDOBJS) $^ $(LOADLIBES) $(LDLIBS) $(OUTPUT_OPTION)

clean:
	$(RM) $(OBJ)
	$(RM) $(WC_SRC:%.C=%.c) $(DBSRC)
	$(RM) $(INST) $(NOINST) *.lst $(DEPS)

install: $(INST:%=$(EXE_INSTALLDIR)/%)

$(EXE_INSTALLDIR)/$(TARGET).out: $(EXE_INSTALLDIR)/%: %
	$(EXE_INSTALL_STRIP)

# ���̥���������
# -f0x40 �ϡ���ʪ��Ͽ�Ѥ˥��ץꥱ�������°�� 1 �����ꤹ�����

$(TARGET).bz: $(NOINST)
	$(CP) $(TARGET).out _t
	$(STRIP) _t
	$(MKBTF) -o$@ -a0x8002 -c -tbchanl -f0x40 \
		9.0x163._t \
		10.0.$(TARGET).dbx \
		7.$(version).$(TARGET).fsn
	$(RM) _t

# �������ΰ�¸�ط�

$(WC_SRC:%.C=%.c):

$(DBSRC):

ifdef DEPENDENCIES_OUTPUT
$(DEPS):	; touch $(DEPS)
else
$(DEPS): $(SRC)	; $(MAKEDEPS) $@ $?
endif

include $(DEPS)