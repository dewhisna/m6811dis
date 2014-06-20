#
#	Motorola 6811 Disassembler
#	Copyright(c)1996 - 2014 by Donna Whisnant
#

#-------------------------------------------------
#
# Project created by QtCreator 2014-06-14T19:33:04
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = m6811dis
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += \
	m6811dis.cpp \
	bindfc.cpp \
	dfc.cpp \
	errmsgs.cpp \
	gdc.cpp \
	inteldfc.cpp \
	m6811gdc.cpp \
	memclass.cpp \
	sfiledfc.cpp

HEADERS += \
	m6811dis.h \
	bindfc.h \
	dfc.h \
	errmsgs.h \
	gdc.h \
	inteldfc.h \
	m6811gdc.h \
	memclass.h \
	sfiledfc.h \
	stringhelp.h


