/**
 Copyright © 2024 COMPAN REF
 @file company_ref_dynamic_dmo_load_actions.cpp
 @brief DMO load/unload functions
 */

#include "company_ref_dynamic_dmo_load_actions.h"

#include <company_ref_dmo/company_ref_dmo_container.h>
#include <company_ref_dmo/company_ref_dmo_file.h>

#include <company_ref_variant_valuestore/company_ref_variant_map_value.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore.h>

//
#include <company_ref_protocol_utils/company_ref_stream.h>
//

using namespace Compan::Edge;

bool DmoLoadActions::loadDmo(std::istream& strm, DmoContainer& container, VariantValueStore& ws)
{
    return DmoFile::read(strm, container, ws);
}

bool DmoLoadActions::unloadDmo(std::istream& strm, DmoContainer& container, VariantValueStore& ws)
{
    DmoContainer tmpContainer;

    if (!DmoFile::read(strm, tmpContainer)) return false;

    std::vector<CompanEdgeProtocol::Value> metaValues;
    tmpContainer.visitMetaData([&metaValues](CompanEdgeProtocol::Value const& value) { metaValues.push_back(value); });

    for (auto& value : metaValues) {
        ValueId id(value.id());

        if (value.type() != CompanEdgeProtocol::Container) continue;

        container.removeValue(id);
        ws.del(id);
    }

    std::vector<ValueId> realValues;
    tmpContainer.visitValues([&realValues](CompanEdgeProtocol::Value const& value) { realValues.push_back(value.id()); });

    for (auto iter = realValues.rbegin(); iter != realValues.rend(); ++iter) {
        VariantValue::Ptr valuePtr = ws.get(*iter);
        if (valuePtr == nullptr) continue;

        if (valuePtr->hasChildren()) continue;

        ws.del(*iter);
    }

    return true;
}

bool DmoLoadActions::reloadDmo(std::istream& strm, DmoContainer&, VariantValueStore& ws)
{
    DmoContainer tmpContainer;

    if (!DmoFile::read(strm, tmpContainer)) return false;

    std::vector<CompanEdgeProtocol::Value> metaValues;
    tmpContainer.visitMetaData([&metaValues](CompanEdgeProtocol::Value const& value) { metaValues.push_back(value); });

    for (auto& value : metaValues) {
        ValueId id(value.id());

        if (value.type() == CompanEdgeProtocol::Container) {
            VariantValue::Ptr mapPtr = ws.get(id);
            if (mapPtr == nullptr) continue;

            VariantValue::ChildMapType children = mapPtr->getChildren();
            for (auto& child : children) ws.del(ValueId(id, child.first));
        }
    }

    tmpContainer.visitValues([&ws](CompanEdgeProtocol::Value const& value) { ws.set(value); });

    return true;
}
