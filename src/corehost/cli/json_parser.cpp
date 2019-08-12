// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#include "json_parser.h"
#include "rapidjson/error/en.h"
#include "utils.h"
#include <cassert>
#include <cstdint>

void json_parser_t::realloc_buffer(size_t size)
{
    m_json.resize(size + 1);
    m_json[size] = '\0';
}

bool json_parser_t::parse_json(const pal::string_t& context)
{
    assert(!m_json.empty());

#ifdef _WIN32
    // Can't use in-situ parsing on Windows, as JSON data is encoded in
    // UTF-8 and the host expects wide strings.  m_document will store
    // data in UTF-16 (with pal::char_t as the character type), but it
    // has to know that data is encoded in UTF-8 to convert during parsing.
    m_document.Parse<kParseDefaultFlags, rapidjson::UTF8<>>(m_json.data());
#else
    m_document.ParseInsitu(m_json.data());
#endif

    if (m_document.HasParseError())
    {
        trace::error(_X("A JSON parsing exception occurred in [%s]: %s"),
                     context.c_str(),
                     rapidjson::GetParseError_En(m_document.GetParseError()));
        return false;
    }

    if (!m_document.IsObject())
    {
        trace::error(_X("Expected a JSON object in [%s]"), context.c_str());
        return false;
    }

    return true;
}

bool json_parser_t::parse_string(const pal::string_t& str, const pal::string_t& context)
{
    realloc_buffer(str.length());
    memcpy(m_json.data(), str.c_str(), str.length());

    return parse_json(context);
}

namespace {

// Try to match 0xEF 0xBB 0xBF byte sequence (no endianness here.)
std::streampos get_utf8_bom_length(pal::istream_t& stream)
{
    if (stream.eof())
    {
        return 0;
    }

    auto peeked = stream.peek();
    if (peeked == EOF || ((peeked & 0xFF) != 0xEF))
    {
        return 0;
    }

    unsigned char bytes[3];
    stream.read(reinterpret_cast<char*>(bytes), 3);
    if ((stream.gcount() < 3) || (bytes[1] != 0xBB) || (bytes[2] != 0xBF))
    {
        return 0;
    }

    return 3;
}

} // empty namespace

bool json_parser_t::parse_stream(pal::istream_t& stream,
                                 const pal::string_t& context)
{
    if (!stream.good())
    {
        trace::error(_X("Cannot use stream for resource [%s]"), context.c_str());
        return false;
    }

    auto current_pos = ::get_utf8_bom_length(stream);
    stream.seekg(0, stream.end);
    auto stream_size = stream.tellg();
    stream.seekg(current_pos, stream.beg);

    realloc_buffer(stream_size - current_pos);
    stream.read(m_json.data(), stream_size - current_pos);

    return parse_json(context);
}
