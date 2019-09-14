#ifndef __SEQ_STATS_H__
#define __SEQ_STATS_H__

#include <string>
#include <vector>
#include <boost/thread.hpp>
#include "fastq_seq.h"

using namespace std;

// a bitmask used to decide which mode to use
enum class SS_OUTPUT_MODE : uint8_t {
	SEQ_COUNT	= 1,	// bit 1 set
	NT_FREQUENCY	= 2,	// bit 2 set
	NT_QUALITY 	= 4,	// bit 3 set
	LENGTH_DISTR	= 8,	// bit 4 set
	FULL_STATS	= 16	// bit 5 set
};

class Seq_stats {
	public:
		// cosntructors
		Seq_stats(): infile(NULL) {}
		Seq_stats(char* in_fn, char* out_fn, uint16_t read_length);

		// destructor
		virtual ~Seq_stats();
	
		// setters
		void set_read_length(uint16_t i);
		void set_output_sep(const string sep);
		void set_input_sep(const string sep);
		void set_load_factor(uint32_t i);
		void set_n_threads(uint8_t i);
		void set_raw_input(bool r);

		// main stat collection functions
		string get_nt_frequency_stats();
		string get_nt_quality_stats();
		string get_len_distribution_stats();
		string get_counts();
		string get_full_stats();

		// general purpose reporting
		void print_params();

		// output stats to an output stream
		void output_stats(SS_OUTPUT_MODE mode);

		// the main function of the class, call this from
		// within your program
		void collect_stats(bool z_in);

	private:
		// for multithreading
		boost::mutex mtx;

		// file input and output
		char* infile;
		char* outfile;
	
		// field separators
		string OUTPUT_SEP;	// output
		string INPUT_SEP;	// input

		// updated during a run
		uint32_t total_seqs;
		uint32_t valid_seqs;

		// allows processing of the raw output from the read extractor module
		bool raw_input;

		// base composition stats
		vector<uint32_t> A;
		vector<uint32_t> T;
		vector<uint32_t> C;
		vector<uint32_t> G;
		vector<uint32_t> N;
		vector<uint32_t> pos_counts;

		// length distribution
		vector<uint32_t> L;

		// per base quality
		vector<uint64_t> Q;

		// general performance options
		uint8_t n_threads;
		uint32_t load_factor;

		// length of the sequence read in this run
		uint16_t read_length;

		// pre-allocate space for different stats
		void init_containers();
		
		// utility function for convertng the output of the read_extractor module
		// to FASTQ sequence format
		void raw2fastq(string& raw, Fastq_seq& fq);
	
		// scales the values of a vector given a denominator
		template<class T1, class T2, class T3>
			vector<T3> scale_vector(T1&, T2&);
		
		// process a sequence file
		template<class T1>
			void process_seqs(T1& in);
};
#endif  //__SEQ_STATS_H__
