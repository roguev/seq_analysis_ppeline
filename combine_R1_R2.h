#ifndef __COMBINE_R1_R2_H__
#define __COMBINE_R1_R2_H__

#include <string>
#include <getopt.h>
#include <stdlib.h>

string cmd = string(getenv("_"));
static string cr_usage = 
	"Usage:	" + cmd + "	[-12oIOltfqh] [--in1] [--in2] [--out] [--in_sep] [--out_sep]\n"
	"			[--lines] [--threads] [--load] [--quiet] [--help]\n\n"
	"	long		short	type		description\n"
	"	====		=====	====		===========\n"
	"Required:\n"
	"	--in1		-1	<filename>	read1 sequences\n"
	"	--in2		-2	<filename>	read2 sequences\n\n"
	"Optional:\n"
	"	--out		-o	<filename>	output file (stdout)\n\n"
	"	--in_sep	-I	<string|char>	input file delimiter (tab)\n"
	"	--out_sep	-O	<string|char>	output file delimiter (tab)\n\n"
	"	--lines		-l	<integer>	maximum number of lines (100,000,000)\n"
	"	--threads	-t	<integer>	set number of threads (15)\n"
	"	--load		-f	<integer>	load factor (10,000)\n\n"
	"	--quiet		-q	<flag>		suppress parameters output\n"
	"	--help		-h	<flag>		print this message and exit\n";
	

static struct option long_options[] = {
	{"in1",		required_argument, 	NULL,	'1'},
	{"in2",		required_argument, 	NULL,	'2'},
	{"out",		optional_argument, 	NULL,	'o'},

	{"lines",	optional_argument, 	NULL,	'l'},
	{"threads",	optional_argument,	NULL,	't'},
	{"load",	optional_argument,	NULL,	'f'},

	{"in_sep",	optional_argument,	NULL,	'I'},
	{"out_sep",	optional_argument,	NULL,	'O'},

	{"quiet",	no_argument,		NULL,	'q'},
	{"help",	no_argument,		NULL,	'h'},
	{0,		0,			0, 	0 }
};

enum CR_ERRORS {
	CREC_NO_ERROR		=	0,
	CREC_BAD_FILENAME	=	10,
	CREC_BAD_COMMAND_LINE	=	11
};

static string CR_BAD_COMMAND_LINE = 	"bad or missing parameter";
static string CR_BAD_FILENAME = 	"bad filename for ";

#endif //__COMBINE_R1_R2_H__
