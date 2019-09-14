#ifndef __EXTRACT_READS_H__
#define __EXTRACT_READS_H__

#include <string>
#include <getopt.h>
#include <stdlib.h>

string cmd = string(getenv("_"));
static string er_usage = 
	"Usage: " + cmd +"	[-ilrmMsvFxOtfLRSqh] [--in] [--left] [--right] [--roi_min]\n"
	"			[--roi_max] [--spacer] [--valid] [--fastq_out] [--rejected] [--out_sep]\n"
	"			[--threads] [--load] [--no_mml] [--no_mmr] [--no_mms] [--quiet] [--help]\n\n"
	"	long		short	type		description\n"
	"	====		=====	====		===========\n"
	"Required:\n"
	"	--left		-l	<string>	left anchor sequence\n"
	"	--right		-r	<string>	right anchor sequence\n"
	"	--roi_min	-m	<integer>	minimum length of ROI\n"
	"	--roi_max	-M	<integer>	maximum length of ROI\n\n"
	"Optional:\n"
	"	--in		-i	<filename>	input FASTQ file (stdin)\n"
	"	--spacer	-s	<string|char>	spacer sequence\n"
	"	--valid		-v	<filename>	accepted reads output (stdout)\n"
	"	--fastq_out	-F	<flag>		write accepted reads in FASTQ format (false)\n"
	"	--rejected	-x	<filename>	rejected reads output (stdout)\n\n"
	"	--out_sep	-O	<string|char>	output file delimiter (tab)\n\n"
	"	--threads	-t	<integer>	number of threads (15)\n"
	"	--load		-f	<integer>	load factor (10,000)\n\n"
	"	--no_mml	-L	<flag>		disallow mismatches in left anchor sequence\n"
	"	--no_mmr	-R	<flag>		disallow mismatches in right anchor sequence\n"
	"	--no_mms	-S	<flag>		disallow mismatches in spacer sequences\n\n"
	"	--quiet		-q	<flag>		suppress parameters output\n"
	"	--help		-h	<flag>		print this message and exit\n";

static struct option long_options[] = {
	{"in",		required_argument, 	NULL,	'i'},

	{"rejected",	optional_argument, 	NULL,	'x'},
	{"valid",	optional_argument, 	NULL,	'v'},
	{"fastq_out",	no_argument, 		NULL,	'F'},

	{"left",	required_argument, 	NULL,	'l'},
	{"right",	required_argument, 	NULL,	'r'},

	{"spacer",	optional_argument, 	NULL,	's'},

	{"roi_min",	required_argument, 	NULL,	'm'},
	{"roi_max",	required_argument, 	NULL,	'M'},

	{"threads",	optional_argument, 	NULL,	't'},
	{"load",	optional_argument, 	NULL,	'f'},

	{"out_sep",	optional_argument, 	NULL,	'O'},

	{"no_mml",	no_argument,		NULL,	'L'},
	{"no_mmr",	no_argument,		NULL,	'R'},
	{"no_mms",	no_argument,		NULL,	'S'},

	{"quiet",	no_argument,		NULL,	'q'},
	{"help",	no_argument,		NULL,	'h'},
	{0,		0,			0,	0 }
};

enum ER_ERRORS {
	EREC_BAD_FILENAME	=	10,
	EREC_BAD_ROI_PARAMS	=	11,
	EREC_BAD_SPACER_COUNT	= 	12,
	EREC_BAD_COMMAND_LINE 	= 	13
};

static string ER_MISSING_ARGUMENT =	"required parameter missing: ";
static string ER_BAD_FILENAME = 	"bad filename for ";
static string ER_BAD_ROI_PARAMS = 	"Incosistent ROI parameters";
static string ER_BAD_SPACER = 		"Bad spacer count";

#endif	//__EXTRACT_READS_H__
