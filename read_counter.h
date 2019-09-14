#ifndef __READ_COUNTER_H__
#define __READ_COUNTER_H__

#include <string>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
using namespace std;

typedef boost::unordered::unordered_map<string, uint32_t> umsi;
typedef boost::unordered::unordered_map<string, uint32_t>::iterator umsi_it;

typedef boost::unordered::unordered_map<string, string> umss;
typedef boost::unordered::unordered_map<string, string>::iterator umss_it;

typedef boost::unordered::unordered_set<string> uss;
typedef boost::unordered::unordered_set<string>::iterator uss_it;

typedef boost::unordered::unordered_map< string, umsi> multi_hash;
typedef boost::unordered::unordered_map< string, umsi>::iterator mh_it;

enum ERRORS {
	RCEC_COLLAPSER_CORRUPT_RECORD		= 2,
	RCEC_COUNTER_CORRUPT_RECORD		= 3,
	RCEC_COUNTER_BAD_MAPPING		= 4,
	RCEC_BAD_INDEX				= 5,
	RCEC_CORRUPT_MAPPING			= 6
};

static string RC_BAD_INDEX 		= "Invalid index";
static string RC_CORRUPT_RECORD		= "Corrupt record found";
static string RC_BAD_MAPPING		= "Insufficient mapping info";
static string RC_CORRUPT_MAP		= "Corrupt mapping record";

class Read_counter {
	public:
		Read_counter(): infile(NULL), outfile(NULL), sample_map(NULL), stats_file(NULL) {}

		Read_counter(
			char* in,
			char* out,
			char* smap,
			const vector<char*>& ms
			);

		virtual ~Read_counter();
		void set_read_unknown_tag(const string& tag);
		void set_read_q_fail_tag(const string& tag);
		void set_idx_undef_tag(const string& tag);

		void set_record_separator(const string& s);

		void set_input_sep(const string& s);
		void set_output_sep(const string& s);
		void set_map_sep(const string& s);
		
		void set_n_threads(uint8_t n);
		void set_collapser_bite_size(uint32_t n);
		void set_counter_bite_size(uint32_t n);

		void set_mm(uint8_t n, uint8_t p);
		void set_mms(const vector<uint8_t>& mm);
		void set_index_mm(uint8_t n);
		
		void set_rc(bool i, uint8_t n);
		void set_rcs(const vector<bool>& rc);
		void set_idxrc(bool i);

		void set_with_fails(bool i);
		void set_with_unknowns(bool i);
		void set_with_undefs(bool i);

		void set_collapse_q_fails(bool i);

		void set_with_raw_stats(bool i);

		void add_map(char* map);
		void set_input(char* f);
		void set_output(char* f);
		void set_smap(char* f);

		void set_stats(char* f);

		void set_min_qual(uint8_t n);
		void set_max_lq_bases(uint8_t n);

		void print_params();
		void count(bool in_z);
		void write_raw_counts();
		void write_table();
		void write_raw_stats();

	private:
		boost::mutex collapse_mtx;
		boost::mutex counter_mtx;
		fstream in;

		uint8_t index_mm;
		bool idxrc;

		uint8_t min_qual;
		uint8_t max_lq_bases;
		
		char* infile;
		char* outfile;
		
		char* sample_map;
		char* stats_file;
	
		vector<char*> read_maps;
		vector <bool> rcs;
		vector <uint8_t> mms;

		uint8_t n_threads;
		uint32_t collapser_bite_size;
		uint32_t counter_bite_size;

		string READ_UNKNOWN_TAG;
		string READ_Q_FAIL_TAG;
		string IDX_UNDEF_TAG;
		string REC_SEPARATOR;

		string HASH_SEP;

		string INPUT_SEP;
		string OUTPUT_SEP;
		string MAP_SEP;

		bool with_fails;
		bool with_unknowns;
		bool with_undefs;

		bool collapse_q_fails;

		bool with_raw_stats;

		umsi* counts_hash;
		umsi* translated;

		umsi* stats_idx;
		vector<umsi*> stats_r;
 
		umss load_mapping(char* fn, bool rc);

		void set_defaults();

		void init_hashes();
		
		void primt_params();

		template<class T>
		void collapse_reads(
				T& in,
				umss& sample_map
				);
		
		void count_reads(vector<umss>& maps);

		string match_with_helper(
				string& seq,      
				vector<string>& lookup, 
				umss& mapping,
				umss& tmm,
				uint8_t mm,
				string& tag);
};
#endif //__READ_COUNTER_H__

