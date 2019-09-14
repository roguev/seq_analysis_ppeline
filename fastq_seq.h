#ifndef __FASTQ_SEQ_H__
#define __FASTQ_SEQ_H__

#include <string>
#include <vector>
using namespace std;

typedef struct seq_header {
	string 		instrument;
	uint16_t 	run_number;
	string		flowcell_id;
	uint8_t		lane;
	uint16_t	tile;
	uint32_t	x;
	uint32_t	y;
	uint8_t		read_number;
	bool		filtered;
	uint16_t	control_number;
	string		index;
} SEQ_HEADER;

class Fastq_seq {
	public:
		Fastq_seq();
		virtual ~Fastq_seq();

		// getters
		const string& get_seq_id() const;
		const string& get_seq() const;
		const string& get_qual_str() const;
		
		// setters
		void set_seq_id(const string& s);
		void set_seq(const string& s);
		void set_qual_str(const string& s);
		
		// io
		bool read(istream& in);
		void write(ostream& out) const;
		void print() const;
		string to_string() const;
		
		// utility
		string trim_seq_id(vector<int>& trim) const;
		string get_index() const;
		string get_unique_id() const;
		SEQ_HEADER parse_seq_id() const;
	
	private:
		string seq_id;
		string seq;
		string q_score_id;
		string qual_str;
};
#endif //__FASTQ_SEQ_H__
