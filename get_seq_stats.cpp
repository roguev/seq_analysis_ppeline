#include <string>
#include <vector>
#include <getopt.h>
#include "seq_stats.h"
#include "get_seq_stats.h"
#include "utils.h"
using namespace std;
using namespace utils;


int main(int argc, char** argv) {

	if (argc == 1) {
		cout << gs_usage << endl;
		exit(GSEC_BAD_COMMAND_LINE);
	}

	char* in_file = NULL;
	char* out_file = NULL;

	vector<string> stats;

	uint8_t mode;

	bool z_in =		false;
	bool quiet =		false;
	bool raw_input =	false;

	uint8_t thr = 	4;

	uint16_t rl = 	0;
	uint32_t load = 25000;

	string out_sep = "\t";
	string in_sep = "\t";

	int opt = 0;
	
	while (1) {
		int long_index = 0;
		opt = getopt_long(argc, argv, "i::o::l:I::O::s::t::f::qrh", long_options, &long_index);
		if (opt == -1) {
			break;
		}

		switch(opt) {
			case 'i' 	: in_file = optarg;			break;
			case 'o' 	: out_file = optarg;			break;
	
			case 'l'	: rl = atoi(optarg);			break;

			case 'O'	: out_sep = string(optarg); 		break;
			case 'I'	: in_sep = string(optarg); 		break;

			case 's'	: stats.push_back(string(optarg)); 	break;

			case 't'	: thr = atoi(optarg);			break;
			case 'f'	: load = atoi(optarg);			break;

			case 'q'	: quiet = true;				break;
			case 'r'	: raw_input = true;			break;

			case '?'	: cout << gs_usage << endl; 	exit(GSEC_BAD_COMMAND_LINE);
			case 'h'	: cout << gs_usage << endl; 	exit(GSEC_BAD_COMMAND_LINE);
			default		: cout << gs_usage << endl; 	exit(GSEC_BAD_COMMAND_LINE);
		}
	}

	if (stats.size() == 0) {
		stats.push_back("all");
	}

	if (rl == 0) {
		report_error(__FILE__,__func__, GS_INVALID_READ_LENGTH);
		exit(GSEC_BAD_COMMAND_LINE);
	}

	if (in_file && (string(in_file).find(".gz") != string::npos)) {
		z_in = true;
	}

	// initialize the object
	Seq_stats ss(in_file, out_file, rl);

	// set parameters
	ss.set_output_sep(out_sep);
	ss.set_load_factor(load);
	ss.set_n_threads(thr);
	ss.set_raw_input(raw_input);

	if (!quiet) {
		// blurb some info
		ss.print_params();

		// decide which mode to use
		cout << "Output_mode:\t";
		for (size_t i = 0; i < stats.size(); ++i) {
			cout << stats.at(i) << " ";
		}
		cout << "\n";
	}
	
	for (size_t i = 0; i < stats.size(); ++i) {
		if (stats.at(i).compare("count") == 0) { mode += 1; }	// set bit 1
		if (stats.at(i).compare("freq") == 0) { mode += 2; }	// set bit 2	
		if (stats.at(i).compare( "qual") == 0) { mode += 4; }	// set bit 3
		if (stats.at(i).compare("length") == 0) { mode += 8; }	// set bit 4
		if (stats.at(i).compare( "all") == 0) { mode = 16; }	// set bit 5, unset bits 1-4
	}

	// collect data
	ss.collect_stats(z_in);

	// output
	ss.output_stats(static_cast<SS_OUTPUT_MODE>(mode));

	return 0;
}
