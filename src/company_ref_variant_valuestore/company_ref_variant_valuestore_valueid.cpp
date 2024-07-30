/**
 Copyright © 2023 COMPAN REF
 @file company_ref_variant_valuestore_valueid.cpp
 @brief ValueId encapsulation object
 */
#include "company_ref_variant_valuestore_valueid.h"

#include <company_ref_utils/company_ref_regex_utils.h>

#include <iostream>
#include <regex>

namespace Compan{
namespace Edge {

bool ValueIdPtrLessCompare::operator()(ValueId::Ptr const& lhs, ValueId::Ptr const& rhs) const
{
    return lhs->operator<(*rhs);
}

std::ostream& operator<<(std::ostream& os, ValueId const& id)
{
    os << id.name();
    return os;
}

ValueId::ValueId()
{
}

ValueId::ValueId(char const* name)
    : ValueId(std::string(name))
{
}

ValueId::ValueId(std::string const& name)
    : name_(trimName(name))
{
}

ValueId::ValueId(ValueId const& parent, char const* name)
    : ValueId(parent, std::string(name))
{
}

ValueId::ValueId(ValueId const& parent, std::string const& name)
    : name_(trimName(parent.name() + Separator + trimName(name)))
{
}

ValueId::ValueId(ValueId const& src)
    : name_(src.name_)
{
}

ValueId::ValueId(ValueId&& src)
    : name_(std::move(src.name_))
{
}

ValueId::~ValueId()
{
}

ValueId& ValueId::operator=(ValueId&& src)
{
    name_ = std::move(src.name_);
    return *this;
}

ValueId& ValueId::operator+=(ValueId const& src)
{
    if (empty())
        name_ = src.name();
    else
        name_ += Separator + src.name();
    return *this;
}

bool ValueId::operator==(ValueId const& id) const
{
    return (name() == id.name());
}

bool ValueId::operator!=(ValueId const& id) const
{
    return !(*this == id);
}

bool ValueId::operator<(ValueId const& id) const
{
    return name_ < id.name_;
}

ValueId::operator std::string() const
{
    return name_;
}

std::string ValueId::name()
{
    return name_;
}

std::string const& ValueId::name() const
{
    return name_;
}

ValueId ValueId::parent() const
{
    size_t const pos = name_.find_last_of(Separator);
    if (pos != std::string::npos) return ValueId(name_.substr(0, pos));
    return ValueId();
}

ValueId ValueId::leaf() const
{
    size_t const pos = name_.find_last_of(Separator);
    if (pos != std::string::npos) return ValueId(name_.substr(pos + 1, std::string::npos));
    return ValueId(name_);
}

bool ValueId::empty() const
{
    return name_.empty();
}

size_t ValueId::size() const
{
    return name_.length();
}

bool ValueId::isSingleName() const
{
    return (name_.find_last_of(Separator) == std::string::npos);
}

ValueIdIterator ValueId::begin() const
{
    ValueIdIterator iter;

    iter.first(name_);

    return iter;
}

ValueIdIterator ValueId::end() const
{
    ValueIdIterator iter;

    iter.name_ = name_;
    iter.pos_ = name_.length() + 2;

    return iter;
}

std::string ValueId::trimName(std::string const& name)
{
    if (name.empty()) return name;

    // drop front/back dots
    // drop embedded double dots

    // can't find a valid character? It's not a name
    std::string::size_type end = name.find_last_not_of(" .");
    if (end == std::string::npos) return std::string();

    end++;

    std::string::size_type begin = name.find_first_not_of(" .");
    if (begin == std::string::npos) begin = 0;

    // drop double dots
    std::regex const doubleDots("[.][.]");
    return std::regex_replace(name.substr(begin, end - begin), doubleDots, ".");
}

ValueIdIterator::ValueIdIterator()
    : pos_(std::string::npos)
{
}

void ValueIdIterator::first(std::string const& name)
{
    if (name.empty()) return;

    name_ = name;
    pos_ = 0;

    std::string::size_type pos = name_.find_first_of(ValueId::Separator);

    if (pos == std::string::npos)
        current_ = ValueId(name_);
    else
        current_ = ValueId(name_.substr(0, pos));

    pos_ += current_.size() + 1;
}

void ValueIdIterator::next()
{
    if (pos_ > name_.length()) {
        current_ = ValueId();

        // clamp so if we go over the last, it will always point to the ::end iterator value
        pos_ = name_.length() + 2;
        return;
    }

    std::string::size_type pos = name_.find(ValueId::Separator, pos_);
    if (pos == std::string::npos)
        current_ = ValueId(name_.substr(pos_));
    else
        current_ = ValueId(name_.substr(pos_, pos - pos_));

    pos_ += current_.size() + 1;
}

ValueIdIterator& ValueIdIterator::operator++()
{
    next();
    return *this;
}

ValueIdIterator ValueIdIterator::operator++(int)
{
    ValueIdIterator retVal = *this;
    retVal.next();
    return retVal;
}

ValueIdIterator ValueIdIterator::operator+(int const inc)
{
    ValueIdIterator retVal = *this;
    for (int count = 0; count < inc; ++count) retVal.next();
    return retVal;
}

bool ValueIdIterator::operator==(ValueIdIterator const& other) const
{
    return name_ == other.name_ && pos_ == other.pos_;
}

bool ValueIdIterator::operator!=(ValueIdIterator const& other) const
{
    return !operator==(other);
}

ValueId const& ValueIdIterator::operator*() const
{
    return current_;
}

} // namespace Edge
} // namespace Compan
