CC        = gcc
CFLAGS    = -fPIC -O2 -Wall
CHIBI_FFI = chibi-ffi
CHIBI     = chibi-scheme

MODULE  = nurbs
GLUE_C  = $(MODULE).c
IMPL_C  = nurbs-impl.c
IMPL_H  = nurbs-impl.h
SO      = $(MODULE).so

all: $(SO)

# genere le glue code C a partir du stub
$(GLUE_C): $(MODULE).stub $(IMPL_H)
	$(CHIBI_FFI) $(MODULE).stub $(GLUE_C)

# compile glue + implementation en lib partagee
$(SO): $(GLUE_C) $(IMPL_C) $(IMPL_H)
	$(CC) $(CFLAGS) -shared -o $(SO) $(GLUE_C) $(IMPL_C)

test: $(SO)
	$(CHIBI) -A. test.scm

clean:
	rm -f $(GLUE_C) $(SO)

.PHONY: all test clean
