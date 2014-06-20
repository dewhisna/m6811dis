//
//	Motorola 6811 Disassembler
//	Copyright(c)1996 - 2014 by Donna Whisnant
//

/*
 *    ERRMSGS
 *
 *    This header file defines the acceptable "exception" messages for
 *    errorcode handling.
 *
 *    Author:  Donna Whisnant
 *    Date:    January 28, 1997
 *
 *		Rewrote on July 20, 1997 to handle better exception syntax.
 */

#ifndef ERRMSGS_H
#define ERRMSGS_H

#include <string>

#define THROW_MEMORY_EXCEPTION_ERROR(code) \
	{	_OUT_OF_MEMORY_EXCEPTION.lcode = code; \
		throw (&_OUT_OF_MEMORY_EXCEPTION); }

#define THROW_EXCEPTION_ERROR(cause, code, name) \
	{	throw (new EXCEPTION_ERROR(1, cause, code, name));	}

#define DELETE_EXCEPTION_ERROR(an_error) \
	{	if (an_error->delflag)	\
			delete an_error;	}

class EXCEPTION_ERROR
{
public:
	enum {
		ERR_NONE,
		ERR_OUT_OF_MEMORY,
		ERR_OUT_OF_RANGE,
		ERR_MAPPING_OVERLAP,
		ERR_OPENREAD,
		ERR_OPENWRITE,
		ERR_FILEEXISTS,
		ERR_CHECKSUM,
		ERR_UNEXPECTED_EOF,
		ERR_OVERFLOW,
		ERR_WRITEFAILED,
		ERR_READFAILED
	};

	int	delflag;					// heap delete flag
	int	cause;						// Set to one of the above error codes by construction
	unsigned long	lcode;			// long extended code value, used for passing parameters
	std::string		name_ex;		// extended code name copied by constructor, useful for filenames, etc

	EXCEPTION_ERROR();
	EXCEPTION_ERROR(EXCEPTION_ERROR& anException);
	EXCEPTION_ERROR(int adelflag, const int acause = EXCEPTION_ERROR::ERR_NONE, const unsigned long alcode = 0, const std::string &anexname = std::string());
};

extern EXCEPTION_ERROR _OUT_OF_MEMORY_EXCEPTION;

// class ERR_OUT_OF_MEMORY {};
// class ERR_OUT_OF_RANGE {};
// class ERR_MAPPING_OVERLAP {};
// class ERR_OPENREAD { public: char *filename; };
// class ERR_OPENWRITE {public: char *filename; };
// class ERR_FILEEXISTS {};
// class ERR_CHECKSUM { public: int linenumber; };
// class ERR_UNEXPECTED_EOF {};
// class ERR_OVERFLOW {};
// class ERR_WRITEFAILED {};

#endif
