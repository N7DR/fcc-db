// Released under the GNU Public License, version 2
//   see: https://www.gnu.org/licenses/gpl-2.0.html

// Principal author: N7DR

// Copyright owners:
//    N7DR

#ifndef FCC_DB_H
#define FCC_DB_H

/*! \file   fcc-db.h

    Headers for program to process and merge FCC .DAT files 
*/

#include "fcc-strings.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <stdexcept>
#include <unordered_map>

using namespace std::string_literals;

// -----------  dat_record  ----------------

/*!     \class dat_record
        \brief generic record in an FCC .DAT file
        
        It's rather easier to do this as a HAS A instead of an IS A
*/

template<typename T>
class dat_record
{
protected:

  std::array<std::string, static_cast<long unsigned int>(T::N_FIELDS)>  _data;
  
public:

// default constructor
  dat_record(void) = default;

// construct from string
  dat_record(const std::string& str)
  { if (str.empty())
      throw std::range_error("Empty record string");
      
    std::vector<std::string> vec { split_string(to_upper(str), "|"s) };    // force upper case
  
// if the last character of the line is a "|", then there is an implied empty string at the end
    if (str[str.length() - 1] == '|')
      vec.push_back(std::string());
  
    if (vec.size() != static_cast<size_t>(T::N_FIELDS))
      throw std::range_error("Incorrect number of fields in record string: "s + str + "; should be " + ::to_string(static_cast<size_t>(T::N_FIELDS)) + "; found " + ::to_string(vec.size()));
  
    std::move(vec.begin(), vec.end(), _data.begin());   
  }
  
/// access the string at a particular field number
  inline const std::string operator[](const T index) const
    { return _data.at(static_cast<size_t>(index)); }

/// access the string at a particular field number    
  inline std::string& operator[](const T index)
    { return _data.at(static_cast<size_t>(index)); }

/// convert to a string: FIELD_1|FIELD_2|FIELD_3...    
  const std::string to_string(void) const
  { std::string rv;
  
    for (size_t n = 0; n < static_cast<size_t>(T::N_FIELDS); ++n)
      rv += ( _data[n] + ( n < (static_cast<size_t>(T::N_FIELDS) - 1) ? "|"s : ""s ) );
      
    return rv;
  }
};

// -----------  dat_file  ----------------

/*!     \class dat_file
        \brief generic FCC .DAT file
*/

template<typename T>
class dat_file : public std::vector<dat_record<T>>
{
protected:

public:

// default constructor
  dat_file(void) = default;
  
// construct from filename
  dat_file(const std::string& fn)
  { std::vector<std::string> lines { to_lines(remove_char(read_file(fn), '\r')) };      // remove CR characters
  
// the FCC sometimes puts new lines inside a record, so instead of a quick run through
// the lines with a lambda, we have to proceed with ridiculous caution
    for (size_t n = 0; n < lines.size(); ++n)
    { std::string this_record { lines[n] };
        
      while ( (std::count(this_record.begin(), this_record.end(), '|') < (static_cast<int>(T::N_FIELDS) - 1) ) and (n < lines.size() - 1) ) 
        this_record += ("<LF>"s + lines[++n]);        // convert any LFs to strings indicating the presence of an LF
        
      try
      { dat_record<T> dr { remove_peripheral_spaces(this_record) };    // this is the line that does all the work
        
        this->push_back(dr);            // add record to the object
      }
        
      catch (const std::range_error& e)
      { std::cerr << "Caught exception while processing file: " << fn << std::endl;
        throw;
      }
    }  
  }
};

/* define the contents of each FCC .DAT file; I note that I haven't been able to find definitive
   statements defining the actual meaning of many of the contents of fields.

   The best resource, for what it's worth, seems to be:
       https://www.fcc.gov/sites/default/files/pubacc_uls_code_def_02162017.txt
       https://www.fcc.gov/sites/default/files/pubacc_uls_code_def_02162017.pdf
   but that still leaves many fields requiring guesswork (or a shrug of the shoulders)
   as to their meaning.
   
   The above has been replaced by:
     https://www.fcc.gov/sites/default/files/uls_code_definitions_20201222.txt
  There is no longer a PDF version.
   
   The document:
       https://www.fcc.gov/sites/default/files/public_access_database_definitions_v3.pdf
   defines the field names for the records in the .DAT files. That at least appears to
   be complete and accurate.
   
   https://www.fcc.gov/wireless/data/public-access-files-database-downloads includes links
   to various possibly-useful resources
*/

// AM -------------------------------------------------------

/*
From https://www.fcc.gov/sites/default/files/public_access_database_definitions_v3.pdf:

Amateur
Position Data Element Definition
[AM]
1   Record Type [AM]            char(2)
2   Unique System Identifier    numeric(9,0)
3   ULS File Number             char(14)
4   EBF Number                  varchar(30)
5   Call Sign                   char(10)
6   Operator Class              char(1)
7   Group Code                  char(1)
8   Region Code                 tinyint
9   Trustee Call Sign           char(10)
10  Trustee Indicator           char(1)
11  Physician Certification     char(1)
12  VE Signature                char(1)
13  Systematic Call Sign Change char(1)
14  Vanity Call Sign Change     char(1)
15  Vanity Relationship         char(12)
16  Previous Call Sign          char(10)
17  Previous Operator Class     char(1)
18  Trustee Name                varchar(50) 
*/

enum class AM { RECORD_TYPE = 0,
                ID,
                ULS_NUMBER,
                EBF_NUMBER,
                CALLSIGN,
                OPERATOR_CLASS,
                GROUP_CODE,
                REGION_CODE,
                TRUSTEE_CALLSIGN,
                TRUSTEE_INDICATOR,
                PHYSICIAN_CERTIFICATION,
                VE_SIGNATURE,
                SYSTEMATIC_CALLSIGN_CHANGE,
                VANITY_CALLSIGN_CHANGE,
                VANITY_RELATIONSHIP,
                PREVIOUS_CALLSIGN,
                PREVIOUS_OPERATOR_CLASS,
                TRUSTEE_NAME,
                N_FIELDS 
              };

using AM_RECORD = dat_record<AM>;
using AM_FILE   = dat_file<AM>;

// CO -------------------------------------------------------

/*
From https://www.fcc.gov/sites/default/files/public_access_database_definitions_v3.pdf:

Comments
Position Data Element Definition
[CO]
1   Record Type [CO]            char(2)
2   Unique System Identifier    numeric(9,0)
3   ULS File Number             char(14)
4   Call Sign                   char(10)
5   Comment Date                mm/dd/yyyy
6   Description                 varchar(255)
7   Status Code                 char(1)
8   Status Date                 mm/dd/yyyy
*/

enum class CO { RECORD_TYPE = 0,
                ID,
                ULS_NUMBER,
                CALLSIGN,
                COMMENT_DATE,
                DESCRIPTION,
                STATUS_CODE,
                STATUS_DATE,
                N_FIELDS 
              };

using CO_RECORD = dat_record<CO>;
using CO_FILE   = dat_file<CO>;;

// EN -------------------------------------------------------

/*
From https://www.fcc.gov/sites/default/files/public_access_database_definitions_v5.pdf:

Entity
Position Data Element Definition
[EN]
1   Record Type [EN]                char(2)
2   Unique System Identifier        numeric(9,0)
3   ULS File Number                 char(14)
4   EBF Number                      varchar(30)
5   Call Sign                       char(10)
6   Entity Type                     char(2)
7   Licensee ID                     char(9)
8   Entity Name                     varchar(200)
9   First Name                      varchar(20)
10  MI                              char(1)
11  Last Name                       varchar(20)
12  Suffix                          char(3)
13  Phone                           char(10)
14  Fax                             char(10)
15  Email                           varchar(50)
16  Street Address                  varchar(60)
17  City                            varchar(20)
18  State                           char(2)
19  Zip Code                        char(9)
20  PO Box                          varchar(20)
21  Attention Line                  varchar(35)
22  SGIN                            char(3)
23  FCC Registration Number (FRN)   char(10)
24  Applicant Type Code             char(1)
25  Applicant Type Code Other       char(40)
26  Status Code                     char(1)
27  Status Date                     mm/dd/yyyy
28  3.7 GHz License Type            char(1)
29  Linked Unique System Identifier numeric(9,0)
30  Linked Call Sign                 char(10)
*/

enum class EN { RECORD_TYPE = 0,
                ID,
                ULS_NUMBER,
                EBF_NUMBER,
                CALLSIGN,
                ENTITY_TYPE,
                LICENSE_ID,
                ENTITY_NAME,
                FIRST_NAME,
                MIDDLE_INITIAL,
                LAST_NAME,
                SUFFIX,
                PHONE,
                FAX,
                EMAIL,
                STREET_ADDRESS,
                CITY,
                STATE,
                ZIP_CODE,
                PO_BOX,
                ATTENTION_LINE,
                SGIN,
                FRN,
                APPLICANT_TYPE_CODE,
                APPLICANT_TYPE_CODE_OTHER,
                STATUS_CODE,
                STATUS_DATE,
                LICENSE_TYPE_37,
                LINKED_ID,
                LINKED_CALLSIGN,
                N_FIELDS 
              };

using EN_RECORD = dat_record<EN>;
using EN_FILE   = dat_file<EN>;

// HD -------------------------------------------------------

/*
From https://www.fcc.gov/sites/default/files/public_access_database_definitions_v6.pdf:

Application / License Header
Position Data Element Definition
[HD]
1   Record Type [HD]                            char(2)
2   Unique System Identifier                    numeric(9,0)
3   ULS File Number                             char(14)
4   EBF Number                                  varchar(30)
5   Call Sign                                   char(10)
6   License Status                              char(1)
7   Radio Service Code                          char(2)
8   Grant Date                                  mm/dd/yyyy
9   Expired Date                                mm/dd/yyyy
10  Cancellation Date                           mm/dd/yyyy
11  Eligibility Rule Num                        char(10)
12  Reserved                                    char(1)
13  Alien                                       char(1)
14  Alien Government                            char(1)
15  Alien Corporation                           char(1)
16  Alien Officer                               char(1)
17  Alien Control                               char(1)
18  Revoked                                     char(1)
19  Convicted                                   char(1)
20  Adjudged                                    char(1)
21  Reserved                                    char(1)
22  Common Carrier                              char(1)
23  Non Common Carrier                          char(1)
24  Private Comm                                char(1)
25  Fixed                                       char(1)
26  Mobile                                      char(1)
27  Radiolocation                               char(1)
28  Satellite                                   char(1)
29  Developmental or STA or Demonstration       char(1)
30  InterconnectedService                       char(1)
31  Certifier First Name                        varchar(20)
32  Certifier MI                                char(1)
33  Certifier Last Name                         varchar(20)
34  Certifier Suffix                            char(3)
35  Certifier Title                             char(40)
36  Female                                      char(1)
37  Black or African-American                   char(1)
38  Native American                             char(1)
39  Hawaiian                                    char(1)
40  Asian                                       char(1)
41  White                                       char(1)
42  Hispanic                                    char(1)
43  Effective Date                              mm/dd/yyyy
44  Last Action Date                            mm/dd/yyyy
45  Auction ID                                  integer
46  Broadcast Services - Regulatory Status      char(1)
47  Band Manager - Regulatory Status            char(1)
48  Broadcast Services - Type of Radio Service  char(1)
49  Alien Ruling                                char(1)
50  Licensee Name Change                        char(1)
51  Whitespace Indicator                        char(1)
52  Operation/Performance Requirement Choice    char(1)
53  Operation/Performance Requirement Answer    char(1)
54  Discontinuation of Service                  char(1)
55  Regulatory Compliance                       char(1)
56  900 MHz Eligibility Certification           char(1)
57  900 MHz Transition Plan Certification       char(1)
58  900 MHz Return Spectrum Certification       char(1)
59  900 MHz Payment Certification               char(1)
*/

enum class HD { RECORD_TYPE = 0,
                ID,
                ULS_NUMBER,
                EBF_NUMBER,
                CALLSIGN,
                LICENSE_STATUS,
                RADIO_SERVICE_CODE,
                GRANT_DATE,
                EXPIRED_DATE,
                CANCELLATION_DATE,
                ELIGIBILITY_RULE_NUM,
                RESERVED_1,
                ALIEN,
                ALIEN_GOVERNMENT,
                ALIEN_CORPORATION,
                ALIEN_OFFICER,
                ALIEN_CONTROL,
                REVOKED,
                CONVICTED,
                ADJUDGED,
                RESERVED_2,
                COMMON_CARRIER,
                NON_COMMON_CARRIER,
                PRIVATE_COMM,
                FIXED,
                MOBILE,
                RADIOLOCATION,
                SATELLITE,
                DEVELOPMENTAL_STA_DEMONSTRATION,
                INTERCONNECTED_SERVICE,
                CERTIFIER_FIRST_NAME,
                CERTIFIER_MIDDLE_INITIAL,
                CERTIFIER_LAST_NAME,
                CERTIFIER_SUFFIX,
                CERTIFIER_TITLE,
                FEMALE,
                BLACK_AFRICAN_AMERICAN,
                NATIVE_AMERICAN,
                HAWAIIAN,
                ASIAN,
                WHITE,
                HISPANIC,
                EFFECTIVE_DATE,
                LAST_ACTION_DATE,
                AUCTION_ID,
                BROADCAST_SERVICES_REGULATORY_STATUS,
                BAND_MANAGER_REGULATORY_STATUS,
                BROADCAST_SERVICES_SERVICE_TYPE,
                ALIEN_RULING,
                LICENSEE_NAME_CHANGE,
                WHITESPACE_INDICATOR,
                REQUIREMENT_CHOICE,
                REQUIREMENT_ANSWER,
                DISCONTINUED_SERVICE,
                REGULATORY_COMPLIANCE,
                ELIGIBILITY_900_MHZ,
                TRANSITION_PLAN_900_MHZ,
                RETURN_SPRCTRUM_900_MHZ,
                PAYMENT_900_MHZ,
                N_FIELDS 
              };

using HD_RECORD = dat_record<HD>;
using HD_FILE   = dat_file<HD>;

// HS -------------------------------------------------------

/*
From https://www.fcc.gov/sites/default/files/public_access_database_definitions_v4.pdf:

History
Position Data Element Definition
[HS]
1   Record Type [HS]            char(2)
2   Unique System Identifier    numeric(9,0)
3   ULS File Number             char(14)
4   Call Sign                   char(10)
5   Log Date                    mm/dd/yyyy
6   Code                        char(6)
*/

enum class HS { RECORD_TYPE = 0,
                ID,
                ULS_NUMBER,
                CALLSIGN,
                LOG_DATE,
                CODE,
                N_FIELDS 
              };
              
using HS_RECORD = dat_record<HS>;
using HS_FILE   = dat_file<HS>;

// LA -------------------------------------------------------

/*
From https://www.fcc.gov/sites/default/files/public_access_database_definitions_v4.pdf:

License Attachment
Position Data Element Definition
[LA]
1   Record Type [LA]            char(2)
2   Unique System Identifier    numeric(9,0)
3   Call Sign                   char(10)
4   Attachment Code             char(1)
5   Attachment Description      varchar(60)
6   Attachment Date             mm/dd/yyyy
7   Attachment File Name        varchar(60)
8   Action Performed            char(1)
*/

enum class LA { RECORD_TYPE = 0,
                ID,
                CALLSIGN,
                ATTACHMENT_CODE,
                ATTACHMENT_DESCRIPTION,
                ATTACHMENT_DATE,
                ATTACHMENT_FILENAME,
                ACTION_PERFORMED,
                N_FIELDS 
              };

using LA_RECORD = dat_record<LA>;
using LA_FILE   = dat_file<LA>;

// SC -------------------------------------------------------

/*
From https://www.fcc.gov/sites/default/files/public_access_database_definitions_v4.pdf:

Special Condition
Position Data Element Definition
[SC]
1   Record Type [SC]            char(2)
2   Unique System Identifier    numeric(9,0)
3   ULS File Number             char(14)
4   EBF Number                  varchar(30)
5   Call Sign                   char(10)
6   Special Condition Type      char(1)
7   Special Condition Code      int
8   Status Code                 char(1)
9   Status Date                 mm/dd/yyyy
*/

enum class SC { RECORD_TYPE = 0,
                ID,
                ULS_NUMBER,
                EBF_NUMBER,
                CALLSIGN,
                SPECIAL_CONDITION_TYPE,
                SPECIAL_CONDITION_CODE,
                STATUS_CODE,
                STATUS_DATE,
                N_FIELDS 
              };

using SC_RECORD = dat_record<SC>;
using SC_FILE   = dat_file<SC>;

// SF -------------------------------------------------------

/*
From https://www.fcc.gov/sites/default/files/public_access_database_definitions_v4.pdf:

License Free Form Special Condition
Position Data Element Definition
[SF]
1   Record Type [SF]                    char(2)
2   Unique System Identifier            numeric(9,0)
3   ULS File Number                     char(14)
4   EBF Number                          varchar(30)
5   Call Sign                           char(10)
6   License Free Form Type              char(1)
7   Unique License Free Form Identifier numeric(9,0)
8   Sequence Number                     integer
9   License Free Form Condition         varchar(255)
10  Status Code                         char(1)
11  Status Date                         mm/dd/yyyy
*/

enum class SF { RECORD_TYPE = 0,
                ID,
                ULS_NUMBER,
                EBF_NUMBER,
                CALLSIGN,
                LICENSE_FREEFORM_TYPE,
                UNIQUE_LICENSE_FREEFORM_ID,
                SEQUENCE_NUMBER,
                LICENSE_FREEFORM_CONDITION,
                STATUS_CODE,
                STATUS_DATE,
                N_FIELDS 
              };

using SF_RECORD = dat_record<SF>;
using SF_FILE   = dat_file<SF>;

/* the fields that go into the output file; these have the same names as the fields in the 
  .DAT files except where those names are duplicated in different .DAT files, in which case 
  the name is here preceded by XX_, where the XX indicates the .DAT file that provides the field
*/
enum class FCC { ID = 0,                        // unique system identifier
                 CALLSIGN,
                 OPERATOR_CLASS,                // called "Operator Class Code" in https://www.fcc.gov/sites/default/files/pubacc_uls_code_def_02162017.txt
                 GROUP_CODE,                    // I have no idea what this is, but it does take different values
                 REGION_CODE,                   // basically, call area; REGION_CODE = 11 => Alaska; REGION_CODE 12 => US Virgin Islands
                 TRUSTEE_CALLSIGN,
                 TRUSTEE_INDICATOR,             // possibly means that this person is a trustee for another call
                 SYSTEMATIC_CALLSIGN_CHANGE,    // I don't know what this means
                 VANITY_CALLSIGN_CHANGE,        // I don't know what this means
                 VANITY_RELATIONSHIP,
                 PREVIOUS_CALLSIGN,
                 PREVIOUS_OPERATOR_CLASS,
                 TRUSTEE_NAME,
                 COMMENT_DATE,
                 DESCRIPTION,
                 CO_STATUS_CODE,
                 CO_STATUS_DATE,
                 ENTITY_NAME,
                 FIRST_NAME,
                 MIDDLE_INITIAL,
                 LAST_NAME,
                 SUFFIX,
                 PHONE,
                 FAX,
                 EMAIL,
                 STREET_ADDRESS,
                 CITY,
                 STATE,
                 ZIP_CODE,
                 PO_BOX,
                 ATTENTION_LINE,
                 FRN,
                 APPLICANT_TYPE_CODE,
                 APPLICANT_TYPE_CODE_OTHER,
                 EN_STATUS_CODE,
                 EN_STATUS_DATE,
                 LICENSE_STATUS,
                 RADIO_SERVICE_CODE,
                 GRANT_DATE,
                 EXPIRED_DATE,
                 CANCELLATION_DATE,
                 ELIGIBILITY_RULE_NUM,
                 REVOKED,
                 CONVICTED,
                 ADJUDGED,
                 EFFECTIVE_DATE,            // I don't really know what this means
                 LAST_ACTION_DATE,          // I don't really know what this means
                 LICENSEE_NAME_CHANGE,
                 LINKED_ID,
                 LINKED_CALLSIGN,
                 N_FIELDS 
               };
              
using FCC_RECORD = dat_record<FCC>;
using FCC_FILE   = dat_file<FCC>;

// -----------  fcc_file  ----------------

/*!     \class fcc_file
        \brief file created from merging .DAT files
*/

class fcc_file : public std::unordered_map<std::string, FCC_RECORD>       // The FCC seems to recommend using ID as the key,
                                                                          // although (of course) they really aren't clear.
                                                                          // The callsign might be another one to try, although
                                                                          // callsigns are relatively transient and it's easy
                                                                          // to believe that all kinds of problems might occur
                                                                          // by using that field. Anyway, ID it is until it is proven
                                                                          // that something else would be better
{
protected:
      
public:

/// add an AM_RECORD to the file
  void operator+=(const AM_RECORD& amr);
  
/// add a CO_RECORD to the file  
  void operator+=(const CO_RECORD& cor);
  
/// add an EN_RECORD to the file  
  void operator+=(const EN_RECORD& enr);
  
/// add an HD_RECORD to the file  
  void operator+=(const HD_RECORD& hdr);
  
/// convert to a string
  const std::string to_string(void) const;
};

#endif    // FCC_DB_H
