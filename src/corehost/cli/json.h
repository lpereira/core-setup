// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#ifndef __JSON_H__
#define __JSON_H__

#include "pal.h"
#include "rapidjson/document.h"
#include "rapidjson/fwd.h"

class json_parser_t {
    public:
        ~json_parser_t() { delete[] m_json; }

        const rapidjson::Document& document() const { return m_document; }

        bool parse_file(const pal::string_t& path);
        bool parse_string(const pal::string_t& str, const pal::string_t& context);

    private:
        pal::char_t *m_json{nullptr};
        rapidjson::Document m_document;

        void realloc_buffer(size_t size);
        bool parse_json(const pal::string_t& context);
};

#endif // __JSON_H__
