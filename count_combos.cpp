#include <string>
#include <iostream>
#include "read_counter.h"
#include "utils.h"
#include "count_combos.h"
#include <getopt.h>
#include <vector>
using namespace std;
using namespace utils;

int main(int argc, char** argv) {

	if (argc == 1) {
		cout << cc_usage << endl;
		exit(CCEC_BAD_CMD_LINE);
	}

	char* in =		NULL;
	char* out =	 	NULL;
	char* smap =	 	NULL;
	char* raw_stats	=	NULL;

	bool in_flag = 		false;
	bool out_flag = 	false;
	bool smap_flag =	false;
	bool maps_flag = 	false;

	vector<char*> maps;
	vector<uint8_t> rcs;
	vector<uint8_t> mms;

	uint8_t min_qual = 	20;
	uint8_t lq_bases = 	5;

	uint8_t n_threads =	15;

	uint8_t idxmm = 	1;
	bool idxrc =	 	false;

	uint32_t col_bs = 	250000;
	uint32_t cnt_bs =	1000;

	int global = 0;

	int out_mode =		RAW;
	bool in_z = 		false;

	bool quiet = 		false;

	bool fails =		true;
	bool unks = 		true;
	bool undefs = 		true;
	bool collapse_fails = 	true;

	string p_sep =	 	":";
	string in_sep = 	"\t";
	string out_sep = 	"\t";
	string map_sep = 	"\t";

	string fail_tag = 	"Q_FAIL";
	string unk_tag = 	"unknown";
	string undef_tag = 	"undef";

	int opt = 0;

	while (1) {
		int long_index = 0;
		opt = getopt_long(argc,
				argv, 
				"xRTFCUYvhi::o::s:m:r::a::b::q::l::t::u::w::P::I::M::O::Q::X::Z::g::c::", 
				cc_long_options, 
				&long_index);

		if (opt == -1) { break; }

		switch(opt) {
			case 'i' 	: in = optarg;  					break;
			case 'o' 	: out = optarg;						break;
			case 's'	: smap = optarg;					break; 
			case 'm'	: maps.push_back(optarg);				break;
			case 'c'	: raw_stats = optarg;					break;

			case 'g'	: global = atoi(optarg);				break;

			case 'r'	: rcs.push_back(atoi(optarg)-1);			break;
			case 'x'	: idxrc = true;					 	break;

			case 'a'	: mms.push_back(atoi(optarg));				break;
			case 'b'	: idxmm = atoi(optarg); 				break;

			case 'q'	: min_qual = atoi(optarg); 				break;
			case 'l'	: lq_bases = atoi(optarg); 				break;

			case 't'	: n_threads = atoi(optarg); 				break;
			case 'u'	: col_bs = atoi(optarg); 				break;
			case 'w'	: cnt_bs = atoi(optarg); 				break;

			case 'v'	: quiet = true;						break;
			
			case 'U'	: unks = false;						break;
			case 'F'	: fails = false;					break;
			case 'Y'	: undefs = false;					break;
			case 'C'	: collapse_fails = false;				break;

			case 'P'	: p_sep = string(optarg);				break;
			case 'I'	: in_sep = string(optarg);				break;
			case 'O'	: out_sep = string(optarg);				break;
			case 'M'	: map_sep = string(optarg);				break;
	
			case 'Q'	: fail_tag = string(optarg);				break;
			case 'X'	: unk_tag = string(optarg);				break;
			case 'Z'	: undef_tag = string(optarg);				break;

			case 'R'	: out_mode = RAW;				 	break;
			case 'T'	: out_mode = TABLE;				 	break;

			case '?'	: cout << cc_usage << endl; 				exit(CCEC_BAD_CMD_LINE);
			case 'h'	: cout << cc_usage << endl; 				exit(CCEC_BAD_CMD_LINE);
			default		: cout << cc_usage << endl; 				exit(CCEC_BAD_CMD_LINE);
		}
	}

	// sanity check
	// sample map defined?
	if (smap == NULL) {
		report_error(__FILE__,__func__, CC_BAD_FILENAME + string("--smap,-s"));
		exit(CCEC_BAD_FILENAME);
	}

	// mappings between sequence and human readable IDs defined
	if (maps.empty()) {
		report_error(__FILE__,__func__, CC_BAD_FILENAME + string("--map,-m"));
		exit(CCEC_BAD_FILENAME);
	}

	if (!maps.empty()) {
		for (size_t i = 0; i < maps.size(); ++i) {
			if (maps.at(i) == NULL) {
				report_error(__FILE__,__func__, CC_BAD_FILENAME + string("--map,-m"));
				exit(CCEC_BAD_FILENAME);
			}

			if (maps.at(i) != NULL) {
				if (string(maps.at(i)).empty()) {
					report_error(__FILE__,__func__, CC_BAD_FILENAME + string("--map,-m"));
					exit(CCEC_BAD_FILENAME);
				}
			}
		}
	}

	// re-create the maps vector if using a global map
	if (global > 0) {
		// more than one map defined?
		if (maps.size() > 1) {
			report_error(__FILE__, __func__, CC_AMBIGUOUS_MAP);
			exit(CCEC_AMBIGUOUS_MAPPING);
		}
		
		// populate a new mapping vector and re-assign
		vector<char*> new_maps;
		for (size_t i = 0; i < global; ++i) {
			new_maps.push_back(maps.at(0));
		}
		maps = new_maps;
	}

	Read_counter rc(in, out, smap, maps);

	// set mm
	if (!mms.empty()) {
		for (size_t i = 0; i < mms.size(); ++i) {
			rc.set_mm(mms.at(i), i);
		}
	}
	rc.set_index_mm(idxmm);
	
	// set rc
	if (!rcs.empty()) { 
		for (size_t i = 0; i < rcs.size(); ++i) {
			rc.set_rc(true, rcs.at(i));
		}
	}
	rc.set_idxrc(idxrc);
	
	rc.set_min_qual(min_qual);
	rc.set_max_lq_bases(lq_bases);

	rc.set_n_threads(n_threads);
	rc.set_collapser_bite_size(col_bs);
	rc.set_counter_bite_size(cnt_bs);

	rc.set_with_undefs(undefs);
	rc.set_with_fails(fails);
	rc.set_with_unknowns(unks);

	rc.set_collapse_q_fails(collapse_fails);

	rc.set_record_separator(p_sep);
	rc.set_input_sep(in_sep);
	rc.set_output_sep(out_sep);
	rc.set_map_sep(map_sep);

	rc.set_read_q_fail_tag(fail_tag);
	rc.set_read_unknown_tag(unk_tag);
	rc.set_idx_undef_tag(undef_tag);

	if (in && string(in).find(".gz") != string::npos) { in_z = true; }
	
	if (raw_stats != NULL) {
		rc.set_stats(raw_stats);
		rc.set_with_raw_stats(true);
	}

	if (!quiet) { rc.print_params(); }

	rc.count(in_z);
	
	if (out_mode == RAW) { rc.write_raw_counts(); }
	if (out_mode == TABLE) { rc.write_table(); }
	if (raw_stats != NULL) { rc.write_raw_stats(); }

	return 0;
}
