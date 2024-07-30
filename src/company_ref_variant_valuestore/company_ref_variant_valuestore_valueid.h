/**
 Copyright © 2023 COMPAN REF
 @file company_ref_variant_valuestore_valueid.h
 @brief ValueId encapsulation object
 */
#ifndef __company_ref_VARIANT_VALUESTORE_VALUEID_H__
#define __company_ref_VARIANT_VALUESTORE_VALUEID_H__

#include <memory>
#include <string>

namespace Compan{
namespace Edge {

// fwd decl
struct ValueIdIterator;

/*!
 * @brief   Value Id encapsulation object
 *
 * Value Id's are dot notation strings. This class enforces proper notation for identification
 * strings, as well as identifying a parent and leaf name in the string.
 */
class ValueId {
public:
    typedef std::shared_ptr<ValueId> Ptr;

    explicit ValueId();

    /// Constructs from a c string
    ValueId(char const* name);

    /// Constructs from a std::string
    ValueId(std::string const& name);

    /// Concatenates a c string with a parent value id
    ValueId(ValueId const& parent, char const* name);

    /// Concatenates a std::string with a parent value id
    ValueId(ValueId const& parent, std::string const& name);

    ValueId(ValueId const& src);
    ValueId(ValueId&& src);
    ~ValueId();

    ValueId& operator=(ValueId const&) = delete;
    ValueId& operator=(ValueId&& source);

    /// Concatenates a a ValueId to this ValueId
    ValueId& operator+=(ValueId const&);

    bool operator==(ValueId const& id) const;
    bool operator!=(ValueId const& id) const;
    bool operator<(ValueId const& id) const;

    /// Returns the fully qualified ValueId name
    operator std::string() const;

    /// Returns the fully qualified ValueId name
    std::string name();
    std::string const& name() const;

    /// returns the parent of this value id name
    ValueId parent() const;

    /// Returns the last element of the name
    ValueId leaf() const;

    /// Returns true if the ValueId is empty
    bool empty() const;

    /// Returns the size of the ValueId string
    size_t size() const;

    /// Returns true if there is only one element in the ValueId
    bool isSingleName() const;

    /// Returns an iterator to the beginning element
    ValueIdIterator begin() const;

    /// Returns an iterator to the end element
    ValueIdIterator end() const;

    static char const Separator = '.';

protected:
    /// strips leading and trailing separators
    static std::string trimName(std::string const& name);

private:
    std::string name_;
};

/// Comparison operators for shared_ptr ValueId's
struct ValueIdPtrLessCompare {
    bool operator()(ValueId::Ptr const& lhs, ValueId::Ptr const& rhs) const;
};

std::ostream& operator<<(std::ostream& os, ValueId const& id);

/*!
 * @brief Iterator used to parse out the elements in a ValueId
 */
struct ValueIdIterator {
    explicit ValueIdIterator();

    ValueIdIterator& operator++();
    ValueIdIterator operator++(int);
    bool operator==(ValueIdIterator const& other) const;
    bool operator!=(ValueIdIterator const& other) const;
    ValueId const& operator*() const;

    ValueIdIterator operator+(int const);

    void first(std::string const& name);
    void next();

    ValueId current_;
    std::string name_;
    std::string::size_type pos_;
};

} // namespace Edge
} // namespace Compan

#endif // __company_ref_VARIANT_VALUESTORE_VALUEID_H__
