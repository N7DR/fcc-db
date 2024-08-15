#ifndef FCC_STRINGS_H
#define FCC_STRINGS_H

// Released under the GNU Public License, version 2
//   see: https://www.gnu.org/licenses/gpl-2.0.html

// Principal author: N7DR

// Copyright owners:
//    N7DR

// A much-reduced and slightly edited version of the file string_functions.h

/*! \file   fcc-strings.h

    Functions related to the manipulation of strings
*/

#include <sstream>
#include <string>
#include <vector>

using namespace std::string_literals;

/*! \brief              Split a string into components
    \param  cs          original string
    \param  separator   separator string (typically a single character)
    \return             vector containing the separate components
*/
std::vector<std::string> split_string(const std::string& cs, const std::string& separator);

/*! \brief              Read the contents of a file into a single string
    \param  filename    name of file to be read
    \return             contents of file <i>filename</i>
  
    Throws exception if the file does not exist, or if any
    of several bad things happen. Assumes that the file is a reasonable length.
*/
std::string read_file(const std::string& filename);

/*! \brief                  Remove all instances of a particular char from a string
    \param  cs              original string
    \param  char_to_remove  character to be removed from <i>cs</i>
    \return                 <i>cs</i> with all instances of <i>char_to_remove</i> removed
*/
std::string remove_char(const std::string& cs, const char char_to_remove);

/*! \brief              Split a string into lines
    \param  cs          original string
    \param  eol_marker  EOL marker
    \return             vector containing the separate lines
*/
inline std::vector<std::string> to_lines(const std::string& cs)
  { return split_string(cs, "\n"s); }

/*! \brief          Generic conversion to string
    \param  val     value to convert
    \return         <i>val</i>converted to a string
*/
template <class T>
std::string to_string(const T val)
{ std::ostringstream stream;
                
  stream << val;
  return stream.str();
}

/*! \brief      Transform a string
    \param  cs  original string
    \param  pf  pointer to transformation function
    \return     <i>cs</i> with the transformation <i>*pf</i> applied
*/
std::string transform_string(const std::string& cs, int(*pf)(int));

/*! \brief      Convert string to upper case
    \param  cs  original string
    \return     <i>cs</i> converted to upper case
*/
inline std::string to_upper(const std::string& cs)
  { return transform_string(cs, std::toupper); }

/*! \brief      Transform an FCC date to an ISO 8601 extended-format date
    \param  cs  original string
    \param  pf  pointer to transformation function
    \return     <i>cs</i> with the transformation <i>*pf</i> applied
*/
std::string transform_date(const std::string& us_date);

/*! \brief          Is one call earlier than another, according to classical callsign sort order?
    \param  call1   first call
    \param  call2   second call
    \return         whether <i>call1</i> appears before <i>call2</i> in callsign sort order
*/
bool compare_calls(const std::string& call1, const std::string& call2);

/*! \brief  Create a string of a certain length, with all characters the same
    \param  c   Character that the string will contain
    \param  n   Length of string to be created
    \return String containing <i>n</i> copies of <i>c</i>
*/
inline const std::string create_string(const char c, const int n = 1)
  { return std::string(n, c); }
  
/*! \brief      Remove all instances of a specific leading character
    \param  cs  original string
    \param  c   leading character to remove (if present)
    \return     <i>cs</i> with any leading octets with the value <i>c</i> removed
*/
std::string remove_leading(const std::string& cs, const char c);

/*! \brief      Remove leading spaces
    \param  cs  original string
    \return     <i>cs</i> with any leading spaces removed
*/
inline std::string remove_leading_spaces(const std::string& cs)
  { return remove_leading(cs, ' '); }

/*! \brief      Remove all instances of a specific trailing character
    \param  cs  original string
    \param  c   trailing character to remove (if present)
    \return     <i>cs</i> with any trailing octets with the value <i>c</i> removed
*/
std::string remove_trailing(const std::string& cs, const char c);

/*! \brief      Remove trailing spaces
    \param  cs  original string
    \return     <i>cs</i> with any trailing spaces removed
*/
inline std::string remove_trailing_spaces(const std::string& cs)
  { return remove_trailing(cs, ' '); }

/*! \brief      Remove leading and trailing spaces
    \param  cs  original string
    \return     <i>cs</i> with any leading or trailing spaces removed
*/
inline std::string remove_peripheral_spaces(const std::string& cs)
  { return remove_trailing_spaces(remove_leading_spaces(cs)); }

/// return the current date as YYYY-MM-DD
std::string date_string(void);

#endif    // FCC_STRINGS_H
