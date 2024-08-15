// Released under the GNU Public License, version 2
//   see: https://www.gnu.org/licenses/gpl-2.0.html

// Principal author: N7DR

// Copyright owners:
//    N7DR

// A much-reduced and slightly edited version of the file string_functions.cpp

/*! \file   fcc-strings.cpp

    Functions related to the manipulation of strings
*/

#include "fcc-strings.h"

#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>

#include <sys/stat.h>

using namespace std;

/*! \brief              Split a string into components
    \param  cs          original string
    \param  separator   separator string (typically a single character)
    \return             vector containing the separate components
*/
vector<string> split_string(const string& cs, const string& separator)
{ size_t         start_posn { 0 };
  vector<string> rv         { };

  while (start_posn < cs.length())
  { unsigned long posn { cs.find(separator, start_posn) };

    if (posn == string::npos)                       // no more separators
    { rv.push_back(cs.substr(start_posn));
      start_posn = cs.length();
    }
    else                                            // at least one separator
    { rv.push_back(cs.substr(start_posn, posn - start_posn));
      start_posn = posn + separator.length();
    }
  }

  return rv;
}

/*! \brief              Read the contents of a file into a single string
    \param  filename    name of file to be read
    \return             contents of file <i>filename</i>
  
    Throws exception if the file does not exist, or if any
    of several bad things happen. Assumes that the file is a reasonable length.
*/
string read_file(const string& filename)
{ FILE* fp { fopen(filename.c_str(), "rb") };

  if (!fp)
  { cerr << ("Cannot open file: "s + filename) << endl;
    throw exception();
  }
  else
    fclose(fp);

// check that the file is not a directory  
  struct stat stat_buffer;

  const int status { ::stat(filename.c_str(), &stat_buffer) };

  if (status)
  { cerr << ("Unable to stat file: "s + filename) << endl;;
    throw exception();
  }

  const bool is_directory { ( (stat_buffer.st_mode bitand S_IFDIR) != 0 ) };

  if (is_directory)
  { cerr << (filename + " is a directory"s) << endl;
    throw exception();
  }

  ifstream file { filename };
  string   str  {std::istreambuf_iterator<char>(file), {} };

  return str;
}

/*! \brief                  Remove all instances of a particular char from a string
    \param  cs              original string
    \param  char_to_remove  character to be removed from <i>cs</i>
    \return                 <i>cs</i> with all instances of <i>char_to_remove</i> removed
*/
string remove_char(const string& cs, const char char_to_remove)
{ string rv { cs };

  rv.erase( remove(rv.begin(), rv.end(), char_to_remove), rv.end() );

  return rv;
} 

/*! \brief      Transform a string
    \param  cs  original string
    \param  pf  pointer to transformation function
    \return     <i>cs</i> with the transformation <i>*pf</i> applied
*/
string transform_string(const string& cs, int(*pf)(int))
{ string rv = cs;
  
  transform(rv.begin(), rv.end(), rv.begin(), pf);
  
  return rv;
}

/*! \brief      Transform an FCC date to an ISO 8601 extended-format date
    \param  cs  original string
    \param  pf  pointer to transformation function
    \return     <i>cs</i> with the transformation <i>*pf</i> applied
*/
string transform_date(const string& us_date)
{ if (us_date.size() != 10)
  { cout << "Error in date: *" << us_date << "*" << endl;
    exit(-1);
  }
  
  return ( us_date.substr(6, 4) + "-"s + us_date.substr(0, 2) + "-"s + us_date.substr(3, 2) );
}

/*! \brief          Is one call earlier than another, according to classical callsign sort order?
    \param  call1   first call
    \param  call2   second call
    \return         whether <i>call1</i> appears before <i>call2</i> in callsign sort order
*/
bool compare_calls(const string& call1, const string& call2)
{
/* callsign sort order

   changes to ordinary sort order:
    '0' is the highest digit
    numbers sort after letters
    '/' comes after all digits and letters
    '-' comes after all digits and letters and '/'; here because names of log files, at least as used by CQ, use "-" in place of of "/"
 */
  const auto compchar = [] (const char c1, const char c2)
    { if (c1 == c2)
        return false;

      if (c2 == '/')
        return true;

      if (c1 == '/')
        return false;

      if ( (c1 == '/') )
        return false;

      if ( (c2 == '/') )
        return true;

      if (isalpha(c1) and isdigit(c2))
        return true;

      if (isdigit(c1) and isalpha(c2))
        return false;

      if (isdigit(c1) and isdigit(c2))
      { if (c1 == '0')
          return false;

        if (c2 == '0')
          return true;
      }

      return (c1 < c2);
    };

  const size_t l1           { call1.size() };
  const size_t l2           { call2.size() };
  const size_t n_to_compare { min(l1, l2) };

  size_t index { 0 };

  while (index < n_to_compare)
  { if (call1[index] != call2[index])
      return compchar(call1[index], call2[index]);

    index++;
  }

  return (l1 < l2);
}
  
/*! \brief      Remove all instances of a specific leading character
    \param  cs  original string
    \param  c   leading character to remove (if present)
    \return     <i>cs</i> with any leading octets with the value <i>c</i> removed
*/
string remove_leading(const string& cs, const char c)
{ const size_t posn { cs.find_first_not_of(create_string(c)) };
  const string rv   { cs.substr(posn) };
  
  return rv;
}

/*! \brief      Remove all instances of a specific trailing character
    \param  cs  original string
    \param  c   trailing character to remove (if present)
    \return     <i>cs</i> with any trailing octets with the value <i>c</i> removed
*/
string remove_trailing(const string& cs, const char c)
{ string rv { cs };

  while (rv.length() && (rv[rv.length() - 1] == c))
    rv = rv.substr(0, rv.length() - 1);
  
  return rv;
}

/// return the current date as YYYY-MM-DD
string date_string(void)
{ constexpr size_t TIME_BUF_LEN { 26 };

  const time_t now { ::time(NULL) };            // get the time from the kernel

  struct tm structured_time;

  gmtime_r(&now, &structured_time);         // convert to UTC

  array<char, TIME_BUF_LEN> buf;                           // buffer to hold the ASCII date/time info; see man page for gmtime()

  asctime_r(&structured_time, buf.data());                     // convert to ASCII

  auto pad_leftz = [] (const string& str, const int n) { if (str.size() == static_cast<size_t>(n))
                                                             return str;
                                                             
                                                          return "0"s + str;        // assume length of str is 1
                                                       };
                                                        
  const string _date { to_string(structured_time.tm_year + 1900) + "-"s + pad_leftz(to_string(structured_time.tm_mon + 1), 2) + "-"s + pad_leftz(to_string(structured_time.tm_mday), 2) };   // yyyy-mm-dd

  return _date;
}
