###########################################################################
# Copyright (C) 2011 Swedish Meteorological and Hydrological Institute, SMHI,
#
# This file is part of bRopo.
#
# bRopo is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# bRopo is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public License
# along with bRopo.  If not, see <http://www.gnu.org/licenses/>.
# ------------------------------------------------------------------------
# 
# Doxygen make file
# @file
# @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
# @date 2011-08-30
###########################################################################
-include ../def.mk

all:

.PHONY: clean
clean:
	@\rm -f *.o
	@\rm -f *~ core

.PHONY: distclean
distclean:
	@\rm -f *~ core

.PHONY: install
install:
	@mkdir -p ${DESTDIR}${prefix}/share/baltrad-ppc/config
	@cp -v -f *.xml ${DESTDIR}${prefix}/share/baltrad-ppc/config
