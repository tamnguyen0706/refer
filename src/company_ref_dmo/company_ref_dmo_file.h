/**
 Copyright © 2023 COMPAN REF
 @file company_ref_dmo_file.h
 @brief datamodel Read/Write functions
 */
#ifndef __company_ref_DMO_company_ref_DMO_FILE_H__
#define __company_ref_DMO_company_ref_DMO_FILE_H__

#include "company_ref_dmo_container.h"

#include <istream>
#include <ostream>

namespace Compan{
namespace Edge {

class VariantValueStore;

struct DmoFile {
    static bool read(std::istream& strm, DmoContainer& container);
    static bool read(std::istream& strm, DmoContainer& container, VariantValueStore& ws);
    static bool read(std::istream& strm, VariantValueStore& ws);
    static bool write(std::ostream& strm, DmoContainer const& container);

    static bool read(std::string const& path, DmoContainer& container);
    static bool read(std::string const& path, DmoContainer& container, VariantValueStore& ws);
    static bool read(std::string const& path, VariantValueStore& ws);
    static bool write(std::string const& path, DmoContainer const& container);
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_DMO_company_ref_DMO_FILE_H__
