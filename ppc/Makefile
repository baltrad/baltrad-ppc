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
# @date 2019-02-16
###########################################################################
-include ../def.mk

# Ropo specific c flags
#
CFLAGS= -I. $(RAVE_MODULE_CFLAGS)

# --------------------------------------------------------------------
# Fixed definitions

SOURCES= pdp_processor.c ppc_options.c ppc_radar_options.c
				
OBJECTS= $(SOURCES:.c=.o)

TARGET= libbaltrad-ppc.so

MAKECDEPEND=$(CC) -MM $(CFLAGS) -MT '$(@F)' -o $(DF).d $<

DEPDIR=.dep
DF=$(DEPDIR)/$(*F)

# Ensures that the .dep directory exists
.PHONY=$(DEPDIR)
$(DEPDIR):
	+@[ -d $@ ] || mkdir -p $@

# And the rest of the make file targets
#
.PHONY=all
all:		$(TARGET)

$(TARGET): $(DEPDIR) $(OBJECTS)
	$(LDSHARED) -o $@ $(OBJECTS)

.PHONY=install
install:
	@mkdir -p ${DESTDIR}${prefix}/include/
	@mkdir -p ${DESTDIR}${prefix}/lib/
	@cp -v -f *.h ${DESTDIR}${prefix}/include/
	@cp -v -f $(TARGET) ${DESTDIR}${prefix}/lib/

.PHONY=clean
clean:
	@\rm -f *.o core *~
	@\rm -fr $(DEPDIR)

.PHONY=distclean		 
distclean:	clean
	@\rm -f $(TARGET)

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
