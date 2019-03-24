TOPDIR	=	$(shell DIR=.; while [ ! "`readlink -f \"$${DIR}\"`" = "/" -a ! -f "$${DIR}/config.mk" ]; do DIR="../$${DIR}"; done; echo "$${DIR}")
ifeq ($(shell readlink -f "$(TOPDIR)"),/)
	$(error Could not find the project top directory with config.mk)
endif
include $(TOPDIR)/config.mk

# Sub directories to build
SUBDIRS		=	$(patsubst %/,%,$(foreach subdir,$(wildcard */Makefile),$(dir $(subdir))))
LIBFILES		=	$(addsuffix .a,$(SUBDIRS))

.PHONY: all clean
.PHONY: $(SUBDIRS)

all: $(LIBFILES)

clean: $(SUBDIRS)
	@echo " [ RM ] $(LIBFILES)"
	@$(RM) $(LIBFILES)

$(SUBDIRS):
	@echo " [ CD ] $(CURRENTPATH)$@/"
	@+make -C "$@" "CURRENTPATH=$(CURRENTPATH)$@/" $(MAKECMDGOALS)

%.a: $(SUBDIRS)
	@$(RM) $@
	@$(AR) -cmT $@ $*/out.a
