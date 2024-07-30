/**
 Copyright © 2024 COMPAN REF
 @file company_ref_dynamic_dmo_load_actions.h
 @brief DMO load/unload functions
 */
#ifndef __company_ref_DYNAMIC_DMO_LOAD_ACTIONS_H__
#define __company_ref_DYNAMIC_DMO_LOAD_ACTIONS_H__

#include <boost/filesystem.hpp>

#include <istream>

namespace Compan{
namespace Edge {

class VariantValueStore;
class DmoContainer;

/*!
 *
 */
class DmoLoadActions {
public:
    ~DmoLoadActions() = delete;

    /// Loads MetaData and Values
    static bool loadDmo(std::istream& strm, DmoContainer& container, VariantValueStore& ws);

    /// Removes MetaData and Values
    static bool unloadDmo(std::istream& strm, DmoContainer& container, VariantValueStore& ws);

    /// Removes container instances
    static bool reloadDmo(std::istream& strm, DmoContainer& container, VariantValueStore& ws);
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_DYNAMIC_DMO_LOAD_ACTIONS_H__
