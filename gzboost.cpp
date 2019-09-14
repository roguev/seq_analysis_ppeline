#include "gzboost.h"

/* compresses a string
 * arguments:
 * 	a reference to a string to be compressed
 * 	*/
std::string compress_string(const std::string& data) {
	namespace bio = boost::iostreams;

	std::stringstream compressed;
	std::stringstream origin(data);

	bio::filtering_streambuf<bio::input> out;

// the following 2 lines are commented out but give flexibility about compression parameters 
//		out.push(bio::gzip_compressor(bio::gzip_params(bio::gzip::best_compression)));
//		out.push(bio::gzip_compressor(bio::gzip_params(bio::gzip::best_speed)));
	out.push(bio::gzip_compressor());
	out.push(origin);
	bio::copy(out, compressed);

	return compressed.str();
}

/* decompresses a string
 * arguments:
 * 	a reference to a string to be decompressed
 * 	*/
std::string decompress_string(const std::string& data) {
	namespace bio = boost::iostreams;

	std::stringstream compressed(data);
	std::stringstream decompressed;

	bio::filtering_streambuf<bio::input> out;
	out.push(bio::gzip_decompressor());
	out.push(compressed);
	bio::copy(out, decompressed);

	return decompressed.str();
}
