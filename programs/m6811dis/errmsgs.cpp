//
//	Motorola 6811 Disassembler
//	Copyright(c)1996 - 2014 by Donna Whisnant
//

/*
 *    ERRMSGS
 *
 *    This file defines the acceptable "exception" messages for
 *    errorcode handling.
 *
 *    Author:  Donna Whisnant
 *    Date:    July 20, 1997
 *
 *	This file was added as part of the rewrite process.
 */

#include "errmsgs.h"
#include <string>

// Preconstruct an out of memory error, cause if we run out of
//	memory, we may not have enough memory to construct it then.
EXCEPTION_ERROR _OUT_OF_MEMORY_EXCEPTION(0,
		EXCEPTION_ERROR::ERR_OUT_OF_MEMORY, 0, std::string());


EXCEPTION_ERROR::EXCEPTION_ERROR()
{
	delflag = 1;
	cause = EXCEPTION_ERROR::ERR_NONE;
	lcode = 0;
	name_ex.clear();
}

EXCEPTION_ERROR::EXCEPTION_ERROR(EXCEPTION_ERROR& anException)
{
	delflag = anException.delflag;
	cause = anException.cause;
	lcode = anException.lcode;
	name_ex = anException.name_ex;
}

EXCEPTION_ERROR::EXCEPTION_ERROR(int adelflag, const int acause, const unsigned long alcode, const std::string &anexname)
{
	delflag = adelflag;
	cause = acause;
	lcode = alcode;
	name_ex = anexname;
}
