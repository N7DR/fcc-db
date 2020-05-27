// Released under the GNU Public License, version 2
//   see: https://www.gnu.org/licenses/gpl-2.0.html

// Principal author: N7DR

// Copyright owners:
//    N7DR

/*! \file   fcc-db.cpp

    Program to process and merge FCC .DAT files, creating a combined
    file that is sent to stdout 
*/

// fcc-db [temporary-directory]

#include "fcc-db.h"

#include <future>
#include <map>
#include <thread>

using namespace std;

/*! \brief      Helper function to return an object of specified type from a string
    \param  fn  filename
    \return     object of type <i>T</i> constructed from <i>fn</i>
*/
template <typename T>
T get_value(const string& fn)
{ return T(fn); }

/// here we go
int main(int argc, char** argv)
{ string dir { ( (argc > 1) ? argv[1] : "./"s) };    // is there a directory on the command line?

  if (dir[dir.size() - 1] != '/')                    // add the trailing slash if necessary
    dir += '/';
    
// extract the data from the files; it's barely worth doing it in parallel, but we might as well....
// don't bother with HS, LA, SC and SF files; they don't seem to contain very interesting data
  future<AM_FILE> am_file_future { async(std::launch::async, get_value<AM_FILE>, dir + "AM.dat"s) };
  future<CO_FILE> co_file_future { async(std::launch::async, get_value<CO_FILE>, dir + "CO.dat"s) };
  future<EN_FILE> en_file_future { async(std::launch::async, get_value<EN_FILE>, dir + "EN.dat"s) };
  future<HD_FILE> hd_file_future { async(std::launch::async, get_value<HD_FILE>, dir + "HD.dat"s) };
 
  fcc_file outfile;
  
// create a closure to append data to the file 
  auto append_data = [](fcc_file& outfile, const auto& data_file)
  { for (const auto& in_record : data_file)
      outfile += in_record;
  };
   
// append the relevant data from the four .dat files that seem to contain data we want
  append_data(outfile, am_file_future.get());
  append_data(outfile, co_file_future.get());
  append_data(outfile, en_file_future.get());
  append_data(outfile, hd_file_future.get());
    
// all done; now output it in callsign order
  cout << outfile.to_string() << endl;      // there; that was easy, wasn't it?
}

/// add an AM_RECORD to the file
void fcc_file::operator+=(const AM_RECORD& amr)
{ const string key { amr[AM::ID] };

  FCC_RECORD& rec = (*this)[key];
  
// we have a record which may or may not be empty; give it the ID if necessary
  if (rec[FCC::ID].empty())
    rec[FCC::ID] = key;
    
  rec[FCC::CALLSIGN]                   = amr[AM::CALLSIGN];
  rec[FCC::OPERATOR_CLASS]             = amr[AM::OPERATOR_CLASS];
  rec[FCC::GROUP_CODE]                 = amr[AM::GROUP_CODE];
  rec[FCC::REGION_CODE]                = amr[AM::REGION_CODE];
  rec[FCC::TRUSTEE_CALLSIGN]           = amr[AM::TRUSTEE_CALLSIGN];
  rec[FCC::TRUSTEE_INDICATOR]          = amr[AM::TRUSTEE_INDICATOR];
  rec[FCC::SYSTEMATIC_CALLSIGN_CHANGE] = amr[AM::SYSTEMATIC_CALLSIGN_CHANGE];
  rec[FCC::VANITY_CALLSIGN_CHANGE]     = amr[AM::VANITY_CALLSIGN_CHANGE];
  rec[FCC::VANITY_RELATIONSHIP]        = amr[AM::VANITY_RELATIONSHIP];
  rec[FCC::PREVIOUS_CALLSIGN]          = amr[AM::PREVIOUS_CALLSIGN];
  rec[FCC::PREVIOUS_OPERATOR_CLASS]    = amr[AM::PREVIOUS_OPERATOR_CLASS];
  rec[FCC::TRUSTEE_NAME]               = amr[AM::TRUSTEE_NAME];
}

/// add a CO_RECORD to the file
void fcc_file::operator+=(const CO_RECORD& cor)
{ const string key { cor[CO::ID] };

// look to see if this key exists
  if (this->find(key) == end())
  { cerr << "CO key " << key << " not in FCC file " << endl;
    exit(-1);
  }

  FCC_RECORD& rec = (*this)[key];
  
  if (rec[FCC::CALLSIGN] != cor[CO::CALLSIGN])
  { cerr << "CO callsign " << cor[CO::CALLSIGN] << " does not match callsign in FCC file: " << rec[FCC::CALLSIGN] << endl;
    exit(-1);
  }    
  
  rec[FCC::COMMENT_DATE] = cor[CO::COMMENT_DATE];
  rec[FCC::DESCRIPTION]  = cor[CO::DESCRIPTION];
  rec[FCC::CO_STATUS_CODE]  = cor[CO::STATUS_CODE];
  
  if (!(cor[CO::STATUS_DATE].empty()))
    rec[FCC::CO_STATUS_DATE] = transform_date(cor[CO::STATUS_DATE]);
}

/// add an EN_RECORD to the file
void fcc_file::operator+=(const EN_RECORD& enr)
{ const string key { enr[EN::ID] };

// look to see if this key exists; for some EN records, there is no extant key;
// probably best to skip the EN record in that case, because we could end up in a horribly
// inconsistent state because the FCC doesn't seem to maintain internal consistency
// amongst the .dat files. With any luck, by the following week this record will be fixed 
// as the state should have changed.
  if (this->find(key) == end())
  { //cout << "EN key " << key << " not in FCC file " << endl;
    return;
  }

  FCC_RECORD& rec = (*this)[key];
  
// we have a record which may or may not be empty; give it the ID if necessary
//  if (rec[FCC::ID].empty())
//    rec[FCC::ID] = key;
  
//  if (rec[FCC::CALLSIGN].empty())               // if we just created the record
//    rec[FCC::CALLSIGN] = enr[EN::CALLSIGN];
  
  if (rec[FCC::CALLSIGN] != enr[EN::CALLSIGN])  // treat this as a fatal error
  { cout << "EN callsign " << enr[EN::CALLSIGN] << " does not match callsign in FCC file: " << rec[FCC::CALLSIGN] << endl;
    exit(-1);
  }    
  
  rec[FCC::ENTITY_NAME]               = enr[EN::ENTITY_NAME];
  rec[FCC::FIRST_NAME]                = enr[EN::FIRST_NAME];
  rec[FCC::MIDDLE_INITIAL]            = enr[EN::MIDDLE_INITIAL];
  rec[FCC::LAST_NAME]                 = enr[EN::LAST_NAME];
  rec[FCC::SUFFIX]                    = enr[EN::SUFFIX];
  rec[FCC::PHONE]                     = enr[EN::PHONE];
  rec[FCC::FAX]                       = enr[EN::FAX];
  rec[FCC::EMAIL]                     = enr[EN::EMAIL];
  rec[FCC::STREET_ADDRESS]            = enr[EN::STREET_ADDRESS];
  rec[FCC::CITY]                      = enr[EN::CITY];
  rec[FCC::STATE]                     = enr[EN::STATE];
  rec[FCC::ZIP_CODE]                  = enr[EN::ZIP_CODE];
  rec[FCC::PO_BOX]                    = enr[EN::PO_BOX];
  rec[FCC::ATTENTION_LINE]            = enr[EN::ATTENTION_LINE];
  rec[FCC::FRN]                       = enr[EN::FRN];
  rec[FCC::APPLICANT_TYPE_CODE]       = enr[EN::APPLICANT_TYPE_CODE];
  rec[FCC::APPLICANT_TYPE_CODE_OTHER] = enr[EN::APPLICANT_TYPE_CODE_OTHER];
  rec[FCC::EN_STATUS_CODE]            = enr[EN::STATUS_CODE];
  
  if (!(enr[EN::STATUS_DATE].empty()))
    rec[FCC::EN_STATUS_DATE] = transform_date(enr[EN::STATUS_DATE]);
}

/// add an HD_RECORD to the file
void fcc_file::operator+=(const HD_RECORD& hdr)
{ const string key { hdr[HD::ID] };

// look to see if this key exists; for some HD records, there is no extant key;
// probably best to skip the HD record in that case, because we could end up in a horribly
// inconsistent state because the FCC doesn't seem to maintain internal consistency
// amongst the .dat files. With any luck, by the following week, this record will be fixed 
// as the state should have changed.
  if (this->find(key) == end())
  { //cerr << "HD key " << key << " not in FCC file " << endl;
    return;
  }

  FCC_RECORD& rec = (*this)[key];
  
  if (rec[FCC::CALLSIGN] != hdr[HD::CALLSIGN])
  { cout << "HD callsign " << hdr[HD::CALLSIGN] << " does not match callsign in FCC file: " << rec[FCC::CALLSIGN] << endl;
    exit(-1);
  }    

  rec[FCC::LICENSE_STATUS]     = hdr[HD::LICENSE_STATUS];
  rec[FCC::RADIO_SERVICE_CODE] = hdr[HD::RADIO_SERVICE_CODE];
  
  if (!(hdr[HD::GRANT_DATE].empty()))
    rec[FCC::GRANT_DATE] = transform_date(hdr[HD::GRANT_DATE]);

  if (!(hdr[HD::EXPIRED_DATE].empty()))
    rec[FCC::EXPIRED_DATE] = transform_date(hdr[HD::EXPIRED_DATE]);

  if (!(hdr[HD::CANCELLATION_DATE].empty()))
    rec[FCC::CANCELLATION_DATE] = transform_date(hdr[HD::CANCELLATION_DATE]);
  
  rec[FCC::ELIGIBILITY_RULE_NUM] = hdr[HD::ELIGIBILITY_RULE_NUM];
  rec[FCC::REVOKED]              = hdr[HD::REVOKED];
  rec[FCC::CONVICTED]            = hdr[HD::CONVICTED];
  rec[FCC::ADJUDGED]             = hdr[HD::ADJUDGED];
  rec[FCC::EFFECTIVE_DATE]       = hdr[HD::EFFECTIVE_DATE];
  rec[FCC::LAST_ACTION_DATE]     = hdr[HD::LAST_ACTION_DATE];
  rec[FCC::LICENSEE_NAME_CHANGE] = hdr[HD::LICENSEE_NAME_CHANGE];
}

/// convert to a string
const string fcc_file::to_string(void) const
{
// put it all in callsign order; this takes a while, but it's simple and fast enough
// note that if multiple records have the same call, only the first is used; no attempt
// is made to merge the records or to decide amongst them
  map<string, FCC_RECORD, decltype(&compare_calls)> output_map(compare_calls);

  for (auto cit = this->cbegin(); cit != this->cend(); ++cit)
  { const FCC_RECORD& rec = cit->second;
  
    output_map.insert( { rec[FCC::CALLSIGN], rec } );
  }
    
// it's now in the right order; build the string and return it
  string rv;
  
  for (auto cit = output_map.cbegin(); cit != output_map.cend(); ++cit)
    rv += ( (cit->second).to_string() + '\n' );
    
  return rv;
}
