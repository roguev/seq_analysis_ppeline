#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include "utils.h"
#include <stdlib.h>

enum MATRIX_ERRORS {
	EC_INCONSISTENT_DIMENSIONS	=	1,
	EC_WRONG_VECTOR			=	2,
	EC_WRONG_DIMENSIONS		=	3,
	EC_WRONG_ROW_COLUMN_INDEX	=	4
};

static std::string MSG_INCONSISTENT_DIMENSIONS	= 	"Inconsistent matrix dimensions";
static std::string MSG_WRONG_VECTOR		=	"Incorrect number of elements in vecotr";
static std::string MSG_WRONG_DIMENSIONS		=	"Wrong dimensions";
static std::string MSG_WRONG_ROW_COLUMN_INDEX	=	"Wrong row/column index";

template <class T>
class Matrix {
	public:
		// constructors
		Matrix();
		Matrix(size_t capacity);
		Matrix(size_t vr, size_t vc);
		Matrix(std::vector<T> vdata, size_t vr, size_t vc);
		Matrix(std::vector<T> vdata);

		// copy constructor
		Matrix(const Matrix<T>& a);

		virtual ~Matrix();

		// operator overloads
		// =
		Matrix& operator=(const Matrix& a) {
			if (this != &a) {
				r = a.r;
				c = a.c;
				data.clear();
				for (size_t i = 0; i < r*c; ++i) {
					data.push_back(a.data.at(i));
				}
			}
			return *this;
		}

		
		// +=
		void operator+=(const Matrix& a) {
			if (r == a.r && c == a.c) {
				for (size_t i = 0; i < a.r*a.c; ++i) { data.at(i) += a.data.at(i); }
			} else {
				utils::report_error(__FILE__,__func__,MSG_INCONSISTENT_DIMENSIONS);
				exit(EC_INCONSISTENT_DIMENSIONS);
			}
		}

		// -=
		void operator-=(const Matrix& a) {
			if (r == a.r && c == a.c) {
				for (size_t i = 0; i < a.r*a.c; ++i) { data.at(i) -= a.data.at(i); }
			} else {
				utils::report_error(__FILE__,__func__,MSG_INCONSISTENT_DIMENSIONS);
				exit(EC_INCONSISTENT_DIMENSIONS);
			}

		}

		// *=
		void operator*=(const Matrix& a) {
			if (r == a.r && c == a.c) {
				for (size_t i = 0; i < a.r*a.c; ++i) { data.at(i) *= a.data.at(i); }
			} else {
				utils::report_error(__FILE__,__func__, MSG_INCONSISTENT_DIMENSIONS);
				exit(EC_INCONSISTENT_DIMENSIONS);
			}			
		}

		// /=
		void operator/=(const Matrix& a) {
			if (r == a.r && c == a.c) {
				for (size_t i = 0; i < a.r*a.c; ++i) { data.at(i) /= a.data.at(i); }
			} else {
				utils::report_error(__FILE__,__func__, MSG_INCONSISTENT_DIMENSIONS);
				exit(EC_INCONSISTENT_DIMENSIONS);
			}
		}

		// +
		friend Matrix operator+(const Matrix& a, const Matrix& b) {
			Matrix c;
			if (a.r == b.r && a.c == b.c) {
				c = a;
				for (size_t i = 0; i < b.data.size(); ++i) { c.data.at(i) += b.data.at(i); }
			} else {
				utils::report_error(__FILE__,__func__, MSG_INCONSISTENT_DIMENSIONS);
				exit(EC_INCONSISTENT_DIMENSIONS);
			}
			return c;
		}
		
		// -
		friend Matrix operator-(const Matrix& a, const Matrix& b) {
			Matrix c;
			if (a.r == b.r && a.c == b.c) {
				c = a;
				for (size_t i = 0; i < b.data.size(); ++i) { c.data.at(i) -= b.data.at(i); }	
			} else {
				utils::report_error(__FILE__,__func__, MSG_INCONSISTENT_DIMENSIONS);
				exit(EC_INCONSISTENT_DIMENSIONS);
			}
			return c;
		}

		// *
		friend Matrix operator*(const Matrix& a, const Matrix& b) {
			Matrix c;
			if (a.r == b.r && a.c == b.c) {
				c = a;
				for (size_t i = 0; i < b.data.size(); ++i) { c.data.at(i) *= b.data.at(i); }
			} else {
				utils::report_error(__FILE__,__func__, MSG_INCONSISTENT_DIMENSIONS);
				exit(EC_INCONSISTENT_DIMENSIONS);
			}
			return c;
		}
		
		// /
		friend Matrix operator/(const Matrix& a, const Matrix& b) {
			Matrix c;
			if (a.r == b.r && a.c == b.c) {
				c = a;
				for (size_t i = 0; i < b.data.size(); ++i) { c.data.at(i) /= b.data.at(i); }
			} else {
				utils::report_error(__FILE__,__func__, MSG_INCONSISTENT_DIMENSIONS);
				exit(EC_INCONSISTENT_DIMENSIONS);
			}
			return c;
		}

		// ==
		friend bool operator==(const Matrix& a, const Matrix& b) {
			if (a.r == b.r && a.c == b.c) {
				for (size_t i = 0; i < b.data.size(); ++i) {
					if (a.data.at(i) != b.data.at(i)) {
						return false;
					}
				}
			} else {
				return false;
			}
			return true;
		}

		// !=
		friend bool operator!=(const Matrix& a, const Matrix& b) {
			return !(a == b);
		}

		// public fields
		size_t r;
		size_t c;

		void reshape(size_t vr, size_t vc);

		// add
		void add_row(size_t vr, T elem);
		void add_row(size_t vr, std::vector<T>& v);

		void add_col(size_t vc, T elem);
		void add_col(size_t vc, std::vector<T>& v);

		void add_rows(size_t vr, size_t n, T elem);
		void add_cols(size_t vc, size_t n, T elem);

		// delete
		void del_row(size_t vr);
		void del_col(size_t vc);

		void del_rows(size_t vr, size_t n);
		void del_cols(size_t vc, size_t n);

		// append
		void append_row(T elem);
		void append_col(T elem);

		void append_rows(size_t n, T elem);
		void append_cols(size_t n, T elem);

		// prepend
		void prepend_row(T elem);
		void prepend_col(T elem);

		void prepend_rows(size_t n, T elem);
		void prepend_cols(size_t n, T elem);

		// concatenate
		void cat_h(const Matrix<T>& m1);
		void cat_v(const Matrix<T>& m1);

		T& at(size_t vr, size_t vc);
		
		std::vector<T> get_row(size_t vr) const;
		std::vector<T> get_col(size_t vc) const;
		
		void print(std::string& delim) const;
		void print() const;

		std::string to_string(std::string& delim) const;
		std::string to_string() const;

		std::string row_to_string(size_t vr, std::string& delim) const;
		std::string row_to_string(size_t vr) const;

		std::string col_to_string(size_t vc, std::string& delim) const;
		std::string col_to_string(size_t vc) const;

		size_t size() const;
		std::vector<T>& get_data() const;

		T& get_min();
		T& get_max();

		// arithmetics using a constant 
		void plus(T c);
		void minus(T c);
		void multiply(T c);
		void divide(T c);

	private:
		std::vector<T> data;
		size_t linear_index(size_t vr, size_t vc) const;
};

// implementation
/* constructors */
// empty
template<class T>
Matrix<T>::Matrix() {
	r = 0;
	c = 0;
}

// pre-allocate space
template <class T>
Matrix<T>::Matrix(size_t capacity) {
	r = 0;
	c = 0;
	data.reserve(capacity);
}

// size r x c
template <class T>
Matrix<T>::Matrix(size_t vr, size_t vc) {
	data.resize(vr*vc);
	r = vr;
	c = vc;
}

// from vector
template <class T>
Matrix<T>::Matrix(std::vector<T> vdata, size_t vr, size_t vc) {
	data = vdata;
	data.resize(vr*vc, 0);
	r = vr;
	c = vc;
}

// copy constructor
template <class T>
Matrix<T>::Matrix(const Matrix<T>& a) {
	r = a.r;
	c = a.c;
	data.clear();
	for (size_t i = 0; i < a.r*a.c; ++i) {
		data.push_back(a.data.at(i));
	}
}

/* destructor */
template <class T> 
Matrix<T>::~Matrix() { 
	data.clear();
}

// arithmetics using a cosntant
template <class T>
void Matrix<T>::plus(T c) {
	for (size_t i = 0; i < data.size(); ++i) {
		data.at(i) += c;
	}
}

template <class T>
void Matrix<T>::minus(T c) {
	for (size_t i = 0; i < data.size(); ++i) {
		data.at(i) -= c;
	}
}

template <class T>
void Matrix<T>::multiply(T c) {
	for (size_t i = 0; i < data.size(); ++i) {
		data.at(i) *= c;
	}
}

template <class T>
void Matrix<T>::divide(T c) {
	for (size_t i = 0; i < data.size(); ++i) {
		data.at(i) /= c;
	}
}


// find minimum
template <class T>
T& Matrix<T>::get_min() {
	return *min_element(data.begin(),data.end());
}

// find maximum
template <class T>
T& Matrix<T>::get_max() {
	return *max_element(data.begin(),data.end());
}


/* matrix size - number of elements */
template <class T>
size_t Matrix<T>::size() const {
	return r*c;
}

/* get container data */
template <class T>
std::vector<T>& Matrix<T>::get_data() const {
	return data;
}

/* get linear index from r x c coordinates */
template <class T>
size_t Matrix<T>::linear_index(size_t vr, size_t vc) const {
	return vr*c + vc;
}

/* creates consisten matirces with the same number of rows and comumns */
template <class T>
void make_consistent(Matrix<T>& a, Matrix<T>& b, T elem) {
	size_t r = (a.r > b.r) ? a.r : b.r;
	size_t c = (a.c > b.c) ? a.c : b.c;

	a.append_rows(r - a.r, elem); 
	a.append_cols(c - a.c, elem);

	b.append_rows(r - b.r, elem); 
	b.append_cols(c - b.c, elem);
}

template <class T>
void make_consistent(Matrix<T>* a, Matrix<T>* b, T elem) {
	size_t r = (a->r > b->r) ? a->r : b->r;
	size_t c = (a->c > b->c) ? a->c : b->c;

	a->append_rows(r - a->r, elem); 
	a->append_cols(c - a->c, elem);

	b->append_rows(r - b->r, elem); 
	b->append_cols(c - b->c, elem);
}
/* change the shape */
template <class T>
void Matrix<T>::reshape(size_t vr, size_t vc) {
	if (vr*vc != data.size()) {
		utils::report_error(__FILE__, __func__, MSG_WRONG_DIMENSIONS);
		exit(EC_WRONG_DIMENSIONS);
	} else {
		r = vr;
		c = vc;
	}
}

/* get / set element at a pareticular position */
template <class T>
T& Matrix<T>::at(size_t vr, size_t vc) {
	return data.at(linear_index(vr,vc));
}

/* add a row */
template <class T>
void Matrix<T>::add_row(size_t vr, T elem) {
	typename std::vector<T>::iterator it;
	// handle addition to the bottom of the matrix
	if (vr >= r) { vr = r; }
	for (size_t i = 0; i < c; ++i) {
		// reset the iterator cuz we are modifying the container
		it = data.begin();
		data.insert(it + linear_index(vr,i), elem);
	}
	r++;
}

template <class T>
void Matrix<T>::add_row(size_t vr, std::vector<T>& v) {
	typename std::vector<T>::iterator it;
	// handle addition to the bottom of the matrix
	if (vr >= r) { vr = r; }

	if (v.size() != c) {
		utils::report_error(__FILE__, __func__, MSG_WRONG_VECTOR);
		exit(EC_WRONG_VECTOR);
	}

	for (size_t i = 0; i < c; ++i) {
		// reset the iterator cuz we are modifying the container
		it = data.begin();
		data.insert(it + linear_index(vr,i), v.at(i));
	}
	r++;
}

/* add a column */
template <class T>
void Matrix<T>::add_col(size_t vc, T elem) {
	typename std::vector<T>::iterator it;
	// handle addition to the end of the matrix
	if (vc >= c) { vc = c; }
	for (size_t i = 0; i < r; ++i) {
		// reset the iterator cuz we are modifying the container
		it = data.begin();
		data.insert(it + linear_index(i,vc) + i, elem);
	}
	c++;
}

template <class T>
void Matrix<T>::add_col(size_t vc, std::vector<T>& v) {
	typename std::vector<T>::iterator it;
	// handle addition to the bottom of the matrix
	if (vc >= c) { vc = c; }
	
	if (v.size() != r) {
		utils::report_error(__FILE__, __func__, MSG_WRONG_VECTOR);
		exit(EC_WRONG_VECTOR);
	}

	for (size_t i = 0; i < r; ++i) {
		// reset the iterator cuz we are modifying the container
		it = data.begin();
		data.insert(it + linear_index(i,vc) + i, v.at(i));
	}
	c++;
}

/* delete a row */
template <class T>
void Matrix<T>::del_row(size_t vr) {
	typename std::vector<T>::iterator it;
	// handle non-existent rows
	if (vr >= r) {
		utils::report_error(__FILE__, __func__, MSG_WRONG_ROW_COLUMN_INDEX);
		exit(EC_WRONG_ROW_COLUMN_INDEX);
	}

	for (size_t i = 0; i < c; ++i) {
		// reset the iterator cuz we are modifying the container
		it = data.begin();
		data.erase(it + linear_index(vr, i) - i);
	}
	r--;
}

/* delete a column */
template <class T>
void Matrix<T>::del_col(size_t vc) {
	typename std::vector<T>::iterator it;
	// handle non-existent cols
	if (vc >= c) {
		utils::report_error(__FILE__, __func__, MSG_WRONG_ROW_COLUMN_INDEX);
		exit(EC_WRONG_ROW_COLUMN_INDEX);
	}

	for (size_t i = 0; i < r; ++i) {
		// reset the iterator cuz we are modifying the container
		it = data.begin();
		data.erase(it + linear_index(i, vc) - i);
	}
	c--;
}

/* delete multiple rows */
template <class T>
void Matrix<T>::del_rows(size_t vr, size_t n) {
	for (size_t i = 0; i < n; ++i) {
		del_row(vr);
	}
}

/* delete multiple cols */
template <class T>
void Matrix<T>::del_cols(size_t vc, size_t n) {
	for (size_t i = 0; i < n; ++i) {
		del_col(vc);
	}
}

/* add multiple rows */
template <class T>
void Matrix<T>::add_rows(size_t vr, size_t n, T elem) {
	for (size_t i = 0; i < n; ++i) {
		add_row(vr, elem);
	}
}

/* add multiple columns */
template <class T>
void Matrix<T>::add_cols(size_t vc, size_t n, T elem) {
	for (size_t i = 0; i < n; ++i) {
		add_col(vc, elem);
	}
}

/* append a row */
template <class T>
void Matrix<T>::append_row(T elem) { add_row(r, elem); }

/* append a column */
template <class T>
void Matrix<T>::append_col(T elem) { add_col(c, elem); }

/* append multiple rows */
template <class T>
void Matrix<T>::append_rows(size_t n, T elem) { add_rows(r, n, elem); }

/* append multiple cols */
template <class T>
void Matrix<T>::append_cols(size_t n, T elem) { add_cols(c ,n, elem); }

/* prepend a row */
template <class T>
void Matrix<T>::prepend_row(T elem) { add_row(0, elem); }

/* prepend a column */
template <class T>
void Matrix<T>::prepend_col(T elem) { add_col(0, elem); }

/* prepend multiple rows */
template <class T>
void Matrix<T>::prepend_rows(size_t n, T elem) { add_rows(0, n, elem); }

/* prepend multiple columns */
template <class T>
void Matrix<T>::prepend_cols(size_t n,T elem) { add_cols(0, n, elem); }

/* get a row as a vector */
template <class T>
std::vector<T> Matrix<T>::get_row(size_t vr) const {
	std::vector<T> res;
	if (vr < r) {
		for (size_t i = 0; i < c; ++i) { res.push_back(data.at(linear_index(vr, i))); }
	} else {
		utils::report_error(__FILE__, __func__, MSG_WRONG_ROW_COLUMN_INDEX);
		exit(EC_WRONG_ROW_COLUMN_INDEX);
	}
	return res;
}

/* ger a columns as a vector */
template <class T>
std::vector<T> Matrix<T>::get_col(size_t vc) const {
	std::vector<T> res;
	if (vc < c) {
		for (size_t i = 0; i < r; ++i) { res.push_back(data.at(linear_index(i, vc))); }
	} else {
		utils::report_error(__FILE__, __func__, MSG_WRONG_ROW_COLUMN_INDEX);
		exit(EC_WRONG_ROW_COLUMN_INDEX);
	}
	return res;
}

/* concatenate matrices horizontaly */
template <class T>
void Matrix<T>::cat_h(const Matrix<T>& m1) {
	for (size_t i = 0; i < m1.c; ++i) {
		std::vector<T> to_add = m1.get_col(i);
		add_col(c, to_add);
	}
}

template <class T>
Matrix<T> cat_h(Matrix<T>& m1, Matrix<T>& m2) {
	Matrix<T> res = m1;
	res.cat_h(m2);
	return res;
}

/* concatenate matrices vertically */
template <class T>
void Matrix<T>::cat_v(const Matrix<T>& m1) {
	for (size_t i = 0; i < m1.r; ++i) {
		std::vector<T> to_add = m1.get_row(i);
		add_row(r, to_add);
	}
}

template <class T>
Matrix<T> cat_v(Matrix<T>& m1, Matrix<T>& m2) {
	Matrix<T> res = m1;
	res.cat_v(m2);
	return res;
}

/* print contents to cout */
// user specified delimiter
template <class T>
void Matrix<T>::print(std::string& delim) const {
	std::cout << to_string(delim) << std::endl;
}

// delimiter set to 'tab'
template <class T>
void Matrix<T>::print() const {
	std::cout << to_string() << std::endl;
}

/* string representation of the container */
// user specified delimiter
template <class T>
std::string Matrix<T>::to_string(std::string& delim) const {
	std::stringstream ss;
	for (size_t i = 0; i < r; ++i) {
		for (size_t j = 0; j < c-1; ++j) {
			ss << data.at(linear_index(i,j)) << delim;
		}
		ss << data.at(linear_index(i, c-1));
		ss<< "\n";
	}
	return ss.str();
}

// delimiter set to 'tab'
template <class T>
std::string Matrix<T>::to_string() const {
	std::string delim("\t");
	return to_string(delim);

}

// print individual rows and columns
// user specified delimiter
template <class T>
std::string Matrix<T>::row_to_string(size_t vr, std::string& delim) const {
	stringstream ss;
	std::vector<T> v = get_row(vr);
	for (size_t i = 0; i < v.size()-1; ++i) {
		ss << v.at(i) << delim;
	}
	ss << v.at(v.size() -1);
	ss << "\n";
	return ss.str();
}

// delimiter set to 'tab'
template <class T>
std::string Matrix<T>::row_to_string(size_t vr) const {
	std::string delim("\t");
	return row_to_string(vr, delim);
}

// user specified delimiter
template <class T>
std::string Matrix<T>::col_to_string(size_t vc, std::string& delim) const {
	stringstream ss;
	std::vector<T> v = get_col(vc);
	for (size_t i = 0; i < v.size()-1; ++i) {
		ss << v.at(i) << delim;
	}
	ss << v.at(v.size()-1);
	ss << "\n";
	return ss.str();
}

// delimiter set to 'tab'
template <class T>
std::string Matrix<T>::col_to_string(size_t vc) const {
	std::string delim("\t");
	return col_to_string(vc, delim);
}	
#endif // __MATRIX_H__
