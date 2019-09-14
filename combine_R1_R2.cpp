#include "map_merger.h"
#include "utils.h"
#include "combine_R1_R2.h"
#include <getopt.h>
using namespace std;
using namespace utils;

int main(int argc, char** argv) {

	if (argc == 1) {
		cout << cr_usage << endl;
		exit(CREC_BAD_COMMAND_LINE);
	}

	char* in1 =		NULL;
	char* in2 =		NULL;
	char* out = 		NULL;

	uint32_t lines =	100000000;
	uint32_t load = 	10000;
	uint8_t threads =	15;

	bool quiet = 		false;

	bool in_z = 		false;
	bool out_z = 		false;

	string in_sep =		"\t";
	string out_sep =	"\t";

	int opt = 0;
	while (1) {
		int long_index = 0;
		opt = getopt_long(argc, argv, "1:2:o::l::t::f::I::O::qh", long_options, &long_index);
		if (opt == -1) { break; }
		switch(opt) {
			case '1' 	: in1 = optarg; 		break;
			case '2' 	: in2 = optarg; 		break;
			case 'o' 	: out = optarg; 		break;

			case 'l'	: lines = atoi(optarg); 	break;
			case 't'	: threads = atoi(optarg);	break;
			case 'f'	: load = atoi(optarg);		break;

			case 'I'	: in_sep = string(optarg);	break;
			case 'O'	: out_sep = string(optarg);	break;
	
			case 'q'	: quiet = true;			break;

			case '?'	: report_error(__FILE__,__func__,CR_BAD_COMMAND_LINE);
					  exit(CREC_BAD_COMMAND_LINE);

			case 'h'	: cout << cr_usage << endl;	exit(CREC_NO_ERROR);;
			default		: cout << cr_usage << endl; 	exit(CREC_BAD_COMMAND_LINE);
		}
	}

	// must speify input filenames
	if ( !in1 || !in2 ) {
		report_error(__FILE__,__func__,CR_BAD_FILENAME + "--in1,-1 or --in2,-2");
		exit(CREC_BAD_FILENAME);
	}

	// set zipped input flag
	if (	(string(in1).find(".gz") != string::npos) ||
		(string(in2).find(".gz") != string::npos)) {
		in_z = true;
	}

	// set zipped output flag
	if (out && (string(out).find(".gz") != string::npos)) {
		out_z = true;
	}

	Map_merger mm(in1, in2, out, lines);
	mm.set_n_threads(threads);
	mm.set_load_factor(load);
	mm.set_input_sep(in_sep);
	mm.set_output_sep(out_sep);

	// blurb parameters if allowed
	if (!quiet) { mm.print_params(); }

	mm.merge_id_maps(in_z, out_z);

	return CREC_NO_ERROR;
}
