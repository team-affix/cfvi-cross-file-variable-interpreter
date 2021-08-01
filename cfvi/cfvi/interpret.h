#pragma once
#define CFVI_DEBUG 1

#include <string>
#include <vector>

using std::string;
using std::vector;

namespace cfvi {
	namespace interpretation {
		class interpreter {
		public:
			struct symbol {
				string identifier;
				string value;
			};
			struct imported_file {
				string file_name;
				bool processing = true;
				size_t times_processed = 0;
			};
			struct define_decl {
				string identifier;
				string value;
			};
			struct undef_decl {
				string identifier;
			};
			struct import_decl {
			public:
				string path;
				string identifier = "";
			};

		public:
			vector<imported_file> m_imported_files;
			vector<symbol> m_symbols;

		public:
			void define(const define_decl& a_decl);
			void undef(const undef_decl& a_decl);
			void import(const import_decl& a_decl);

		protected:
			bool interpret_file(const string& a_file_path);
			void interpret_line(const string& a_directory_path, string& a_line);

		protected:
			define_decl parse_define(const vector<string>& a_args);
			undef_decl parse_undef(const vector<string>& a_args);
			import_decl parse_import(const string& a_directory_path, const vector<string>& a_args);

		protected:
			bool depend(const string& a_absolute_file_path);
			void import_file(const string& a_import_path);
			void import_directory(const string& a_import_path);

		};
	}
}