#include "table_printer.h"
#include <stdexcept>
#include <iomanip>
#include <stdexcept>

TablePrinter::TablePrinter(const std::string & separator) {
	i_ = 0;
	j_ = 0;
	separator_ = separator;
	table_width_ = 0;
}

TablePrinter::~TablePrinter() {

}

int TablePrinter::get_num_columns() const {
	return column_headers_.size();
}

int TablePrinter::get_table_width() const {
	return table_width_;
}

void TablePrinter::set_separator(const std::string &separator) {
	separator_ = separator;
}

/** \brief Add a column to our table
 **
 ** \param header_name Name to be print for the header
 ** \param column_width the width of the column (has to be >=5)
 ** */
void TablePrinter::AddColumn(const std::string & header_name, int column_width) {
	column_headers_.push_back(header_name);
	column_widths_.push_back(column_width);
	table_width_ += column_width + separator_.size(); // for the separator  
}

void TablePrinter::AddHorizontalLine() {
	oss_ << "+"; // the left bar

	for (int i = 0; i < table_width_ - 1; ++i)
		oss_ << "-";

	oss_ << "+"; // the right bar
	oss_ << "\n";
}

void TablePrinter::AddHeader() {
	AddHorizontalLine();
	oss_ << "|";

	for (int i = 0; i < get_num_columns(); ++i) {
		oss_ << std::setw(column_widths_.at(i)) << column_headers_.at(i).substr(0, column_widths_.at(i));
		if (i != get_num_columns() - 1) {
			oss_ << separator_;
		}
	}

	oss_ << "|\n";
	AddHorizontalLine();
}

void TablePrinter::AddFooter() {
	AddHorizontalLine();
}

TablePrinter& TablePrinter::operator<<(float input) {
	OutputDecimalNumber<float>(input);
	return *this;
}

TablePrinter& TablePrinter::operator<<(double input) {
	OutputDecimalNumber<double>(input);
	return *this;
}

std::string TablePrinter::ToString() const {
	return oss_.str();
}

void TablePrinter::ClearBuffer() {
	oss_.clear();
}
