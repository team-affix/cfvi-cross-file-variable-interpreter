#pragma once
#define CFVI_DEBUG 0

#include <string>
#include <vector>

using std::string;
using std::vector;

namespace cfvi {
	namespace interpretation {
		struct symbol {
			string identifier;
			string value;
		};
		int interpret(const string& a_file_path, vector<symbol>& a_symbols);
		void interpret_line(const string& a_directory_path, string& a_line, vector<symbol>& a_symbols);
		void interpret_define(string& a_line, vector<symbol>& a_symbols);
		void interpret_undef(string& a_line, vector<symbol>& a_symbols);
		void interpret_import(const string& a_directory_path, string& a_line, vector<symbol>& a_symbols);
	}
}