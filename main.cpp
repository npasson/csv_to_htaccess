/*==================================================================================*\
 *
 *   CSV to .htaccess
 *   Copyright (C) 2018  Nicholas Passon
 *   Documentation: http://www.npasson.com/fractiontype
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Affero General Public License as published
 *   by the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Affero General Public License for more details.
 *
 *   You should have received a copy of the GNU Affero General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
\*==================================================================================*/

#include "functions.hpp"

constexpr const char* version    = "0.2";
constexpr const char* delimiters = ";,\t";


int main(int argc, char* argv[]) {

	// exit if no file specified.
	if (argc < 2 || argc % 2 != 0) {
		std::cerr
				<< "Usage: " << argv[0] << " <csv path> [--prefix path/to/prefix] [--suffix path/to/suffix]"
				<< std::endl;
		exit(1);
	}

	// open files and exit if we couldn't.
	std::ifstream csv(argv[1]);

	if (csv.bad() || !csv) {
		std::cerr
				<< "Could not open file: " << argv[1]
				<< std::endl;
		csv.close();
		exit(1);
	}

	// initailize a few variables
	std::ifstream prefix;
	std::ifstream suffix;
	bool          has_prefix = false;
	bool          has_suffix = false;
	std::string   prefix_path;
	std::string   suffix_path;

	for (unsigned int i = 2; i < argc; i += 2) {

		// open prefix file if exists
		if (std::string(argv[i]) == "--prefix") {
			prefix_path = argv[i + 1];
			prefix.open(prefix_path);
			if (!prefix) {
				std::cerr
						<< "Could not open file: " << argv[1]
						<< std::endl;
				exit(1);
			} else {
				has_prefix = true;
			}
			suffix.close();
		}

		// open suffix file if exists
		if (std::string(argv[i]) == "--suffix") {
			suffix_path = argv[i + 1];
			suffix.open(suffix_path);
			if (!prefix) {
				std::cerr
						<< "Could not open file: " << argv[1]
						<< std::endl;
				suffix.close();
				exit(1);
			} else {
				has_suffix = true;
			}
			suffix.close();
		}
	}

	// initialize buffer and chosen delimiter
	std::string line_buffer;
	char        delim   = '\0';

	// try to find the delimiter in the first line.
	std::getline(csv, line_buffer);
	for (unsigned int i = 0; delimiters[i] != 0; ++i) {
		if (line_buffer.find(delimiters[i]) != std::string::npos) {
			delim = delimiters[i];
			break;
		}
	}

	// exit if delimiter wasn't found.
	if (delim == '\0') {
		std::cerr
				<< "Can't find an accepted delimiter in the first line. Exiting."
				<< std::endl;
		exit(1);
	}

	// reset file state
	csv.clear();
	csv.seekg(0, std::ifstream::beg);

	// open htaccess file for writing
	std::ofstream htaccess("./.htaccess");
	if (htaccess.bad() || !htaccess) {
		std::cerr
				<< "Could not open .htaccess for writing. Is the folder write-protected?"
				<< std::endl;
		csv.close();
		htaccess.close();
	}

	std::time_t now_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	// write generator header
	htaccess
			<< "# npasson CSV to .htaccess generator\n"
			   "# Version: " << version << "\n"
			<< "# Generation started at " << std::ctime(&now_time) << "\n"
			<< "# github.com/npasson\n"
			   "# The license of the generator does not apply to the content of this file.\n\n";


	if (has_prefix) {
		// write prefix file to htaccess
		htaccess
				<< "# BEGIN GENERATOR PREFIX\n\n"
				<< get_file_contents(prefix_path) << "\n\n"
				<< "# END GENERATOR PREFIX\n\n";
	}

	/* ************************************************************ *\
	 *  Write lines from the CSV                                    *
	\* ************************************************************ */

	// initialize regex
	const std::regex reg_get_domain = std::regex(R"(^((?:https?:\/\/)?(?:[A-Za-z]+\.)+[A-Za-z]+)/)");
	const std::regex reg_get_path   = std::regex(R"(^(?:(?:https?:\/\/)?(?:(?:[A-Za-z]+\.)+[A-Za-z]+))?\/?(.*)$)");

	// initialize FROM
	//std::unordered_map<unsigned int, std::string> from {};
	std::string from_domain;
	std::string from_path           = "/";
	std::string from_towrite;

	// initialize TO
	//std::unordered_map<unsigned int, std::string> to {};
	std::string to_domain;
	std::string to_path             = "/";
	std::string to_towrite;

	while (getline(csv, line_buffer)) {
		static int               line_number = 1;
		std::vector<std::string> tokens      = split(line_buffer, delim);

		if (tokens.size() < 2) {
			std::cerr << "Line " << line_number++ << " has less than two columns.\n";
			continue;
		}

		to_towrite   = "";
		from_towrite = "";

		std::string status_code = ( tokens.size() >= 3 ? tokens[2] : "302" );

		/* Extract info */

		std::string from_fullpath = tokens[0];
		std::string to_fullpath   = tokens[1];


		auto matches = std::cmatch {};

		// set from-prefix if found
		if (std::regex_match(from_fullpath.c_str(), matches, reg_get_domain)) {
			from_domain = matches[1];
		}

		// set to-prefix if found, otherwise assume same domain
		if (std::regex_match(to_fullpath.c_str(), matches, reg_get_domain)) {
			to_domain = matches[1];
		} else {
			to_domain = from_domain;
		}

		// set to-path
		std::regex_match(from_fullpath.c_str(), matches, reg_get_path);
		from_path = matches[1];

		// set from-path
		std::regex_match(to_fullpath.c_str(), matches, reg_get_path);
		to_path = matches[1];

		// only add domain to `to` if they're different
		if (from_domain != to_domain) { to_towrite += to_domain; }

		// add the paths
		from_towrite += from_path;
		to_towrite += to_path;

		// remove the slashes from the front and back of the paths, they get added automatically
		if (from_towrite.back() == '/') { from_towrite.pop_back(); }
		if (from_towrite[0] == '/') { from_towrite.erase(0, 1); }
		if (to_towrite.back() == '/') { to_towrite.pop_back(); }
		if (to_towrite[0] == '/') { to_towrite.erase(0, 1); }

		// add a single slash if "to" is empty
		if (from_towrite.empty()) {
			from_towrite = "/";
		}

		// go!
		write_htaccess_line(htaccess, from_towrite, to_towrite, status_code);

		++line_number;
	}

	csv.close();

	if (has_suffix) {
		// write suffix file to htaccess
		htaccess
				<< "\n"
				   "# BEGIN GENERATOR SUFFIX\n\n"
				<< get_file_contents(suffix_path) << "\n\n"
				<< "# END GENERATOR SUFFIX\n\n";
	}

	htaccess.close();

	return EXIT_SUCCESS;
}