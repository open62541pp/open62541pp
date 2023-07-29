#include "open62541pp/DataType.h"

#include <algorithm>  // copy
#include <cassert>
#include <cstring>
#include <utility>  // move, swap

#include "open62541pp/overloads/comparison.h"
#include "open62541pp/types/NodeId.h"

namespace opcua {

static void clearMembers(UA_DataType* native) {
    if (native != nullptr) {
        delete[] native->members;  // NOLINT
        native->members = nullptr;
        native->membersSize = 0;
    }
}

static void copyMembers(const DataTypeMember* members, size_t membersSize, UA_DataType* dst) {
    if (dst != nullptr) {
        dst->members = new DataTypeMember[membersSize];  // NOLINT
        dst->membersSize = membersSize;
        std::copy(
            members,
            members + membersSize,  // NOLINT
            dst->members
        );
    }
}

static void copy(const UA_DataType* src, UA_DataType* dst) {
    if (src != nullptr && dst != nullptr) {
        clearMembers(dst);
        *dst = *src;
        copyMembers(dst->members, dst->membersSize, dst);
    }
}

DataType::DataType(const UA_DataType& native) {
    copy(&native, handle());
}

DataType::DataType(UA_DataType&& native)
    : data_(native) {}

DataType::DataType(TypeIndex typeIndex)
    : DataType(UA_TYPES[typeIndex])  // NOLINT
{
    assert(typeIndex < UA_TYPES_COUNT);  // NOLINT
}

DataType::~DataType() {
    clearMembers(handle());
}

DataType::DataType(const DataType& other) {
    copy(other.handle(), handle());
}

DataType::DataType(DataType&& other) noexcept {
    std::swap(data_, other.data_);
}

DataType& DataType::operator=(const DataType& other) {
    if (this != &other) {
        copy(other.handle(), handle());
    }
    return *this;
}

DataType& DataType::operator=(DataType&& other) noexcept {
    if (this != &other) {
        std::swap(data_, other.data_);
    }
    return *this;
}

const char* DataType::getTypeName() const noexcept {
#ifdef UA_ENABLE_TYPEDESCRIPTION
    return handle()->typeName;
#endif
    return nullptr;
}

void DataType::setTypeName([[maybe_unused]] const char* typeName) noexcept {
#ifdef UA_ENABLE_TYPEDESCRIPTION
    handle()->typeName = typeName;
#endif
}

const NodeId& DataType::getTypeId() const noexcept {
    return asWrapper<NodeId>(handle()->typeId);
}

void DataType::setTypeId(const NodeId& typeId) {
    asWrapper<NodeId>(handle()->typeId) = typeId;
}

void DataType::setTypeId(NodeId&& typeId) noexcept {
    asWrapper<NodeId>(handle()->typeId) = std::move(typeId);
}

const NodeId& DataType::getBinaryEncodingId() const noexcept {
    return asWrapper<NodeId>(handle()->binaryEncodingId);
}

void DataType::setBinaryEncodingId(const NodeId& binaryEncodingId) {
    asWrapper<NodeId>(handle()->binaryEncodingId) = binaryEncodingId;
}

void DataType::setBinaryEncodingId(NodeId&& binaryEncodingId) noexcept {
    asWrapper<NodeId>(handle()->binaryEncodingId) = std::move(binaryEncodingId);
}

uint16_t DataType::getMemSize() const noexcept {
    return handle()->memSize;
}

void DataType::setMemSize(uint16_t memSize) noexcept {
    handle()->memSize = memSize;
}

uint8_t DataType::getTypeKind() const noexcept {
    return handle()->typeKind;
}

void DataType::setTypeKind(uint8_t typeKind) noexcept {
    assert(typeKind < UA_DATATYPEKINDS);  // NOLINT
    handle()->typeKind = typeKind;
}

bool DataType::getPointerFree() const noexcept {
    return handle()->pointerFree;
}

void DataType::setPointerFree(bool pointerFree) noexcept {
    handle()->pointerFree = pointerFree;
}

bool DataType::getOverlayable() const noexcept {
    return handle()->overlayable;
}

void DataType::setOverlayable(bool overlayable) noexcept {
    handle()->overlayable = overlayable;
}

uint8_t DataType::getMembersSize() const noexcept {
    return handle()->membersSize;
}

std::vector<DataTypeMember> DataType::getMembers() const {
    std::vector<DataTypeMember> result(handle()->membersSize);
    std::copy(
        handle()->members,
        handle()->members + handle()->membersSize,  // NOLINT
        result.begin()
    );
    return result;
}

void DataType::setMembers(const std::vector<DataTypeMember>& members) {
    assert(members.size() < (1U << 8U));
    clearMembers(handle());
    copyMembers(members.data(), members.size(), handle());
}

bool operator==(const UA_DataTypeMember& lhs, const UA_DataTypeMember& rhs) noexcept {
    if (lhs.padding != rhs.padding) {
        return false;
    }
    if (lhs.isArray != rhs.isArray) {
        return false;
    }
    if (lhs.isOptional != rhs.isOptional) {
        return false;
    }
    return true;
}

bool operator!=(const UA_DataTypeMember& lhs, const UA_DataTypeMember& rhs) noexcept {
    return !(lhs == rhs);
}

bool operator==(const UA_DataType& lhs, const UA_DataType& rhs) noexcept {
#ifdef UA_ENABLE_TYPEDESCRIPTION
    auto emptyIfNullptr = [](const char* name) { return name == nullptr ? "" : name; };
    if (std::strcmp(emptyIfNullptr(lhs.typeName), emptyIfNullptr(rhs.typeName)) != 0) {
        return false;
    }
#endif
    if (lhs.typeId != rhs.typeId) {
        return false;
    }
    if (lhs.binaryEncodingId != rhs.binaryEncodingId) {
        return false;
    }
    if (lhs.memSize != rhs.memSize) {
        return false;
    }
    if (lhs.typeKind != rhs.typeKind) {
        return false;
    }
    if (lhs.pointerFree != rhs.pointerFree) {
        return false;
    }
    if (lhs.overlayable != rhs.overlayable) {
        return false;
    }
    if (lhs.membersSize != rhs.membersSize) {
        return false;
    }
    for (size_t i = 0; i < lhs.membersSize; ++i) {
        if (lhs.members[i] != rhs.members[i]) {  // NOLINT
            return false;
        }
    }
    return true;
}

bool operator!=(const UA_DataType& lhs, const UA_DataType& rhs) noexcept {
    return !(lhs == rhs);
}

bool operator==(const DataType& lhs, const DataType& rhs) noexcept {
    return (*lhs.handle() == *rhs.handle());
}

bool operator!=(const DataType& lhs, const DataType& rhs) noexcept {
    return !(lhs == rhs);
}

}  // namespace opcua
