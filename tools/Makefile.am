INCLUDES =		\
	-I$(top_srcdir) \
	-I$(top_srcdir)/liblouisxml

LDADD = $(top_builddir)/liblouisxml/liblouisxml.la $(BASE_DEPENDENCIES_LIBS)

AM_CFLAGS =	\
	$(BASE_DEPENDENCIES_CFLAGS)

AM_LDFLAGS =	\
	$(BASE_DEPENDENCIES_LDFLAGS)

bin_PROGRAMS = \
	any2brl \
	brl2brl \
	brl2xml \
	mathml2brl \
	txt2xml \
	xml2brl

bin_SCRIPTS = \
	msword2brl

any2brl_SOURCES = any2brl.c

brl2brl_SOURCES = brl2brl.c

brl2xml_SOURCES = brl2xml.c

mathml2brl_SOURCES = mathml2brl.c

txt2xml_SOURCES = txt2xml.c

xml2brl_SOURCES = xml2brl.c
