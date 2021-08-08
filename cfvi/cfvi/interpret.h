#pragma once

#include <string>
#include <vector>

namespace cfvi {
	namespace interpretation {
		using std::string;
		using std::vector;
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
				vector<string> identifiers;
			};
			struct import_decl {
			public:
				vector<string> paths;
				string prefix = "";
				vector<string> identifiers;
			};

		public:
			interpreter(string a_parent_directory);

		public:
			string m_parent_directory;
			vector<imported_file> m_imported_files;
			vector<symbol> m_symbols;

		public:
			void process_define(const define_decl& a_decl);

		public:
			void process_undef(const undef_decl& a_decl);

		public:
			void process_import(const import_decl& a_decl);
			void process_import(const string& a_file_path);

		protected:
			bool interpret_file(const string& a_file_path);

		public:
			void interpret_line(string& a_line);

		protected:
			define_decl parse_define(const vector<string>& a_args);
			undef_decl parse_undef(const vector<string>& a_args);
			import_decl parse_import(const vector<string>& a_args);

		protected:
			bool depend(const string& a_absolute_file_path);

		};
	}
}