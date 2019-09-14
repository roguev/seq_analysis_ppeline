#ifndef __RUN_STATS_H__
#define __RUN_STATS_H__

#include <vector>
#include <string>
#include <boost/thread.hpp>
#include "matrix.h"
#include <map>
#include <memory>
#include <sstream>
using namespace std;

/* These classes encapsulate a hierarchical data structure implemented as a nested map.
 * Thus the Run_stats class contains a map of instances of the Read class. The Read class
 * contains a map of instances of the Lane class. The Lane class class contains a map of 
 * instances of the Tile class which contains a map of instances of the View class.
 * e.g.
 * 	Run_stats-> Read-> Lane-> Tile-> View
 *
 * The View class contains a dynamic 2D data structure (Matrix) of doubles, implemented
 * in the header-only library matrix.h
 * */

enum class STAT : int {
	CLUST		= 1,
	QUAL		= 2,
	LEN		= 3,
	SEQ		= 4
};

class View {
	public:
		View(STAT _kind, char _NT) : kind(_kind), NT(_NT), data(new Matrix<double>()) {};
		View(STAT _kind) : kind(_kind), NT(0), data(new Matrix<double>()) {};

		// copy constructor
		View(View& a) {
			kind = a.kind;
			NT = a.NT;
			data = new Matrix<double>;
			*data = *(a.data);
		}

		//destructor
		virtual ~View() { delete(data); }
		Matrix<double>* _data() const { return data; }
		STAT _kind() const { return kind; }
		char _NT() const { return NT; }
	
	private:
		// 2D dynamic data container
		Matrix<double>* data;

		STAT kind;
		char NT;
};

/* Tile class */
typedef std::map<uint16_t, View*> tile_t;
class Tile {
	public:
		// constructor
		Tile() : data(new tile_t) {};

		// copy constructor
		Tile(const Tile& a) {
			data = new tile_t;
			*(data) = *(a.data);
		}

		Tile(const Tile* a) {
			data = new tile_t;
			*data = *(a->data);
		}

		// destructor
		virtual ~Tile() { delete(data); }

		// accessors
		View* add_view(STAT kind, char NT);
		View* add_view(STAT kind); 
		View* get_view(uint16_t n) const;

		bool empty() const { return data->empty(); } 

		vector<uint16_t> view_ids() const;

		tile_t* _data() const { return data; }

	private:
		// a map of View*
		tile_t* data;	
};

/* Lane class */
typedef std::map<uint16_t, Tile*> lane_t;
class Lane {
	public:
		// constructor
		Lane() : data(new lane_t) {};

		// copy constructor
		Lane(const Lane& a) {
			data = new lane_t;
			*(data) = *(a.data);
		}

		Lane(const Lane* a) {
			data = new lane_t;
			*(data) = *(a->data);
		}

		// destructor
		virtual ~Lane() { delete(data); }

		// accessors
		Tile* add_tile(uint16_t n);
		Tile* get_tile(uint16_t n) const;
		bool empty() const { return data->empty(); }

		vector<uint16_t> tile_ids() const;
		
		lane_t* _data() const { return data; }

	private:
		// a map of Tile*
		lane_t* data;
};

/* Read class */
typedef std::map<uint8_t, Lane*> read_t;
class Read {
	public:
		// constructor
		Read() : data(new read_t) {};

		// copy constructor
		Read(const Read& a) {
			data = new read_t;
			*(data) = *(a.data);
		}

		Read(const Read* a) {
			data = new read_t;
			*(data) = *(a->data);
		}

		// destructor
		virtual ~Read() { delete(data); }

		// accessors
		Lane* add_lane(uint8_t n);
		Lane* get_lane(uint8_t n) const;
		bool empty() const { return data->empty(); }

		vector<uint8_t> lane_ids() const;
		
		read_t* _data() const { return data; }

	private:
		// a map of Lane*
		read_t* data;
};

/* Main class */
typedef std::map<uint8_t, Read*> run_t;
typedef std::vector<std::pair<uint8_t, char>> stat_t;
class Run_stats {
	public:
		// constructors
		Run_stats() : data(new run_t) {};
		Run_stats(char* _in, char* _out, uint16_t _xbin, uint16_t _ybin) :
			infile(_in),
			outfile(_out),
			xbin_size(_xbin),
			ybin_size(_ybin),
			OUTPUT_SEP("\t"),
			n_threads(4),
			load_factor(100000),
			data(new run_t) {}

		// destructor
		virtual ~Run_stats() { delete(data); }

		// add desired views
		void add_stat(STAT kind, char NT);
		void add_stat(STAT kind);

		// get desired views
		vector<STAT>& get_stats() const;

		// getters and setters for data manipulation
		// read accessors
		Read* add_read(uint8_t r);
		Read* get_read(uint8_t r) const;

		// lane accessors
		Lane* add_lane(uint8_t r, uint8_t l);
		Lane* get_lane(uint8_t r, uint8_t l) const;

		// tile accessors
		Tile* add_tile(uint8_t r, uint8_t l, uint16_t t);
		Tile* get_tile(uint8_t r, uint8_t l, uint16_t t) const;

		// view accessors
		View* add_view(uint8_t r, uint8_t l, uint16_t t, STAT k, char NT);
		View* add_view(uint8_t r, uint8_t l, uint16_t t, STAT k);
		
		View* get_view(uint8_t r, uint8_t l, uint16_t t, uint16_t v) const;

		vector<uint8_t> read_ids() const;

		const stat_t& _stats() const ;
		run_t* _data() const { return data; }

		// accessors for desired reads, lanes and tiles
		const vector<uint8_t>& _collect_reads() const { return collect_reads; }
		const vector<uint8_t>& _collect_lanes() const { return collect_lanes; }
		const vector<uint16_t>& _collect_tiles() const { return collect_tiles; }
	
		// setters for the binning parameters
		void set_xbin_size(uint16_t i);
		void set_ybin_size(uint16_t i);

		// add to the desired features (reads, lanes and tiles)
		void collect_read(uint8_t r) { collect_reads.push_back(r); }
		void collect_lane(uint8_t l) { collect_lanes.push_back(l); }
		void collect_tile(uint16_t t) { collect_tiles.push_back(t); }

		void add_collected(const string c, View* v) { 
			collected.insert(
					pair<string, pair<double,double>>
					(c, pair<double, double>(v->_data()->get_min(), v->_data()->get_max()))
					);
		}

		// cosmetics
		void print_params() const;
		void print_collected() const;

		void set_output_sep(const std::string& sep);

		void set_load_factor(uint32_t i);
		void set_n_threads(uint8_t i);

		void collect_stats(bool z_in);
		void output_stats();

	private:
		boost::mutex mtx;
		
		// holds data of what is to be collected
		vector<uint16_t> collect_tiles;
		vector<uint8_t> collect_lanes;
		vector<uint8_t> collect_reads;

		map<string, pair<double, double>> collected;

		// which views
		stat_t stats;

		// main data structure (a map or Read*)
		run_t* data;

		char* infile;
		char* outfile;
		std::string OUTPUT_SEP;
		
		uint16_t xbin_size;
		uint16_t ybin_size;

		uint32_t load_factor;
		uint8_t n_threads;

		template  <class T1>
			void process_seqs(T1& in);
};

/*----------------------------------------------------*/
// namespace run_stats
namespace run_stats {
	// add Read
	Read* _add_read(run_t* r, uint8_t i);

	// add Lane
	Lane* _add_lane(read_t* r, uint8_t i);

	// add Tile
	Tile* _add_tile(lane_t* l, uint16_t i);

	// add View
	View* _add_view(tile_t* t, STAT kind, char NT);

	// add View
	View* _add_view(tile_t* t, STAT kind);

	// get keys of a map
	template<class T1, class T2>
	vector<T1> get_keys(std::map<T1, T2>& m) {
		vector<T1> res;
		for (typename std::map<T1, T2>::iterator it = m.begin(); it != m.end(); ++it) {
			res.push_back(it->first);
		} // for
		return res;
	}

	// get a value of a map at a key
	template <class T1, class T2>
	T2 get_value(std::map<T1, T2>& m, T1 k) {
		if (m.find(k) == m.end()) { return NULL; }
		return m.at(k);
	}

	// anything to string
	template <class T>
	string to_string(T& a) {
		stringstream ss;
		ss << a;
		return ss.str();
	}
}; // namespace

#endif	// _RUN_STATS_H__
