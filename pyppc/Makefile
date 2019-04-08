###########################################################################
# Copyright (C) 2019 Swedish Meteorological and Hydrological Institute, SMHI,
#
# This file is part of baltrad-ppc.
#
# baltrad-ppc is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# baltrad-ppc is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public License
# along with baltrad-ppc.  If not, see <http://www.gnu.org/licenses/>.
# ------------------------------------------------------------------------
# 
# baltrad-ppc make file
# @file
# @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
# @date 2019-02-17
###########################################################################
-include ../def.mk

# c flags
#
CFLAGS= -I../ppc -I. $(RAVE_MODULE_PYCFLAGS)

# Linker flags
#
LDFLAGS= -L../ppc -L. $(RAVE_MODULE_LDFLAGS)

# We need to ensure that raves extra components are managed
LIBRARIES= -lbaltrad-ppc $(RAVE_MODULE_PYLIBRARIES)

# --------------------------------------------------------------------
# Fixed definitions

PDP_PROCESSOR_SOURCE= pypdpprocessor.c
PDP_PROCESSOR_OBJECTS= $(PDP_PROCESSOR_SOURCE:.c=.o)
PDP_PROCESSOR_TARGET= _pdpprocessor.so

MAKECDEPEND=$(CC) -MM $(CFLAGS) -MT '$(@D)/$(@F)' -o $(DF).d $<

DEPDIR=.dep
DF=$(DEPDIR)/$(*F)

# Ensures that the .dep directory exists
.PHONY=$(DEPDIR)
$(DEPDIR):
	+@[ -d $@ ] || mkdir -p $@

# And the rest of the make file targets
#
.PHONY=all
all:		$(PDP_PROCESSOR_TARGET)

$(PDP_PROCESSOR_TARGET): $(DEPDIR) $(PDP_PROCESSOR_OBJECTS) ../ppc/libbaltrad-ppc.so
	$(LDSHARED) -o $@ $(PDP_PROCESSOR_OBJECTS) $(LDFLAGS) $(LIBRARIES)

.PHONY=install
install:
	@mkdir -p ${DESTDIR}${prefix}/share/baltrad-ppc/pyppc
	@cp -v -f $(PDP_PROCESSOR_TARGET) ${DESTDIR}${prefix}/share/baltrad-ppc/pyppc
	@cp -v -f *.py ${DESTDIR}${prefix}/share/baltrad-ppc/pyppc
	@-mkdir -p ${DESTDIR}$(SITEPACK_PYTHON)
	@-echo "$(prefix)/share/baltrad-ppc/pyppc" > "${DESTDIR}$(SITEPACK_PYTHON)/pybaltradppc.pth"

.PHONY=clean
clean:
	@\rm -f *.o core *~
	@\rm -fr $(DEPDIR)

.PHONY=distclean		 
distclean:	clean
	@\rm -f $(PDP_PROCESSOR_TARGET)

# --------------------------------------------------------------------
# Rules

# Contains dependency generation as well, so if you are not using
# gcc, comment out everything until the $(CC) statement.
%.o : %.c
	@$(MAKECDEPEND); \
	cp $(DF).d $(DF).P; \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
		-e '/^$$/ d' -e 's/$$/ :/' < $(DF).d >> $(DF).P; \
	\rm -f $(DF).d
	$(CC) -c $(CFLAGS) $< -o $@

# NOTE! This ensures that the dependencies are setup at the right time so this should not be moved
-include $(SOURCES:%.c=$(DEPDIR)/%.P)
