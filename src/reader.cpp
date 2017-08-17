#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cassert>
#include <array>
#include <algorithm>
#include <cctype>
#include <csvf/reader.hpp>

namespace
{
    template<typename T>
    std::basic_ostream<T>& operator<<(std::basic_ostream<T>& stream, csvf::reader::quote_rule rule)
    {
        switch(rule)
        {
        case csvf::reader::quote_rule::doubled:
            stream<<"doubled";
            break;
        case csvf::reader::quote_rule::escaped:
            stream<<"escaped";
            break;
        case csvf::reader::quote_rule::verbatim:
            stream<<"verbatim";
            break;
        case csvf::reader::quote_rule::none:
            stream<<"none";
            break;
        default:
            std::logic_error("internal error");
        }
        return stream;
    }

    std::string sep2str(char sep)
    {
        if (sep==' ') return "blank";
        else if(sep=='\t') return "\\t";
        else return std::string(1,sep);
    }
}

namespace csvf
{
    reader::~reader()
    {
        m_file.close();
    }

    reader& reader::strip_if_bom()
    {
        assert(m_begin && m_end);
        if (m_verbose)
            std::cout << "detect and skip BOM" << std::endl;
        if(get_file_size() >= 3 &&
           std::memcmp(m_begin, "\xEF\xBB\xBF", 3) == 0)
        {
            m_begin += 3;
            if (m_verbose)
                std::cout<<" UTF-8 byte order mark EF BB BF found"
                    " at the start of the file and skipped."
                         <<std::endl;
        }
        else if(get_file_size() >= 4 &&
                std::memcmp(m_begin, "\x84\x31\x95\x33", 4) == 0)
        {
            m_begin += 4;
            if (m_verbose)
                std::cout<<"  GB-18030 byte order mark 84 31 95 33 "
                    "found at the start of the file and skipped."
                         <<std::endl;
        }
        else if (get_file_size() >= 2 &&
                 m_begin[0] + m_begin[1] == '\xFE' + '\xFF')
        {  // either 0xFE 0xFF or 0xFF 0xFE
            throw std::runtime_error(
                "File is encoded in UTF-16, this encoding is"
                " not supported by fread(). Please recode "
                "the file to UTF-8.");
        }
        if (m_end[-1] == '\x1A')
        {
            m_end--;
            if (m_verbose)
                std::cout<<"  Last byte of input found to be 0x1A "
                    "(Ctrl+Z) and removed.\n"<<std::endl;
        }
        m_pos = m_begin;
        return *this;
    }

    reader& reader::detect_eol()
    {
        if (m_verbose)
            std::cout<<"detect EOL"<<std::endl;
        m_eol.clear();
        const char *pos = m_begin;
        while (pos<m_end && *pos!='\n' && *pos!='\r')
        {
            if (*pos==m_quote)
                while (++pos<m_end && *pos!=m_quote) {};
            pos++;
        }

        if (pos>=m_end)
        {
            if (m_verbose)
                std::cout<<"  Input ends before any \\r or \\n "
                    "observed. Input will be treated as a single row."
                         <<std::endl;
            m_eol.push_back('\n');
        }
        else
        {
            m_eol.push_back(*pos);
            if(*pos=='\r')
            {
                if(pos+1<m_end && *(pos+1)=='\n')
                {
                    if (m_verbose)
                        std::cout<<"detected eol as \\r\\n (CRLF)"
                                 <<std::endl;
                    m_eol.push_back(*(pos+1));
                }
                else
                {
                    if(pos+1<m_end && *(pos+1)=='\r')
                        throw(std::runtime_error("line ending is\\r\\r\\n"));
                    if (m_verbose)
                        std::cout<<"detected eol as \\r only"
                                 <<std::endl;
                }
            }
            else if (*pos=='\n')
            {
                if(pos+1<m_end && *(pos+1)=='\r')
                {
                    if (m_verbose)
                        std::cout<<"detected eol as \\n\\r"<<std::endl;
                    m_eol.push_back('\r');
                }
                else if(m_verbose)
                    std::cout<<"detected eol as \\n"<<std::endl;
            }
            else
                throw std::logic_error("some logic error in the code");
        }
    }

    reader& reader::strip_space()
    {
        while (m_begin<m_end && isspace(*m_begin)) m_begin++;
        return *this;
    }
    
    reader& reader::detect_sep_quote_rule_nfields()
    {
        if (m_verbose)
            std::cout<<"detect separator, quoting rule, and nFields"
                     <<std::endl;
        // candidate seps in order of preference
        std::vector<char> seps(0);
        if (m_sep=='\0')
            seps = {',' , '|', ';', '\t', ' '};
        else
            seps.push_back(m_sep);
        
        // the most number of lines with the same number of
        // fields, so far
        int topNumLines = 0;

        // how many fields that was to resolve ties
        int topNumFields = 1;

        assert(!m_eol.empty());
        char topSep = m_eol[0];
        
        quote_rule topquote_rule = quote_rule::doubled;
        int topNmax = 1; // maximum number of columns

        // remember where the winning jumpline from jump 0 ends,
        // to know its size excluding header
        const char *firstJumpEnd = nullptr;
        constexpr int JUMPLINES = 100;

        // group info store the number of fields and number of lines
        // for each contiguous group with the same number of fields
        // the first and second element in pairs are number of fields
        // and number of lines respectively.
        std::array<quote_rule,4> rules{
            quote_rule::doubled, quote_rule::escaped,
                quote_rule::verbatim, quote_rule::none};
        for (auto sep : seps)
        {
            m_sep = sep;
            for (auto rule : rules)
            {
                //std::cout<<"sep="<<sep2str(sep)<<" rule="<<rule<<std::endl;
                m_quote_rule = rule;

                // analyse contiguous group for current candidate
                // sep and quote rule
                std::vector<std::pair<int,int>> group_info(0);
                m_pos = m_begin;
                int thisLine=0, lastncol=-1;
                while (m_pos<m_end && thisLine++<JUMPLINES)
                {
                    int thisncol = -1;
                    try
                    {
                        skip_record(thisncol);
                    }
                    catch(std::exception& e)
                    {
                        group_info.clear(); break;
                    }
                    if (thisncol!=lastncol)
                        group_info.emplace_back(thisncol, 0);
                    group_info.back().second++;

                    lastncol = thisncol;
                }

                if (group_info.empty()) continue;
                if (!firstJumpEnd) firstJumpEnd = m_pos;
                bool updated = false;
                int nmax = 0;

                // find the contiguous group with the maximum
                // number of lines
                for (auto info : group_info)
                {
                    if (info.first > nmax) nmax=info.first;
                    if (info.first>1 &&
                        (info.second>topNumLines ||
                          (info.second==topNumLines &&
                           info.first>topNumFields &&
                           sep!=' ')))
                    {
                        topNumLines = info.second;
                        topNumFields = info.first;
                        topSep = sep;
                        topquote_rule = rule;
                        firstJumpEnd = m_pos;
                        updated = true;
                    }
                }
                if (updated) topNmax = nmax;
            }
        }
        if (!firstJumpEnd) throw bad_format("Failed to detect sep");

        m_quote_rule = topquote_rule;
        m_sep = topSep;
        m_pos = m_begin;
        if (m_fill)
            m_nfields = topNmax;
        else
        {
            // find the top line with the consistent number of fields.
            // There might be irregular header lines above it.
            m_nfields = topNumFields;
            int thisLine=-1;
            while (m_pos<m_end && ++thisLine<JUMPLINES)
            {
                const char *pos = m_pos;
                int nFields;
                skip_record(nFields);
                skip_if_eol();
                if (nFields == m_nfields)
                {
                    m_pos = m_begin = pos;
                    break;
                }
            }
        }
    
        if (m_nfields<1)
            throw std::runtime_error("Internal error");
        int nfields;
        const char *pos = m_pos;
        skip_record(nfields);
        m_pos = pos;
        if (!m_fill && nfields!=m_nfields)
            throw std::logic_error("Internal error");

        if (m_verbose)
        {
            std::cout
                <<"detected sep="<<sep2str(topSep)
                <<" quote rule="<<topquote_rule
                <<" top nlines="<<topNumLines
                <<" top nfields="<<topNumFields
                <<std::endl;
        }
    }

    reader& reader::skip_if_white()
    {
        // skip space if it is not separator
        // and skip tab if it is not separator
        if (m_sep==' ')
            while (*m_pos=='\t') m_pos++;
        else if (m_sep=='\t')
            while (*m_pos==' ') m_pos++;
        else
            while (*m_pos==' ' || *m_pos=='\t') m_pos++;
        return *this;
    }

    reader& reader::skip_field_content(
        const char **nwBegin, const char**nwEnd)
    {
        // The current position should be at the begin of a field
        if(m_pos!=m_begin && *(m_pos-1)!=m_sep &&
               *(m_pos-1)!=m_eol.back())
            throw bad_format("Wrong field begin");

        if (m_strip_white) skip_if_white();
        bool quoted = false;

        // record actual content begin, remove quote char later
        if(nwBegin) *nwBegin = m_pos; 
      

        if (*m_pos!=m_quote || m_quote_rule==quote_rule::none)
        {
            // not quoted, looking for sep or eol
            while (m_pos<m_end && *m_pos!=m_sep && *m_pos!=m_eol[0])
                m_pos++;
        }
        else
        {
            int eolCount = 0;
            quoted = true;
            switch(m_quote_rule)
            {
            case quote_rule::doubled:
                // doubled quote, looking for closing quote
                while (++m_pos<m_end && eolCount<100)
                {
                    eolCount += (*m_pos==m_eol[0]);
                    if (*m_pos==m_quote)
                    {
                        if (m_pos+1<m_end && *(m_pos+1)==m_quote)
                            m_pos++;
                        else
                            break; // closing quote or last char
                    }
                }
                if (m_pos>=m_end || *m_pos!=m_quote)
                    throw std::logic_error("unknown quoting style");
                break;

            case quote_rule::escaped:
                while (++m_pos<m_end && *m_pos!=m_quote && eolCount<100)
                {
                    eolCount += (*m_pos==m_eol[0]);
                    m_pos += (*m_pos=='\\');
                }
                if (m_pos>=m_end || *m_pos!=m_quote)
                    throw std::logic_error("unknown quoting style");
                break;

            case quote_rule::verbatim:
                // verbatim quoting, looking for quote followed by
                // sep or eol
                quoted = false;
                while (++m_pos<m_end && *m_pos!=m_eol[0])
                {
                    if (*m_pos==m_quote &&
                        (m_pos+1>=m_end || *(m_pos+1)==m_sep ||
                         *(m_pos+1)==m_eol[0]))
                    {
                        quoted = true; break;
                    }
                    if (*m_pos==m_sep)
                    {
                        // first sep encountered, we need to
                        // decide whether it is a field end
                        // we look into the following sequence,
                        // if there is quote+sep, that quote+sep
                        // should be field end, otherwise, the current
                        // sep is field end.
                        const char *pos = m_pos;
                        while (++pos<m_end && *pos!=m_eol[0])
                        {
                            if (*pos==m_quote &&
                                (pos+1>=m_end || *(pos+1)==m_sep ||
                                 *(pos+1)==m_eol[0]))
                            {
                                m_pos = pos;  quoted=true; break;
                            }
                        }
                        break;
                    }// endif
                }//end while
            break;
            default:
                assert(false);
            }//end switch
        }//end if...else
        // we can only land on sep, quote, EOL, or EOF
        assert(m_pos>=m_end || *m_pos==m_sep ||
               *m_pos==m_eol[0] || *m_pos==m_quote);
              
        // record content end
        if(nwEnd) *nwEnd = m_pos;
        
        if (quoted)
        {
            assert(*m_pos==m_quote);
            assert(!nwBegin || **nwBegin==m_quote);
            assert(!nwEnd || **nwEnd==m_quote);
            m_pos++;

            // *nwEnd should be on char after quote
            if (nwEnd) (*nwEnd)++;
            if (m_strip_white) skip_if_white();
        }
        if (m_pos<m_end && *m_pos!=m_eol[0] && *m_pos!=m_sep)
            throw std::logic_error("parsing field failed.");
        assert(m_pos>=m_end || *m_pos==m_eol[0] || *m_pos==m_sep);
        
        return *this;
    }// end function

    reader& reader::skip_field()
    {
        skip_field_content();
        skip_if_sep();
    }
    
    reader& reader::skip_sep()
    {
        assert(*m_pos==m_sep);
        m_pos++;
        if (m_sep==' ')
            while (m_pos<m_end && *m_pos==' ')
                m_pos++;
        return *this;
    }

    reader& reader::skip_if_sep()
    {
        if (*m_pos==m_sep) skip_sep();
        return *this;
    }

    reader& reader::skip_eol()
    {
        assert(*m_pos==m_eol[0]);
        m_pos += m_eol.size();
        assert(m_pos<=m_end);
        return *this;
    }

    reader& reader::skip_if_eol()
    {
        if (*m_pos==m_eol[0])
            skip_eol();
        return *this;
    }

    reader& reader::skip_record(int& nfields)
    {
        nfields = 0;
        while (m_pos<m_end && *m_pos!=m_eol[0])
        {
            skip_field();
            nfields++;
        }
        skip_if_eol();
        return *this;
    }

    reader& reader::skip_record()
    {
        int nfield;
        return skip_record(nfield);
    }

    reader& reader::read_field(std::string& field)
    {
        const char *contentBegin, *contentEnd;
        skip_field_content(&contentBegin, &contentEnd);
        field.assign(contentBegin, contentEnd);
        skip_if_sep();
        return *this;
    }

    std::string reader::read_field()
    {
        std::string field;
        read_field(field);
        return field;
    }

    reader& reader::read_record(std::vector<std::string>& content)
    {
        content.resize(get_nfields());
        int i=0;
        while (!is_eol_or_end())
        {
            if (i>=get_nfields())
                throw bad_format("nfields not match.");
            content[i] = read_field();
            i++;
        }
        assert(i<=get_nfields());
        if (i<get_nfields())
        {
            if (m_fill)
                std::fill(content.begin()+i, content.end(), "");
            else
                throw bad_format("nfields not match.");
        }
        skip_if_eol();
    }

    std::vector<std::string> reader::read_record()
    {
        std::vector<std::string> content;
        read_record(content);
        return content;
    }

    bool reader::is_sep() const
    {
        return m_pos<m_end && *m_pos==m_sep;
    }

    bool reader::is_eol() const
    {
        return m_pos<m_end && *m_pos==m_eol[0];
    }

    bool reader::is_end() const
    {
        return m_pos>=m_end;
    }

    bool reader::is_eol_or_end() const
    {
        return m_pos>=m_end || *m_pos==m_eol[0];
    }
}
