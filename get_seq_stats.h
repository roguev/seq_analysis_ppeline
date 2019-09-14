#ifndef __GET_SEQ_STATS_H__
#define __GET_SEQ_STATS_H__

#include <string>
#include <getopt.h>
#include <stdlib.h>

string cmd = string(getenv("_"));
static string gs_usage = 
	"Usage:	" + cmd + "	[-irolsIOtfqh] [--in] [--raw] [--out] [--r_len] [--stats]\n"
	"			[--in_sep] [--out_sep] [--threads] [--load] [--quiet] [--help]\n\n"
	"	long		short	type		description\n"
	"	====		=====	====		===========\n"
	"Required:\n"
	"	--r_len		-l	<integer>	read length\n\n"
	"Optional:\n"
	"	--in		-i	<filename>	input FASTQ file (stdin)\n"
	"	--raw		-r	<flag>		input data is in raw format (false)\n"
	"	--out		-o	<filename>	output file (stdout)\n\n"
	"	--stats		-s	<string>	stats to collect [count|freq|qual|length|all] (all)\n\n"
	"	--in_sep	-I	<string|char>	input file delimiter (tab)\n"
	"	--out_sep	-O	<string|char>	output file delimiter (tab)\n"
	"	--threads	-t	<integer>	number of threads (4)\n"
	"	--load		-f	<integer>	load factor (25,000)\n\n"
	"	--quiet		-q	<flag>		supperss parameters output\n"
	"	--help		-h	<flag>		pritnt this message and exit\n";

static struct option long_options[] = {
	{"in",		optional_argument, 	NULL,	'i'},
	{"out",		optional_argument, 	NULL,	'o'},
	{"r_len",	required_argument, 	NULL,	'l'},

	{"stats",	optional_argument,	NULL,	's'},

	{"threads",	optional_argument, 	NULL,	't'},
	{"load",	optional_argument, 	NULL,	'f'},

	{"out_sep",	optional_argument, 	NULL,	'O'},
	{"in_sep",	optional_argument, 	NULL,	'I'},

	{"raw",		no_argument,		NULL,	'r'},
	{"quiet",	no_argument,	 	NULL,	'q'},
	{"help",	no_argument,		NULL,	'h'},
	{0,		0,			0,	0 }
};

enum GS_ERRORS {
	GSEC_BAD_COMMAND_LINE 	= 	10
};

static string GS_MISSING_ARGUMENT =	"required parameter missing: ";
static string GS_INVALID_READ_LENGTH = 	"invalid read length: ";

#endif   //__GET_STATS_H__
