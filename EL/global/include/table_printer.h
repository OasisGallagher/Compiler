#pragma once

#include <iomanip>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>

/** \class TablePrinter

  Print a pretty table into your output of choice.

  Usage:
    TablePrinter tp(&std::cout);
    tp.AddColumn("Name", 25);
    tp.AddColumn("Age", 3);
    tp.AddColumn("Position", 30);

    tp.PrintHeader();
    tp << "Dat Chu" << 25 << "Research Assistant";
    tp << "John Doe" << 26 << "Professional Anonymity";
    tp << "Jane Doe" << tp.SkipToNextLine();
    tp << "Tom Doe" << 7 << "Student";
    tp.PrintFooter();

  \todo Add support for padding in each table cell
  */
class TablePrinter {
public:
	TablePrinter(const std::string & separator = "|");
	~TablePrinter();

	int get_num_columns() const;
	int get_table_width() const;
	void set_separator(const std::string & separator);

	void AddColumn(const std::string & header_name, int column_width);
	void AddHeader();
	void AddFooter();

	void ClearBuffer();

	std::string ToString() const;

	// Can we merge these?
	TablePrinter& operator<<(float input);
	TablePrinter& operator<<(double input);

	template<typename T> TablePrinter& operator<<(T input) {
		if (j_ == 0)
			oss_ << "|";

		// Leave 3 extra space: One for negative sign, one for zero, one for decimal
		oss_ << std::setw(column_widths_.at(j_))
			<< input;

		if (j_ == get_num_columns() - 1) {
			oss_ << "|\n";
			i_ = i_ + 1;
			j_ = 0;
		}
		else {
			oss_ << separator_;
			j_ = j_ + 1;
		}

		return *this;
	}

private:
	void AddHorizontalLine();

	template<typename T> void OutputDecimalNumber(T input);

	std::vector<std::string> column_headers_;
	std::vector<int> column_widths_;
	std::string separator_;

	std::ostringstream oss_;

	int i_; // index of current row
	int j_; // index of current column

	int table_width_;
};

template<typename T>
void TablePrinter::OutputDecimalNumber(T input) {
	// If we cannot handle this number, indicate so
	if (input < 10 * (column_widths_.at(j_) - 1) || input > 10 * column_widths_.at(j_)) {
		std::stringstream string_out;
		string_out << std::setiosflags(std::ios::fixed)
			<< std::setprecision(column_widths_.at(j_))
			<< std::setw(column_widths_.at(j_))
			<< input;

		std::string string_rep_of_number = string_out.str();

		string_rep_of_number[column_widths_.at(j_) - 1] = '*';
		std::string string_to_print = string_rep_of_number.substr(0, column_widths_.at(j_));
		oss_ << string_to_print;
	}
	else {

		// determine what precision we need
		int precision = column_widths_.at(j_) - 1; // leave room for the decimal point
		if (input < 0)
			--precision; // leave room for the minus sign

		// leave room for digits before the decimal?
		if (input < -1 || input > 1) {
			int num_digits_before_decimal = 1 + (int)log10(std::abs(input));
			precision -= num_digits_before_decimal;
		}
		else
			precision--; // e.g. 0.12345 or -0.1234

		if (precision < 0)
			precision = 0; // don't go negative with precision

		oss_ << std::setiosflags(std::ios::fixed)
			<< std::setprecision(precision)
			<< std::setw(column_widths_.at(j_))
			<< input;
	}

	if (j_ == get_num_columns() - 1) {
		oss_ << "|\n";
		i_ = i_ + 1;
		j_ = 0;
	}
	else {
		oss_ << separator_;
		j_ = j_ + 1;
	}
}
