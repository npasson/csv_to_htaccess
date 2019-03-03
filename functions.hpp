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

#ifndef CSV_TO_HTACCESS_FUNCTIONS_HPP
#define CSV_TO_HTACCESS_FUNCTIONS_HPP

#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <regex>

/**
 * Splits a string into parts, returning the result as a vector of the parts.
 *
 * @param line The string to be split.
 * @param delim The delimiter to split at.
 * @return The line, in pieces.
 */
std::vector<std::string> split(const std::string& line, char delim) {

	std::string              buffer;
	std::stringstream        line_stream(line);
	std::vector<std::string> to_ret;

	while (std::getline(line_stream, buffer, delim)) {
		to_ret.push_back(buffer);
	}

	return to_ret;
}

/**
 * Write a line to the file specified. The line is in the format:
 *
 * <tt>RewriteRule ^/?from/? /to [NC,R=status_code,L]</tt>
 *
 * @param file The file stream to write to.
 * @param from The left hand side of the redirect, with no slashes at the beginning or end.
 * @param to The right hand side of the redirect, with no slashes at the beginning or end.
 * @param status_code The redirect status code to use.
 */
void write_htaccess_line(std::ofstream& file,
                         std::string from,
                         std::string to,
                         std::string status_code) {

	std::string to_clean[] = {
			std::move(from),
			std::move(to),
			std::move(status_code)
	};

	// remove newlines and other things
	for (auto& i : to_clean) {
		auto remove = [&](char delim_to_remove) -> void {
			i.erase(std::remove(i.begin(), i.end(), delim_to_remove), i.end());
		};

		for (auto ch : {'\n', '\r', '\0'}) {
			remove(ch);
		}
	}

	// handle some edge cases:

	// TO is absolute, i.e. starts with http:// or https://
	bool to_relative = !std::regex_match(to_clean[1].c_str(), std::regex(R"(^https?://.*)"));

	// FROM is literally just a single slash ( "/" )
	bool from_is_single_slash = ( to_clean[0] == "/" );

	// write to file
	file
			<< "RewriteRule "
			<< ( from_is_single_slash ? "^" : "^/?" ) << to_clean[0] << ( from_is_single_slash ? "$ " : "/?$ " )
			<< ( to_relative ? "/" : "" ) << to_clean[1]
			<< " [NC,R=" << to_clean[2] << ",L]\n";

	// *actually* write to file
	file.flush();
}

/**
 * Returns file contents, or empty string if file can't be opened.
 *
 * @param filename The file path.
 * @return The file contents, or empty string.
 */
std::string get_file_contents(const std::string& filename) {

	std::ifstream file(filename);

	if (file) {
		std::string to_return;

		file.seekg(0, std::ios::end);
		to_return.resize(file.tellg());
		file.seekg(0, std::ios::beg);

		file.read(&to_return[0], to_return.size());
		file.close();

		return to_return;
	} else {
		return std::string {};
	}
}

#endif //CSV_TO_HTACCESS_FUNCTIONS_HPP
