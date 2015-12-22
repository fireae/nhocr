bin_PROGRAMS = nhocr
nhocr_SOURCES = nhocr.cpp segline.cpp
AM_CXXFLAGS = -I../include
nhocr_LDADD = -L../libnhocr -L../O2-tools/lib -lm -lnhocr -lufp -lsip -logrp -limgo $(X_EXTRA_LIBS)
#dist_man_MANS = nhocr.1
