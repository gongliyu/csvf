#include <string>

#include <csvf/writer.hpp>

namespace csvf
{
    writer& writer::write_field(const std::string& content)
    {
        m_stream << content << m_sep;
    }

    writer& writer::write_record(const std::vector<std::string>&record)
    {
        int n = record.size();
        int i = 0;
        for (auto& field : record)
        {
            i++;
            m_stream<<field;
            m_stream<<(i==n?'\n':m_sep);
        }
    }

    writer& writer::operator<<(const std::vector<std::string>& record)
    {
        return write_record(record);
    }

    writer& writer::flush()
    {
        m_stream.flush();
    }

    writer& writer::close()
    {
        m_stream.close();
    }
}
