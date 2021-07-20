#include "interpret.h"
#include <fstream>
#include <iostream>

#define CFVI_DEREFERENCE '%'

using namespace cfvi;
using interpretation::symbol;
using std::ifstream;

#pragma region structs
struct range {
	size_t start;
	size_t length;
};
#pragma endregion
#pragma region directory
string directory_path_from_file_path(const string& a_file_path) {
	size_t l_last_separator = a_file_path.find_last_of('/');
	if (l_last_separator == string::npos)
		return "";
	return a_file_path.substr(0, l_last_separator + 1);
}
#pragma endregion
#pragma region string
vector<string> split(const string& a_string, const char& a_char) {
	vector<string> result = { "" };
	for (int i = 0; i < a_string.size(); i++)
		if (a_string[i] != a_char) {
			result.back() += a_string[i];
		}
		else {
			result.push_back("");
		}
	return result;
}

string join(const vector<string>& a_strings) {
	string result;
	for (int i = 0; i < a_strings.size(); i++)
		result += a_strings[i];
	return result;
}

string trim(const string& a_string, const char& a_char) {
	return join(split(a_string, a_char));
}

string trim(const string& a_string, const vector<char>& a_chars) {
	string result = a_string;
	for (int i = 0; i < a_chars.size(); i++)
		result = trim(result, a_chars[i]);
	return result;
}

size_t find(const string& a_string, const char& a_char, const int& a_find_index = 0) {
	int l_find_index = 0;
	for (int i = 0; i < a_string.size(); i++)
		if (a_string[i] == a_char)
			if (l_find_index == a_find_index)
				return i;
			else
				l_find_index++;
	return a_string.size();
}
#pragma endregion
#pragma region symbol
vector<symbol>::iterator find_symbol(vector<symbol>& a_symbols, const string& a_identifier) {
	return std::find_if(a_symbols.begin(), a_symbols.end(), [&](const symbol& a_symbol) { return a_symbol.identifier == a_identifier; });
}

void prefix_symbols(vector<symbol>& a_symbols, const string& a_prefix) {
	for (int i = 0; i < a_symbols.size(); i++)
		a_symbols[i].identifier = a_prefix + "." + a_symbols[i].identifier;
}

void merge_symbol(vector<symbol>& a_current, const symbol& a_symbol) {
	vector<symbol>::iterator loc = find_symbol(a_current, a_symbol.identifier);
	if (loc == a_current.end())
		a_current.push_back(a_symbol);
	else
		loc->value = a_symbol.value;
}

void merge_symbols(vector<symbol>& a_current, const vector<symbol>& a_new) {
	for (const symbol& l_symbol : a_new)
		merge_symbol(a_current, l_symbol);
}

void dereference_symbols(string& a_line, vector<symbol>& a_symbols) {
	vector<string> l_split = split(a_line, CFVI_DEREFERENCE);
	for (int i = 1; i < l_split.size() - 1; i++) {
		vector<symbol>::iterator l_loc = find_symbol(a_symbols, l_split[i]);
		if (l_loc != a_symbols.end())
			l_split[i] = l_loc->value;
	}
	a_line = join(l_split);
}
#pragma endregion
#pragma region interpreting
int interpretation::interpret(const string & a_file_path, vector<symbol>& a_symbols) {
	ifstream ifs(a_file_path);
	if (!ifs.is_open())
		return EXIT_FAILURE;

	string l_directory_path = directory_path_from_file_path(a_file_path);

	string l_line;
	while (std::getline(ifs, l_line)) {
		interpret_line(l_directory_path, l_line, a_symbols);
	}
	ifs.close();

	return EXIT_SUCCESS;
}

void interpretation::interpret_line(const string& a_directory_path, string& a_line, vector<symbol>& a_symbols) {
	dereference_symbols(a_line, a_symbols);
	if (a_line.size() > 4 && a_line.substr(0, 7) == "define ") {
		interpret_define(a_line, a_symbols);
	}
	else if (a_line.size() > 7 && a_line.substr(0, 6) == "undef ") {
		interpret_undef(a_line, a_symbols);
	}
	else if (a_line.size() > 7 && a_line.substr(0, 7) == "import ") {
		interpret_import(a_directory_path, a_line, a_symbols);
	}

}

void interpretation::interpret_define(string& a_line, vector<symbol>& a_symbols) {
	size_t l_identifier_begin = a_line.find(' ') + 1;
	size_t l_identifier_end = find(a_line, ' ', 1);
	size_t l_identifier_length = l_identifier_end - l_identifier_begin;
	string l_identifier = a_line.substr(l_identifier_begin, l_identifier_length);

	size_t l_value_begin = l_identifier_end + 1;
	size_t l_value_end = a_line.size();
	size_t l_value_length = l_value_end - l_value_begin;
	string l_value;

	if (l_value_begin < a_line.size())
		l_value = a_line.substr(l_value_begin, l_value_length);

	merge_symbol(a_symbols, { l_identifier, l_value });

#if CFVI_DEBUG
	std::cout << "[ define ] SUCCESS: Merged symbol: " << l_identifier << " into dictionary." << std::endl;
#endif
}

void interpretation::interpret_undef(string& a_line, vector<symbol>& a_symbols) {
	size_t l_identifier_begin = find(a_line, ' ', 0) + 1;
	size_t l_identifier_end = a_line.size();
	size_t l_identifier_length = l_identifier_end - l_identifier_begin;
	string l_identifier = a_line.substr(l_identifier_begin, l_identifier_length);

	vector<symbol>::iterator l_identifier_pos = find_symbol(a_symbols, l_identifier);
	// SYMBOL ALREADY EXISTS
	if (l_identifier_pos != a_symbols.end()) {
		a_symbols.erase(l_identifier_pos);
#if CFVI_DEBUG
		std::cout << "[ undef ] SUCCESS: Erased symbol: " << l_identifier << " from the dictionary." << std::endl;
#endif
	}
	else {
#if CFVI_DEBUG
		std::cout << "[ undef ] ERROR: Could not find symbol: " << l_identifier << " in the dictionary." << std::endl;
#endif
	}
}

void interpretation::interpret_import(const string& a_directory_path, string& a_line, vector<symbol>& a_symbols) {
	const size_t l_path_begin = a_line.find('<');
	size_t l_path_end = a_line.find('>');
	size_t l_path_length = l_path_end - l_path_begin;
	string l_file_path = a_line.substr(l_path_begin + 1, l_path_length - 1);

	string l_full_file_path;
	if (l_file_path.find(':') != string::npos)
		l_full_file_path = l_file_path;
	else
		l_full_file_path = a_directory_path + l_file_path;



	vector<symbol> l_import_symbols;

	size_t l_identifier_begin = a_line.find(" as ");
	if (l_identifier_begin != string::npos) {
		l_identifier_begin += 4;
		size_t l_identifier_end = a_line.size();
		size_t l_identifier_length = l_identifier_end - l_identifier_begin;
		string l_identifier = a_line.substr(l_identifier_begin, l_identifier_length);
		if (!interpret(l_full_file_path, l_import_symbols)) {
			prefix_symbols(l_import_symbols, l_identifier);
			merge_symbols(a_symbols, l_import_symbols);
#if CFVI_DEBUG
			std::cout << "[ import ] SUCCESS: Imported file: <" << l_full_file_path << "> with prefix: " << l_identifier << " merging: " << l_import_symbols.size() << " symbols." << std::endl;
#endif
		}
		else {
#if CFVI_DEBUG
			std::cout << "[ import ] ERROR: Could not import file: <" << l_full_file_path << "> with prefix: " << l_identifier << std::endl;
#endif
		}
	}
	else {
		if (!interpret(l_full_file_path, l_import_symbols)) {
			merge_symbols(a_symbols, l_import_symbols);
#if CFVI_DEBUG
			std::cout << "[ import ] SUCCESS: Imported file: <" << l_full_file_path << ">, " << " merging: " << l_import_symbols.size() << " symbols in local dictionary." << std::endl;
#endif
		}
		else {
#if CFVI_DEBUG
			std::cout << "[ import ] ERROR: Could not import file: <" << l_full_file_path << ">" << std::endl;
#endif
		}
	}
}
#pragma endregion