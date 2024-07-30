/**
  Copyright © 2023 COMPAN REF
  @file company_ref_variant_valuestore.cpp
  @brief CompanEdge Variant ValueStore Map
*/

#include "company_ref_variant_valuestore.h"

#include "company_ref_variant_container_util.h"
#include "company_ref_variant_factory.h"
#include "company_ref_variant_valuestore_dispatcher.h"
#include "company_ref_variant_valuestore_valueid.h"
#include "company_ref_variant_valuestore_visitor.h"

#include "company_ref_variant_map_value.h" // required for add/remove container notifier

#include <company_ref_protocol/company_ref_protocol.pb.h>
#include <company_ref_protocol_utils/company_ref_pb_traits.h>
#include <company_ref_protocol_utils/company_ref_stream.h>
#include <company_ref_protocol_utils/protobuf.h>
#include <company_ref_utils/company_ref_weak_bind.h>

#include <Compan_logger/Compan_logger.h>

namespace Compan{
namespace Edge {

CompanLogger VariantValueStoreLog("variantvalue.store", LogLevel::Information);

} // namespace Edge
} // namespace Compan

using namespace Compan::Edge;

VariantValueStore::VariantValueStore(boost::asio::io_context& ctx)
    : ctx_(ctx)
    , bucketizer_()
    , wsMap_()
    , root_(std::make_shared<VariantValue>(ctx_, "", CompanEdgeProtocol::Unknown))
    , onValueAddedSignal_(ctx_)
    , onValueChangedSignal_(ctx_)
    , onValueRemovedSignal_(ctx_)
    , onValueAddToContainerSignal_(ctx_)
    , onValueRemoveFromContainerSignal_(ctx_)
    , dataDispatcher_(std::make_shared<VariantValueDispatcher>())
{
    Protobuf::instance();
    dataDispatcher_->start();
}

VariantValueStore::~VariantValueStore()
{
    dataDispatcher_->stop();
    dataDispatcher_.reset();

    onValueRemovedSignal_.disconnectAll();
    onValueRemoveFromContainerSignal_.disconnectAll();

    while (auto child = root_->getFirst()) {
        // the child->id() get's freed - save a copy
        ValueId childId(child->id());

        delChildren(child, VariantValue::Local);

        root_->delChild(childId);
    }

    std::lock_guard<std::mutex> lock(mutex_);
    bucketizer_.clear();
    wsMap_.clear();

    root_.reset();
}

bool VariantValueStore::set(CompanEdgeProtocol::Value const& wsValue, VariantValue::SetUpdateType const updateType)
{
    if (wsValue.id().empty()) {
        ErrorLog(VariantValueStoreLog) << "Set value failed: Empty Id" << std::endl;
        return false;
    }

    if (!ValueTypeTraits::isValid(wsValue)) {
        ErrorLog(VariantValueStoreLog) << "Set value failed: " << wsValue.id() << " - Invalid data" << std::endl;
        return false;
    }

    VariantValue::Ptr valueData(get(wsValue.id()));
    if (valueData) {

        HashToken hashToken;
        {
            std::lock_guard<std::mutex> lock(mutex_);

            // we will always make sure that the hash token is correct
            hashToken = bucketizer_.make(wsValue.id());
        }

        valueData->hashToken(hashToken.transportToken());

        ValueDataSet::Results result = valueData->set(wsValue, updateType);

        // same value isn't a failure, per-se, it's simply a warning
        bool const success = (result == ValueDataSet::Success || result == ValueDataSet::SameValue);

        if (success && result == ValueDataSet::SameValue)
            DebugLog(VariantValueStoreLog)
                    << "Set value: " << valueData->id() << " - " << ValueDataSet::resultStr(result) << std::endl;
        else if (!success)
            ErrorLog(VariantValueStoreLog)
                    << "Set value failed: " << valueData->id() << " - " << ValueDataSet::resultStr(result) << std::endl;

        return success;
    }

    return (add(VariantFactory::make(ctx_, wsValue, updateType)) != nullptr);
}

VariantValue::Ptr VariantValueStore::get(ValueId const& valueId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return getSafe(valueId);
}

bool VariantValueStore::del(ValueId const& valueId, VariantValue::SetUpdateType const updateType)
{
    FunctionArgLog(VariantValueStoreLog) << valueId << std::endl;

    VariantValue::Ptr valuePtr = get(valueId);
    if (valuePtr == nullptr) return false;

    // first, unhook this value from the map
    std::lock_guard<std::mutex> lock(mutex_);

    {
        bucketizer_.removeToken(HashToken(valuePtr->hashToken()));
        wsMap_.erase(valuePtr->id());
    }

    for (auto& children : valuePtr->getChildren()) delChildren(children.second, updateType);

    valuePtr->setUpdateType(updateType);
    doRemovedSignal(valuePtr);
    delContainer(valuePtr, updateType, true);

    if (valuePtr->parent() != nullptr) { valuePtr->parent()->delChild(valueId.leaf()); }

    valuePtr->disconnect();

    return true;
}

VariantValue::Ptr VariantValueStore::add(VariantValue::Ptr wsValue)
{
    if (get(wsValue->id()) != nullptr) return nullptr;
    if (wsValue->id().empty()) return nullptr;

    wsValue->setDataDispatcher(dataDispatcher_);
    wsValue->setWsChangedSignal(std::bind(&VariantValueStore::doChangedSignal, this, std::placeholders::_1));

    if (wsValue->type() == CompanEdgeProtocol::Container) {
        VariantMapValue::Ptr mapCasted = std::dynamic_pointer_cast<VariantMapValue>(wsValue);
        if (mapCasted) {

            mapCasted->setWsAddToContainerSignal(
                    std::bind(&VariantValueStore::doAddToContainerSignal, this, std::placeholders::_1));
            mapCasted->setWsRemoveFromContainerSignal(
                    std::bind(&VariantValueStore::doRemoveFromContainerSignal, this, std::placeholders::_1));
        } else
            WarnLog(VariantValueStoreLog) << "Adding : " << wsValue->id() << " is not a VariantMapValue" << std::endl;
    }

    ValueId valueId(wsValue->id());

    VariantValue::Ptr parentValue = getParent(wsValue);
    if (parentValue) {

        parentValue->addChild(wsValue);
        wsValue->connectChangedListener(WeakBind(&VariantValue::parentSignal, parentValue, std::placeholders::_1));
        wsValue->parent(parentValue);
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);

        // we will always make sure that the hash token is correct
        HashToken hashToken = bucketizer_.make(wsValue->id());

        wsValue->hashToken(hashToken.transportToken());

        // Set the shared name from the bucketizer
        wsValue->valueId_ = bucketizer_.get(hashToken);

        auto insert_iter = wsMap_.emplace(wsValue->id(), wsValue);

        // maybe this is a glitch?
        if (!insert_iter.second) return nullptr;
    }

    doAddedSignal(wsValue);

    return wsValue;
}

bool VariantValueStore::has(ValueId const& valueId)
{
    return get(valueId) != nullptr;
}

void VariantValueStore::visitValues(VisitFunction const& visitFunction)
{
    if (!visitFunction) return;

    // doesn't require locking, since we are always using a copy of
    // the VariantValue children
    for (auto& children : root_->getChildren()) {
        VariantValue::Ptr childPtr = children.second;

        visitFunction(childPtr);
        VariantValueVisitor::visitChildren(childPtr, visitFunction);
    }
}

size_t VariantValueStore::size()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return wsMap_.size();
}

HashToken VariantValueStore::findHashToken(ValueId const& valueId)
{
    VariantValue::Ptr wsValue = get(valueId);
    if (wsValue == nullptr) { return HashToken(); }

    return HashToken(wsValue->hashToken());
}

ValueId::Ptr VariantValueStore::findValueId(HashToken const& hashToken)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return bucketizer_.get(hashToken);
}

VariantValue::Ptr VariantValueStore::addToContainer(
        ValueId const& valueId,
        std::string const& key,
        VariantValue::SetUpdateType const updateType)
{
    return addToContainer(valueId, key, "", updateType);
}

VariantValue::Ptr VariantValueStore::addToContainer(
        ValueId const& valueId,
        std::string const& key,
        std::string const& /*value*/,
        VariantValue::SetUpdateType const updateType)
{
    // doesn't touch the wsMap_ - don't need lock
    VariantValue::Ptr containerPtr = get(valueId);
    if (containerPtr == nullptr || containerPtr->type() != CompanEdgeProtocol::Container) return nullptr;

    VariantValue::Ptr addToPtr = VariantContainerUtil::makeAddToContainer(containerPtr, key, updateType);
    if (addToPtr == nullptr) return nullptr;

    containerPtr->signal(addToPtr);

    return addToPtr;
}

VariantValue::Ptr VariantValueStore::removeFromContainer(
        ValueId const& valueId,
        std::string const& key,
        VariantValue::SetUpdateType const updateType)
{
    // doesn't touch the wsMap_ - don't need lock
    VariantValue::Ptr containerPtr = get(valueId);
    if (containerPtr == nullptr || containerPtr->type() != CompanEdgeProtocol::Container) return nullptr;

    VariantValue::Ptr removeFromPtr = VariantContainerUtil::makeRemoveFromContainer(containerPtr, key, updateType);
    if (removeFromPtr == nullptr) return nullptr;

    containerPtr->signal(removeFromPtr);

    return removeFromPtr;
}

/// These are private functions - no need for mutext locks
///

VariantValue::Ptr VariantValueStore::getSafe(ValueId const& valueId)
{
    auto it = wsMap_.find(valueId.name());
    if (it == wsMap_.end()) return nullptr;

    return it->second;
}

bool VariantValueStore::delContainer(
        VariantValue::Ptr valuePtr,
        VariantValue::SetUpdateType const updateType,
        bool const notifyParentChange)
{
    if (valuePtr == nullptr) return false;

    VariantValue::Ptr containerPtr = valuePtr->parent();

    if (containerPtr == nullptr || containerPtr->type() != CompanEdgeProtocol::Container) return false;

    // can't use ::removeFromContainer here, since it requires the mutex
    VariantValue::Ptr removeFromPtr =
            VariantContainerUtil::makeRemoveFromContainer(containerPtr, valuePtr->id().leaf(), updateType);
    if (removeFromPtr == nullptr) return false;

    // this calls the underlaying map's signal handler - which sends the message
    //  as a doRemoveFromContainerSignal and a change notification

    containerPtr->signal(removeFromPtr);

    if (notifyParentChange) containerPtr->signal(containerPtr);

    return true;
}

void VariantValueStore::delChildren(VariantValue::Ptr valuePtr, VariantValue::SetUpdateType const updateType)
{
    if (valuePtr == nullptr) return;

    for (auto& children : valuePtr->getChildren()) delChildren(children.second, updateType);

    {
        bucketizer_.removeToken(HashToken(valuePtr->hashToken()));
        wsMap_.erase(valuePtr->id());
    }

    valuePtr->setUpdateType(updateType);

    doRemovedSignal(valuePtr);
    delContainer(valuePtr, updateType);

    valuePtr->disconnect();
}

VariantValue::Ptr VariantValueStore::getParent(VariantValue::Ptr const valuePtr)
{
    if (valuePtr == nullptr) return nullptr;

    ValueId parentId(valuePtr->id().parent());

    // nothing more to create
    if (parentId.empty()) return root_;

    VariantValue::Ptr parentPtr = getSafe(parentId.name());
    if (!parentPtr) {
        CompanEdgeProtocol::Value parentValue;

        parentValue.set_id(parentId);

        bool const isPlaceHolder =
                (valuePtr->type() == CompanEdgeProtocol::Unset || valuePtr->type() == CompanEdgeProtocol::Unknown);

        if (isPlaceHolder) {

            // The Value::type is unset on a valuestore mirror, due to subscription process
            //  So, in that case, we create parent's of Unknown type, so the value will get
            //  set properly on subscription result
            parentValue.set_type(CompanEdgeProtocol::Unknown);
        } else {
            // we fill in the blank
            parentValue.set_type(CompanEdgeProtocol::Struct);
            parentValue.mutable_unknownvalue()->set_value(std::string("Struct"));
        }

        parentPtr = std::make_shared<VariantValue>(ctx_, parentValue);
        parentPtr->setUpdateType_ = valuePtr->setUpdateType();
        if (isPlaceHolder) {
            // we are forcing the place holder to not transmit
            parentPtr->setUpdateType_ = VariantValue::Remote;
        }

        return add(parentPtr);
    }

    return parentPtr;
}

void VariantValueStore::doAddedSignal(VariantValue::Ptr const variantPtr)
{
    onValueAddedSignal_(variantPtr);
}

void VariantValueStore::doChangedSignal(VariantValue::Ptr const variantPtr)
{
    onValueChangedSignal_(variantPtr);
}

void VariantValueStore::doRemovedSignal(VariantValue::Ptr const variantPtr)
{
    onValueRemovedSignal_(variantPtr);
}

void VariantValueStore::doAddToContainerSignal(VariantValue::Ptr const variantPtr)
{
    onValueAddToContainerSignal_(variantPtr);
}

void VariantValueStore::doRemoveFromContainerSignal(VariantValue::Ptr const variantPtr)
{
    onValueRemoveFromContainerSignal_(variantPtr);
}

void VariantValueStore::print(std::ostream& os)
{
    visitValues([&os](VariantValue::Ptr const& valuePtr) { os << valuePtr->get() << std::endl; });
}
