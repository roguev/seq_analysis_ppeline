#include <string>
#include <vector>
#include <getopt.h>
#include "run_stats.h"
#include "get_run_stats.h"
#include "utils.h"
using namespace std;
using namespace utils;

int main(int argc, char** argv) {

	char* in_file = NULL;
	char* out_file = NULL;

	uint8_t mode;

	uint16_t xbin = 500;
	uint16_t ybin = 500;

	vector<uint16_t> tiles;
	vector<uint8_t> lanes;
	vector<uint8_t> reads;

	bool quiet = false;

	vector<string> stats;

	bool z_in =	false;

	uint8_t thr = 	4;
	uint32_t load = 25000;

	string out_sep = "\t";

	int opt = 0;
	
	while (1) {
		int long_index = 0;
		opt = getopt_long(argc, argv, "i::o::s::x::y::R::L::T::O::t::f::qh", long_options, &long_index);
		if (opt == -1) {
			break;
		}

		switch(opt) {
			case 'i' 	: in_file = optarg;					break;
			case 'o' 	: out_file = optarg;					break;

			case 's' 	: stats.push_back(string(optarg));			break;
			case 'x' 	: xbin = static_cast<uint16_t>(atoi(optarg));		break;
			case 'y' 	: ybin = static_cast<uint16_t>(atoi(optarg));		break;

			case 'R' 	: reads.push_back(static_cast<uint8_t>(atoi(optarg)));	break;
			case 'L' 	: lanes.push_back(static_cast<uint8_t>(atoi(optarg)));	break;
			case 'T' 	: tiles.push_back(static_cast<uint16_t>(atoi(optarg)));	break;

			case 'O'	: out_sep = string(optarg); 				break;

			case 't'	: thr = static_cast<uint8_t>(atoi(optarg));		break;
			case 'f'	: load = static_cast<uint32_t>(atoi(optarg));		break;
			
			case 'q'	: quiet = true;						break;

			case '?'	: cout << gs_usage << endl; 	exit(GRSEC_BAD_COMMAND_LINE);
			case 'h'	: cout << gs_usage << endl; 	exit(GRSEC_BAD_COMMAND_LINE);
			default		: cout << gs_usage << endl; 	exit(GRSEC_BAD_COMMAND_LINE);
		}
	}

	if (xbin == 0) {
		report_error(__FILE__,__func__, GRS_BAD_BIN_SIZE);
		exit(GRSEC_BAD_COMMAND_LINE);
	}

	if (ybin == 0) {
		report_error(__FILE__,__func__, GRS_BAD_BIN_SIZE);
		exit(GRSEC_BAD_COMMAND_LINE);
	}

	if (in_file && (string(in_file).find(".gz") != string::npos)) {
		z_in = true;
	}

	// initialize the object
	Run_stats rs(in_file, out_file, xbin, ybin);
	if (stats.empty()) {
		rs.add_stat(STAT::CLUST);
	} else {
		for (size_t s = 0; s < stats.size(); ++ s) {
			if (stats.at(s).compare("clust") == 0) { rs.add_stat(STAT::CLUST); }
			if (stats.at(s).compare("len") == 0) { rs.add_stat(STAT::LEN); }
			if (stats.at(s).compare("qual") == 0) { rs.add_stat(STAT::QUAL); }
			if (stats.at(s).compare("A") == 0) { rs.add_stat(STAT::SEQ, 'A'); }
			if (stats.at(s).compare("T") == 0) { rs.add_stat(STAT::SEQ, 'T'); }
			if (stats.at(s).compare("G") == 0) { rs.add_stat(STAT::SEQ, 'G'); }
			if (stats.at(s).compare("C") == 0) { rs.add_stat(STAT::SEQ, 'C'); }
			if (stats.at(s).compare("N") == 0) { rs.add_stat(STAT::SEQ, 'N'); }
		} // for
	} // if

	// select lanes to collect
	for (size_t l = 0; l < lanes.size(); ++l) {
		rs.collect_lane(lanes.at(l));
	}

	// select tiles to collect
	for (size_t t = 0; t < tiles.size(); ++t) {
		rs.collect_tile(tiles.at(t));
	}

	for (size_t r = 0; r < reads.size(); ++r) {
		rs.collect_read(reads.at(r));
	}

	// set parameters
	rs.set_output_sep(out_sep);
	rs.set_load_factor(load);
	rs.set_n_threads(thr);

	if (!quiet) { rs.print_params(); }

	// collect data
	rs.collect_stats(z_in);

	// output
	rs.output_stats();

	if (!quiet) { rs.print_collected(); }

	return 0;
}
