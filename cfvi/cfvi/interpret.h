#pragma once
#define CFVI_DEBUG 1

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
		string get_chunk(const string& a_contents);
		int interpret(const string& a_file_path, vector<symbol>& a_symbols);
		void interpret_chunk(const string& a_directory_path, string& a_chunk, vector<symbol>& a_symbols);
		void interpret_set(string& a_chunk, vector<symbol>& a_symbols);
		void interpret_delete(string& a_chunk, vector<symbol>& a_symbols);
		void interpret_import(const string& a_directory_path, string& a_chunk, vector<symbol>& a_symbols);
	}
}