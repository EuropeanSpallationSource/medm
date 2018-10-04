#*************************************************************************
# Copyright (c) 2002 The University of Chicago, as Operator of Argonne
# National Laboratory.
# Copyright (c) 2002 The Regents of the University of California, as
# Operator of Los Alamos National Laboratory.
# This file is distributed subject to a Software License Agreement found
# in the file LICENSE that is included with this distribution. 
#*************************************************************************
# Makefile for medm top level

TOP = ../..
LINUX_DISTRO = $(shell sh -c '$(TOP)/epicsextfinddist.sh || echo PWD=$$PWD TOP=$(TOP)')

# Install for Debian
ifeq ($(shell expr "$(LINUX_DISTRO)" : 'Debian[89]'),7)
build: /usr/include/Xm/XmP.h

/usr/include/Xm/XmP.h:
	sudo apt-get install -y x11proto-print-dev libmotif-dev xfonts-100dpi libxp-dev libxmu-dev libxpm-dev
endif

# Install for Raspbian
ifeq ($(shell expr "$(LINUX_DISTRO)" : 'Raspbian[89]'),9)
build: /usr/include/Xm/XmP.h

/usr/include/Xm/XmP.h:
	sudo apt-get install -y x11proto-print-dev libmotif-dev xfonts-100dpi libxp-dev libxmu-dev libxpm-dev
endif

build: medmecholinuxditro

medmecholinuxditro:
	@echo medm/Makefile: LINUX_DISTRO=$(LINUX_DISTRO)


include $(TOP)/configure/CONFIG
DIRS = printUtils xc medm
include $(TOP)/configure/RULES_DIRS

medm_DEPEND_DIRS = printUtils xc
