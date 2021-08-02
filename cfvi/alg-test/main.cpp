#include "interpret.h"
#include <iostream>

using namespace cfvi::interpretation;

//vector<string> split(const string& a_string, const char& a_char) {
//	vector<string> result = { "" };
//	for (int i = 0; i < a_string.size(); i++)
//		if (a_string[i] != a_char) {
//			result.back() += a_string[i];
//		}
//		else {
//			result.push_back("");
//		}
//	return result;
//}
//
//bool wildcard_match(const string& a_data, const string& a_query) {
//	vector<string> match_split = split(a_query, '*');
//	size_t next_match_start = 0;
//	for (int i = 0; i < match_split.size(); i++) {
//		const string& l_string = match_split[i];
//		if (l_string != "") {
//			size_t this_match = a_data.find(l_string, next_match_start);
//			if (this_match == string::npos)
//				return false;
//			if (i == 0 && this_match != 0)
//				return false;
//			if (i == match_split.size() - 1 && this_match + l_string.size() != a_data.size())
//				return false;
//			next_match_start = this_match + l_string.size();
//		}
//	}
//
//	return true;
//}

void shell_test() {
	interpreter i = interpreter("./");
	while (true) {
		string line;
		std::getline(std::cin, line);
		i.interpret_line(line);
	}
}

int main() {

	shell_test();
	interpreter i = interpreter("./");
	i.process_import("account/secure_data/identities.cfvi");

	std::cout << "\n\n\n\n\n\n\n";

	return 0;
}
