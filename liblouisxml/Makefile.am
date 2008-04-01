INCLUDES =		\
	-I$(top_srcdir)

liblouisxmlincludedir = $(includedir)/liblouisxml

liblouisxmlinclude_HEADERS = \
	liblouisxml.h

lib_LTLIBRARIES = liblouisxml.la

liblouisxml_la_CFLAGS =	\
	$(BASE_DEPENDENCIES_CFLAGS)

liblouisxml_la_LDFLAGS =	\
	-version-info \
	$(LIBLOUISXML_CURRENT):$(LIBLOUISXML_REVISION):$(LIBLOUISXML_AGE) \
	-no-undefined \
	$(BASE_DEPENDENCIES_LDFLAGS)

liblouisxml_la_SOURCES =			\
	change_table.c \
	examine_document.c \
	extext.c \
	liblouisxml.c \
	liblouisxml.h \
	louisxml.h \
	readconfig.c \
	semantics.c \
	sem_enum.h \
	sem_names.h \
	transcribe_chemistry.c \
	transcribe_computerCode.c \
	transcribe_document.c \
	transcribe_graphic.c \
	transcribe_math.c \
	transcribe_music.c \
	transcribe_paragraph.c \
	transcriber.c \
	transcriber.h

nodist_liblouisxml_la_SOURCES =			\
		choices.h

EXTRA_DIST = make_sem_names.c
