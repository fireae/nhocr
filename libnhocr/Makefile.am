lib_LIBRARIES = libnhocr.a

if BUILD_GRAMDC
libnhocr_a_SOURCES = codelist.cpp dicfile.cpp discrim.c \
  feature_PLOVE.cpp nhrec.cpp ocrbase.cpp otsubin.cpp \
  segchar_adhoc.cpp gramdclient.cpp
else
libnhocr_a_SOURCES = codelist.cpp dicfile.cpp discrim.c \
  feature_PLOVE.cpp nhrec.cpp ocrbase.cpp otsubin.cpp \
  segchar_adhoc.cpp
endif

AM_CFLAGS = -I. -I../include
AM_CXXFLAGS = -I. -I../include
