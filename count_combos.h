#ifndef __COUNT_COMBOS_H__
#define __COUNT_COMBOS_H__

#include <string>
#include <getopt.h>
#include <stdlib.h>

string cmd = string(getenv("_"));
static string cc_usage = 
		"Usage:	" + cmd + "	[-iosmgrxabqltuwYFCUZQXPMIOcRTvh] [--in] [--out] [--smap] [--map]\n"
		"			[--global] [--rcr] [--rci] [--rmm] [--imm] [--min_q] [--lq_base]\n"
		"			[--threads] [--col_bs] [--cnt_bs] [--no_undef] [--no_fail] [--no_c_fail]\n"
		"			[--no_unk] [--undef_t] [--fail_t] [--unk_t] [--p_sep] [--map_sep]\n"
		"			[--in_sep] [--out_sep] [--stats] [--raw] [--table] [--quiet] [--help]\n\n"
		"	Long		short	type		description\n"
		"	====		=====	====		===========\n"
		"Required:\n" 
		"	--smap		-s	<filename>	sample map\n"
		"	--map		-m	<filename>	ID to sequence mapping\n\n"
		"Optional:\n"
		"	--in		-i	<filename>	input file (stdin)\n"
		"	--out		-o	<filename>	output file (stdout)\n\n"
		"	--global	-g	<integer>	treat the map as global\n"
		"	--rcr		-r	<integer>	reverse complement read #\n"
		"	--rci		-x	<flag>		reverse complement index\n\n"
		"	--rmm		-a	<integer>	allowed mismatches for reads (1)\n"
		"	--imm		-b	<integer>	allowed mismatches for index (1)\n\n"
		"	--min_q		-q	<integer>	minimum per base quality (20)\n"
		"	--lq_base	-l	<integer>	maximum low quality bases allowed (5)\n\n"
		"	--threads	-t	<integer>	number of threads to run (15)\n"
		"	--col_bs	-u	<integer>	collapeser bite size (250000)\n"
		"	--cnt_bs	-w	<integer>	counter bites size (1000)\n\n"
		"	--no_undef	-Y	<flag>		do not include reads with undefined index\n"
		"	--no_fail	-F	<flag>		do not include failed read counts\n"
		"	--no_c_fail	-C	<flag>		do not collapse quality failed reads\n"
		"	--no_unk	-U	<flag>		do not include unknown read counts\n\n"
		"	--undef_t	-Z	<string|char>	undefined index tag (undef)\n"
		"	--fail_t	-Q	<string|char>	quality fail tag (Q_FAIL)\n"
		"	--unk_t		-X	<string|char>	unknown sequence tag (unknown)\n\n"
		"	--p_sep		-P	<string|char>	combo ID delimiter (:)\n"
		"	--map_sep	-M	<string|char>	mapping delimiter (tab)\n"
		"	--in_sep	-I	<string|char>	input file delimiter (tab)\n"
		"	--out_sep	-O	<string|char>	output file delimiter (tab)\n\n"
		"	--stats		-c	<filename>	output raw stats\n"
		"	--raw		-R	<flag>		outout raw counts (default)\n"
		"	--table		-T	<flag>		output counts table\n\n"
		"	--quiet		-v	<flag>		suppress parameters output\n"
		"	--help		-h	<flag>		print this message and exit\n";

static struct option cc_long_options[] = {
		{"in",		optional_argument, 	NULL,	'i'},
		{"out",		optional_argument, 	NULL,	'o'},
		{"smap",	required_argument, 	NULL,	's'},
		{"map",		required_argument, 	NULL,	'm'},
		{"stats",	optional_argument, 	NULL,	'c'},
		{"global",	optional_argument, 	NULL,	'g'},

		{"rcr",		optional_argument,	NULL,	'r'},
		{"rci",		no_argument, 		NULL,	'x'},

		{"rmm",		optional_argument, 	NULL,	'a'},
		{"imm",		optional_argument, 	NULL,	'b'},

		{"min_q",	optional_argument, 	NULL,	'q'},
		{"lq_base",	optional_argument, 	NULL,	'l'},
		
		{"n_thr",	optional_argument, 	NULL,	't'},
		{"col_bs",	optional_argument, 	NULL,	'u'},
		{"cnt_bs",	optional_argument, 	NULL,	'w'},

		{"p_sep",	optional_argument, 	NULL,	'P'},
		{"in_sep",	optional_argument, 	NULL,	'I'},
		{"map_sep",	optional_argument, 	NULL,	'M'},
		{"out_sep",	optional_argument, 	NULL,	'O'},

		{"fail_t",	optional_argument, 	NULL,	'Q'},
		{"unk_t",	optional_argument, 	NULL,	'X'},
		{"undef_t",	optional_argument, 	NULL,	'Z'},

		{"raw",		no_argument, 		NULL,	'R'},
		{"table",	no_argument, 		NULL,	'T'},

		{"no_fail",	no_argument, 		NULL,	'F'},
		{"no_c_fail",	no_argument, 		NULL,	'C'},
		{"no_unk",	no_argument, 		NULL,	'U'},
		{"no_undef",	no_argument, 		NULL,	'Y'},
		{"quiet",	no_argument, 		NULL,	'v'},
		{"help",	no_argument, 		NULL,	'h'},
		{0,		0,			0, 	0 }
		};

enum CC_ERRORS {
	CCEC_BAD_CMD_LINE		= 10,
	CCEC_BAD_FILENAME	 	= 11,
	CCEC_AMBIGUOUS_MAPPING		= 12
};

enum OUT_MODE {
	RAW			= 0,
	TABLE			= 1
};

static string CC_BAD_FILENAME = 	"invalid or missing filename for ";
static string CC_AMBIGUOUS_MAP = 	"ambiguous global mapping";

#endif  // __COUNT_COMBOS_H__	
