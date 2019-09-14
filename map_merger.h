#ifndef __MAP_MERGER_H__
#define __MAP_MERGER_H__

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include "utils.h"
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
using namespace std;

typedef boost::unordered::unordered_map<string, vector<string> > umsvs;
typedef boost::unordered::unordered_set<string> uss;
typedef boost::unordered::unordered_set<string>::iterator uss_it;

class Map_merger {
	public:
		Map_merger(): R1fn(NULL), R2fn(NULL), outfile(NULL), unpR1(NULL), unpR2(NULL) {}
		Map_merger(char* i1, char* i2, char* out);  
		Map_merger(char* i1, char* i2, char* out, uint32_t mlines);  
		Map_merger(char* i1, char* i2, char* o1, char* o2);  
		virtual ~Map_merger();
		void merge_id_maps(bool in_z, bool out_z);
		void get_unpaired_reads(bool in_z);
		void set_max_lines(uint32_t mlines);
		void set_n_threads(uint8_t nthr);
		void set_load_factor(uint32_t i);
		void set_input_sep(const string& sep);
		void set_output_sep(const string& sep);
		void print_params();

	private:
		char* R1fn;
		char* R2fn;
		char* outfile;
		char* unpR1;
		char* unpR2;
		
		boost::mutex mtx;

		string INPUT_SEP;
		string OUTPUT_SEP;

		uint32_t lines_read;
		uint32_t max_lines;
		uint8_t n_threads;
		uint32_t load_factor;

		umsvs* R1_hash;

		uss* unpaired_R1;
		uss* unpaired_R2;

		uss* R1_ids; 
		uss* R2_ids;

		void init_hashes();
		template<class T>
			void read_id_map(T& inR1);

		template<class T1>
			void match_reads(T1& inR2, ofstream& outf, bool z_out);

		template<class T>
			void get_ids(T& in, uss& s);

		template<class T1>
			void extract_reads(T1& in, uss& s, ofstream& out, bool z_out);

		void set_diff(uss& s1, uss& s2, uss& r);
};
#endif // __MAP_MERGER_H__
