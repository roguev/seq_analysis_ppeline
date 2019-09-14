#include <string>
#include <getopt.h>
#include "read_extractor.h"
#include "extract_reads.h"
#include "utils.h"
using namespace std;
using namespace utils;

int main(int argc, char** argv) {

	if (argc == 1) {
		cout << er_usage << endl;
		exit(EREC_BAD_COMMAND_LINE);
	}

	char* in_file = NULL;
	char* out_file = NULL;
	char* rej_file = NULL;

	string pat_l;
	string pat_r;

	vector<uint16_t> roi_min;
	vector<uint16_t> roi_max;

	vector<string> spacers;

	bool z_in =	false;
	bool z_out =	false;
	bool z_rej = 	false;

	bool w_valid = 	false;
	bool w_rej = 	false;

	bool quiet =	false;

	uint8_t thr = 	15;
	uint32_t load = 10000;

	bool mml = 	true;
	bool mmr = 	true;
	bool mms = 	true;
	bool fq_out = 	false;

	string out_sep = "\t";

	int opt = 0;

	while (1) {
		int long_index = 0;
		opt = getopt_long(argc, argv, "LRSFi::x::v::l:r:m:M:t::f::O::s::qh", long_options, &long_index);
		if (opt == -1) {
			break;
		}

		switch(opt) {
			case 'i' 	: in_file = optarg;			break; 
			case 'x' 	: rej_file = optarg; w_rej = true;	break;
			case 'v' 	: out_file = optarg; w_valid = true;	break;

			case 'l'	: pat_l = string(optarg); 		break;
			case 'r'	: pat_r = string(optarg); 		break;

			case 's'	: spacers.push_back(string(optarg));	break;

			case 'O'	: out_sep = string(optarg); 		break;

			case 'm'	: roi_min.push_back(atoi(optarg));	break;
			case 'M'	: roi_max.push_back(atoi(optarg));	break;

			case 't'	: thr = atoi(optarg);			break;
			case 'f'	: load = atoi(optarg);			break;

			case 'L'	: mml = false;				break;
			case 'R'	: mmr = false;				break;
			case 'S'	: mms = false;				break;
			case 'F'	: fq_out = true;			break;

			case 'q'	: quiet = true;				break;

			case '?'	: cout << er_usage << endl; 	exit(EREC_BAD_COMMAND_LINE);
			case 'h'	: cout << er_usage << endl; 	exit(EREC_BAD_COMMAND_LINE);
			default		: cout << er_usage << endl; 	exit(EREC_BAD_COMMAND_LINE);
		}
	}

	// 2 simultaneous stdout outputs
	if (w_valid && w_rej) {
		if (!out_file && !rej_file) {	
			report_error(__FILE__, __func__, ER_MISSING_ARGUMENT + "at least one of --valid,-v or --rejected,-x needs to be specified");
			exit(EREC_BAD_COMMAND_LINE);
		}
	}

	if (roi_min.empty()) {
		report_error(__FILE__, __func__, ER_MISSING_ARGUMENT + string("--roi_min,-m"));
		exit(EREC_BAD_COMMAND_LINE);
	}

	if (roi_max.empty()) {
		report_error(__FILE__, __func__, ER_MISSING_ARGUMENT + string("--roi_max,-M"));
		exit(EREC_BAD_COMMAND_LINE);
	}

	if (roi_min.size() != roi_max.size()) {
		report_error(__FILE__, __func__, ER_BAD_ROI_PARAMS);
		exit(EREC_BAD_ROI_PARAMS);
	}

	if (!spacers.empty() && (roi_min.size() - 1 != spacers.size())) {
		report_error(__FILE__, __func__, ER_BAD_SPACER);
		exit(EREC_BAD_SPACER_COUNT);
	}

	if (in_file && (string(in_file).find(".gz") != string::npos)) { z_in = true; }
	if (rej_file && (string(rej_file).find(".gz") != string::npos)) { z_rej = true; }
	if (out_file && (string(out_file).find(".gz") != string::npos)) { z_out = true; }

	Read_extractor rx(
			in_file,
			pat_l,
			pat_r,
			roi_min,
			roi_max,
			spacers,
			out_file,
			rej_file);

	rx.set_n_threads(thr);
	rx.set_load_factor(load);

	rx.set_mm_l(mml);
	rx.set_mm_r(mmr);
	rx.set_mm_s(mms);

	rx.set_with_valid(w_valid);
	rx.set_with_rejected(w_rej);
	rx.set_fastq_out(fq_out);

	rx.set_output_sep(out_sep);
	if (!quiet) { rx.print_params(); }

	rx.extract(z_in, z_out, z_rej);

	return 0;
}
