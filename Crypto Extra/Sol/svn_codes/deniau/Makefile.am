ACLOCAL_AMFLAGS = -I m4
EXTRA_DIST = autogen.sh

BUILT_SOURCES = NativeDes_wrap.cpp des_sboxes.h NativeDes.py
SWIG_SOURCES = NativeDes.i

pkgpython_PYTHON = NativeDes.py src/*py
pkgpyexec_LTLIBRARIES = _NativeDes.la
_NativeDes_la_SOURCES = NativeDes_wrap.cpp trace.cpp des_block.cpp $(SWIG_SOURCES)
_NativeDes_la_CPPFLAGS = $(SWIG_PYTHON_CPPFLAGS) -I$(top_srcdir)/src
_NativeDes_la_LDFLAGS = -module

MOSTLYCLEANFILES = NativeDes.py NativeDes_wrap.cpp des_sboxes.h

NativeDes_wrap.cpp : $(SWIG_SOURCES)
	$(SWIG) $(SWIG_PYTHON_OPT) -o $@ $<

des_sboxes.h:
	$(PERL) des_sboxes.pl > des_sboxes.h