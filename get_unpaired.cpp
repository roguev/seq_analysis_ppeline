#include "map_merger.h"
#include "utils.h"
#include "get_unpaired.h"
#include <getopt.h>
using namespace std;
using namespace utils;

int main(int argc, char** argv) {

	if (argc == 1) {
		cout << gu_usage << endl;
		exit(GUEC_NO_ERROR);
	}

	char* in1 =	NULL;
	char* in2 =	NULL;
	char* out1 = 	NULL;
	char* out2 = 	NULL;
	
	uint8_t thr =	15;
	uint32_t load = 10000;
	
	bool z_in =	false;
	string in_sep = "\t";

	bool quiet = 	false;

	int opt = 0;
	while (1) {
		int long_index = 0;
		opt = getopt_long(argc, argv, "1:2:3::4::t::f::I::qh", long_options, &long_index);
		if (opt == -1) { break; }
		switch(opt) {
			case '1' 	: in1 = optarg; 		break;
			case '2' 	: in2 = optarg; 		break;
			case '3' 	: out1 = optarg; 		break;
			case '4'	: out2 = optarg;	 	break;

			case 't'	: thr = atoi(optarg);		break;
			case 'f'	: load = atoi(optarg);		break;

			case 'I'	: in_sep = string(optarg);	break;

			case 'q'	: quiet = true;			break;

			case 'h'	: cout << gu_usage << endl;	exit(GUEC_NO_ERROR);
			case '?'	: report_error(	__FILE__,__func__,GU_BAD_COMMAND_LINE);
					  exit(GUEC_BAD_COMMAND_LINE);

			default		: cout << gu_usage << endl; 	exit(GUEC_BAD_COMMAND_LINE);
		}
	}

	if ( !in1 || !in2 ) {
		report_error(__FILE__,__func__, GU_BAD_FILENAME + " for --in1,-1 or --in2,-2");
		exit(GUEC_BAD_FILENAME);
	}

	if (	(string(in1).find(".gz") != string::npos) ||
		(string(in2).find(".gz") != string::npos)) {
		z_in = true;
	}

	Map_merger mm(in1, in2, out1, out2);

	// set parameters
	mm.set_n_threads(thr);
	mm.set_load_factor(load);
	mm.set_input_sep(in_sep);

	// blurb parameters if allowed
	if (!quiet) { mm.print_params(); }

	mm.get_unpaired_reads(z_in);

	return GUEC_NO_ERROR;
}
