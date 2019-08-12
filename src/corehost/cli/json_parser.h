// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#ifndef __JSON_PARSER_H__
#define __JSON_PARSER_H__

#include "pal.h"
#include "rapidjson/document.h"
#include "rapidjson/fwd.h"
#include <vector>

class json_parser_t {
    public:
#ifdef _WIN32
        using json_internal_encoding_type_t = rapidjson::UTF16<pal::char_t>;
#else
        using json_internal_encoding_type_t = rapidjson::UTF8<pal::char_t>;
#endif

        using json_document_t = rapidjson::GenericDocument<json_internal_encoding_type_t,
                                                           rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>,
                                                           rapidjson::CrtAllocator>;

        const json_document_t& document() const { return m_document; }

        bool parse_stream(pal::istream_t& stream, const pal::string_t& context);
        bool parse_file(const pal::string_t& path)
        {
            pal::ifstream_t file{path};
            return parse_stream(file, path);
        }

    private:
        // JSON data parsed by this class are always encoded in UTF-8, so use a vector
        // of char instead of pal::char_t.  On Windows, where wide strings are used,
        // this data is kept in UTF-8, but it's converted to UTF-16 as it's parsed by
        // m_document.
        std::vector<char> m_json;
        json_document_t m_document;

        void realloc_buffer(size_t size);
        bool parse_json(const pal::string_t& context);
        bool parse_string(const pal::string_t& str, const pal::string_t& context);
};

#endif // __JSON_PARSER_H__
