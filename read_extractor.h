#ifndef __READ_EXTRACTOR_H__
#define __READ_EXTRACTOR_H__

#include <string>
#include <iostream>
#include <fstream>
#include <pcrecpp.h>
#include <boost/thread.hpp>
#include <cstdint>
using namespace std;

static string RE_BAD_INDEX =	"Bad index";

enum RE_ERRORS {
	REEC_ROI_BAD_INDEX 	=	1,
	REEC_SPACER_BAD_INDEX = 	2
};

class Read_extractor {
	public:
		Read_extractor(): infile(NULL), outfile(NULL), rejected(NULL){}
		Read_extractor(
				char* in_fn, 
				const string& p_l, 
				const string& p_r, 
				const vector<uint16_t>& mins, 
				const vector<uint16_t>& maxs,
				const vector<string>& ss,
				char* outfile,
				char* rejected
				);

		virtual ~Read_extractor();

		void set_pat_l(const string& s);
		void set_pat_r(const string& s);
		void set_mm_l(bool m);
		void set_mm_r(bool m);
		void set_mm_s(bool m);
	
		void set_with_valid(bool v);
		void set_with_rejected(bool r);
		void set_fastq_out(bool f);

		void set_roi_mins(vector<uint16_t> i);
		void set_roi_min(uint16_t i, size_t p);

		void set_roi_maxs(vector<uint16_t> i);
		void set_roi_max(uint16_t i, size_t p);

		void set_spacer(const string& s, size_t p);
		void set_spacers(const vector<string>& ss);
	
		void set_load_factor(uint32_t i);
		void set_n_threads(uint8_t i);

		void extract(bool z_in, bool z_out, bool z_rej);
		void print_params();

		void set_output_sep(const string& sep);

	private:
		boost::mutex mtx;

		char* infile;
		char* outfile;
		char* rejected;
		
		string pat_l;
		string pat_r;
		string re_str;
		string OUTPUT_SEP;

		bool mm_l;
		bool mm_r;
		bool mm_s;

		bool fastq_out;

		bool with_rejected;
		bool with_valid;

		vector<uint16_t> roi_min;
		vector<uint16_t> roi_max;
		vector<string> spacers;

		uint8_t n_threads;
		uint32_t load_factor;
	
		template<class T1>
			void extract_seq_reads(
				T1& in,
				ofstream& out,
				ofstream& rej,
				pcrecpp::RE& re, 
				uint32_t load_factor,
				bool z_out,
				bool z_rej);
};
#endif //__READ_EXTRACTOR_H__
