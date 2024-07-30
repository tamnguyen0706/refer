/**
 Copyright © 2023 COMPAN REF
 @file company_ref_dmo_file.cpp
 @brief datamodel Read/Write functions
 */

#include "company_ref_dmo_file.h"

#include "company_ref_dmo_helper.h"

#include <company_ref_protocol/company_ref_datamodel.pb.h>
#include <company_ref_variant_valuestore/company_ref_variant_valuestore.h>
#include <Compan_logger/Compan_logger.h>

#include <fstream>
#include <map>

using namespace Compan::Edge;

CompanLogger DmoFileLog("dmo.file", LogLevel::Information);

namespace {

void readDmoMessage(CompanEdgeDataModel::DataModelMessage& dmoData, DmoContainer& container)
{
    for (auto& metaDataTypeDefs : dmoData.metadatadefs().metadatatypedefs()) {

        ValueId parent(metaDataTypeDefs.parent_id());

        // Special case of a container that is key/value pair, as opoosed to key/struct
        if (metaDataTypeDefs.members_size() == 1) {
            CompanEdgeProtocol::Value metaValue = metaDataTypeDefs.members(0);

            if (ValueId(metaValue.id()).leaf() == DmoContainer::MetaContainerDelimId) {
                if (!container.insertMetaContainer(parent, CompanEdgeProtocol::Container, metaValue))
                    ErrorLog(DmoFileLog) << "Failed to insert MetaData: " << parent << " - "
                                         << CompanEdgeProtocol::Value_Type_Name(metaValue.type()) << std::endl;

                continue;
            }
        }

        container.insertMetaContainer(parent.parent(), CompanEdgeProtocol::Container);
        // insert the normal children definitions
        for (auto& rawValue : metaDataTypeDefs.members()) {

            ValueId valueId(parent, rawValue.id());

            if (!container.insertMetaData(valueId, rawValue))
                ErrorLog(DmoFileLog) << "Failed to insert MetaData: " << valueId << std::endl;
        }
    }

    container.sort();
}

} // namespace

bool DmoFile::read(std::istream& strm, DmoContainer& container)
{
    CompanEdgeDataModel::DataModelMessage dmoData;

    if (!dmoData.ParseFromIstream(&strm)) {
        ErrorLog(DmoFileLog) << "Failed to parse stream" << std::endl;
        return false;
    }

    if (dmoData.has_dataentities()) {
        for (auto& element : dmoData.dataentities().value()) container.insertValue(element, false);
    }

    if (dmoData.has_metadatadefs()) { readDmoMessage(dmoData, container); }

    return true;
}

bool DmoFile::read(std::istream& strm, DmoContainer& container, VariantValueStore& ws)
{
    CompanEdgeDataModel::DataModelMessage dmoData;

    if (!dmoData.ParseFromIstream(&strm)) {
        ErrorLog(DmoFileLog) << "Failed to parse stream" << std::endl;
        return false;
    }

    if (dmoData.has_metadatadefs()) { readDmoMessage(dmoData, container); }

    // build up the structures required for meta data
    container.visitMetaData([&ws](CompanEdgeProtocol::Value const& value) {
        if (value.id().find("+") != std::string::npos) return;

        ws.set(value);
    });

    if (!dmoData.has_dataentities()) return true;

    DmoValueStoreHelper dmoHelper(container, ws);

    for (auto& value : dmoData.dataentities().value()) {

        if (value.type() == CompanEdgeProtocol::Container || value.type() == CompanEdgeProtocol::Struct) continue;

        bool metaDataCorrect = true;

        // Here we validate that the element being added has
        // the correct entries in the WS - meaning, if it is from
        // meta data, it needs to have the meta data parent fully created

        ValueId id(value.id());

        if (container.isInstance(id) && !ws.has(id)) {

            // walk the ValueId path and insert key elements
            ValueId metaPath = container.getMetaDataPath(id);
            auto iterMeta = metaPath.begin();
            auto iterId = id.begin();

            ValueId parentId;
            for (; iterId != id.end(); ++iterId, ++iterMeta) {
                if (*iterMeta == DmoContainer::MetaContainerDelim) {
                    if (!dmoHelper.insertChild(parentId, *iterId)) {
                        ErrorLog(DmoFileLog)
                                << "Failed to insert key: " << parentId << "[" << *iterId << "]" << std::endl;

                        metaDataCorrect = false;
                        break;
                    }
                }

                parentId += *iterId;
            }

            if (!ws.has(id)) {
                WarnLog(DmoFileLog) << "Value is not present in metaData: " << id << std::endl;
                metaDataCorrect = false;
            }
        }

        if (metaDataCorrect) ws.set(value);
    }

    return true;
}

bool DmoFile::read(std::istream& strm, VariantValueStore& ws)
{
    CompanEdgeDataModel::DataModelMessage dmoData;

    if (!dmoData.ParseFromIstream(&strm)) {
        ErrorLog(DmoFileLog) << "Failed to parse stream" << std::endl;
        return false;
    }

    if (dmoData.has_dataentities()) {
        for (auto& element : dmoData.dataentities().value()) ws.set(element);
    }

    return true;
}

bool DmoFile::write(std::ostream& strm, DmoContainer const& container)
{
    CompanEdgeDataModel::DataModelMessage dmoData;
    CompanEdgeProtocol::ValueChanged* entities = dmoData.mutable_dataentities();

    container.visitValues([entities](CompanEdgeProtocol::Value const& value) { *entities->add_value() = value; });

    typedef std::vector<CompanEdgeProtocol::Value> MetaDataDefsType;
    typedef std::map<std::string, MetaDataDefsType> MetaMapType;
    MetaMapType metaMap;
    container.visitMetaData([&metaMap, &container](CompanEdgeProtocol::Value const& value) {
        ValueId valueId(value.id());

        if (value.type() == CompanEdgeProtocol::Container) {
            // Special case of a container that is key/value pair, as opoosed to key/struct
            ValueId metaDefId(valueId, DmoContainer::MetaContainerDelim);

            DmoContainer::OptionalTreeType parentBranch = container.root().get_child_optional(metaDefId.name());
            if (parentBranch) {
                if (parentBranch.get().data().type() != CompanEdgeProtocol::Struct) {

                    // single element insertion
                    CompanEdgeProtocol::Value metaValue = parentBranch.get().data();

                    metaMap[valueId].push_back(metaValue);
                    return;
                }
            }
        }

        if (value.type() != CompanEdgeProtocol::Container) {

            if (!container.isMetaData(valueId)) return;

            // This element was handled above
            if (valueId.leaf() == DmoContainer::MetaContainerDelimId) return;

            CompanEdgeProtocol::Value metaValue(value);
            metaValue.set_id(valueId.leaf().name());

            metaMap[valueId.parent().name()].push_back(metaValue);
        }
    });

    CompanEdgeDataModel::MetaDataDefs* metaDataDefs = dmoData.mutable_metadatadefs();

    for (auto& elements : metaMap) {

        CompanEdgeDataModel::MetaDataTypeDefs metaDataTypeDefs;
        metaDataTypeDefs.set_parent_id(elements.first);

        for (auto& children : elements.second) { *metaDataTypeDefs.add_members() = children; }

        *metaDataDefs->add_metadatatypedefs() = metaDataTypeDefs;
    }

    return dmoData.SerializeToOstream(&strm);
}

bool DmoFile::read(std::string const& path, DmoContainer& container)
{
    std::ifstream iFile(path.c_str(), std::ios::binary);
    if (!iFile.is_open()) {
        ErrorLog(DmoFileLog) << "Failed to open file for read: " << path << std::endl;
        return false;
    }

    return DmoFile::read(iFile, container);
}

bool DmoFile::read(std::string const& path, DmoContainer& container, VariantValueStore& ws)
{
    std::ifstream iFile(path.c_str(), std::ios::binary);
    if (!iFile.is_open()) {
        ErrorLog(DmoFileLog) << "Failed to open file for read: " << path << std::endl;
        return false;
    }

    return DmoFile::read(iFile, container, ws);
}

bool DmoFile::read(std::string const& path, VariantValueStore& ws)
{
    std::ifstream iFile(path.c_str(), std::ios::binary);
    if (!iFile.is_open()) {
        ErrorLog(DmoFileLog) << "Failed to open file for read: " << path << std::endl;
        return false;
    }

    return DmoFile::read(iFile, ws);
}

bool DmoFile::write(std::string const& path, DmoContainer const& container)
{
    std::ofstream oFile(path.c_str(), std::ios::binary);
    if (!oFile.is_open()) {
        ErrorLog(DmoFileLog) << "Failed to open file for read: " << path << std::endl;
        return false;
    }

    return DmoFile::write(oFile, container);
}
