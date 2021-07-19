#include "interpret.h"
#include <fstream>
#include <iostream>

using namespace cfvi;
using interpretation::symbol;
using std::ifstream;

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

vector<symbol>::iterator find_symbol(vector<symbol>& a_symbols, const string& a_identifier) {
	return std::find_if(a_symbols.begin(), a_symbols.end(), [&](const symbol& a_symbol) { return a_symbol.identifier == a_identifier; });
}

void prefix_symbols(vector<symbol>& a_symbols, const string& a_prefix) {
	for (int i = 0; i < a_symbols.size(); i++)
		a_symbols[i].identifier = a_prefix + "." + a_symbols[i].identifier;
}

string interpretation::get_chunk(const string& a_contents) {
	size_t l_rank = 0;
	size_t l_start_index = a_contents.find('{');
	size_t l_end_index = a_contents.size();
	for (int i = l_start_index; i < a_contents.size(); i++) {
		if (a_contents[i] == '{') {
			l_rank++;
		}
		else if (a_contents[i] == '}') {
			if (l_rank == 1) {
				l_end_index = i;
				break;
			}
			else {
				l_rank--;
			}
		}
	}
	if (l_end_index == a_contents.size())
		return string();
	size_t l_length = l_end_index - l_start_index + 1;
	return a_contents.substr(l_start_index, l_length);
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

int interpretation::interpret(const string & a_file_path, vector<symbol>&a_symbols) {
	ifstream ifs(a_file_path);
	if (!ifs.is_open())
		return EXIT_FAILURE;
	string line;
	string file_contents;
	while (std::getline(ifs, line)) {
		file_contents.insert(file_contents.end(), line.begin(), line.end());
		file_contents.push_back('\n');
	}

	size_t l_last_separator = a_file_path.find_last_of('/');
	string l_directory_path = a_file_path.substr(0, l_last_separator + 1);
	if (l_last_separator == string::npos)
		l_directory_path = "";

	string l_chunk = get_chunk(file_contents);
	while (l_chunk.size() > 0) {
		interpret_chunk(l_directory_path, l_chunk, a_symbols);
		file_contents.erase(0, l_chunk.size());
		l_chunk = get_chunk(file_contents);
	}
	ifs.close();
	return EXIT_SUCCESS;
}

void interpretation::interpret_chunk(const string& a_directory_path, string& a_chunk, vector<symbol>& a_symbols) {
	a_chunk = trim(a_chunk, { '{', '}', '\n' });
	if (a_chunk.size() > 4 && a_chunk.substr(0, 4) == "set ") {
		interpret_set(a_chunk, a_symbols);
	}
	else if (a_chunk.size() > 7 && a_chunk.substr(0, 7) == "delete ") {
		interpret_delete(a_chunk, a_symbols);
	}
	else if (a_chunk.size() > 7 && a_chunk.substr(0, 7) == "import ") {
		interpret_import(a_directory_path, a_chunk, a_symbols);
	}

}

void interpretation::interpret_set(string& a_chunk, vector<symbol>& a_symbols) {
	const size_t l_identifier_begin = 4;
	size_t l_identifier_end = a_chunk.find('=');
	size_t l_identifier_length = l_identifier_end - l_identifier_begin;
	string l_identifier = a_chunk.substr(l_identifier_begin, l_identifier_length);
	l_identifier = trim(l_identifier, ' ');

	size_t l_value_begin = l_identifier_end + 1;
	size_t l_value_end = a_chunk.length();
	size_t l_value_length = l_value_end - l_value_begin;
	string l_value = a_chunk.substr(l_value_begin, l_value_length);
	l_value = trim(l_value, ' ');

	merge_symbol(a_symbols, { l_identifier, l_value });

#if CFVI_DEBUG
	std::cout << "[ set ] SUCCESS: Merged symbol: " << l_identifier << " into dictionary." << std::endl;
#endif

}

void interpretation::interpret_delete(string& a_chunk, vector<symbol>& a_symbols) {
	const size_t l_identifier_begin = 7;
	size_t l_identifier_end = a_chunk.size();
	size_t l_identifier_length = l_identifier_end - l_identifier_begin;
	string l_identifier = a_chunk.substr(l_identifier_begin, l_identifier_length);

	vector<symbol>::iterator l_identifier_pos = find_symbol(a_symbols, l_identifier);
	// SYMBOL ALREADY EXISTS
	if (l_identifier_pos != a_symbols.end()) {
		a_symbols.erase(l_identifier_pos);
#if CFVI_DEBUG
		std::cout << "[ delete ] SUCCESS: Erased symbol: " << l_identifier << " from the dictionary." << std::endl;
#endif
	}
	else {
#if CFVI_DEBUG
		std::cout << "[ delete ] ERROR: Could not find symbol: " << l_identifier << " in the dictionary." << std::endl;
#endif
	}
}

void interpretation::interpret_import(const string& a_directory_path, string& a_chunk, vector<symbol>& a_symbols) {
	const size_t l_path_begin = a_chunk.find('<');
	size_t l_path_end = a_chunk.find('>');
	size_t l_path_length = l_path_end - l_path_begin;
	string l_file_path = a_chunk.substr(l_path_begin + 1, l_path_length - 1);

	string l_full_file_path;
	if (l_file_path.find(':') != string::npos)
		l_full_file_path = l_file_path;
	else
		l_full_file_path = a_directory_path + l_file_path;

	vector<symbol> l_import_symbols;

	size_t l_identifier_begin = a_chunk.find(" as ");
	if (l_identifier_begin != string::npos) {
		l_identifier_begin += 4;
		size_t l_identifier_end = a_chunk.size();
		size_t l_identifier_length = l_identifier_end - l_identifier_begin;
		string l_identifier = a_chunk.substr(l_identifier_begin, l_identifier_length);
		if (!interpret(l_full_file_path, l_import_symbols)) {
			prefix_symbols(l_import_symbols, l_identifier);
			merge_symbols(a_symbols, l_import_symbols);
#if CFVI_DEBUG
			std::cout << "[ import ] SUCCESS: Imported file: \"" << l_full_file_path << "\" with prefix: " << l_identifier << " merging: " << l_import_symbols.size() << " symbols." << std::endl;
#endif
		}
		else {
#if CFVI_DEBUG
			std::cout << "[ import ] ERROR: Could not import file: \"" << l_full_file_path << "\" with prefix: " << l_identifier << std::endl;
#endif
		}
	}
	else {
		if (!interpret(l_full_file_path, l_import_symbols)) {
			merge_symbols(a_symbols, l_import_symbols);
#if CFVI_DEBUG
			std::cout << "[ import ] SUCCESS: Imported file: \"" << l_full_file_path << "\", " << " merging: " << l_import_symbols.size() << " symbols in local dictionary." << std::endl;
#endif
		}
		else {
#if CFVI_DEBUG
			std::cout << "[ import ] ERROR: Could not import file: \"" << l_full_file_path << "\"" << std::endl;
#endif
		}
	}
}