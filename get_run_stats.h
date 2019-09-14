#ifndef __GET_RUN_STATS_H__
#define __GET_RUN_STATS_H__

#include <string>
#include <getopt.h>
#include <stdlib.h>

string cmd = string(getenv("_"));
static string gs_usage = 
	"Usage:	" + cmd + "	[-ioxyRLTsOtfqh] [--in] [--out] [--xbin] [--ybin] [--read] [--lane] [--tile]\n"
	"			[--out_sep] [--threads] [--load] [--quiet] [--help]\n\n"
	"	long		short	type		description\n"
	"	====		=====	====		===========\n"
	"Required:\n"
	"	None\n\n"
	"Optional:\n"
	"	--in		-i	<filename>	input FASTQ file (stdin)\n"
	"	--out		-o	<filename>	output file (stdout)\n\n"
	"	--stat		-s	<string>	stats to collect [clust|len|qual|N|A|T|G|C] (clust)\n\n"
	"	--xbin		-x	<integer>	x-bin size (250)\n"
	"	--ybin		-y	<integer>	y-bin size (250)\n\n"
	"	--read		-R	<integer>	collect data for specific read (all)\n"
	"	--lane		-L	<integer>	collect data for specific lane (all)\n"
	"	--tile		-T	<integer>	collect data for specific tile (all)\n\n"
	"	--out_sep	-O	<string|char>	output file delimiter (tab)\n"
	"	--threads	-t	<integer>	number of threads (4)\n"
	"	--load		-f	<integer>	load factor (25,000)\n\n"
	"	--quiet		-q	<flag>		supperss parameters output\n"
	"	--help		-h	<flag>		print this message and exit\n";

static struct option long_options[] = {
	{"in",		optional_argument, 	NULL,	'i'},
	{"out",		optional_argument, 	NULL,	'o'},

	{"xbin",	optional_argument, 	NULL,	'x'},
	{"ybin",	optional_argument, 	NULL,	'y'},

	{"read",	optional_argument, 	NULL,	'R'},
	{"lane",	optional_argument, 	NULL,	'L'},
	{"tile",	optional_argument, 	NULL,	'T'},

	{"stat",	optional_argument, 	NULL,	's'},

	{"threads",	optional_argument, 	NULL,	't'},
	{"load",	optional_argument, 	NULL,	'f'},

	{"out_sep",	optional_argument, 	NULL,	'O'},

	{"quiet",	no_argument,		NULL,	'q'},
	{"help",	no_argument,		NULL,	'h'},
	{0,		0,			0,	0 }
};

enum GRS_ERRORS {
	GRSEC_BAD_COMMAND_LINE 	= 	10
};

static string GRS_BAD_BIN_SIZE =		"invalid bin size: ";
#endif   //__GET_RUN_STATS_H__
