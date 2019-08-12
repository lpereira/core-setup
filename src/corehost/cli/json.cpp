// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#include "json.h"
#include "pal.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "utils.h"
#include <cassert>
#include <cstdint>

void json_parser_t::realloc_buffer(size_t size)
{
    delete[] m_json;
    m_json = new pal::char_t[size + 1];
    m_json[size] = '\0';
}

bool json_parser_t::parse_json(const pal::string_t& context)
{
    assert(m_json != nullptr);

    m_document.ParseInsitu(m_json);

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
    memcpy(m_json, str.c_str(), str.length());

    return parse_json(context);
}

bool json_parser_t::parse_file(const pal::string_t& path)
{
    pal::ifstream_t file{path};

    if (!file.good())
    {
        trace::error(_X("Could not open file [%s]"), path.c_str());
        return false;
    }

    skip_utf8_bom(&file);

    auto current_pos = file.tellg();
    file.seekg(0, file.end);
    auto file_size = file.tellg();
    file.seekg(current_pos, file.beg);

    realloc_buffer(file_size - current_pos);
    file.read(m_json, file_size - current_pos);

    return parse_json(path);
}
