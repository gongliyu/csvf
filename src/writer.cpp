#include <string>

#include <csvf/writer.hpp>

namespace csvf
{
    writer& writer::flush()
    {
        m_stream.flush();
    }

    writer& writer::close()
    {
        m_stream.close();
    }
}
