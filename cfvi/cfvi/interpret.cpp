#include "interpret.h"
#include <fstream>
#include <iostream>
#include <filesystem>

#define CFVI_DEREFERENCE '%'
#define ARG_SEPARATE ' '
#define ARG_LIST ','

using namespace cfvi;
using interpretation::interpreter;
using std::ifstream;
namespace fs = std::filesystem;

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
vector<string> split_respect_quotes(const string& a_string, const char& a_split_char) {
	vector<string> result = { "" };
	bool respect = false;
	for (int i = 0; i < a_string.size(); i++) {
		const char& l_char = a_string[i];
		if (l_char == '\"')
			respect = !respect;
		else if (l_char == a_split_char && !respect)
			result.push_back("");
		else
			result.back() += l_char;
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
bool can_query(const string& a_data, const string& a_query) {
	vector<string> match_split = split(a_query, '*');

	size_t next_match_start = 0;
	for (int i = 0; i < match_split.size(); i++) {
		const string& l_string = match_split[i];
		if (l_string != "") {
			size_t this_match = a_data.find(l_string, next_match_start);
			if (this_match == string::npos)
				return false;
			if (i == 0 && this_match != 0)
				return false;
			if (i == match_split.size() - 1 && this_match + l_string.size() != a_data.size())
				return false;
			next_match_start = this_match + l_string.size();
		}
	}

	return true;
}
bool any_can_query(const string& a_data, const vector<string>& a_queries) {
	return std::find_if(a_queries.begin(), a_queries.end(), [&a_data](const string& a_elem) {
		return can_query(a_data, a_elem);
	}) != a_queries.end();
}

#pragma endregion
#pragma region vector
template<typename T>
vector<T>::iterator find(vector<T>& a_vec, const T& a_val) {
	return std::find(a_vec.begin(), a_vec.end(), a_val);
}
template<typename T>
vector<T>::const_iterator find(const vector<T>& a_vec, const T& a_val) {
	return std::find(a_vec.begin(), a_vec.end(), a_val);
}
template<typename T>
bool exists(const vector<T>& a_vec, const T& a_val) {
	return find(a_vec, a_val) != a_vec.end();
}
#pragma endregion
#pragma region symbol

vector<interpreter::symbol>::iterator find_symbol(vector<interpreter::symbol>& a_symbols, const string& a_identifier) {
	return std::find_if(a_symbols.begin(), a_symbols.end(), [&](const interpreter::symbol& a_symbol) { return a_symbol.identifier == a_identifier; });
}
vector<interpreter::symbol>::iterator query_symbols_for_first_result(vector<interpreter::symbol>& a_symbols, const string& a_query, size_t a_offset = 0) {
	return std::find_if(a_symbols.begin() + a_offset, a_symbols.end(), [&a_query](const interpreter::symbol& l_symbol) {
		return can_query(l_symbol.identifier, a_query);
	});
}
void prefix_symbols(vector<interpreter::symbol>& a_symbols, const string& a_prefix) {
	for (int i = 0; i < a_symbols.size(); i++)
		a_symbols[i].identifier = a_prefix + "." + a_symbols[i].identifier;
}
void merge_symbol(vector<interpreter::symbol>& a_current, const interpreter::symbol& a_symbol) {
	vector<interpreter::symbol>::iterator loc = find_symbol(a_current, a_symbol.identifier);
	if (loc == a_current.end())
		a_current.push_back(a_symbol);
	else
		loc->value = a_symbol.value;
}
void merge_symbols(vector<interpreter::symbol>& a_current, const vector<interpreter::symbol>& a_new) {
	for (const interpreter::symbol& l_symbol : a_new)
		merge_symbol(a_current, l_symbol);

}
void dereference_symbols(string& a_line, vector<interpreter::symbol>& a_symbols) {
	vector<string> l_split = split(a_line, CFVI_DEREFERENCE);
	for (int i = 1; i < l_split.size() - 1; i++) {
		vector<interpreter::symbol>::iterator l_loc = find_symbol(a_symbols, l_split[i]);
		if (l_loc != a_symbols.end())
			l_split[i] = l_loc->value;
	}
	a_line = join(l_split);
}

#pragma endregion
#pragma region interpreter

vector<interpreter::imported_file>::iterator find_imported_file(vector<interpreter::imported_file>& a_imported_files, const string& a_file_name) {
	return std::find_if(a_imported_files.begin(), a_imported_files.end(), [&](const interpreter::imported_file& file) {
		return file.file_name == a_file_name;
	});
}
void merge_imported_files(vector<interpreter::imported_file>& a_current, const vector<interpreter::imported_file>& a_new) {
	for (const interpreter::imported_file& l_new : a_new) {
		vector<interpreter::imported_file>::iterator l_imported_file = find_imported_file(a_current, l_new.file_name);
		if (l_imported_file != a_current.end()) {
			l_imported_file->processing = l_new.processing;
			l_imported_file->times_processed = l_new.times_processed;
		}
		else {
			a_current.push_back(l_new);
		}
	}
}

#pragma endregion
#pragma region interpreting

void interpreter::define(const define_decl& a_decl) {
	
	if (a_decl.identifier == "") {
#if CFVI_DEBUG
		std::cout << "[ define ] ERROR: Cannot merge a symbol without an identifier into the dictionary. Specify an identifier with flags: -i or --identifier" << std::endl;
#endif
		return;
	}

	merge_symbol(m_symbols, {a_decl.identifier, a_decl.value});

#if CFVI_DEBUG
	std::cout << "[ define ] SUCCESS: Merged symbol: " << a_decl.identifier << " into dictionary." << std::endl;
#endif

}
void interpreter::undef(const undef_decl& a_decl) {

	for (const string& l_identifier : a_decl.identifiers) {

		size_t last_result_pos = 0;
		vector<symbol>::iterator l_result_pos;

		while (true) {

			l_result_pos = query_symbols_for_first_result(m_symbols, l_identifier, last_result_pos);
			last_result_pos = l_result_pos - m_symbols.begin();

			if (l_result_pos == m_symbols.end())
				break;

			if (l_result_pos != m_symbols.end()) {
				// SYMBOL EXISTS
#if CFVI_DEBUG
				std::cout << "[ undef ] SUCCESS: Erased symbol: " << l_result_pos->identifier << " from the dictionary." << std::endl;
#endif
				m_symbols.erase(l_result_pos);
			}
			else {
#if CFVI_DEBUG
				std::cout << "[ undef ] ERROR: Could not find symbol: " << l_result_pos->identifier << " in the dictionary." << std::endl;
#endif
			}
		}

	}

}
void interpreter::import(const import_decl& a_decl) {
	
	interpreter l_interpreter = interpreter();
	l_interpreter.m_imported_files = m_imported_files;
	
	fs::path l_path = fs::absolute(a_decl.path);

	if (fs::is_directory(l_path))
		l_interpreter.import_directory(l_path.string());
	else
		l_interpreter.import_file(l_path.string());

	// ONLY IMPORT SYMBOLS WITH SPEICFIED IDENTIFIERS
	if (a_decl.identifiers.size() > 0) {
		std::erase_if(l_interpreter.m_symbols, [&](const symbol& a_symbol) {
			return !any_can_query(a_symbol.identifier, a_decl.identifiers);
		});
	}

	if (a_decl.prefix != "")
		prefix_symbols(l_interpreter.m_symbols, a_decl.prefix);
	
	merge_imported_files(m_imported_files, l_interpreter.m_imported_files);
	merge_symbols(m_symbols, l_interpreter.m_symbols);

#if CFVI_DEBUG
	std::cout << "[ import ] SUCCESS: Imported module: <" << a_decl.path << "> merging: " << l_interpreter.m_symbols.size() << " symbols." << std::endl;
#endif

}
bool interpreter::interpret_file(const string & a_file_path) {

	ifstream ifs(a_file_path);
	if (!ifs.is_open())
		return false;

	std::filesystem::path l_absolute_file_path = std::filesystem::absolute(a_file_path);

	if (!depend(l_absolute_file_path.string()))
		return false;

	string l_directory_path = l_absolute_file_path.parent_path().string();

	string l_line;
	while (std::getline(ifs, l_line)) {
		interpret_line(l_directory_path, l_line);
	}
	ifs.close();

	vector<imported_file>::iterator l_imported_file = find_imported_file(m_imported_files, l_absolute_file_path.string());

	l_imported_file->processing = false;
	l_imported_file->times_processed++;

	return true;
}
void interpreter::interpret_line(const string& a_directory_path, string& a_line) {
	dereference_symbols(a_line, m_symbols);

	vector<string> l_args = split_respect_quotes(a_line, ARG_SEPARATE);
	if (l_args.size() == 0)
		return;
	if (l_args[0] == "define") {
		define(parse_define(l_args));
	}
	else if (l_args[0] == "undef") {
		undef(parse_undef(l_args));
	}
	else if (l_args[0] == "import") {
		import(parse_import(a_directory_path, l_args));
	}

}
interpreter::define_decl interpreter::parse_define(const vector<string>& a_args) {

	string l_identifier;
	string l_value;

	for (int i = 1; i < a_args.size(); i++) {

		const string& l_arg = a_args[i];

		if (i + 1 < a_args.size()) {

			const string& l_next_arg = a_args[i + 1];

			if (l_arg == "-i" || l_arg == "--identifier") {
				l_identifier = l_next_arg;
				continue;
			}
			else if (l_arg == "-v" || l_arg == "--value") {
				l_value = l_next_arg;
				continue;
			}

		}

		// DEFAULT ARGS
		if (i == 1) {
			l_identifier = l_arg;
			continue;
		}
		if (i == 2) {
			l_value = l_arg;
			continue;
		}

	}

	return { l_identifier, l_value };
}
interpreter::undef_decl interpreter::parse_undef(const vector<string>& a_args) {

	vector<string> l_identifiers;

	for (int i = 1; i < a_args.size(); i++) {

		const string& l_arg = a_args[i];

		if (i + 1 < a_args.size()) {

			const string& l_next_arg = a_args[i + 1];

			if (l_arg == "-i" || l_arg == "--identifiers") {
				l_identifiers = split(l_next_arg, ARG_LIST);
				continue;
			}

		}

		// DEFAULT ARGS
		if (i == 1) {
			l_identifiers = split(l_arg, ARG_LIST);
			continue;
		}

	}

	return { l_identifiers };
}
interpreter::import_decl interpreter::parse_import(const string& a_directory_path, const vector<string>& a_args) {

	fs::path l_full_path;
	string l_prefix;
	vector<string> l_identifiers;

	for (int i = 1; i < a_args.size(); i++) {

		const string& l_arg = a_args[i];

		if (i + 1 < a_args.size()) {

			const string& l_next_arg = a_args[i + 1];

			if (l_arg == "-v" || l_arg == "--value") {
				l_full_path = fs::absolute(a_directory_path + "/" + l_next_arg);
				continue;
			}
			if (l_arg == "-p" || l_arg == "--prefix") {
				l_prefix = l_next_arg;
				continue;
			}
			if (l_arg == "-i" || l_arg == "--identifiers") {
				l_identifiers = split_respect_quotes(l_next_arg, ARG_LIST);
				continue;
			}

		}

		// DEFAULT ARGS
		if (i == 1) {
			l_full_path = fs::absolute(a_directory_path + "/" + l_arg);
			continue;
		}
		if (i == 2) {
			l_identifiers = split_respect_quotes(l_arg, ARG_LIST);
			continue;
		}

	}

	return { l_full_path.string(), l_prefix, l_identifiers };
}
bool interpreter::depend(const string& a_absolute_file_path) {

	vector<imported_file>::iterator l_imported_file = find_imported_file(m_imported_files, a_absolute_file_path);
	if (l_imported_file != m_imported_files.end()) {
		if (l_imported_file->processing) {
			// CIRCULAR DEPENDENCY
			return false;
		}
		else {
			l_imported_file->processing = true;
		}
	}
	else {
		m_imported_files.push_back({ a_absolute_file_path });
	}

	return true;
}
void interpreter::import_file(const string& a_import_path) {

	if (interpret_file(a_import_path)) {
#if CFVI_DEBUG
		std::cout << "[ import ] SUCCESS: Imported file: <" << a_import_path << "> staging: " << m_symbols.size() << " symbols for merge." << std::endl;
#endif
	}
	else {
#if CFVI_DEBUG
		std::cout << "[ import ] ERROR: Could not import file: <" << a_import_path << ">" << std::endl;
#endif
	}

}
void interpreter::import_directory(const string& a_import_path) {
	for (const std::filesystem::path& l_path : fs::directory_iterator(a_import_path))
		if (fs::is_directory(l_path))
			import_directory(l_path.string());
		else
			import_file(l_path.string());
}

#pragma endregion