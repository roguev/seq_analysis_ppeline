#ifndef __GET_UNPAIRED_H__
#define __GET_UNPAIRED_H__

#include <string>
#include <getopt.h>
#include <stdlib.h>

string cmd = string(getenv("_"));
static string gu_usage = 
	"Usage:	" + cmd + "	[-1234Itfqh] [--in1] [--in2] [--out1] [--out2]\n"
	"			[--in_sep] [--threads] [--load] [--quiet] [--help]\n\n"
	"	long		short	type		description\n"
	"	====		=====	====		===========\n"
	"Required:\n"
	"	--in1		-1	<filename>	read1 sequences\n"
	"	--in2		-2	<filename>	read2 sequences\n\n"
	"Optional:\n"
	"	--out1		-3	<filename>	unpaired reads read1 (stdout)\n"
	"	--out2		-4	<filename>	unpaired reads read2 (stdout)\n\n"
	"	--in_sep	-I	<string|char>>	input file delimiter (tab)\n"
	"	--threads	-t	<integer>	set number of threads (15)\n"
	"	--load		-f	<integer>	load factor (10,000)\n\n"
	"	--quiet		-q	<flag>		suppress parameters output\n"
	"	--help		-h	<flag>		print this message and exit\n";

static struct option long_options[] = {
	{"in1",		required_argument, 	NULL,	'1'},
	{"in2",		required_argument, 	NULL,	'2'},
	{"out1",	optional_argument, 	NULL,	'3'},
	{"out2",	optional_argument, 	NULL,	'4'},
	{"threads",	optional_argument,	NULL,	't'},
	{"load",	optional_argument,	NULL,	'f'},
	{"in_sep",	optional_argument,	NULL,	'I'},
	{"quiet",	no_argument,		NULL,	'q'},
	{"help",	no_argument,		NULL,	'h'},
	{0,		0,			0, 	0 }
};

enum CR_ERRORS {
	GUEC_NO_ERROR		=	0,
	GUEC_BAD_FILENAME	=	10,
	GUEC_BAD_COMMAND_LINE	=	11
};

static string GU_BAD_COMMAND_LINE = 	"bad or missing parameter";
static string GU_BAD_FILENAME = 	"bad filename for ";

#endif //__GET_UNPAIRED_H__
