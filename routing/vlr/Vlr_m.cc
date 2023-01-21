//
// Generated file, do not edit! Created by opp_msgtool 6.0 from routing/vlr/Vlr.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wshadow"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wunused-parameter"
#  pragma clang diagnostic ignored "-Wc++98-compat"
#  pragma clang diagnostic ignored "-Wunreachable-code-break"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=noreturn"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#include <iostream>
#include <sstream>
#include <memory>
#include <type_traits>
#include "Vlr_m.h"

namespace omnetpp {

// Template pack/unpack rules. They are declared *after* a1l type-specific pack functions for multiple reasons.
// They are in the omnetpp namespace, to allow them to be found by argument-dependent lookup via the cCommBuffer argument

// Packing/unpacking an std::vector
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::vector<T,A>& v)
{
    int n = v.size();
    doParsimPacking(buffer, n);
    for (int i = 0; i < n; i++)
        doParsimPacking(buffer, v[i]);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::vector<T,A>& v)
{
    int n;
    doParsimUnpacking(buffer, n);
    v.resize(n);
    for (int i = 0; i < n; i++)
        doParsimUnpacking(buffer, v[i]);
}

// Packing/unpacking an std::list
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::list<T,A>& l)
{
    doParsimPacking(buffer, (int)l.size());
    for (typename std::list<T,A>::const_iterator it = l.begin(); it != l.end(); ++it)
        doParsimPacking(buffer, (T&)*it);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::list<T,A>& l)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        l.push_back(T());
        doParsimUnpacking(buffer, l.back());
    }
}

// Packing/unpacking an std::set
template<typename T, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::set<T,Tr,A>& s)
{
    doParsimPacking(buffer, (int)s.size());
    for (typename std::set<T,Tr,A>::const_iterator it = s.begin(); it != s.end(); ++it)
        doParsimPacking(buffer, *it);
}

template<typename T, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::set<T,Tr,A>& s)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        T x;
        doParsimUnpacking(buffer, x);
        s.insert(x);
    }
}

// Packing/unpacking an std::map
template<typename K, typename V, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::map<K,V,Tr,A>& m)
{
    doParsimPacking(buffer, (int)m.size());
    for (typename std::map<K,V,Tr,A>::const_iterator it = m.begin(); it != m.end(); ++it) {
        doParsimPacking(buffer, it->first);
        doParsimPacking(buffer, it->second);
    }
}

template<typename K, typename V, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::map<K,V,Tr,A>& m)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        K k; V v;
        doParsimUnpacking(buffer, k);
        doParsimUnpacking(buffer, v);
        m[k] = v;
    }
}

// Default pack/unpack function for arrays
template<typename T>
void doParsimArrayPacking(omnetpp::cCommBuffer *b, const T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimPacking(b, t[i]);
}

template<typename T>
void doParsimArrayUnpacking(omnetpp::cCommBuffer *b, T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimUnpacking(b, t[i]);
}

// Default rule to prevent compiler from choosing base class' doParsimPacking() function
template<typename T>
void doParsimPacking(omnetpp::cCommBuffer *, const T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimPacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

template<typename T>
void doParsimUnpacking(omnetpp::cCommBuffer *, T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimUnpacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

}  // namespace omnetpp

namespace omnetvlr {

class VlrIntVidSetDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
    };
  public:
    VlrIntVidSetDescriptor();
    virtual ~VlrIntVidSetDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(VlrIntVidSetDescriptor)

VlrIntVidSetDescriptor::VlrIntVidSetDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(omnetvlr::VlrIntVidSet)), "")
{
    propertyNames = nullptr;
}

VlrIntVidSetDescriptor::~VlrIntVidSetDescriptor()
{
    delete[] propertyNames;
}

bool VlrIntVidSetDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<VlrIntVidSet *>(obj)!=nullptr;
}

const char **VlrIntVidSetDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = { "existingClass",  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *VlrIntVidSetDescriptor::getProperty(const char *propertyName) const
{
    if (!strcmp(propertyName, "existingClass")) return "";
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int VlrIntVidSetDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 0+base->getFieldCount() : 0;
}

unsigned int VlrIntVidSetDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    return 0;
}

const char *VlrIntVidSetDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    return nullptr;
}

int VlrIntVidSetDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->findField(fieldName) : -1;
}

const char *VlrIntVidSetDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    return nullptr;
}

const char **VlrIntVidSetDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *VlrIntVidSetDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int VlrIntVidSetDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    VlrIntVidSet *pp = omnetpp::fromAnyPtr<VlrIntVidSet>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void VlrIntVidSetDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntVidSet *pp = omnetpp::fromAnyPtr<VlrIntVidSet>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'VlrIntVidSet'", field);
    }
}

const char *VlrIntVidSetDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    VlrIntVidSet *pp = omnetpp::fromAnyPtr<VlrIntVidSet>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string VlrIntVidSetDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    VlrIntVidSet *pp = omnetpp::fromAnyPtr<VlrIntVidSet>(object); (void)pp;
    switch (field) {
        default: return "";
    }
}

void VlrIntVidSetDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntVidSet *pp = omnetpp::fromAnyPtr<VlrIntVidSet>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VlrIntVidSet'", field);
    }
}

omnetpp::cValue VlrIntVidSetDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    VlrIntVidSet *pp = omnetpp::fromAnyPtr<VlrIntVidSet>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'VlrIntVidSet' as cValue -- field index out of range?", field);
    }
}

void VlrIntVidSetDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntVidSet *pp = omnetpp::fromAnyPtr<VlrIntVidSet>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VlrIntVidSet'", field);
    }
}

const char *VlrIntVidSetDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    return nullptr;
}

omnetpp::any_ptr VlrIntVidSetDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    VlrIntVidSet *pp = omnetpp::fromAnyPtr<VlrIntVidSet>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void VlrIntVidSetDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntVidSet *pp = omnetpp::fromAnyPtr<VlrIntVidSet>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VlrIntVidSet'", field);
    }
}

class VlrIntVidVecDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
    };
  public:
    VlrIntVidVecDescriptor();
    virtual ~VlrIntVidVecDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(VlrIntVidVecDescriptor)

VlrIntVidVecDescriptor::VlrIntVidVecDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(omnetvlr::VlrIntVidVec)), "")
{
    propertyNames = nullptr;
}

VlrIntVidVecDescriptor::~VlrIntVidVecDescriptor()
{
    delete[] propertyNames;
}

bool VlrIntVidVecDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<VlrIntVidVec *>(obj)!=nullptr;
}

const char **VlrIntVidVecDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = { "existingClass",  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *VlrIntVidVecDescriptor::getProperty(const char *propertyName) const
{
    if (!strcmp(propertyName, "existingClass")) return "";
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int VlrIntVidVecDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 0+base->getFieldCount() : 0;
}

unsigned int VlrIntVidVecDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    return 0;
}

const char *VlrIntVidVecDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    return nullptr;
}

int VlrIntVidVecDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->findField(fieldName) : -1;
}

const char *VlrIntVidVecDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    return nullptr;
}

const char **VlrIntVidVecDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *VlrIntVidVecDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int VlrIntVidVecDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    VlrIntVidVec *pp = omnetpp::fromAnyPtr<VlrIntVidVec>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void VlrIntVidVecDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntVidVec *pp = omnetpp::fromAnyPtr<VlrIntVidVec>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'VlrIntVidVec'", field);
    }
}

const char *VlrIntVidVecDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    VlrIntVidVec *pp = omnetpp::fromAnyPtr<VlrIntVidVec>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string VlrIntVidVecDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    VlrIntVidVec *pp = omnetpp::fromAnyPtr<VlrIntVidVec>(object); (void)pp;
    switch (field) {
        default: return "";
    }
}

void VlrIntVidVecDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntVidVec *pp = omnetpp::fromAnyPtr<VlrIntVidVec>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VlrIntVidVec'", field);
    }
}

omnetpp::cValue VlrIntVidVecDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    VlrIntVidVec *pp = omnetpp::fromAnyPtr<VlrIntVidVec>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'VlrIntVidVec' as cValue -- field index out of range?", field);
    }
}

void VlrIntVidVecDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntVidVec *pp = omnetpp::fromAnyPtr<VlrIntVidVec>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VlrIntVidVec'", field);
    }
}

const char *VlrIntVidVecDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    return nullptr;
}

omnetpp::any_ptr VlrIntVidVecDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    VlrIntVidVec *pp = omnetpp::fromAnyPtr<VlrIntVidVec>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void VlrIntVidVecDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntVidVec *pp = omnetpp::fromAnyPtr<VlrIntVidVec>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VlrIntVidVec'", field);
    }
}

class VlrIntVidToPathidSetMapDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
    };
  public:
    VlrIntVidToPathidSetMapDescriptor();
    virtual ~VlrIntVidToPathidSetMapDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(VlrIntVidToPathidSetMapDescriptor)

VlrIntVidToPathidSetMapDescriptor::VlrIntVidToPathidSetMapDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(omnetvlr::VlrIntVidToPathidSetMap)), "")
{
    propertyNames = nullptr;
}

VlrIntVidToPathidSetMapDescriptor::~VlrIntVidToPathidSetMapDescriptor()
{
    delete[] propertyNames;
}

bool VlrIntVidToPathidSetMapDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<VlrIntVidToPathidSetMap *>(obj)!=nullptr;
}

const char **VlrIntVidToPathidSetMapDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = { "existingClass",  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *VlrIntVidToPathidSetMapDescriptor::getProperty(const char *propertyName) const
{
    if (!strcmp(propertyName, "existingClass")) return "";
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int VlrIntVidToPathidSetMapDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 0+base->getFieldCount() : 0;
}

unsigned int VlrIntVidToPathidSetMapDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    return 0;
}

const char *VlrIntVidToPathidSetMapDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    return nullptr;
}

int VlrIntVidToPathidSetMapDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->findField(fieldName) : -1;
}

const char *VlrIntVidToPathidSetMapDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    return nullptr;
}

const char **VlrIntVidToPathidSetMapDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *VlrIntVidToPathidSetMapDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int VlrIntVidToPathidSetMapDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    VlrIntVidToPathidSetMap *pp = omnetpp::fromAnyPtr<VlrIntVidToPathidSetMap>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void VlrIntVidToPathidSetMapDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntVidToPathidSetMap *pp = omnetpp::fromAnyPtr<VlrIntVidToPathidSetMap>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'VlrIntVidToPathidSetMap'", field);
    }
}

const char *VlrIntVidToPathidSetMapDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    VlrIntVidToPathidSetMap *pp = omnetpp::fromAnyPtr<VlrIntVidToPathidSetMap>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string VlrIntVidToPathidSetMapDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    VlrIntVidToPathidSetMap *pp = omnetpp::fromAnyPtr<VlrIntVidToPathidSetMap>(object); (void)pp;
    switch (field) {
        default: return "";
    }
}

void VlrIntVidToPathidSetMapDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntVidToPathidSetMap *pp = omnetpp::fromAnyPtr<VlrIntVidToPathidSetMap>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VlrIntVidToPathidSetMap'", field);
    }
}

omnetpp::cValue VlrIntVidToPathidSetMapDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    VlrIntVidToPathidSetMap *pp = omnetpp::fromAnyPtr<VlrIntVidToPathidSetMap>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'VlrIntVidToPathidSetMap' as cValue -- field index out of range?", field);
    }
}

void VlrIntVidToPathidSetMapDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntVidToPathidSetMap *pp = omnetpp::fromAnyPtr<VlrIntVidToPathidSetMap>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VlrIntVidToPathidSetMap'", field);
    }
}

const char *VlrIntVidToPathidSetMapDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    return nullptr;
}

omnetpp::any_ptr VlrIntVidToPathidSetMapDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    VlrIntVidToPathidSetMap *pp = omnetpp::fromAnyPtr<VlrIntVidToPathidSetMap>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void VlrIntVidToPathidSetMapDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntVidToPathidSetMap *pp = omnetpp::fromAnyPtr<VlrIntVidToPathidSetMap>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VlrIntVidToPathidSetMap'", field);
    }
}

class VlrIntPathidToVidVecMapDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
    };
  public:
    VlrIntPathidToVidVecMapDescriptor();
    virtual ~VlrIntPathidToVidVecMapDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(VlrIntPathidToVidVecMapDescriptor)

VlrIntPathidToVidVecMapDescriptor::VlrIntPathidToVidVecMapDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(omnetvlr::VlrIntPathidToVidVecMap)), "")
{
    propertyNames = nullptr;
}

VlrIntPathidToVidVecMapDescriptor::~VlrIntPathidToVidVecMapDescriptor()
{
    delete[] propertyNames;
}

bool VlrIntPathidToVidVecMapDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<VlrIntPathidToVidVecMap *>(obj)!=nullptr;
}

const char **VlrIntPathidToVidVecMapDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = { "existingClass",  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *VlrIntPathidToVidVecMapDescriptor::getProperty(const char *propertyName) const
{
    if (!strcmp(propertyName, "existingClass")) return "";
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int VlrIntPathidToVidVecMapDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 0+base->getFieldCount() : 0;
}

unsigned int VlrIntPathidToVidVecMapDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    return 0;
}

const char *VlrIntPathidToVidVecMapDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    return nullptr;
}

int VlrIntPathidToVidVecMapDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->findField(fieldName) : -1;
}

const char *VlrIntPathidToVidVecMapDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    return nullptr;
}

const char **VlrIntPathidToVidVecMapDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *VlrIntPathidToVidVecMapDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int VlrIntPathidToVidVecMapDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    VlrIntPathidToVidVecMap *pp = omnetpp::fromAnyPtr<VlrIntPathidToVidVecMap>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void VlrIntPathidToVidVecMapDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntPathidToVidVecMap *pp = omnetpp::fromAnyPtr<VlrIntPathidToVidVecMap>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'VlrIntPathidToVidVecMap'", field);
    }
}

const char *VlrIntPathidToVidVecMapDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    VlrIntPathidToVidVecMap *pp = omnetpp::fromAnyPtr<VlrIntPathidToVidVecMap>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string VlrIntPathidToVidVecMapDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    VlrIntPathidToVidVecMap *pp = omnetpp::fromAnyPtr<VlrIntPathidToVidVecMap>(object); (void)pp;
    switch (field) {
        default: return "";
    }
}

void VlrIntPathidToVidVecMapDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntPathidToVidVecMap *pp = omnetpp::fromAnyPtr<VlrIntPathidToVidVecMap>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VlrIntPathidToVidVecMap'", field);
    }
}

omnetpp::cValue VlrIntPathidToVidVecMapDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    VlrIntPathidToVidVecMap *pp = omnetpp::fromAnyPtr<VlrIntPathidToVidVecMap>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'VlrIntPathidToVidVecMap' as cValue -- field index out of range?", field);
    }
}

void VlrIntPathidToVidVecMapDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntPathidToVidVecMap *pp = omnetpp::fromAnyPtr<VlrIntPathidToVidVecMap>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VlrIntPathidToVidVecMap'", field);
    }
}

const char *VlrIntPathidToVidVecMapDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    return nullptr;
}

omnetpp::any_ptr VlrIntPathidToVidVecMapDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    VlrIntPathidToVidVecMap *pp = omnetpp::fromAnyPtr<VlrIntPathidToVidVecMap>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void VlrIntPathidToVidVecMapDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntPathidToVidVecMap *pp = omnetpp::fromAnyPtr<VlrIntPathidToVidVecMap>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VlrIntPathidToVidVecMap'", field);
    }
}

VlrIntRepState::VlrIntRepState()
{
}

void __doPacking(omnetpp::cCommBuffer *b, const VlrIntRepState& a)
{
    doParsimPacking(b,a.vid);
    doParsimPacking(b,a.sequencenumber);
    doParsimPacking(b,a.inNetwork);
}

void __doUnpacking(omnetpp::cCommBuffer *b, VlrIntRepState& a)
{
    doParsimUnpacking(b,a.vid);
    doParsimUnpacking(b,a.sequencenumber);
    doParsimUnpacking(b,a.inNetwork);
}

class VlrIntRepStateDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_vid,
        FIELD_sequencenumber,
        FIELD_inNetwork,
    };
  public:
    VlrIntRepStateDescriptor();
    virtual ~VlrIntRepStateDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(VlrIntRepStateDescriptor)

VlrIntRepStateDescriptor::VlrIntRepStateDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(omnetvlr::VlrIntRepState)), "")
{
    propertyNames = nullptr;
}

VlrIntRepStateDescriptor::~VlrIntRepStateDescriptor()
{
    delete[] propertyNames;
}

bool VlrIntRepStateDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<VlrIntRepState *>(obj)!=nullptr;
}

const char **VlrIntRepStateDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *VlrIntRepStateDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int VlrIntRepStateDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 3+base->getFieldCount() : 3;
}

unsigned int VlrIntRepStateDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_vid
        FD_ISEDITABLE,    // FIELD_sequencenumber
        FD_ISEDITABLE,    // FIELD_inNetwork
    };
    return (field >= 0 && field < 3) ? fieldTypeFlags[field] : 0;
}

const char *VlrIntRepStateDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "vid",
        "sequencenumber",
        "inNetwork",
    };
    return (field >= 0 && field < 3) ? fieldNames[field] : nullptr;
}

int VlrIntRepStateDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "vid") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "sequencenumber") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "inNetwork") == 0) return baseIndex + 2;
    return base ? base->findField(fieldName) : -1;
}

const char *VlrIntRepStateDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "unsigned int",    // FIELD_vid
        "unsigned int",    // FIELD_sequencenumber
        "bool",    // FIELD_inNetwork
    };
    return (field >= 0 && field < 3) ? fieldTypeStrings[field] : nullptr;
}

const char **VlrIntRepStateDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *VlrIntRepStateDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int VlrIntRepStateDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    VlrIntRepState *pp = omnetpp::fromAnyPtr<VlrIntRepState>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void VlrIntRepStateDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntRepState *pp = omnetpp::fromAnyPtr<VlrIntRepState>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'VlrIntRepState'", field);
    }
}

const char *VlrIntRepStateDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    VlrIntRepState *pp = omnetpp::fromAnyPtr<VlrIntRepState>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string VlrIntRepStateDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    VlrIntRepState *pp = omnetpp::fromAnyPtr<VlrIntRepState>(object); (void)pp;
    switch (field) {
        case FIELD_vid: return ulong2string(pp->vid);
        case FIELD_sequencenumber: return ulong2string(pp->sequencenumber);
        case FIELD_inNetwork: return bool2string(pp->inNetwork);
        default: return "";
    }
}

void VlrIntRepStateDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntRepState *pp = omnetpp::fromAnyPtr<VlrIntRepState>(object); (void)pp;
    switch (field) {
        case FIELD_vid: pp->vid = string2ulong(value); break;
        case FIELD_sequencenumber: pp->sequencenumber = string2ulong(value); break;
        case FIELD_inNetwork: pp->inNetwork = string2bool(value); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VlrIntRepState'", field);
    }
}

omnetpp::cValue VlrIntRepStateDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    VlrIntRepState *pp = omnetpp::fromAnyPtr<VlrIntRepState>(object); (void)pp;
    switch (field) {
        case FIELD_vid: return (omnetpp::intval_t)(pp->vid);
        case FIELD_sequencenumber: return (omnetpp::intval_t)(pp->sequencenumber);
        case FIELD_inNetwork: return pp->inNetwork;
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'VlrIntRepState' as cValue -- field index out of range?", field);
    }
}

void VlrIntRepStateDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntRepState *pp = omnetpp::fromAnyPtr<VlrIntRepState>(object); (void)pp;
    switch (field) {
        case FIELD_vid: pp->vid = omnetpp::checked_int_cast<unsigned int>(value.intValue()); break;
        case FIELD_sequencenumber: pp->sequencenumber = omnetpp::checked_int_cast<unsigned int>(value.intValue()); break;
        case FIELD_inNetwork: pp->inNetwork = value.boolValue(); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VlrIntRepState'", field);
    }
}

const char *VlrIntRepStateDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr VlrIntRepStateDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    VlrIntRepState *pp = omnetpp::fromAnyPtr<VlrIntRepState>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void VlrIntRepStateDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntRepState *pp = omnetpp::fromAnyPtr<VlrIntRepState>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VlrIntRepState'", field);
    }
}

Register_Class(WaitSetupReqIntTimer)

WaitSetupReqIntTimer::WaitSetupReqIntTimer(const char *name, short kind) : ::omnetpp::cMessage(name, kind)
{
}

WaitSetupReqIntTimer::WaitSetupReqIntTimer(const WaitSetupReqIntTimer& other) : ::omnetpp::cMessage(other)
{
    copy(other);
}

WaitSetupReqIntTimer::~WaitSetupReqIntTimer()
{
}

WaitSetupReqIntTimer& WaitSetupReqIntTimer::operator=(const WaitSetupReqIntTimer& other)
{
    if (this == &other) return *this;
    ::omnetpp::cMessage::operator=(other);
    copy(other);
    return *this;
}

void WaitSetupReqIntTimer::copy(const WaitSetupReqIntTimer& other)
{
    this->dst = other.dst;
    this->retryCount = other.retryCount;
    this->repairRoute = other.repairRoute;
    this->patchedRoute = other.patchedRoute;
    this->reqVnei = other.reqVnei;
    this->alterPendingVnei = other.alterPendingVnei;
    this->timerType = other.timerType;
}

void WaitSetupReqIntTimer::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cMessage::parsimPack(b);
    doParsimPacking(b,this->dst);
    doParsimPacking(b,this->retryCount);
    doParsimPacking(b,this->repairRoute);
    doParsimPacking(b,this->patchedRoute);
    doParsimPacking(b,this->reqVnei);
    doParsimPacking(b,this->alterPendingVnei);
    doParsimPacking(b,this->timerType);
}

void WaitSetupReqIntTimer::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cMessage::parsimUnpack(b);
    doParsimUnpacking(b,this->dst);
    doParsimUnpacking(b,this->retryCount);
    doParsimUnpacking(b,this->repairRoute);
    doParsimUnpacking(b,this->patchedRoute);
    doParsimUnpacking(b,this->reqVnei);
    doParsimUnpacking(b,this->alterPendingVnei);
    doParsimUnpacking(b,this->timerType);
}

unsigned int WaitSetupReqIntTimer::getDst() const
{
    return this->dst;
}

void WaitSetupReqIntTimer::setDst(unsigned int dst)
{
    this->dst = dst;
}

int WaitSetupReqIntTimer::getRetryCount() const
{
    return this->retryCount;
}

void WaitSetupReqIntTimer::setRetryCount(int retryCount)
{
    this->retryCount = retryCount;
}

bool WaitSetupReqIntTimer::getRepairRoute() const
{
    return this->repairRoute;
}

void WaitSetupReqIntTimer::setRepairRoute(bool repairRoute)
{
    this->repairRoute = repairRoute;
}

const VlrPathID& WaitSetupReqIntTimer::getPatchedRoute() const
{
    return this->patchedRoute;
}

void WaitSetupReqIntTimer::setPatchedRoute(const VlrPathID& patchedRoute)
{
    this->patchedRoute = patchedRoute;
}

bool WaitSetupReqIntTimer::getReqVnei() const
{
    return this->reqVnei;
}

void WaitSetupReqIntTimer::setReqVnei(bool reqVnei)
{
    this->reqVnei = reqVnei;
}

unsigned int WaitSetupReqIntTimer::getAlterPendingVnei() const
{
    return this->alterPendingVnei;
}

void WaitSetupReqIntTimer::setAlterPendingVnei(unsigned int alterPendingVnei)
{
    this->alterPendingVnei = alterPendingVnei;
}

char WaitSetupReqIntTimer::getTimerType() const
{
    return this->timerType;
}

void WaitSetupReqIntTimer::setTimerType(char timerType)
{
    this->timerType = timerType;
}

class WaitSetupReqIntTimerDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_dst,
        FIELD_retryCount,
        FIELD_repairRoute,
        FIELD_patchedRoute,
        FIELD_reqVnei,
        FIELD_alterPendingVnei,
        FIELD_timerType,
    };
  public:
    WaitSetupReqIntTimerDescriptor();
    virtual ~WaitSetupReqIntTimerDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(WaitSetupReqIntTimerDescriptor)

WaitSetupReqIntTimerDescriptor::WaitSetupReqIntTimerDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(omnetvlr::WaitSetupReqIntTimer)), "omnetpp::cMessage")
{
    propertyNames = nullptr;
}

WaitSetupReqIntTimerDescriptor::~WaitSetupReqIntTimerDescriptor()
{
    delete[] propertyNames;
}

bool WaitSetupReqIntTimerDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<WaitSetupReqIntTimer *>(obj)!=nullptr;
}

const char **WaitSetupReqIntTimerDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *WaitSetupReqIntTimerDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int WaitSetupReqIntTimerDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 7+base->getFieldCount() : 7;
}

unsigned int WaitSetupReqIntTimerDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_dst
        FD_ISEDITABLE,    // FIELD_retryCount
        FD_ISEDITABLE,    // FIELD_repairRoute
        0,    // FIELD_patchedRoute
        FD_ISEDITABLE,    // FIELD_reqVnei
        FD_ISEDITABLE,    // FIELD_alterPendingVnei
        FD_ISEDITABLE,    // FIELD_timerType
    };
    return (field >= 0 && field < 7) ? fieldTypeFlags[field] : 0;
}

const char *WaitSetupReqIntTimerDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "dst",
        "retryCount",
        "repairRoute",
        "patchedRoute",
        "reqVnei",
        "alterPendingVnei",
        "timerType",
    };
    return (field >= 0 && field < 7) ? fieldNames[field] : nullptr;
}

int WaitSetupReqIntTimerDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "dst") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "retryCount") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "repairRoute") == 0) return baseIndex + 2;
    if (strcmp(fieldName, "patchedRoute") == 0) return baseIndex + 3;
    if (strcmp(fieldName, "reqVnei") == 0) return baseIndex + 4;
    if (strcmp(fieldName, "alterPendingVnei") == 0) return baseIndex + 5;
    if (strcmp(fieldName, "timerType") == 0) return baseIndex + 6;
    return base ? base->findField(fieldName) : -1;
}

const char *WaitSetupReqIntTimerDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "unsigned int",    // FIELD_dst
        "int",    // FIELD_retryCount
        "bool",    // FIELD_repairRoute
        "omnetvlr::VlrPathID",    // FIELD_patchedRoute
        "bool",    // FIELD_reqVnei
        "unsigned int",    // FIELD_alterPendingVnei
        "char",    // FIELD_timerType
    };
    return (field >= 0 && field < 7) ? fieldTypeStrings[field] : nullptr;
}

const char **WaitSetupReqIntTimerDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *WaitSetupReqIntTimerDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int WaitSetupReqIntTimerDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    WaitSetupReqIntTimer *pp = omnetpp::fromAnyPtr<WaitSetupReqIntTimer>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void WaitSetupReqIntTimerDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    WaitSetupReqIntTimer *pp = omnetpp::fromAnyPtr<WaitSetupReqIntTimer>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'WaitSetupReqIntTimer'", field);
    }
}

const char *WaitSetupReqIntTimerDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    WaitSetupReqIntTimer *pp = omnetpp::fromAnyPtr<WaitSetupReqIntTimer>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string WaitSetupReqIntTimerDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    WaitSetupReqIntTimer *pp = omnetpp::fromAnyPtr<WaitSetupReqIntTimer>(object); (void)pp;
    switch (field) {
        case FIELD_dst: return ulong2string(pp->getDst());
        case FIELD_retryCount: return long2string(pp->getRetryCount());
        case FIELD_repairRoute: return bool2string(pp->getRepairRoute());
        case FIELD_patchedRoute: return ulong2string(pp->getPatchedRoute());
        case FIELD_reqVnei: return bool2string(pp->getReqVnei());
        case FIELD_alterPendingVnei: return ulong2string(pp->getAlterPendingVnei());
        case FIELD_timerType: return long2string(pp->getTimerType());
        default: return "";
    }
}

void WaitSetupReqIntTimerDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    WaitSetupReqIntTimer *pp = omnetpp::fromAnyPtr<WaitSetupReqIntTimer>(object); (void)pp;
    switch (field) {
        case FIELD_dst: pp->setDst(string2ulong(value)); break;
        case FIELD_retryCount: pp->setRetryCount(string2long(value)); break;
        case FIELD_repairRoute: pp->setRepairRoute(string2bool(value)); break;
        case FIELD_reqVnei: pp->setReqVnei(string2bool(value)); break;
        case FIELD_alterPendingVnei: pp->setAlterPendingVnei(string2ulong(value)); break;
        case FIELD_timerType: pp->setTimerType(string2long(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'WaitSetupReqIntTimer'", field);
    }
}

omnetpp::cValue WaitSetupReqIntTimerDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    WaitSetupReqIntTimer *pp = omnetpp::fromAnyPtr<WaitSetupReqIntTimer>(object); (void)pp;
    switch (field) {
        case FIELD_dst: return (omnetpp::intval_t)(pp->getDst());
        case FIELD_retryCount: return pp->getRetryCount();
        case FIELD_repairRoute: return pp->getRepairRoute();
        case FIELD_patchedRoute: return omnetpp::toAnyPtr(&pp->getPatchedRoute()); break;
        case FIELD_reqVnei: return pp->getReqVnei();
        case FIELD_alterPendingVnei: return (omnetpp::intval_t)(pp->getAlterPendingVnei());
        case FIELD_timerType: return pp->getTimerType();
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'WaitSetupReqIntTimer' as cValue -- field index out of range?", field);
    }
}

void WaitSetupReqIntTimerDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    WaitSetupReqIntTimer *pp = omnetpp::fromAnyPtr<WaitSetupReqIntTimer>(object); (void)pp;
    switch (field) {
        case FIELD_dst: pp->setDst(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        case FIELD_retryCount: pp->setRetryCount(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_repairRoute: pp->setRepairRoute(value.boolValue()); break;
        case FIELD_reqVnei: pp->setReqVnei(value.boolValue()); break;
        case FIELD_alterPendingVnei: pp->setAlterPendingVnei(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        case FIELD_timerType: pp->setTimerType(omnetpp::checked_int_cast<char>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'WaitSetupReqIntTimer'", field);
    }
}

const char *WaitSetupReqIntTimerDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr WaitSetupReqIntTimerDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    WaitSetupReqIntTimer *pp = omnetpp::fromAnyPtr<WaitSetupReqIntTimer>(object); (void)pp;
    switch (field) {
        case FIELD_patchedRoute: return omnetpp::toAnyPtr(&pp->getPatchedRoute()); break;
        default: return omnetpp::any_ptr(nullptr);
    }
}

void WaitSetupReqIntTimerDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    WaitSetupReqIntTimer *pp = omnetpp::fromAnyPtr<WaitSetupReqIntTimer>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'WaitSetupReqIntTimer'", field);
    }
}

Register_Class(FailedPacketDelayTimer)

FailedPacketDelayTimer::FailedPacketDelayTimer(const char *name, short kind) : ::omnetpp::cMessage(name, kind)
{
}

FailedPacketDelayTimer::FailedPacketDelayTimer(const FailedPacketDelayTimer& other) : ::omnetpp::cMessage(other)
{
    copy(other);
}

FailedPacketDelayTimer::~FailedPacketDelayTimer()
{
}

FailedPacketDelayTimer& FailedPacketDelayTimer::operator=(const FailedPacketDelayTimer& other)
{
    if (this == &other) return *this;
    ::omnetpp::cMessage::operator=(other);
    copy(other);
    return *this;
}

void FailedPacketDelayTimer::copy(const FailedPacketDelayTimer& other)
{
    this->failedGateIndex = other.failedGateIndex;
    this->failedPnei = other.failedPnei;
    this->failedPacket = other.failedPacket;
}

void FailedPacketDelayTimer::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cMessage::parsimPack(b);
    doParsimPacking(b,this->failedGateIndex);
    doParsimPacking(b,this->failedPnei);
    doParsimPacking(b,this->failedPacket);
}

void FailedPacketDelayTimer::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cMessage::parsimUnpack(b);
    doParsimUnpacking(b,this->failedGateIndex);
    doParsimUnpacking(b,this->failedPnei);
    doParsimUnpacking(b,this->failedPacket);
}

int FailedPacketDelayTimer::getFailedGateIndex() const
{
    return this->failedGateIndex;
}

void FailedPacketDelayTimer::setFailedGateIndex(int failedGateIndex)
{
    this->failedGateIndex = failedGateIndex;
}

unsigned int FailedPacketDelayTimer::getFailedPnei() const
{
    return this->failedPnei;
}

void FailedPacketDelayTimer::setFailedPnei(unsigned int failedPnei)
{
    this->failedPnei = failedPnei;
}

const ::omnetpp::cPacket * FailedPacketDelayTimer::getFailedPacket() const
{
    return this->failedPacket;
}

void FailedPacketDelayTimer::setFailedPacket(::omnetpp::cPacket * failedPacket)
{
    this->failedPacket = failedPacket;
}

class FailedPacketDelayTimerDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_failedGateIndex,
        FIELD_failedPnei,
        FIELD_failedPacket,
    };
  public:
    FailedPacketDelayTimerDescriptor();
    virtual ~FailedPacketDelayTimerDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(FailedPacketDelayTimerDescriptor)

FailedPacketDelayTimerDescriptor::FailedPacketDelayTimerDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(omnetvlr::FailedPacketDelayTimer)), "omnetpp::cMessage")
{
    propertyNames = nullptr;
}

FailedPacketDelayTimerDescriptor::~FailedPacketDelayTimerDescriptor()
{
    delete[] propertyNames;
}

bool FailedPacketDelayTimerDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<FailedPacketDelayTimer *>(obj)!=nullptr;
}

const char **FailedPacketDelayTimerDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *FailedPacketDelayTimerDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int FailedPacketDelayTimerDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 3+base->getFieldCount() : 3;
}

unsigned int FailedPacketDelayTimerDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_failedGateIndex
        FD_ISEDITABLE,    // FIELD_failedPnei
        FD_ISCOMPOUND | FD_ISPOINTER | FD_ISCOBJECT | FD_ISCOWNEDOBJECT | FD_ISREPLACEABLE,    // FIELD_failedPacket
    };
    return (field >= 0 && field < 3) ? fieldTypeFlags[field] : 0;
}

const char *FailedPacketDelayTimerDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "failedGateIndex",
        "failedPnei",
        "failedPacket",
    };
    return (field >= 0 && field < 3) ? fieldNames[field] : nullptr;
}

int FailedPacketDelayTimerDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "failedGateIndex") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "failedPnei") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "failedPacket") == 0) return baseIndex + 2;
    return base ? base->findField(fieldName) : -1;
}

const char *FailedPacketDelayTimerDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",    // FIELD_failedGateIndex
        "unsigned int",    // FIELD_failedPnei
        "omnetpp::cPacket",    // FIELD_failedPacket
    };
    return (field >= 0 && field < 3) ? fieldTypeStrings[field] : nullptr;
}

const char **FailedPacketDelayTimerDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *FailedPacketDelayTimerDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int FailedPacketDelayTimerDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    FailedPacketDelayTimer *pp = omnetpp::fromAnyPtr<FailedPacketDelayTimer>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void FailedPacketDelayTimerDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    FailedPacketDelayTimer *pp = omnetpp::fromAnyPtr<FailedPacketDelayTimer>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'FailedPacketDelayTimer'", field);
    }
}

const char *FailedPacketDelayTimerDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    FailedPacketDelayTimer *pp = omnetpp::fromAnyPtr<FailedPacketDelayTimer>(object); (void)pp;
    switch (field) {
        case FIELD_failedPacket: { const ::omnetpp::cPacket * value = pp->getFailedPacket(); return omnetpp::opp_typename(typeid(*value)); }
        default: return nullptr;
    }
}

std::string FailedPacketDelayTimerDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    FailedPacketDelayTimer *pp = omnetpp::fromAnyPtr<FailedPacketDelayTimer>(object); (void)pp;
    switch (field) {
        case FIELD_failedGateIndex: return long2string(pp->getFailedGateIndex());
        case FIELD_failedPnei: return ulong2string(pp->getFailedPnei());
        case FIELD_failedPacket: { auto obj = pp->getFailedPacket(); return obj == nullptr ? "" : obj->str(); }
        default: return "";
    }
}

void FailedPacketDelayTimerDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    FailedPacketDelayTimer *pp = omnetpp::fromAnyPtr<FailedPacketDelayTimer>(object); (void)pp;
    switch (field) {
        case FIELD_failedGateIndex: pp->setFailedGateIndex(string2long(value)); break;
        case FIELD_failedPnei: pp->setFailedPnei(string2ulong(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'FailedPacketDelayTimer'", field);
    }
}

omnetpp::cValue FailedPacketDelayTimerDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    FailedPacketDelayTimer *pp = omnetpp::fromAnyPtr<FailedPacketDelayTimer>(object); (void)pp;
    switch (field) {
        case FIELD_failedGateIndex: return pp->getFailedGateIndex();
        case FIELD_failedPnei: return (omnetpp::intval_t)(pp->getFailedPnei());
        case FIELD_failedPacket: return omnetpp::toAnyPtr(pp->getFailedPacket()); break;
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'FailedPacketDelayTimer' as cValue -- field index out of range?", field);
    }
}

void FailedPacketDelayTimerDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    FailedPacketDelayTimer *pp = omnetpp::fromAnyPtr<FailedPacketDelayTimer>(object); (void)pp;
    switch (field) {
        case FIELD_failedGateIndex: pp->setFailedGateIndex(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_failedPnei: pp->setFailedPnei(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        case FIELD_failedPacket: pp->setFailedPacket(omnetpp::fromAnyPtr<::omnetpp::cPacket>(value.pointerValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'FailedPacketDelayTimer'", field);
    }
}

const char *FailedPacketDelayTimerDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        case FIELD_failedPacket: return omnetpp::opp_typename(typeid(::omnetpp::cPacket));
        default: return nullptr;
    };
}

omnetpp::any_ptr FailedPacketDelayTimerDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    FailedPacketDelayTimer *pp = omnetpp::fromAnyPtr<FailedPacketDelayTimer>(object); (void)pp;
    switch (field) {
        case FIELD_failedPacket: return omnetpp::toAnyPtr(pp->getFailedPacket()); break;
        default: return omnetpp::any_ptr(nullptr);
    }
}

void FailedPacketDelayTimerDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    FailedPacketDelayTimer *pp = omnetpp::fromAnyPtr<FailedPacketDelayTimer>(object); (void)pp;
    switch (field) {
        case FIELD_failedPacket: pp->setFailedPacket(omnetpp::fromAnyPtr<::omnetpp::cPacket>(ptr)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'FailedPacketDelayTimer'", field);
    }
}

Register_Class(WaitRepairLinkIntTimer)

WaitRepairLinkIntTimer::WaitRepairLinkIntTimer(const char *name, short kind) : ::omnetpp::cMessage(name, kind)
{
}

WaitRepairLinkIntTimer::WaitRepairLinkIntTimer(const WaitRepairLinkIntTimer& other) : ::omnetpp::cMessage(other)
{
    copy(other);
}

WaitRepairLinkIntTimer::~WaitRepairLinkIntTimer()
{
}

WaitRepairLinkIntTimer& WaitRepairLinkIntTimer::operator=(const WaitRepairLinkIntTimer& other)
{
    if (this == &other) return *this;
    ::omnetpp::cMessage::operator=(other);
    copy(other);
    return *this;
}

void WaitRepairLinkIntTimer::copy(const WaitRepairLinkIntTimer& other)
{
    this->dst = other.dst;
    this->retryCount = other.retryCount;
}

void WaitRepairLinkIntTimer::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cMessage::parsimPack(b);
    doParsimPacking(b,this->dst);
    doParsimPacking(b,this->retryCount);
}

void WaitRepairLinkIntTimer::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cMessage::parsimUnpack(b);
    doParsimUnpacking(b,this->dst);
    doParsimUnpacking(b,this->retryCount);
}

unsigned int WaitRepairLinkIntTimer::getDst() const
{
    return this->dst;
}

void WaitRepairLinkIntTimer::setDst(unsigned int dst)
{
    this->dst = dst;
}

int WaitRepairLinkIntTimer::getRetryCount() const
{
    return this->retryCount;
}

void WaitRepairLinkIntTimer::setRetryCount(int retryCount)
{
    this->retryCount = retryCount;
}

class WaitRepairLinkIntTimerDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_dst,
        FIELD_retryCount,
    };
  public:
    WaitRepairLinkIntTimerDescriptor();
    virtual ~WaitRepairLinkIntTimerDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(WaitRepairLinkIntTimerDescriptor)

WaitRepairLinkIntTimerDescriptor::WaitRepairLinkIntTimerDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(omnetvlr::WaitRepairLinkIntTimer)), "omnetpp::cMessage")
{
    propertyNames = nullptr;
}

WaitRepairLinkIntTimerDescriptor::~WaitRepairLinkIntTimerDescriptor()
{
    delete[] propertyNames;
}

bool WaitRepairLinkIntTimerDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<WaitRepairLinkIntTimer *>(obj)!=nullptr;
}

const char **WaitRepairLinkIntTimerDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *WaitRepairLinkIntTimerDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int WaitRepairLinkIntTimerDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 2+base->getFieldCount() : 2;
}

unsigned int WaitRepairLinkIntTimerDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_dst
        FD_ISEDITABLE,    // FIELD_retryCount
    };
    return (field >= 0 && field < 2) ? fieldTypeFlags[field] : 0;
}

const char *WaitRepairLinkIntTimerDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "dst",
        "retryCount",
    };
    return (field >= 0 && field < 2) ? fieldNames[field] : nullptr;
}

int WaitRepairLinkIntTimerDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "dst") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "retryCount") == 0) return baseIndex + 1;
    return base ? base->findField(fieldName) : -1;
}

const char *WaitRepairLinkIntTimerDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "unsigned int",    // FIELD_dst
        "int",    // FIELD_retryCount
    };
    return (field >= 0 && field < 2) ? fieldTypeStrings[field] : nullptr;
}

const char **WaitRepairLinkIntTimerDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *WaitRepairLinkIntTimerDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int WaitRepairLinkIntTimerDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    WaitRepairLinkIntTimer *pp = omnetpp::fromAnyPtr<WaitRepairLinkIntTimer>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void WaitRepairLinkIntTimerDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    WaitRepairLinkIntTimer *pp = omnetpp::fromAnyPtr<WaitRepairLinkIntTimer>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'WaitRepairLinkIntTimer'", field);
    }
}

const char *WaitRepairLinkIntTimerDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    WaitRepairLinkIntTimer *pp = omnetpp::fromAnyPtr<WaitRepairLinkIntTimer>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string WaitRepairLinkIntTimerDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    WaitRepairLinkIntTimer *pp = omnetpp::fromAnyPtr<WaitRepairLinkIntTimer>(object); (void)pp;
    switch (field) {
        case FIELD_dst: return ulong2string(pp->getDst());
        case FIELD_retryCount: return long2string(pp->getRetryCount());
        default: return "";
    }
}

void WaitRepairLinkIntTimerDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    WaitRepairLinkIntTimer *pp = omnetpp::fromAnyPtr<WaitRepairLinkIntTimer>(object); (void)pp;
    switch (field) {
        case FIELD_dst: pp->setDst(string2ulong(value)); break;
        case FIELD_retryCount: pp->setRetryCount(string2long(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'WaitRepairLinkIntTimer'", field);
    }
}

omnetpp::cValue WaitRepairLinkIntTimerDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    WaitRepairLinkIntTimer *pp = omnetpp::fromAnyPtr<WaitRepairLinkIntTimer>(object); (void)pp;
    switch (field) {
        case FIELD_dst: return (omnetpp::intval_t)(pp->getDst());
        case FIELD_retryCount: return pp->getRetryCount();
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'WaitRepairLinkIntTimer' as cValue -- field index out of range?", field);
    }
}

void WaitRepairLinkIntTimerDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    WaitRepairLinkIntTimer *pp = omnetpp::fromAnyPtr<WaitRepairLinkIntTimer>(object); (void)pp;
    switch (field) {
        case FIELD_dst: pp->setDst(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        case FIELD_retryCount: pp->setRetryCount(omnetpp::checked_int_cast<int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'WaitRepairLinkIntTimer'", field);
    }
}

const char *WaitRepairLinkIntTimerDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr WaitRepairLinkIntTimerDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    WaitRepairLinkIntTimer *pp = omnetpp::fromAnyPtr<WaitRepairLinkIntTimer>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void WaitRepairLinkIntTimerDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    WaitRepairLinkIntTimer *pp = omnetpp::fromAnyPtr<WaitRepairLinkIntTimer>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'WaitRepairLinkIntTimer'", field);
    }
}

Register_Class(VlrIntBeacon)

VlrIntBeacon::VlrIntBeacon(const char *name, short kind) : ::omnetpp::cPacket(name, kind)
{
}

VlrIntBeacon::VlrIntBeacon(const VlrIntBeacon& other) : ::omnetpp::cPacket(other)
{
    copy(other);
}

VlrIntBeacon::~VlrIntBeacon()
{
    delete [] this->psetNeighbour;
    delete [] this->psetNeighbourIsLinked;
    delete [] this->psetNeighbourIsInNetwork;
}

VlrIntBeacon& VlrIntBeacon::operator=(const VlrIntBeacon& other)
{
    if (this == &other) return *this;
    ::omnetpp::cPacket::operator=(other);
    copy(other);
    return *this;
}

void VlrIntBeacon::copy(const VlrIntBeacon& other)
{
    this->vid = other.vid;
    this->inNetwork = other.inNetwork;
    this->repstate = other.repstate;
    this->repstate2 = other.repstate2;
    delete [] this->psetNeighbour;
    this->psetNeighbour = (other.psetNeighbour_arraysize==0) ? nullptr : new unsigned int[other.psetNeighbour_arraysize];
    psetNeighbour_arraysize = other.psetNeighbour_arraysize;
    for (size_t i = 0; i < psetNeighbour_arraysize; i++) {
        this->psetNeighbour[i] = other.psetNeighbour[i];
    }
    delete [] this->psetNeighbourIsLinked;
    this->psetNeighbourIsLinked = (other.psetNeighbourIsLinked_arraysize==0) ? nullptr : new bool[other.psetNeighbourIsLinked_arraysize];
    psetNeighbourIsLinked_arraysize = other.psetNeighbourIsLinked_arraysize;
    for (size_t i = 0; i < psetNeighbourIsLinked_arraysize; i++) {
        this->psetNeighbourIsLinked[i] = other.psetNeighbourIsLinked[i];
    }
    delete [] this->psetNeighbourIsInNetwork;
    this->psetNeighbourIsInNetwork = (other.psetNeighbourIsInNetwork_arraysize==0) ? nullptr : new bool[other.psetNeighbourIsInNetwork_arraysize];
    psetNeighbourIsInNetwork_arraysize = other.psetNeighbourIsInNetwork_arraysize;
    for (size_t i = 0; i < psetNeighbourIsInNetwork_arraysize; i++) {
        this->psetNeighbourIsInNetwork[i] = other.psetNeighbourIsInNetwork[i];
    }
}

void VlrIntBeacon::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cPacket::parsimPack(b);
    doParsimPacking(b,this->vid);
    doParsimPacking(b,this->inNetwork);
    doParsimPacking(b,this->repstate);
    doParsimPacking(b,this->repstate2);
    b->pack(psetNeighbour_arraysize);
    doParsimArrayPacking(b,this->psetNeighbour,psetNeighbour_arraysize);
    b->pack(psetNeighbourIsLinked_arraysize);
    doParsimArrayPacking(b,this->psetNeighbourIsLinked,psetNeighbourIsLinked_arraysize);
    b->pack(psetNeighbourIsInNetwork_arraysize);
    doParsimArrayPacking(b,this->psetNeighbourIsInNetwork,psetNeighbourIsInNetwork_arraysize);
}

void VlrIntBeacon::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->vid);
    doParsimUnpacking(b,this->inNetwork);
    doParsimUnpacking(b,this->repstate);
    doParsimUnpacking(b,this->repstate2);
    delete [] this->psetNeighbour;
    b->unpack(psetNeighbour_arraysize);
    if (psetNeighbour_arraysize == 0) {
        this->psetNeighbour = nullptr;
    } else {
        this->psetNeighbour = new unsigned int[psetNeighbour_arraysize];
        doParsimArrayUnpacking(b,this->psetNeighbour,psetNeighbour_arraysize);
    }
    delete [] this->psetNeighbourIsLinked;
    b->unpack(psetNeighbourIsLinked_arraysize);
    if (psetNeighbourIsLinked_arraysize == 0) {
        this->psetNeighbourIsLinked = nullptr;
    } else {
        this->psetNeighbourIsLinked = new bool[psetNeighbourIsLinked_arraysize];
        doParsimArrayUnpacking(b,this->psetNeighbourIsLinked,psetNeighbourIsLinked_arraysize);
    }
    delete [] this->psetNeighbourIsInNetwork;
    b->unpack(psetNeighbourIsInNetwork_arraysize);
    if (psetNeighbourIsInNetwork_arraysize == 0) {
        this->psetNeighbourIsInNetwork = nullptr;
    } else {
        this->psetNeighbourIsInNetwork = new bool[psetNeighbourIsInNetwork_arraysize];
        doParsimArrayUnpacking(b,this->psetNeighbourIsInNetwork,psetNeighbourIsInNetwork_arraysize);
    }
}

unsigned int VlrIntBeacon::getVid() const
{
    return this->vid;
}

void VlrIntBeacon::setVid(unsigned int vid)
{
    this->vid = vid;
}

bool VlrIntBeacon::getInNetwork() const
{
    return this->inNetwork;
}

void VlrIntBeacon::setInNetwork(bool inNetwork)
{
    this->inNetwork = inNetwork;
}

const VlrIntRepState& VlrIntBeacon::getRepstate() const
{
    return this->repstate;
}

void VlrIntBeacon::setRepstate(const VlrIntRepState& repstate)
{
    this->repstate = repstate;
}

const VlrIntRepState& VlrIntBeacon::getRepstate2() const
{
    return this->repstate2;
}

void VlrIntBeacon::setRepstate2(const VlrIntRepState& repstate2)
{
    this->repstate2 = repstate2;
}

size_t VlrIntBeacon::getPsetNeighbourArraySize() const
{
    return psetNeighbour_arraysize;
}

unsigned int VlrIntBeacon::getPsetNeighbour(size_t k) const
{
    if (k >= psetNeighbour_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)psetNeighbour_arraysize, (unsigned long)k);
    return this->psetNeighbour[k];
}

void VlrIntBeacon::setPsetNeighbourArraySize(size_t newSize)
{
    unsigned int *psetNeighbour2 = (newSize==0) ? nullptr : new unsigned int[newSize];
    size_t minSize = psetNeighbour_arraysize < newSize ? psetNeighbour_arraysize : newSize;
    for (size_t i = 0; i < minSize; i++)
        psetNeighbour2[i] = this->psetNeighbour[i];
    for (size_t i = minSize; i < newSize; i++)
        psetNeighbour2[i] = 0;
    delete [] this->psetNeighbour;
    this->psetNeighbour = psetNeighbour2;
    psetNeighbour_arraysize = newSize;
}

void VlrIntBeacon::setPsetNeighbour(size_t k, unsigned int psetNeighbour)
{
    if (k >= psetNeighbour_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)psetNeighbour_arraysize, (unsigned long)k);
    this->psetNeighbour[k] = psetNeighbour;
}

void VlrIntBeacon::insertPsetNeighbour(size_t k, unsigned int psetNeighbour)
{
    if (k > psetNeighbour_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)psetNeighbour_arraysize, (unsigned long)k);
    size_t newSize = psetNeighbour_arraysize + 1;
    unsigned int *psetNeighbour2 = new unsigned int[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        psetNeighbour2[i] = this->psetNeighbour[i];
    psetNeighbour2[k] = psetNeighbour;
    for (i = k + 1; i < newSize; i++)
        psetNeighbour2[i] = this->psetNeighbour[i-1];
    delete [] this->psetNeighbour;
    this->psetNeighbour = psetNeighbour2;
    psetNeighbour_arraysize = newSize;
}

void VlrIntBeacon::appendPsetNeighbour(unsigned int psetNeighbour)
{
    insertPsetNeighbour(psetNeighbour_arraysize, psetNeighbour);
}

void VlrIntBeacon::erasePsetNeighbour(size_t k)
{
    if (k >= psetNeighbour_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)psetNeighbour_arraysize, (unsigned long)k);
    size_t newSize = psetNeighbour_arraysize - 1;
    unsigned int *psetNeighbour2 = (newSize == 0) ? nullptr : new unsigned int[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        psetNeighbour2[i] = this->psetNeighbour[i];
    for (i = k; i < newSize; i++)
        psetNeighbour2[i] = this->psetNeighbour[i+1];
    delete [] this->psetNeighbour;
    this->psetNeighbour = psetNeighbour2;
    psetNeighbour_arraysize = newSize;
}

size_t VlrIntBeacon::getPsetNeighbourIsLinkedArraySize() const
{
    return psetNeighbourIsLinked_arraysize;
}

bool VlrIntBeacon::getPsetNeighbourIsLinked(size_t k) const
{
    if (k >= psetNeighbourIsLinked_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)psetNeighbourIsLinked_arraysize, (unsigned long)k);
    return this->psetNeighbourIsLinked[k];
}

void VlrIntBeacon::setPsetNeighbourIsLinkedArraySize(size_t newSize)
{
    bool *psetNeighbourIsLinked2 = (newSize==0) ? nullptr : new bool[newSize];
    size_t minSize = psetNeighbourIsLinked_arraysize < newSize ? psetNeighbourIsLinked_arraysize : newSize;
    for (size_t i = 0; i < minSize; i++)
        psetNeighbourIsLinked2[i] = this->psetNeighbourIsLinked[i];
    for (size_t i = minSize; i < newSize; i++)
        psetNeighbourIsLinked2[i] = false;
    delete [] this->psetNeighbourIsLinked;
    this->psetNeighbourIsLinked = psetNeighbourIsLinked2;
    psetNeighbourIsLinked_arraysize = newSize;
}

void VlrIntBeacon::setPsetNeighbourIsLinked(size_t k, bool psetNeighbourIsLinked)
{
    if (k >= psetNeighbourIsLinked_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)psetNeighbourIsLinked_arraysize, (unsigned long)k);
    this->psetNeighbourIsLinked[k] = psetNeighbourIsLinked;
}

void VlrIntBeacon::insertPsetNeighbourIsLinked(size_t k, bool psetNeighbourIsLinked)
{
    if (k > psetNeighbourIsLinked_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)psetNeighbourIsLinked_arraysize, (unsigned long)k);
    size_t newSize = psetNeighbourIsLinked_arraysize + 1;
    bool *psetNeighbourIsLinked2 = new bool[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        psetNeighbourIsLinked2[i] = this->psetNeighbourIsLinked[i];
    psetNeighbourIsLinked2[k] = psetNeighbourIsLinked;
    for (i = k + 1; i < newSize; i++)
        psetNeighbourIsLinked2[i] = this->psetNeighbourIsLinked[i-1];
    delete [] this->psetNeighbourIsLinked;
    this->psetNeighbourIsLinked = psetNeighbourIsLinked2;
    psetNeighbourIsLinked_arraysize = newSize;
}

void VlrIntBeacon::appendPsetNeighbourIsLinked(bool psetNeighbourIsLinked)
{
    insertPsetNeighbourIsLinked(psetNeighbourIsLinked_arraysize, psetNeighbourIsLinked);
}

void VlrIntBeacon::erasePsetNeighbourIsLinked(size_t k)
{
    if (k >= psetNeighbourIsLinked_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)psetNeighbourIsLinked_arraysize, (unsigned long)k);
    size_t newSize = psetNeighbourIsLinked_arraysize - 1;
    bool *psetNeighbourIsLinked2 = (newSize == 0) ? nullptr : new bool[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        psetNeighbourIsLinked2[i] = this->psetNeighbourIsLinked[i];
    for (i = k; i < newSize; i++)
        psetNeighbourIsLinked2[i] = this->psetNeighbourIsLinked[i+1];
    delete [] this->psetNeighbourIsLinked;
    this->psetNeighbourIsLinked = psetNeighbourIsLinked2;
    psetNeighbourIsLinked_arraysize = newSize;
}

size_t VlrIntBeacon::getPsetNeighbourIsInNetworkArraySize() const
{
    return psetNeighbourIsInNetwork_arraysize;
}

bool VlrIntBeacon::getPsetNeighbourIsInNetwork(size_t k) const
{
    if (k >= psetNeighbourIsInNetwork_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)psetNeighbourIsInNetwork_arraysize, (unsigned long)k);
    return this->psetNeighbourIsInNetwork[k];
}

void VlrIntBeacon::setPsetNeighbourIsInNetworkArraySize(size_t newSize)
{
    bool *psetNeighbourIsInNetwork2 = (newSize==0) ? nullptr : new bool[newSize];
    size_t minSize = psetNeighbourIsInNetwork_arraysize < newSize ? psetNeighbourIsInNetwork_arraysize : newSize;
    for (size_t i = 0; i < minSize; i++)
        psetNeighbourIsInNetwork2[i] = this->psetNeighbourIsInNetwork[i];
    for (size_t i = minSize; i < newSize; i++)
        psetNeighbourIsInNetwork2[i] = false;
    delete [] this->psetNeighbourIsInNetwork;
    this->psetNeighbourIsInNetwork = psetNeighbourIsInNetwork2;
    psetNeighbourIsInNetwork_arraysize = newSize;
}

void VlrIntBeacon::setPsetNeighbourIsInNetwork(size_t k, bool psetNeighbourIsInNetwork)
{
    if (k >= psetNeighbourIsInNetwork_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)psetNeighbourIsInNetwork_arraysize, (unsigned long)k);
    this->psetNeighbourIsInNetwork[k] = psetNeighbourIsInNetwork;
}

void VlrIntBeacon::insertPsetNeighbourIsInNetwork(size_t k, bool psetNeighbourIsInNetwork)
{
    if (k > psetNeighbourIsInNetwork_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)psetNeighbourIsInNetwork_arraysize, (unsigned long)k);
    size_t newSize = psetNeighbourIsInNetwork_arraysize + 1;
    bool *psetNeighbourIsInNetwork2 = new bool[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        psetNeighbourIsInNetwork2[i] = this->psetNeighbourIsInNetwork[i];
    psetNeighbourIsInNetwork2[k] = psetNeighbourIsInNetwork;
    for (i = k + 1; i < newSize; i++)
        psetNeighbourIsInNetwork2[i] = this->psetNeighbourIsInNetwork[i-1];
    delete [] this->psetNeighbourIsInNetwork;
    this->psetNeighbourIsInNetwork = psetNeighbourIsInNetwork2;
    psetNeighbourIsInNetwork_arraysize = newSize;
}

void VlrIntBeacon::appendPsetNeighbourIsInNetwork(bool psetNeighbourIsInNetwork)
{
    insertPsetNeighbourIsInNetwork(psetNeighbourIsInNetwork_arraysize, psetNeighbourIsInNetwork);
}

void VlrIntBeacon::erasePsetNeighbourIsInNetwork(size_t k)
{
    if (k >= psetNeighbourIsInNetwork_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)psetNeighbourIsInNetwork_arraysize, (unsigned long)k);
    size_t newSize = psetNeighbourIsInNetwork_arraysize - 1;
    bool *psetNeighbourIsInNetwork2 = (newSize == 0) ? nullptr : new bool[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        psetNeighbourIsInNetwork2[i] = this->psetNeighbourIsInNetwork[i];
    for (i = k; i < newSize; i++)
        psetNeighbourIsInNetwork2[i] = this->psetNeighbourIsInNetwork[i+1];
    delete [] this->psetNeighbourIsInNetwork;
    this->psetNeighbourIsInNetwork = psetNeighbourIsInNetwork2;
    psetNeighbourIsInNetwork_arraysize = newSize;
}

class VlrIntBeaconDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_vid,
        FIELD_inNetwork,
        FIELD_repstate,
        FIELD_repstate2,
        FIELD_psetNeighbour,
        FIELD_psetNeighbourIsLinked,
        FIELD_psetNeighbourIsInNetwork,
    };
  public:
    VlrIntBeaconDescriptor();
    virtual ~VlrIntBeaconDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(VlrIntBeaconDescriptor)

VlrIntBeaconDescriptor::VlrIntBeaconDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(omnetvlr::VlrIntBeacon)), "omnetpp::cPacket")
{
    propertyNames = nullptr;
}

VlrIntBeaconDescriptor::~VlrIntBeaconDescriptor()
{
    delete[] propertyNames;
}

bool VlrIntBeaconDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<VlrIntBeacon *>(obj)!=nullptr;
}

const char **VlrIntBeaconDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *VlrIntBeaconDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int VlrIntBeaconDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 7+base->getFieldCount() : 7;
}

unsigned int VlrIntBeaconDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_vid
        FD_ISEDITABLE,    // FIELD_inNetwork
        FD_ISCOMPOUND,    // FIELD_repstate
        FD_ISCOMPOUND,    // FIELD_repstate2
        FD_ISARRAY | FD_ISEDITABLE | FD_ISRESIZABLE,    // FIELD_psetNeighbour
        FD_ISARRAY | FD_ISEDITABLE | FD_ISRESIZABLE,    // FIELD_psetNeighbourIsLinked
        FD_ISARRAY | FD_ISEDITABLE | FD_ISRESIZABLE,    // FIELD_psetNeighbourIsInNetwork
    };
    return (field >= 0 && field < 7) ? fieldTypeFlags[field] : 0;
}

const char *VlrIntBeaconDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "vid",
        "inNetwork",
        "repstate",
        "repstate2",
        "psetNeighbour",
        "psetNeighbourIsLinked",
        "psetNeighbourIsInNetwork",
    };
    return (field >= 0 && field < 7) ? fieldNames[field] : nullptr;
}

int VlrIntBeaconDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "vid") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "inNetwork") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "repstate") == 0) return baseIndex + 2;
    if (strcmp(fieldName, "repstate2") == 0) return baseIndex + 3;
    if (strcmp(fieldName, "psetNeighbour") == 0) return baseIndex + 4;
    if (strcmp(fieldName, "psetNeighbourIsLinked") == 0) return baseIndex + 5;
    if (strcmp(fieldName, "psetNeighbourIsInNetwork") == 0) return baseIndex + 6;
    return base ? base->findField(fieldName) : -1;
}

const char *VlrIntBeaconDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "unsigned int",    // FIELD_vid
        "bool",    // FIELD_inNetwork
        "omnetvlr::VlrIntRepState",    // FIELD_repstate
        "omnetvlr::VlrIntRepState",    // FIELD_repstate2
        "unsigned int",    // FIELD_psetNeighbour
        "bool",    // FIELD_psetNeighbourIsLinked
        "bool",    // FIELD_psetNeighbourIsInNetwork
    };
    return (field >= 0 && field < 7) ? fieldTypeStrings[field] : nullptr;
}

const char **VlrIntBeaconDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *VlrIntBeaconDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int VlrIntBeaconDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    VlrIntBeacon *pp = omnetpp::fromAnyPtr<VlrIntBeacon>(object); (void)pp;
    switch (field) {
        case FIELD_psetNeighbour: return pp->getPsetNeighbourArraySize();
        case FIELD_psetNeighbourIsLinked: return pp->getPsetNeighbourIsLinkedArraySize();
        case FIELD_psetNeighbourIsInNetwork: return pp->getPsetNeighbourIsInNetworkArraySize();
        default: return 0;
    }
}

void VlrIntBeaconDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntBeacon *pp = omnetpp::fromAnyPtr<VlrIntBeacon>(object); (void)pp;
    switch (field) {
        case FIELD_psetNeighbour: pp->setPsetNeighbourArraySize(size); break;
        case FIELD_psetNeighbourIsLinked: pp->setPsetNeighbourIsLinkedArraySize(size); break;
        case FIELD_psetNeighbourIsInNetwork: pp->setPsetNeighbourIsInNetworkArraySize(size); break;
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'VlrIntBeacon'", field);
    }
}

const char *VlrIntBeaconDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    VlrIntBeacon *pp = omnetpp::fromAnyPtr<VlrIntBeacon>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string VlrIntBeaconDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    VlrIntBeacon *pp = omnetpp::fromAnyPtr<VlrIntBeacon>(object); (void)pp;
    switch (field) {
        case FIELD_vid: return ulong2string(pp->getVid());
        case FIELD_inNetwork: return bool2string(pp->getInNetwork());
        case FIELD_repstate: return "";
        case FIELD_repstate2: return "";
        case FIELD_psetNeighbour: return ulong2string(pp->getPsetNeighbour(i));
        case FIELD_psetNeighbourIsLinked: return bool2string(pp->getPsetNeighbourIsLinked(i));
        case FIELD_psetNeighbourIsInNetwork: return bool2string(pp->getPsetNeighbourIsInNetwork(i));
        default: return "";
    }
}

void VlrIntBeaconDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntBeacon *pp = omnetpp::fromAnyPtr<VlrIntBeacon>(object); (void)pp;
    switch (field) {
        case FIELD_vid: pp->setVid(string2ulong(value)); break;
        case FIELD_inNetwork: pp->setInNetwork(string2bool(value)); break;
        case FIELD_psetNeighbour: pp->setPsetNeighbour(i,string2ulong(value)); break;
        case FIELD_psetNeighbourIsLinked: pp->setPsetNeighbourIsLinked(i,string2bool(value)); break;
        case FIELD_psetNeighbourIsInNetwork: pp->setPsetNeighbourIsInNetwork(i,string2bool(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VlrIntBeacon'", field);
    }
}

omnetpp::cValue VlrIntBeaconDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    VlrIntBeacon *pp = omnetpp::fromAnyPtr<VlrIntBeacon>(object); (void)pp;
    switch (field) {
        case FIELD_vid: return (omnetpp::intval_t)(pp->getVid());
        case FIELD_inNetwork: return pp->getInNetwork();
        case FIELD_repstate: return omnetpp::toAnyPtr(&pp->getRepstate()); break;
        case FIELD_repstate2: return omnetpp::toAnyPtr(&pp->getRepstate2()); break;
        case FIELD_psetNeighbour: return (omnetpp::intval_t)(pp->getPsetNeighbour(i));
        case FIELD_psetNeighbourIsLinked: return pp->getPsetNeighbourIsLinked(i);
        case FIELD_psetNeighbourIsInNetwork: return pp->getPsetNeighbourIsInNetwork(i);
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'VlrIntBeacon' as cValue -- field index out of range?", field);
    }
}

void VlrIntBeaconDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntBeacon *pp = omnetpp::fromAnyPtr<VlrIntBeacon>(object); (void)pp;
    switch (field) {
        case FIELD_vid: pp->setVid(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        case FIELD_inNetwork: pp->setInNetwork(value.boolValue()); break;
        case FIELD_psetNeighbour: pp->setPsetNeighbour(i,omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        case FIELD_psetNeighbourIsLinked: pp->setPsetNeighbourIsLinked(i,value.boolValue()); break;
        case FIELD_psetNeighbourIsInNetwork: pp->setPsetNeighbourIsInNetwork(i,value.boolValue()); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VlrIntBeacon'", field);
    }
}

const char *VlrIntBeaconDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        case FIELD_repstate: return omnetpp::opp_typename(typeid(VlrIntRepState));
        case FIELD_repstate2: return omnetpp::opp_typename(typeid(VlrIntRepState));
        default: return nullptr;
    };
}

omnetpp::any_ptr VlrIntBeaconDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    VlrIntBeacon *pp = omnetpp::fromAnyPtr<VlrIntBeacon>(object); (void)pp;
    switch (field) {
        case FIELD_repstate: return omnetpp::toAnyPtr(&pp->getRepstate()); break;
        case FIELD_repstate2: return omnetpp::toAnyPtr(&pp->getRepstate2()); break;
        default: return omnetpp::any_ptr(nullptr);
    }
}

void VlrIntBeaconDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntBeacon *pp = omnetpp::fromAnyPtr<VlrIntBeacon>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VlrIntBeacon'", field);
    }
}

Register_Class(NotifyLinkFailureInt)

NotifyLinkFailureInt::NotifyLinkFailureInt(const char *name, short kind) : ::omnetpp::cPacket(name, kind)
{
}

NotifyLinkFailureInt::NotifyLinkFailureInt(const NotifyLinkFailureInt& other) : ::omnetpp::cPacket(other)
{
    copy(other);
}

NotifyLinkFailureInt::~NotifyLinkFailureInt()
{
}

NotifyLinkFailureInt& NotifyLinkFailureInt::operator=(const NotifyLinkFailureInt& other)
{
    if (this == &other) return *this;
    ::omnetpp::cPacket::operator=(other);
    copy(other);
    return *this;
}

void NotifyLinkFailureInt::copy(const NotifyLinkFailureInt& other)
{
    this->src = other.src;
    this->simLinkUp = other.simLinkUp;
}

void NotifyLinkFailureInt::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cPacket::parsimPack(b);
    doParsimPacking(b,this->src);
    doParsimPacking(b,this->simLinkUp);
}

void NotifyLinkFailureInt::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->src);
    doParsimUnpacking(b,this->simLinkUp);
}

unsigned int NotifyLinkFailureInt::getSrc() const
{
    return this->src;
}

void NotifyLinkFailureInt::setSrc(unsigned int src)
{
    this->src = src;
}

bool NotifyLinkFailureInt::getSimLinkUp() const
{
    return this->simLinkUp;
}

void NotifyLinkFailureInt::setSimLinkUp(bool simLinkUp)
{
    this->simLinkUp = simLinkUp;
}

class NotifyLinkFailureIntDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_src,
        FIELD_simLinkUp,
    };
  public:
    NotifyLinkFailureIntDescriptor();
    virtual ~NotifyLinkFailureIntDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(NotifyLinkFailureIntDescriptor)

NotifyLinkFailureIntDescriptor::NotifyLinkFailureIntDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(omnetvlr::NotifyLinkFailureInt)), "omnetpp::cPacket")
{
    propertyNames = nullptr;
}

NotifyLinkFailureIntDescriptor::~NotifyLinkFailureIntDescriptor()
{
    delete[] propertyNames;
}

bool NotifyLinkFailureIntDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<NotifyLinkFailureInt *>(obj)!=nullptr;
}

const char **NotifyLinkFailureIntDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *NotifyLinkFailureIntDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int NotifyLinkFailureIntDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 2+base->getFieldCount() : 2;
}

unsigned int NotifyLinkFailureIntDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_src
        FD_ISEDITABLE,    // FIELD_simLinkUp
    };
    return (field >= 0 && field < 2) ? fieldTypeFlags[field] : 0;
}

const char *NotifyLinkFailureIntDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "src",
        "simLinkUp",
    };
    return (field >= 0 && field < 2) ? fieldNames[field] : nullptr;
}

int NotifyLinkFailureIntDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "src") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "simLinkUp") == 0) return baseIndex + 1;
    return base ? base->findField(fieldName) : -1;
}

const char *NotifyLinkFailureIntDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "unsigned int",    // FIELD_src
        "bool",    // FIELD_simLinkUp
    };
    return (field >= 0 && field < 2) ? fieldTypeStrings[field] : nullptr;
}

const char **NotifyLinkFailureIntDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *NotifyLinkFailureIntDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int NotifyLinkFailureIntDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    NotifyLinkFailureInt *pp = omnetpp::fromAnyPtr<NotifyLinkFailureInt>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void NotifyLinkFailureIntDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    NotifyLinkFailureInt *pp = omnetpp::fromAnyPtr<NotifyLinkFailureInt>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'NotifyLinkFailureInt'", field);
    }
}

const char *NotifyLinkFailureIntDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    NotifyLinkFailureInt *pp = omnetpp::fromAnyPtr<NotifyLinkFailureInt>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string NotifyLinkFailureIntDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    NotifyLinkFailureInt *pp = omnetpp::fromAnyPtr<NotifyLinkFailureInt>(object); (void)pp;
    switch (field) {
        case FIELD_src: return ulong2string(pp->getSrc());
        case FIELD_simLinkUp: return bool2string(pp->getSimLinkUp());
        default: return "";
    }
}

void NotifyLinkFailureIntDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    NotifyLinkFailureInt *pp = omnetpp::fromAnyPtr<NotifyLinkFailureInt>(object); (void)pp;
    switch (field) {
        case FIELD_src: pp->setSrc(string2ulong(value)); break;
        case FIELD_simLinkUp: pp->setSimLinkUp(string2bool(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'NotifyLinkFailureInt'", field);
    }
}

omnetpp::cValue NotifyLinkFailureIntDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    NotifyLinkFailureInt *pp = omnetpp::fromAnyPtr<NotifyLinkFailureInt>(object); (void)pp;
    switch (field) {
        case FIELD_src: return (omnetpp::intval_t)(pp->getSrc());
        case FIELD_simLinkUp: return pp->getSimLinkUp();
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'NotifyLinkFailureInt' as cValue -- field index out of range?", field);
    }
}

void NotifyLinkFailureIntDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    NotifyLinkFailureInt *pp = omnetpp::fromAnyPtr<NotifyLinkFailureInt>(object); (void)pp;
    switch (field) {
        case FIELD_src: pp->setSrc(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        case FIELD_simLinkUp: pp->setSimLinkUp(value.boolValue()); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'NotifyLinkFailureInt'", field);
    }
}

const char *NotifyLinkFailureIntDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr NotifyLinkFailureIntDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    NotifyLinkFailureInt *pp = omnetpp::fromAnyPtr<NotifyLinkFailureInt>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void NotifyLinkFailureIntDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    NotifyLinkFailureInt *pp = omnetpp::fromAnyPtr<NotifyLinkFailureInt>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'NotifyLinkFailureInt'", field);
    }
}

VlrIntOption::VlrIntOption()
{
}

VlrIntOption::VlrIntOption(const VlrIntOption& other)
{
    copy(other);
}

VlrIntOption::~VlrIntOption()
{
}

VlrIntOption& VlrIntOption::operator=(const VlrIntOption& other)
{
    if (this == &other) return *this;
    copy(other);
    return *this;
}

void VlrIntOption::copy(const VlrIntOption& other)
{
    this->dstVid = other.dstVid;
    this->towardVid = other.towardVid;
    this->currentPathid = other.currentPathid;
    this->tempTowardVid = other.tempTowardVid;
    this->tempPathid = other.tempPathid;
    this->prevHopVid = other.prevHopVid;
}

void VlrIntOption::parsimPack(omnetpp::cCommBuffer *b) const
{
    doParsimPacking(b,this->dstVid);
    doParsimPacking(b,this->towardVid);
    doParsimPacking(b,this->currentPathid);
    doParsimPacking(b,this->tempTowardVid);
    doParsimPacking(b,this->tempPathid);
    doParsimPacking(b,this->prevHopVid);
}

void VlrIntOption::parsimUnpack(omnetpp::cCommBuffer *b)
{
    doParsimUnpacking(b,this->dstVid);
    doParsimUnpacking(b,this->towardVid);
    doParsimUnpacking(b,this->currentPathid);
    doParsimUnpacking(b,this->tempTowardVid);
    doParsimUnpacking(b,this->tempPathid);
    doParsimUnpacking(b,this->prevHopVid);
}

unsigned int VlrIntOption::getDstVid() const
{
    return this->dstVid;
}

void VlrIntOption::setDstVid(unsigned int dstVid)
{
    this->dstVid = dstVid;
}

unsigned int VlrIntOption::getTowardVid() const
{
    return this->towardVid;
}

void VlrIntOption::setTowardVid(unsigned int towardVid)
{
    this->towardVid = towardVid;
}

const VlrPathID& VlrIntOption::getCurrentPathid() const
{
    return this->currentPathid;
}

void VlrIntOption::setCurrentPathid(const VlrPathID& currentPathid)
{
    this->currentPathid = currentPathid;
}

unsigned int VlrIntOption::getTempTowardVid() const
{
    return this->tempTowardVid;
}

void VlrIntOption::setTempTowardVid(unsigned int tempTowardVid)
{
    this->tempTowardVid = tempTowardVid;
}

const VlrPathID& VlrIntOption::getTempPathid() const
{
    return this->tempPathid;
}

void VlrIntOption::setTempPathid(const VlrPathID& tempPathid)
{
    this->tempPathid = tempPathid;
}

unsigned int VlrIntOption::getPrevHopVid() const
{
    return this->prevHopVid;
}

void VlrIntOption::setPrevHopVid(unsigned int prevHopVid)
{
    this->prevHopVid = prevHopVid;
}

class VlrIntOptionDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_dstVid,
        FIELD_towardVid,
        FIELD_currentPathid,
        FIELD_tempTowardVid,
        FIELD_tempPathid,
        FIELD_prevHopVid,
    };
  public:
    VlrIntOptionDescriptor();
    virtual ~VlrIntOptionDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(VlrIntOptionDescriptor)

VlrIntOptionDescriptor::VlrIntOptionDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(omnetvlr::VlrIntOption)), "")
{
    propertyNames = nullptr;
}

VlrIntOptionDescriptor::~VlrIntOptionDescriptor()
{
    delete[] propertyNames;
}

bool VlrIntOptionDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<VlrIntOption *>(obj)!=nullptr;
}

const char **VlrIntOptionDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *VlrIntOptionDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int VlrIntOptionDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 6+base->getFieldCount() : 6;
}

unsigned int VlrIntOptionDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_dstVid
        FD_ISEDITABLE,    // FIELD_towardVid
        0,    // FIELD_currentPathid
        FD_ISEDITABLE,    // FIELD_tempTowardVid
        0,    // FIELD_tempPathid
        FD_ISEDITABLE,    // FIELD_prevHopVid
    };
    return (field >= 0 && field < 6) ? fieldTypeFlags[field] : 0;
}

const char *VlrIntOptionDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "dstVid",
        "towardVid",
        "currentPathid",
        "tempTowardVid",
        "tempPathid",
        "prevHopVid",
    };
    return (field >= 0 && field < 6) ? fieldNames[field] : nullptr;
}

int VlrIntOptionDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "dstVid") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "towardVid") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "currentPathid") == 0) return baseIndex + 2;
    if (strcmp(fieldName, "tempTowardVid") == 0) return baseIndex + 3;
    if (strcmp(fieldName, "tempPathid") == 0) return baseIndex + 4;
    if (strcmp(fieldName, "prevHopVid") == 0) return baseIndex + 5;
    return base ? base->findField(fieldName) : -1;
}

const char *VlrIntOptionDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "unsigned int",    // FIELD_dstVid
        "unsigned int",    // FIELD_towardVid
        "omnetvlr::VlrPathID",    // FIELD_currentPathid
        "unsigned int",    // FIELD_tempTowardVid
        "omnetvlr::VlrPathID",    // FIELD_tempPathid
        "unsigned int",    // FIELD_prevHopVid
    };
    return (field >= 0 && field < 6) ? fieldTypeStrings[field] : nullptr;
}

const char **VlrIntOptionDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *VlrIntOptionDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int VlrIntOptionDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    VlrIntOption *pp = omnetpp::fromAnyPtr<VlrIntOption>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void VlrIntOptionDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntOption *pp = omnetpp::fromAnyPtr<VlrIntOption>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'VlrIntOption'", field);
    }
}

const char *VlrIntOptionDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    VlrIntOption *pp = omnetpp::fromAnyPtr<VlrIntOption>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string VlrIntOptionDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    VlrIntOption *pp = omnetpp::fromAnyPtr<VlrIntOption>(object); (void)pp;
    switch (field) {
        case FIELD_dstVid: return ulong2string(pp->getDstVid());
        case FIELD_towardVid: return ulong2string(pp->getTowardVid());
        case FIELD_currentPathid: return ulong2string(pp->getCurrentPathid());
        case FIELD_tempTowardVid: return ulong2string(pp->getTempTowardVid());
        case FIELD_tempPathid: return ulong2string(pp->getTempPathid());
        case FIELD_prevHopVid: return ulong2string(pp->getPrevHopVid());
        default: return "";
    }
}

void VlrIntOptionDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntOption *pp = omnetpp::fromAnyPtr<VlrIntOption>(object); (void)pp;
    switch (field) {
        case FIELD_dstVid: pp->setDstVid(string2ulong(value)); break;
        case FIELD_towardVid: pp->setTowardVid(string2ulong(value)); break;
        case FIELD_tempTowardVid: pp->setTempTowardVid(string2ulong(value)); break;
        case FIELD_prevHopVid: pp->setPrevHopVid(string2ulong(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VlrIntOption'", field);
    }
}

omnetpp::cValue VlrIntOptionDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    VlrIntOption *pp = omnetpp::fromAnyPtr<VlrIntOption>(object); (void)pp;
    switch (field) {
        case FIELD_dstVid: return (omnetpp::intval_t)(pp->getDstVid());
        case FIELD_towardVid: return (omnetpp::intval_t)(pp->getTowardVid());
        case FIELD_currentPathid: return omnetpp::toAnyPtr(&pp->getCurrentPathid()); break;
        case FIELD_tempTowardVid: return (omnetpp::intval_t)(pp->getTempTowardVid());
        case FIELD_tempPathid: return omnetpp::toAnyPtr(&pp->getTempPathid()); break;
        case FIELD_prevHopVid: return (omnetpp::intval_t)(pp->getPrevHopVid());
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'VlrIntOption' as cValue -- field index out of range?", field);
    }
}

void VlrIntOptionDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntOption *pp = omnetpp::fromAnyPtr<VlrIntOption>(object); (void)pp;
    switch (field) {
        case FIELD_dstVid: pp->setDstVid(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        case FIELD_towardVid: pp->setTowardVid(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        case FIELD_tempTowardVid: pp->setTempTowardVid(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        case FIELD_prevHopVid: pp->setPrevHopVid(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VlrIntOption'", field);
    }
}

const char *VlrIntOptionDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr VlrIntOptionDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    VlrIntOption *pp = omnetpp::fromAnyPtr<VlrIntOption>(object); (void)pp;
    switch (field) {
        case FIELD_currentPathid: return omnetpp::toAnyPtr(&pp->getCurrentPathid()); break;
        case FIELD_tempPathid: return omnetpp::toAnyPtr(&pp->getTempPathid()); break;
        default: return omnetpp::any_ptr(nullptr);
    }
}

void VlrIntOptionDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntOption *pp = omnetpp::fromAnyPtr<VlrIntOption>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VlrIntOption'", field);
    }
}

Register_Class(VlrIntUniPacket)

VlrIntUniPacket::VlrIntUniPacket(const char *name, short kind) : ::omnetpp::cPacket(name, kind)
{
}

VlrIntUniPacket::VlrIntUniPacket(const VlrIntUniPacket& other) : ::omnetpp::cPacket(other)
{
    copy(other);
}

VlrIntUniPacket::~VlrIntUniPacket()
{
}

VlrIntUniPacket& VlrIntUniPacket::operator=(const VlrIntUniPacket& other)
{
    if (this == &other) return *this;
    ::omnetpp::cPacket::operator=(other);
    copy(other);
    return *this;
}

void VlrIntUniPacket::copy(const VlrIntUniPacket& other)
{
    this->messageId = other.messageId;
    this->vlrOption = other.vlrOption;
    this->hopcount = other.hopcount;
}

void VlrIntUniPacket::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cPacket::parsimPack(b);
    doParsimPacking(b,this->messageId);
    doParsimPacking(b,this->vlrOption);
    doParsimPacking(b,this->hopcount);
}

void VlrIntUniPacket::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->messageId);
    doParsimUnpacking(b,this->vlrOption);
    doParsimUnpacking(b,this->hopcount);
}

unsigned int VlrIntUniPacket::getMessageId() const
{
    return this->messageId;
}

void VlrIntUniPacket::setMessageId(unsigned int messageId)
{
    this->messageId = messageId;
}

const VlrIntOption& VlrIntUniPacket::getVlrOption() const
{
    return this->vlrOption;
}

void VlrIntUniPacket::setVlrOption(const VlrIntOption& vlrOption)
{
    this->vlrOption = vlrOption;
}

unsigned int VlrIntUniPacket::getHopcount() const
{
    return this->hopcount;
}

void VlrIntUniPacket::setHopcount(unsigned int hopcount)
{
    this->hopcount = hopcount;
}

class VlrIntUniPacketDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_messageId,
        FIELD_vlrOption,
        FIELD_hopcount,
    };
  public:
    VlrIntUniPacketDescriptor();
    virtual ~VlrIntUniPacketDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(VlrIntUniPacketDescriptor)

VlrIntUniPacketDescriptor::VlrIntUniPacketDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(omnetvlr::VlrIntUniPacket)), "omnetpp::cPacket")
{
    propertyNames = nullptr;
}

VlrIntUniPacketDescriptor::~VlrIntUniPacketDescriptor()
{
    delete[] propertyNames;
}

bool VlrIntUniPacketDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<VlrIntUniPacket *>(obj)!=nullptr;
}

const char **VlrIntUniPacketDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *VlrIntUniPacketDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int VlrIntUniPacketDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 3+base->getFieldCount() : 3;
}

unsigned int VlrIntUniPacketDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_messageId
        FD_ISCOMPOUND,    // FIELD_vlrOption
        FD_ISEDITABLE,    // FIELD_hopcount
    };
    return (field >= 0 && field < 3) ? fieldTypeFlags[field] : 0;
}

const char *VlrIntUniPacketDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "messageId",
        "vlrOption",
        "hopcount",
    };
    return (field >= 0 && field < 3) ? fieldNames[field] : nullptr;
}

int VlrIntUniPacketDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "messageId") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "vlrOption") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "hopcount") == 0) return baseIndex + 2;
    return base ? base->findField(fieldName) : -1;
}

const char *VlrIntUniPacketDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "unsigned int",    // FIELD_messageId
        "omnetvlr::VlrIntOption",    // FIELD_vlrOption
        "unsigned int",    // FIELD_hopcount
    };
    return (field >= 0 && field < 3) ? fieldTypeStrings[field] : nullptr;
}

const char **VlrIntUniPacketDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *VlrIntUniPacketDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int VlrIntUniPacketDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    VlrIntUniPacket *pp = omnetpp::fromAnyPtr<VlrIntUniPacket>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void VlrIntUniPacketDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntUniPacket *pp = omnetpp::fromAnyPtr<VlrIntUniPacket>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'VlrIntUniPacket'", field);
    }
}

const char *VlrIntUniPacketDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    VlrIntUniPacket *pp = omnetpp::fromAnyPtr<VlrIntUniPacket>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string VlrIntUniPacketDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    VlrIntUniPacket *pp = omnetpp::fromAnyPtr<VlrIntUniPacket>(object); (void)pp;
    switch (field) {
        case FIELD_messageId: return ulong2string(pp->getMessageId());
        case FIELD_vlrOption: return "";
        case FIELD_hopcount: return ulong2string(pp->getHopcount());
        default: return "";
    }
}

void VlrIntUniPacketDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntUniPacket *pp = omnetpp::fromAnyPtr<VlrIntUniPacket>(object); (void)pp;
    switch (field) {
        case FIELD_messageId: pp->setMessageId(string2ulong(value)); break;
        case FIELD_hopcount: pp->setHopcount(string2ulong(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VlrIntUniPacket'", field);
    }
}

omnetpp::cValue VlrIntUniPacketDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    VlrIntUniPacket *pp = omnetpp::fromAnyPtr<VlrIntUniPacket>(object); (void)pp;
    switch (field) {
        case FIELD_messageId: return (omnetpp::intval_t)(pp->getMessageId());
        case FIELD_vlrOption: return omnetpp::toAnyPtr(&pp->getVlrOption()); break;
        case FIELD_hopcount: return (omnetpp::intval_t)(pp->getHopcount());
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'VlrIntUniPacket' as cValue -- field index out of range?", field);
    }
}

void VlrIntUniPacketDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntUniPacket *pp = omnetpp::fromAnyPtr<VlrIntUniPacket>(object); (void)pp;
    switch (field) {
        case FIELD_messageId: pp->setMessageId(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        case FIELD_hopcount: pp->setHopcount(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VlrIntUniPacket'", field);
    }
}

const char *VlrIntUniPacketDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        case FIELD_vlrOption: return omnetpp::opp_typename(typeid(VlrIntOption));
        default: return nullptr;
    };
}

omnetpp::any_ptr VlrIntUniPacketDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    VlrIntUniPacket *pp = omnetpp::fromAnyPtr<VlrIntUniPacket>(object); (void)pp;
    switch (field) {
        case FIELD_vlrOption: return omnetpp::toAnyPtr(&pp->getVlrOption()); break;
        default: return omnetpp::any_ptr(nullptr);
    }
}

void VlrIntUniPacketDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntUniPacket *pp = omnetpp::fromAnyPtr<VlrIntUniPacket>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VlrIntUniPacket'", field);
    }
}

Register_Class(VlrIntSetupPacket)

VlrIntSetupPacket::VlrIntSetupPacket(const char *name) : ::omnetvlr::VlrIntUniPacket(name)
{
}

VlrIntSetupPacket::VlrIntSetupPacket(const VlrIntSetupPacket& other) : ::omnetvlr::VlrIntUniPacket(other)
{
    copy(other);
}

VlrIntSetupPacket::~VlrIntSetupPacket()
{
    delete [] this->srcVset;
}

VlrIntSetupPacket& VlrIntSetupPacket::operator=(const VlrIntSetupPacket& other)
{
    if (this == &other) return *this;
    ::omnetvlr::VlrIntUniPacket::operator=(other);
    copy(other);
    return *this;
}

void VlrIntSetupPacket::copy(const VlrIntSetupPacket& other)
{
    delete [] this->srcVset;
    this->srcVset = (other.srcVset_arraysize==0) ? nullptr : new unsigned int[other.srcVset_arraysize];
    srcVset_arraysize = other.srcVset_arraysize;
    for (size_t i = 0; i < srcVset_arraysize; i++) {
        this->srcVset[i] = other.srcVset[i];
    }
}

void VlrIntSetupPacket::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetvlr::VlrIntUniPacket::parsimPack(b);
    b->pack(srcVset_arraysize);
    doParsimArrayPacking(b,this->srcVset,srcVset_arraysize);
}

void VlrIntSetupPacket::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetvlr::VlrIntUniPacket::parsimUnpack(b);
    delete [] this->srcVset;
    b->unpack(srcVset_arraysize);
    if (srcVset_arraysize == 0) {
        this->srcVset = nullptr;
    } else {
        this->srcVset = new unsigned int[srcVset_arraysize];
        doParsimArrayUnpacking(b,this->srcVset,srcVset_arraysize);
    }
}

size_t VlrIntSetupPacket::getSrcVsetArraySize() const
{
    return srcVset_arraysize;
}

unsigned int VlrIntSetupPacket::getSrcVset(size_t k) const
{
    if (k >= srcVset_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)srcVset_arraysize, (unsigned long)k);
    return this->srcVset[k];
}

void VlrIntSetupPacket::setSrcVsetArraySize(size_t newSize)
{
    unsigned int *srcVset2 = (newSize==0) ? nullptr : new unsigned int[newSize];
    size_t minSize = srcVset_arraysize < newSize ? srcVset_arraysize : newSize;
    for (size_t i = 0; i < minSize; i++)
        srcVset2[i] = this->srcVset[i];
    for (size_t i = minSize; i < newSize; i++)
        srcVset2[i] = 0;
    delete [] this->srcVset;
    this->srcVset = srcVset2;
    srcVset_arraysize = newSize;
}

void VlrIntSetupPacket::setSrcVset(size_t k, unsigned int srcVset)
{
    if (k >= srcVset_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)srcVset_arraysize, (unsigned long)k);
    this->srcVset[k] = srcVset;
}

void VlrIntSetupPacket::insertSrcVset(size_t k, unsigned int srcVset)
{
    if (k > srcVset_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)srcVset_arraysize, (unsigned long)k);
    size_t newSize = srcVset_arraysize + 1;
    unsigned int *srcVset2 = new unsigned int[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        srcVset2[i] = this->srcVset[i];
    srcVset2[k] = srcVset;
    for (i = k + 1; i < newSize; i++)
        srcVset2[i] = this->srcVset[i-1];
    delete [] this->srcVset;
    this->srcVset = srcVset2;
    srcVset_arraysize = newSize;
}

void VlrIntSetupPacket::appendSrcVset(unsigned int srcVset)
{
    insertSrcVset(srcVset_arraysize, srcVset);
}

void VlrIntSetupPacket::eraseSrcVset(size_t k)
{
    if (k >= srcVset_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)srcVset_arraysize, (unsigned long)k);
    size_t newSize = srcVset_arraysize - 1;
    unsigned int *srcVset2 = (newSize == 0) ? nullptr : new unsigned int[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        srcVset2[i] = this->srcVset[i];
    for (i = k; i < newSize; i++)
        srcVset2[i] = this->srcVset[i+1];
    delete [] this->srcVset;
    this->srcVset = srcVset2;
    srcVset_arraysize = newSize;
}

class VlrIntSetupPacketDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_srcVset,
    };
  public:
    VlrIntSetupPacketDescriptor();
    virtual ~VlrIntSetupPacketDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(VlrIntSetupPacketDescriptor)

VlrIntSetupPacketDescriptor::VlrIntSetupPacketDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(omnetvlr::VlrIntSetupPacket)), "omnetvlr::VlrIntUniPacket")
{
    propertyNames = nullptr;
}

VlrIntSetupPacketDescriptor::~VlrIntSetupPacketDescriptor()
{
    delete[] propertyNames;
}

bool VlrIntSetupPacketDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<VlrIntSetupPacket *>(obj)!=nullptr;
}

const char **VlrIntSetupPacketDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *VlrIntSetupPacketDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int VlrIntSetupPacketDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 1+base->getFieldCount() : 1;
}

unsigned int VlrIntSetupPacketDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISARRAY | FD_ISEDITABLE | FD_ISRESIZABLE,    // FIELD_srcVset
    };
    return (field >= 0 && field < 1) ? fieldTypeFlags[field] : 0;
}

const char *VlrIntSetupPacketDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "srcVset",
    };
    return (field >= 0 && field < 1) ? fieldNames[field] : nullptr;
}

int VlrIntSetupPacketDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "srcVset") == 0) return baseIndex + 0;
    return base ? base->findField(fieldName) : -1;
}

const char *VlrIntSetupPacketDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "unsigned int",    // FIELD_srcVset
    };
    return (field >= 0 && field < 1) ? fieldTypeStrings[field] : nullptr;
}

const char **VlrIntSetupPacketDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *VlrIntSetupPacketDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int VlrIntSetupPacketDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    VlrIntSetupPacket *pp = omnetpp::fromAnyPtr<VlrIntSetupPacket>(object); (void)pp;
    switch (field) {
        case FIELD_srcVset: return pp->getSrcVsetArraySize();
        default: return 0;
    }
}

void VlrIntSetupPacketDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntSetupPacket *pp = omnetpp::fromAnyPtr<VlrIntSetupPacket>(object); (void)pp;
    switch (field) {
        case FIELD_srcVset: pp->setSrcVsetArraySize(size); break;
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'VlrIntSetupPacket'", field);
    }
}

const char *VlrIntSetupPacketDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    VlrIntSetupPacket *pp = omnetpp::fromAnyPtr<VlrIntSetupPacket>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string VlrIntSetupPacketDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    VlrIntSetupPacket *pp = omnetpp::fromAnyPtr<VlrIntSetupPacket>(object); (void)pp;
    switch (field) {
        case FIELD_srcVset: return ulong2string(pp->getSrcVset(i));
        default: return "";
    }
}

void VlrIntSetupPacketDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntSetupPacket *pp = omnetpp::fromAnyPtr<VlrIntSetupPacket>(object); (void)pp;
    switch (field) {
        case FIELD_srcVset: pp->setSrcVset(i,string2ulong(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VlrIntSetupPacket'", field);
    }
}

omnetpp::cValue VlrIntSetupPacketDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    VlrIntSetupPacket *pp = omnetpp::fromAnyPtr<VlrIntSetupPacket>(object); (void)pp;
    switch (field) {
        case FIELD_srcVset: return (omnetpp::intval_t)(pp->getSrcVset(i));
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'VlrIntSetupPacket' as cValue -- field index out of range?", field);
    }
}

void VlrIntSetupPacketDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntSetupPacket *pp = omnetpp::fromAnyPtr<VlrIntSetupPacket>(object); (void)pp;
    switch (field) {
        case FIELD_srcVset: pp->setSrcVset(i,omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VlrIntSetupPacket'", field);
    }
}

const char *VlrIntSetupPacketDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr VlrIntSetupPacketDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    VlrIntSetupPacket *pp = omnetpp::fromAnyPtr<VlrIntSetupPacket>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void VlrIntSetupPacketDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntSetupPacket *pp = omnetpp::fromAnyPtr<VlrIntSetupPacket>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VlrIntSetupPacket'", field);
    }
}

Register_Class(SetupReqInt)

SetupReqInt::SetupReqInt(const char *name) : ::omnetvlr::VlrIntSetupPacket(name)
{
}

SetupReqInt::SetupReqInt(const SetupReqInt& other) : ::omnetvlr::VlrIntSetupPacket(other)
{
    copy(other);
}

SetupReqInt::~SetupReqInt()
{
}

SetupReqInt& SetupReqInt::operator=(const SetupReqInt& other)
{
    if (this == &other) return *this;
    ::omnetvlr::VlrIntSetupPacket::operator=(other);
    copy(other);
    return *this;
}

void SetupReqInt::copy(const SetupReqInt& other)
{
    this->dst = other.dst;
    this->newnode = other.newnode;
    this->proxy = other.proxy;
    this->transferNode = other.transferNode;
    this->knownSet = other.knownSet;
    this->reqDispatch = other.reqDispatch;
    this->repairRoute = other.repairRoute;
    this->patchedRoute = other.patchedRoute;
    this->recordTrace = other.recordTrace;
    this->reqVnei = other.reqVnei;
    this->traceVec = other.traceVec;
    this->indexInTrace = other.indexInTrace;
}

void SetupReqInt::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetvlr::VlrIntSetupPacket::parsimPack(b);
    doParsimPacking(b,this->dst);
    doParsimPacking(b,this->newnode);
    doParsimPacking(b,this->proxy);
    doParsimPacking(b,this->transferNode);
    doParsimPacking(b,this->knownSet);
    doParsimPacking(b,this->reqDispatch);
    doParsimPacking(b,this->repairRoute);
    doParsimPacking(b,this->patchedRoute);
    doParsimPacking(b,this->recordTrace);
    doParsimPacking(b,this->reqVnei);
    doParsimPacking(b,this->traceVec);
    doParsimPacking(b,this->indexInTrace);
}

void SetupReqInt::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetvlr::VlrIntSetupPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->dst);
    doParsimUnpacking(b,this->newnode);
    doParsimUnpacking(b,this->proxy);
    doParsimUnpacking(b,this->transferNode);
    doParsimUnpacking(b,this->knownSet);
    doParsimUnpacking(b,this->reqDispatch);
    doParsimUnpacking(b,this->repairRoute);
    doParsimUnpacking(b,this->patchedRoute);
    doParsimUnpacking(b,this->recordTrace);
    doParsimUnpacking(b,this->reqVnei);
    doParsimUnpacking(b,this->traceVec);
    doParsimUnpacking(b,this->indexInTrace);
}

unsigned int SetupReqInt::getDst() const
{
    return this->dst;
}

void SetupReqInt::setDst(unsigned int dst)
{
    this->dst = dst;
}

unsigned int SetupReqInt::getNewnode() const
{
    return this->newnode;
}

void SetupReqInt::setNewnode(unsigned int newnode)
{
    this->newnode = newnode;
}

unsigned int SetupReqInt::getProxy() const
{
    return this->proxy;
}

void SetupReqInt::setProxy(unsigned int proxy)
{
    this->proxy = proxy;
}

unsigned int SetupReqInt::getTransferNode() const
{
    return this->transferNode;
}

void SetupReqInt::setTransferNode(unsigned int transferNode)
{
    this->transferNode = transferNode;
}

const VlrIntVidSet& SetupReqInt::getKnownSet() const
{
    return this->knownSet;
}

void SetupReqInt::setKnownSet(const VlrIntVidSet& knownSet)
{
    this->knownSet = knownSet;
}

bool SetupReqInt::getReqDispatch() const
{
    return this->reqDispatch;
}

void SetupReqInt::setReqDispatch(bool reqDispatch)
{
    this->reqDispatch = reqDispatch;
}

bool SetupReqInt::getRepairRoute() const
{
    return this->repairRoute;
}

void SetupReqInt::setRepairRoute(bool repairRoute)
{
    this->repairRoute = repairRoute;
}

const VlrPathID& SetupReqInt::getPatchedRoute() const
{
    return this->patchedRoute;
}

void SetupReqInt::setPatchedRoute(const VlrPathID& patchedRoute)
{
    this->patchedRoute = patchedRoute;
}

bool SetupReqInt::getRecordTrace() const
{
    return this->recordTrace;
}

void SetupReqInt::setRecordTrace(bool recordTrace)
{
    this->recordTrace = recordTrace;
}

bool SetupReqInt::getReqVnei() const
{
    return this->reqVnei;
}

void SetupReqInt::setReqVnei(bool reqVnei)
{
    this->reqVnei = reqVnei;
}

const VlrIntVidVec& SetupReqInt::getTraceVec() const
{
    return this->traceVec;
}

void SetupReqInt::setTraceVec(const VlrIntVidVec& traceVec)
{
    this->traceVec = traceVec;
}

unsigned int SetupReqInt::getIndexInTrace() const
{
    return this->indexInTrace;
}

void SetupReqInt::setIndexInTrace(unsigned int indexInTrace)
{
    this->indexInTrace = indexInTrace;
}

class SetupReqIntDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_dst,
        FIELD_newnode,
        FIELD_proxy,
        FIELD_transferNode,
        FIELD_knownSet,
        FIELD_reqDispatch,
        FIELD_repairRoute,
        FIELD_patchedRoute,
        FIELD_recordTrace,
        FIELD_reqVnei,
        FIELD_traceVec,
        FIELD_indexInTrace,
    };
  public:
    SetupReqIntDescriptor();
    virtual ~SetupReqIntDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(SetupReqIntDescriptor)

SetupReqIntDescriptor::SetupReqIntDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(omnetvlr::SetupReqInt)), "omnetvlr::VlrIntSetupPacket")
{
    propertyNames = nullptr;
}

SetupReqIntDescriptor::~SetupReqIntDescriptor()
{
    delete[] propertyNames;
}

bool SetupReqIntDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<SetupReqInt *>(obj)!=nullptr;
}

const char **SetupReqIntDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *SetupReqIntDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int SetupReqIntDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 12+base->getFieldCount() : 12;
}

unsigned int SetupReqIntDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_dst
        FD_ISEDITABLE,    // FIELD_newnode
        FD_ISEDITABLE,    // FIELD_proxy
        FD_ISEDITABLE,    // FIELD_transferNode
        FD_ISCOMPOUND,    // FIELD_knownSet
        FD_ISEDITABLE,    // FIELD_reqDispatch
        FD_ISEDITABLE,    // FIELD_repairRoute
        0,    // FIELD_patchedRoute
        FD_ISEDITABLE,    // FIELD_recordTrace
        FD_ISEDITABLE,    // FIELD_reqVnei
        FD_ISCOMPOUND,    // FIELD_traceVec
        FD_ISEDITABLE,    // FIELD_indexInTrace
    };
    return (field >= 0 && field < 12) ? fieldTypeFlags[field] : 0;
}

const char *SetupReqIntDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "dst",
        "newnode",
        "proxy",
        "transferNode",
        "knownSet",
        "reqDispatch",
        "repairRoute",
        "patchedRoute",
        "recordTrace",
        "reqVnei",
        "traceVec",
        "indexInTrace",
    };
    return (field >= 0 && field < 12) ? fieldNames[field] : nullptr;
}

int SetupReqIntDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "dst") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "newnode") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "proxy") == 0) return baseIndex + 2;
    if (strcmp(fieldName, "transferNode") == 0) return baseIndex + 3;
    if (strcmp(fieldName, "knownSet") == 0) return baseIndex + 4;
    if (strcmp(fieldName, "reqDispatch") == 0) return baseIndex + 5;
    if (strcmp(fieldName, "repairRoute") == 0) return baseIndex + 6;
    if (strcmp(fieldName, "patchedRoute") == 0) return baseIndex + 7;
    if (strcmp(fieldName, "recordTrace") == 0) return baseIndex + 8;
    if (strcmp(fieldName, "reqVnei") == 0) return baseIndex + 9;
    if (strcmp(fieldName, "traceVec") == 0) return baseIndex + 10;
    if (strcmp(fieldName, "indexInTrace") == 0) return baseIndex + 11;
    return base ? base->findField(fieldName) : -1;
}

const char *SetupReqIntDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "unsigned int",    // FIELD_dst
        "unsigned int",    // FIELD_newnode
        "unsigned int",    // FIELD_proxy
        "unsigned int",    // FIELD_transferNode
        "omnetvlr::VlrIntVidSet",    // FIELD_knownSet
        "bool",    // FIELD_reqDispatch
        "bool",    // FIELD_repairRoute
        "omnetvlr::VlrPathID",    // FIELD_patchedRoute
        "bool",    // FIELD_recordTrace
        "bool",    // FIELD_reqVnei
        "omnetvlr::VlrIntVidVec",    // FIELD_traceVec
        "unsigned int",    // FIELD_indexInTrace
    };
    return (field >= 0 && field < 12) ? fieldTypeStrings[field] : nullptr;
}

const char **SetupReqIntDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *SetupReqIntDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int SetupReqIntDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    SetupReqInt *pp = omnetpp::fromAnyPtr<SetupReqInt>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void SetupReqIntDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    SetupReqInt *pp = omnetpp::fromAnyPtr<SetupReqInt>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'SetupReqInt'", field);
    }
}

const char *SetupReqIntDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    SetupReqInt *pp = omnetpp::fromAnyPtr<SetupReqInt>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string SetupReqIntDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    SetupReqInt *pp = omnetpp::fromAnyPtr<SetupReqInt>(object); (void)pp;
    switch (field) {
        case FIELD_dst: return ulong2string(pp->getDst());
        case FIELD_newnode: return ulong2string(pp->getNewnode());
        case FIELD_proxy: return ulong2string(pp->getProxy());
        case FIELD_transferNode: return ulong2string(pp->getTransferNode());
        case FIELD_knownSet: return "";
        case FIELD_reqDispatch: return bool2string(pp->getReqDispatch());
        case FIELD_repairRoute: return bool2string(pp->getRepairRoute());
        case FIELD_patchedRoute: return ulong2string(pp->getPatchedRoute());
        case FIELD_recordTrace: return bool2string(pp->getRecordTrace());
        case FIELD_reqVnei: return bool2string(pp->getReqVnei());
        case FIELD_traceVec: return "";
        case FIELD_indexInTrace: return ulong2string(pp->getIndexInTrace());
        default: return "";
    }
}

void SetupReqIntDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    SetupReqInt *pp = omnetpp::fromAnyPtr<SetupReqInt>(object); (void)pp;
    switch (field) {
        case FIELD_dst: pp->setDst(string2ulong(value)); break;
        case FIELD_newnode: pp->setNewnode(string2ulong(value)); break;
        case FIELD_proxy: pp->setProxy(string2ulong(value)); break;
        case FIELD_transferNode: pp->setTransferNode(string2ulong(value)); break;
        case FIELD_reqDispatch: pp->setReqDispatch(string2bool(value)); break;
        case FIELD_repairRoute: pp->setRepairRoute(string2bool(value)); break;
        case FIELD_recordTrace: pp->setRecordTrace(string2bool(value)); break;
        case FIELD_reqVnei: pp->setReqVnei(string2bool(value)); break;
        case FIELD_indexInTrace: pp->setIndexInTrace(string2ulong(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'SetupReqInt'", field);
    }
}

omnetpp::cValue SetupReqIntDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    SetupReqInt *pp = omnetpp::fromAnyPtr<SetupReqInt>(object); (void)pp;
    switch (field) {
        case FIELD_dst: return (omnetpp::intval_t)(pp->getDst());
        case FIELD_newnode: return (omnetpp::intval_t)(pp->getNewnode());
        case FIELD_proxy: return (omnetpp::intval_t)(pp->getProxy());
        case FIELD_transferNode: return (omnetpp::intval_t)(pp->getTransferNode());
        case FIELD_knownSet: return omnetpp::toAnyPtr(&pp->getKnownSet()); break;
        case FIELD_reqDispatch: return pp->getReqDispatch();
        case FIELD_repairRoute: return pp->getRepairRoute();
        case FIELD_patchedRoute: return omnetpp::toAnyPtr(&pp->getPatchedRoute()); break;
        case FIELD_recordTrace: return pp->getRecordTrace();
        case FIELD_reqVnei: return pp->getReqVnei();
        case FIELD_traceVec: return omnetpp::toAnyPtr(&pp->getTraceVec()); break;
        case FIELD_indexInTrace: return (omnetpp::intval_t)(pp->getIndexInTrace());
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'SetupReqInt' as cValue -- field index out of range?", field);
    }
}

void SetupReqIntDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    SetupReqInt *pp = omnetpp::fromAnyPtr<SetupReqInt>(object); (void)pp;
    switch (field) {
        case FIELD_dst: pp->setDst(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        case FIELD_newnode: pp->setNewnode(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        case FIELD_proxy: pp->setProxy(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        case FIELD_transferNode: pp->setTransferNode(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        case FIELD_reqDispatch: pp->setReqDispatch(value.boolValue()); break;
        case FIELD_repairRoute: pp->setRepairRoute(value.boolValue()); break;
        case FIELD_recordTrace: pp->setRecordTrace(value.boolValue()); break;
        case FIELD_reqVnei: pp->setReqVnei(value.boolValue()); break;
        case FIELD_indexInTrace: pp->setIndexInTrace(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'SetupReqInt'", field);
    }
}

const char *SetupReqIntDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        case FIELD_knownSet: return omnetpp::opp_typename(typeid(VlrIntVidSet));
        case FIELD_traceVec: return omnetpp::opp_typename(typeid(VlrIntVidVec));
        default: return nullptr;
    };
}

omnetpp::any_ptr SetupReqIntDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    SetupReqInt *pp = omnetpp::fromAnyPtr<SetupReqInt>(object); (void)pp;
    switch (field) {
        case FIELD_knownSet: return omnetpp::toAnyPtr(&pp->getKnownSet()); break;
        case FIELD_patchedRoute: return omnetpp::toAnyPtr(&pp->getPatchedRoute()); break;
        case FIELD_traceVec: return omnetpp::toAnyPtr(&pp->getTraceVec()); break;
        default: return omnetpp::any_ptr(nullptr);
    }
}

void SetupReqIntDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    SetupReqInt *pp = omnetpp::fromAnyPtr<SetupReqInt>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'SetupReqInt'", field);
    }
}

Register_Class(SetupReplyInt)

SetupReplyInt::SetupReplyInt(const char *name) : ::omnetvlr::VlrIntSetupPacket(name)
{
}

SetupReplyInt::SetupReplyInt(const SetupReplyInt& other) : ::omnetvlr::VlrIntSetupPacket(other)
{
    copy(other);
}

SetupReplyInt::~SetupReplyInt()
{
}

SetupReplyInt& SetupReplyInt::operator=(const SetupReplyInt& other)
{
    if (this == &other) return *this;
    ::omnetvlr::VlrIntSetupPacket::operator=(other);
    copy(other);
    return *this;
}

void SetupReplyInt::copy(const SetupReplyInt& other)
{
    this->proxy = other.proxy;
    this->newnode = other.newnode;
    this->src = other.src;
    this->pathid = other.pathid;
    this->trace = other.trace;
    this->prevhopVids = other.prevhopVids;
    this->oldestPrevhopIndex = other.oldestPrevhopIndex;
}

void SetupReplyInt::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetvlr::VlrIntSetupPacket::parsimPack(b);
    doParsimPacking(b,this->proxy);
    doParsimPacking(b,this->newnode);
    doParsimPacking(b,this->src);
    doParsimPacking(b,this->pathid);
    doParsimPacking(b,this->trace);
    doParsimPacking(b,this->prevhopVids);
    doParsimPacking(b,this->oldestPrevhopIndex);
}

void SetupReplyInt::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetvlr::VlrIntSetupPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->proxy);
    doParsimUnpacking(b,this->newnode);
    doParsimUnpacking(b,this->src);
    doParsimUnpacking(b,this->pathid);
    doParsimUnpacking(b,this->trace);
    doParsimUnpacking(b,this->prevhopVids);
    doParsimUnpacking(b,this->oldestPrevhopIndex);
}

unsigned int SetupReplyInt::getProxy() const
{
    return this->proxy;
}

void SetupReplyInt::setProxy(unsigned int proxy)
{
    this->proxy = proxy;
}

unsigned int SetupReplyInt::getNewnode() const
{
    return this->newnode;
}

void SetupReplyInt::setNewnode(unsigned int newnode)
{
    this->newnode = newnode;
}

unsigned int SetupReplyInt::getSrc() const
{
    return this->src;
}

void SetupReplyInt::setSrc(unsigned int src)
{
    this->src = src;
}

const VlrPathID& SetupReplyInt::getPathid() const
{
    return this->pathid;
}

void SetupReplyInt::setPathid(const VlrPathID& pathid)
{
    this->pathid = pathid;
}

const VlrIntVidVec& SetupReplyInt::getTrace() const
{
    return this->trace;
}

void SetupReplyInt::setTrace(const VlrIntVidVec& trace)
{
    this->trace = trace;
}

const VlrIntVidVec& SetupReplyInt::getPrevhopVids() const
{
    return this->prevhopVids;
}

void SetupReplyInt::setPrevhopVids(const VlrIntVidVec& prevhopVids)
{
    this->prevhopVids = prevhopVids;
}

unsigned int SetupReplyInt::getOldestPrevhopIndex() const
{
    return this->oldestPrevhopIndex;
}

void SetupReplyInt::setOldestPrevhopIndex(unsigned int oldestPrevhopIndex)
{
    this->oldestPrevhopIndex = oldestPrevhopIndex;
}

class SetupReplyIntDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_proxy,
        FIELD_newnode,
        FIELD_src,
        FIELD_pathid,
        FIELD_trace,
        FIELD_prevhopVids,
        FIELD_oldestPrevhopIndex,
    };
  public:
    SetupReplyIntDescriptor();
    virtual ~SetupReplyIntDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(SetupReplyIntDescriptor)

SetupReplyIntDescriptor::SetupReplyIntDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(omnetvlr::SetupReplyInt)), "omnetvlr::VlrIntSetupPacket")
{
    propertyNames = nullptr;
}

SetupReplyIntDescriptor::~SetupReplyIntDescriptor()
{
    delete[] propertyNames;
}

bool SetupReplyIntDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<SetupReplyInt *>(obj)!=nullptr;
}

const char **SetupReplyIntDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *SetupReplyIntDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int SetupReplyIntDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 7+base->getFieldCount() : 7;
}

unsigned int SetupReplyIntDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_proxy
        FD_ISEDITABLE,    // FIELD_newnode
        FD_ISEDITABLE,    // FIELD_src
        0,    // FIELD_pathid
        FD_ISCOMPOUND,    // FIELD_trace
        FD_ISCOMPOUND,    // FIELD_prevhopVids
        FD_ISEDITABLE,    // FIELD_oldestPrevhopIndex
    };
    return (field >= 0 && field < 7) ? fieldTypeFlags[field] : 0;
}

const char *SetupReplyIntDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "proxy",
        "newnode",
        "src",
        "pathid",
        "trace",
        "prevhopVids",
        "oldestPrevhopIndex",
    };
    return (field >= 0 && field < 7) ? fieldNames[field] : nullptr;
}

int SetupReplyIntDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "proxy") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "newnode") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "src") == 0) return baseIndex + 2;
    if (strcmp(fieldName, "pathid") == 0) return baseIndex + 3;
    if (strcmp(fieldName, "trace") == 0) return baseIndex + 4;
    if (strcmp(fieldName, "prevhopVids") == 0) return baseIndex + 5;
    if (strcmp(fieldName, "oldestPrevhopIndex") == 0) return baseIndex + 6;
    return base ? base->findField(fieldName) : -1;
}

const char *SetupReplyIntDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "unsigned int",    // FIELD_proxy
        "unsigned int",    // FIELD_newnode
        "unsigned int",    // FIELD_src
        "omnetvlr::VlrPathID",    // FIELD_pathid
        "omnetvlr::VlrIntVidVec",    // FIELD_trace
        "omnetvlr::VlrIntVidVec",    // FIELD_prevhopVids
        "unsigned int",    // FIELD_oldestPrevhopIndex
    };
    return (field >= 0 && field < 7) ? fieldTypeStrings[field] : nullptr;
}

const char **SetupReplyIntDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *SetupReplyIntDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int SetupReplyIntDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    SetupReplyInt *pp = omnetpp::fromAnyPtr<SetupReplyInt>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void SetupReplyIntDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    SetupReplyInt *pp = omnetpp::fromAnyPtr<SetupReplyInt>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'SetupReplyInt'", field);
    }
}

const char *SetupReplyIntDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    SetupReplyInt *pp = omnetpp::fromAnyPtr<SetupReplyInt>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string SetupReplyIntDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    SetupReplyInt *pp = omnetpp::fromAnyPtr<SetupReplyInt>(object); (void)pp;
    switch (field) {
        case FIELD_proxy: return ulong2string(pp->getProxy());
        case FIELD_newnode: return ulong2string(pp->getNewnode());
        case FIELD_src: return ulong2string(pp->getSrc());
        case FIELD_pathid: return ulong2string(pp->getPathid());
        case FIELD_trace: return "";
        case FIELD_prevhopVids: return "";
        case FIELD_oldestPrevhopIndex: return ulong2string(pp->getOldestPrevhopIndex());
        default: return "";
    }
}

void SetupReplyIntDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    SetupReplyInt *pp = omnetpp::fromAnyPtr<SetupReplyInt>(object); (void)pp;
    switch (field) {
        case FIELD_proxy: pp->setProxy(string2ulong(value)); break;
        case FIELD_newnode: pp->setNewnode(string2ulong(value)); break;
        case FIELD_src: pp->setSrc(string2ulong(value)); break;
        case FIELD_oldestPrevhopIndex: pp->setOldestPrevhopIndex(string2ulong(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'SetupReplyInt'", field);
    }
}

omnetpp::cValue SetupReplyIntDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    SetupReplyInt *pp = omnetpp::fromAnyPtr<SetupReplyInt>(object); (void)pp;
    switch (field) {
        case FIELD_proxy: return (omnetpp::intval_t)(pp->getProxy());
        case FIELD_newnode: return (omnetpp::intval_t)(pp->getNewnode());
        case FIELD_src: return (omnetpp::intval_t)(pp->getSrc());
        case FIELD_pathid: return omnetpp::toAnyPtr(&pp->getPathid()); break;
        case FIELD_trace: return omnetpp::toAnyPtr(&pp->getTrace()); break;
        case FIELD_prevhopVids: return omnetpp::toAnyPtr(&pp->getPrevhopVids()); break;
        case FIELD_oldestPrevhopIndex: return (omnetpp::intval_t)(pp->getOldestPrevhopIndex());
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'SetupReplyInt' as cValue -- field index out of range?", field);
    }
}

void SetupReplyIntDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    SetupReplyInt *pp = omnetpp::fromAnyPtr<SetupReplyInt>(object); (void)pp;
    switch (field) {
        case FIELD_proxy: pp->setProxy(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        case FIELD_newnode: pp->setNewnode(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        case FIELD_src: pp->setSrc(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        case FIELD_oldestPrevhopIndex: pp->setOldestPrevhopIndex(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'SetupReplyInt'", field);
    }
}

const char *SetupReplyIntDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        case FIELD_trace: return omnetpp::opp_typename(typeid(VlrIntVidVec));
        case FIELD_prevhopVids: return omnetpp::opp_typename(typeid(VlrIntVidVec));
        default: return nullptr;
    };
}

omnetpp::any_ptr SetupReplyIntDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    SetupReplyInt *pp = omnetpp::fromAnyPtr<SetupReplyInt>(object); (void)pp;
    switch (field) {
        case FIELD_pathid: return omnetpp::toAnyPtr(&pp->getPathid()); break;
        case FIELD_trace: return omnetpp::toAnyPtr(&pp->getTrace()); break;
        case FIELD_prevhopVids: return omnetpp::toAnyPtr(&pp->getPrevhopVids()); break;
        default: return omnetpp::any_ptr(nullptr);
    }
}

void SetupReplyIntDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    SetupReplyInt *pp = omnetpp::fromAnyPtr<SetupReplyInt>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'SetupReplyInt'", field);
    }
}

Register_Class(SetupFailInt)

SetupFailInt::SetupFailInt(const char *name) : ::omnetvlr::VlrIntSetupPacket(name)
{
}

SetupFailInt::SetupFailInt(const SetupFailInt& other) : ::omnetvlr::VlrIntSetupPacket(other)
{
    copy(other);
}

SetupFailInt::~SetupFailInt()
{
}

SetupFailInt& SetupFailInt::operator=(const SetupFailInt& other)
{
    if (this == &other) return *this;
    ::omnetvlr::VlrIntSetupPacket::operator=(other);
    copy(other);
    return *this;
}

void SetupFailInt::copy(const SetupFailInt& other)
{
    this->proxy = other.proxy;
    this->newnode = other.newnode;
    this->src = other.src;
    this->trace = other.trace;
}

void SetupFailInt::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetvlr::VlrIntSetupPacket::parsimPack(b);
    doParsimPacking(b,this->proxy);
    doParsimPacking(b,this->newnode);
    doParsimPacking(b,this->src);
    doParsimPacking(b,this->trace);
}

void SetupFailInt::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetvlr::VlrIntSetupPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->proxy);
    doParsimUnpacking(b,this->newnode);
    doParsimUnpacking(b,this->src);
    doParsimUnpacking(b,this->trace);
}

unsigned int SetupFailInt::getProxy() const
{
    return this->proxy;
}

void SetupFailInt::setProxy(unsigned int proxy)
{
    this->proxy = proxy;
}

unsigned int SetupFailInt::getNewnode() const
{
    return this->newnode;
}

void SetupFailInt::setNewnode(unsigned int newnode)
{
    this->newnode = newnode;
}

unsigned int SetupFailInt::getSrc() const
{
    return this->src;
}

void SetupFailInt::setSrc(unsigned int src)
{
    this->src = src;
}

const VlrIntVidVec& SetupFailInt::getTrace() const
{
    return this->trace;
}

void SetupFailInt::setTrace(const VlrIntVidVec& trace)
{
    this->trace = trace;
}

class SetupFailIntDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_proxy,
        FIELD_newnode,
        FIELD_src,
        FIELD_trace,
    };
  public:
    SetupFailIntDescriptor();
    virtual ~SetupFailIntDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(SetupFailIntDescriptor)

SetupFailIntDescriptor::SetupFailIntDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(omnetvlr::SetupFailInt)), "omnetvlr::VlrIntSetupPacket")
{
    propertyNames = nullptr;
}

SetupFailIntDescriptor::~SetupFailIntDescriptor()
{
    delete[] propertyNames;
}

bool SetupFailIntDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<SetupFailInt *>(obj)!=nullptr;
}

const char **SetupFailIntDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *SetupFailIntDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int SetupFailIntDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 4+base->getFieldCount() : 4;
}

unsigned int SetupFailIntDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_proxy
        FD_ISEDITABLE,    // FIELD_newnode
        FD_ISEDITABLE,    // FIELD_src
        FD_ISCOMPOUND,    // FIELD_trace
    };
    return (field >= 0 && field < 4) ? fieldTypeFlags[field] : 0;
}

const char *SetupFailIntDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "proxy",
        "newnode",
        "src",
        "trace",
    };
    return (field >= 0 && field < 4) ? fieldNames[field] : nullptr;
}

int SetupFailIntDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "proxy") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "newnode") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "src") == 0) return baseIndex + 2;
    if (strcmp(fieldName, "trace") == 0) return baseIndex + 3;
    return base ? base->findField(fieldName) : -1;
}

const char *SetupFailIntDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "unsigned int",    // FIELD_proxy
        "unsigned int",    // FIELD_newnode
        "unsigned int",    // FIELD_src
        "omnetvlr::VlrIntVidVec",    // FIELD_trace
    };
    return (field >= 0 && field < 4) ? fieldTypeStrings[field] : nullptr;
}

const char **SetupFailIntDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *SetupFailIntDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int SetupFailIntDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    SetupFailInt *pp = omnetpp::fromAnyPtr<SetupFailInt>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void SetupFailIntDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    SetupFailInt *pp = omnetpp::fromAnyPtr<SetupFailInt>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'SetupFailInt'", field);
    }
}

const char *SetupFailIntDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    SetupFailInt *pp = omnetpp::fromAnyPtr<SetupFailInt>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string SetupFailIntDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    SetupFailInt *pp = omnetpp::fromAnyPtr<SetupFailInt>(object); (void)pp;
    switch (field) {
        case FIELD_proxy: return ulong2string(pp->getProxy());
        case FIELD_newnode: return ulong2string(pp->getNewnode());
        case FIELD_src: return ulong2string(pp->getSrc());
        case FIELD_trace: return "";
        default: return "";
    }
}

void SetupFailIntDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    SetupFailInt *pp = omnetpp::fromAnyPtr<SetupFailInt>(object); (void)pp;
    switch (field) {
        case FIELD_proxy: pp->setProxy(string2ulong(value)); break;
        case FIELD_newnode: pp->setNewnode(string2ulong(value)); break;
        case FIELD_src: pp->setSrc(string2ulong(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'SetupFailInt'", field);
    }
}

omnetpp::cValue SetupFailIntDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    SetupFailInt *pp = omnetpp::fromAnyPtr<SetupFailInt>(object); (void)pp;
    switch (field) {
        case FIELD_proxy: return (omnetpp::intval_t)(pp->getProxy());
        case FIELD_newnode: return (omnetpp::intval_t)(pp->getNewnode());
        case FIELD_src: return (omnetpp::intval_t)(pp->getSrc());
        case FIELD_trace: return omnetpp::toAnyPtr(&pp->getTrace()); break;
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'SetupFailInt' as cValue -- field index out of range?", field);
    }
}

void SetupFailIntDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    SetupFailInt *pp = omnetpp::fromAnyPtr<SetupFailInt>(object); (void)pp;
    switch (field) {
        case FIELD_proxy: pp->setProxy(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        case FIELD_newnode: pp->setNewnode(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        case FIELD_src: pp->setSrc(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'SetupFailInt'", field);
    }
}

const char *SetupFailIntDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        case FIELD_trace: return omnetpp::opp_typename(typeid(VlrIntVidVec));
        default: return nullptr;
    };
}

omnetpp::any_ptr SetupFailIntDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    SetupFailInt *pp = omnetpp::fromAnyPtr<SetupFailInt>(object); (void)pp;
    switch (field) {
        case FIELD_trace: return omnetpp::toAnyPtr(&pp->getTrace()); break;
        default: return omnetpp::any_ptr(nullptr);
    }
}

void SetupFailIntDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    SetupFailInt *pp = omnetpp::fromAnyPtr<SetupFailInt>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'SetupFailInt'", field);
    }
}

Register_Class(AddRouteInt)

AddRouteInt::AddRouteInt(const char *name) : ::omnetvlr::VlrIntSetupPacket(name)
{
}

AddRouteInt::AddRouteInt(const AddRouteInt& other) : ::omnetvlr::VlrIntSetupPacket(other)
{
    copy(other);
}

AddRouteInt::~AddRouteInt()
{
}

AddRouteInt& AddRouteInt::operator=(const AddRouteInt& other)
{
    if (this == &other) return *this;
    ::omnetvlr::VlrIntSetupPacket::operator=(other);
    copy(other);
    return *this;
}

void AddRouteInt::copy(const AddRouteInt& other)
{
    this->dst = other.dst;
    this->src = other.src;
    this->proxy = other.proxy;
    this->pathid = other.pathid;
    this->trace = other.trace;
    this->prevhopVids = other.prevhopVids;
    this->oldestPrevhopIndex = other.oldestPrevhopIndex;
}

void AddRouteInt::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetvlr::VlrIntSetupPacket::parsimPack(b);
    doParsimPacking(b,this->dst);
    doParsimPacking(b,this->src);
    doParsimPacking(b,this->proxy);
    doParsimPacking(b,this->pathid);
    doParsimPacking(b,this->trace);
    doParsimPacking(b,this->prevhopVids);
    doParsimPacking(b,this->oldestPrevhopIndex);
}

void AddRouteInt::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetvlr::VlrIntSetupPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->dst);
    doParsimUnpacking(b,this->src);
    doParsimUnpacking(b,this->proxy);
    doParsimUnpacking(b,this->pathid);
    doParsimUnpacking(b,this->trace);
    doParsimUnpacking(b,this->prevhopVids);
    doParsimUnpacking(b,this->oldestPrevhopIndex);
}

unsigned int AddRouteInt::getDst() const
{
    return this->dst;
}

void AddRouteInt::setDst(unsigned int dst)
{
    this->dst = dst;
}

unsigned int AddRouteInt::getSrc() const
{
    return this->src;
}

void AddRouteInt::setSrc(unsigned int src)
{
    this->src = src;
}

unsigned int AddRouteInt::getProxy() const
{
    return this->proxy;
}

void AddRouteInt::setProxy(unsigned int proxy)
{
    this->proxy = proxy;
}

const VlrPathID& AddRouteInt::getPathid() const
{
    return this->pathid;
}

void AddRouteInt::setPathid(const VlrPathID& pathid)
{
    this->pathid = pathid;
}

const VlrIntVidVec& AddRouteInt::getTrace() const
{
    return this->trace;
}

void AddRouteInt::setTrace(const VlrIntVidVec& trace)
{
    this->trace = trace;
}

const VlrIntVidVec& AddRouteInt::getPrevhopVids() const
{
    return this->prevhopVids;
}

void AddRouteInt::setPrevhopVids(const VlrIntVidVec& prevhopVids)
{
    this->prevhopVids = prevhopVids;
}

unsigned int AddRouteInt::getOldestPrevhopIndex() const
{
    return this->oldestPrevhopIndex;
}

void AddRouteInt::setOldestPrevhopIndex(unsigned int oldestPrevhopIndex)
{
    this->oldestPrevhopIndex = oldestPrevhopIndex;
}

class AddRouteIntDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_dst,
        FIELD_src,
        FIELD_proxy,
        FIELD_pathid,
        FIELD_trace,
        FIELD_prevhopVids,
        FIELD_oldestPrevhopIndex,
    };
  public:
    AddRouteIntDescriptor();
    virtual ~AddRouteIntDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(AddRouteIntDescriptor)

AddRouteIntDescriptor::AddRouteIntDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(omnetvlr::AddRouteInt)), "omnetvlr::VlrIntSetupPacket")
{
    propertyNames = nullptr;
}

AddRouteIntDescriptor::~AddRouteIntDescriptor()
{
    delete[] propertyNames;
}

bool AddRouteIntDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<AddRouteInt *>(obj)!=nullptr;
}

const char **AddRouteIntDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *AddRouteIntDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int AddRouteIntDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 7+base->getFieldCount() : 7;
}

unsigned int AddRouteIntDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_dst
        FD_ISEDITABLE,    // FIELD_src
        FD_ISEDITABLE,    // FIELD_proxy
        0,    // FIELD_pathid
        FD_ISCOMPOUND,    // FIELD_trace
        FD_ISCOMPOUND,    // FIELD_prevhopVids
        FD_ISEDITABLE,    // FIELD_oldestPrevhopIndex
    };
    return (field >= 0 && field < 7) ? fieldTypeFlags[field] : 0;
}

const char *AddRouteIntDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "dst",
        "src",
        "proxy",
        "pathid",
        "trace",
        "prevhopVids",
        "oldestPrevhopIndex",
    };
    return (field >= 0 && field < 7) ? fieldNames[field] : nullptr;
}

int AddRouteIntDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "dst") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "src") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "proxy") == 0) return baseIndex + 2;
    if (strcmp(fieldName, "pathid") == 0) return baseIndex + 3;
    if (strcmp(fieldName, "trace") == 0) return baseIndex + 4;
    if (strcmp(fieldName, "prevhopVids") == 0) return baseIndex + 5;
    if (strcmp(fieldName, "oldestPrevhopIndex") == 0) return baseIndex + 6;
    return base ? base->findField(fieldName) : -1;
}

const char *AddRouteIntDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "unsigned int",    // FIELD_dst
        "unsigned int",    // FIELD_src
        "unsigned int",    // FIELD_proxy
        "omnetvlr::VlrPathID",    // FIELD_pathid
        "omnetvlr::VlrIntVidVec",    // FIELD_trace
        "omnetvlr::VlrIntVidVec",    // FIELD_prevhopVids
        "unsigned int",    // FIELD_oldestPrevhopIndex
    };
    return (field >= 0 && field < 7) ? fieldTypeStrings[field] : nullptr;
}

const char **AddRouteIntDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *AddRouteIntDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int AddRouteIntDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    AddRouteInt *pp = omnetpp::fromAnyPtr<AddRouteInt>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void AddRouteIntDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    AddRouteInt *pp = omnetpp::fromAnyPtr<AddRouteInt>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'AddRouteInt'", field);
    }
}

const char *AddRouteIntDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    AddRouteInt *pp = omnetpp::fromAnyPtr<AddRouteInt>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string AddRouteIntDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    AddRouteInt *pp = omnetpp::fromAnyPtr<AddRouteInt>(object); (void)pp;
    switch (field) {
        case FIELD_dst: return ulong2string(pp->getDst());
        case FIELD_src: return ulong2string(pp->getSrc());
        case FIELD_proxy: return ulong2string(pp->getProxy());
        case FIELD_pathid: return ulong2string(pp->getPathid());
        case FIELD_trace: return "";
        case FIELD_prevhopVids: return "";
        case FIELD_oldestPrevhopIndex: return ulong2string(pp->getOldestPrevhopIndex());
        default: return "";
    }
}

void AddRouteIntDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    AddRouteInt *pp = omnetpp::fromAnyPtr<AddRouteInt>(object); (void)pp;
    switch (field) {
        case FIELD_dst: pp->setDst(string2ulong(value)); break;
        case FIELD_src: pp->setSrc(string2ulong(value)); break;
        case FIELD_proxy: pp->setProxy(string2ulong(value)); break;
        case FIELD_oldestPrevhopIndex: pp->setOldestPrevhopIndex(string2ulong(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'AddRouteInt'", field);
    }
}

omnetpp::cValue AddRouteIntDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    AddRouteInt *pp = omnetpp::fromAnyPtr<AddRouteInt>(object); (void)pp;
    switch (field) {
        case FIELD_dst: return (omnetpp::intval_t)(pp->getDst());
        case FIELD_src: return (omnetpp::intval_t)(pp->getSrc());
        case FIELD_proxy: return (omnetpp::intval_t)(pp->getProxy());
        case FIELD_pathid: return omnetpp::toAnyPtr(&pp->getPathid()); break;
        case FIELD_trace: return omnetpp::toAnyPtr(&pp->getTrace()); break;
        case FIELD_prevhopVids: return omnetpp::toAnyPtr(&pp->getPrevhopVids()); break;
        case FIELD_oldestPrevhopIndex: return (omnetpp::intval_t)(pp->getOldestPrevhopIndex());
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'AddRouteInt' as cValue -- field index out of range?", field);
    }
}

void AddRouteIntDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    AddRouteInt *pp = omnetpp::fromAnyPtr<AddRouteInt>(object); (void)pp;
    switch (field) {
        case FIELD_dst: pp->setDst(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        case FIELD_src: pp->setSrc(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        case FIELD_proxy: pp->setProxy(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        case FIELD_oldestPrevhopIndex: pp->setOldestPrevhopIndex(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'AddRouteInt'", field);
    }
}

const char *AddRouteIntDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        case FIELD_trace: return omnetpp::opp_typename(typeid(VlrIntVidVec));
        case FIELD_prevhopVids: return omnetpp::opp_typename(typeid(VlrIntVidVec));
        default: return nullptr;
    };
}

omnetpp::any_ptr AddRouteIntDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    AddRouteInt *pp = omnetpp::fromAnyPtr<AddRouteInt>(object); (void)pp;
    switch (field) {
        case FIELD_pathid: return omnetpp::toAnyPtr(&pp->getPathid()); break;
        case FIELD_trace: return omnetpp::toAnyPtr(&pp->getTrace()); break;
        case FIELD_prevhopVids: return omnetpp::toAnyPtr(&pp->getPrevhopVids()); break;
        default: return omnetpp::any_ptr(nullptr);
    }
}

void AddRouteIntDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    AddRouteInt *pp = omnetpp::fromAnyPtr<AddRouteInt>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'AddRouteInt'", field);
    }
}

Register_Class(TeardownInt)

TeardownInt::TeardownInt(const char *name) : ::omnetvlr::VlrIntSetupPacket(name)
{
}

TeardownInt::TeardownInt(const TeardownInt& other) : ::omnetvlr::VlrIntSetupPacket(other)
{
    copy(other);
}

TeardownInt::~TeardownInt()
{
    delete [] this->pathids;
}

TeardownInt& TeardownInt::operator=(const TeardownInt& other)
{
    if (this == &other) return *this;
    ::omnetvlr::VlrIntSetupPacket::operator=(other);
    copy(other);
    return *this;
}

void TeardownInt::copy(const TeardownInt& other)
{
    delete [] this->pathids;
    this->pathids = (other.pathids_arraysize==0) ? nullptr : new VlrPathID[other.pathids_arraysize];
    pathids_arraysize = other.pathids_arraysize;
    for (size_t i = 0; i < pathids_arraysize; i++) {
        this->pathids[i] = other.pathids[i];
    }
    this->src = other.src;
    this->rebuild = other.rebuild;
    this->dismantled = other.dismantled;
}

void TeardownInt::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetvlr::VlrIntSetupPacket::parsimPack(b);
    b->pack(pathids_arraysize);
    doParsimArrayPacking(b,this->pathids,pathids_arraysize);
    doParsimPacking(b,this->src);
    doParsimPacking(b,this->rebuild);
    doParsimPacking(b,this->dismantled);
}

void TeardownInt::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetvlr::VlrIntSetupPacket::parsimUnpack(b);
    delete [] this->pathids;
    b->unpack(pathids_arraysize);
    if (pathids_arraysize == 0) {
        this->pathids = nullptr;
    } else {
        this->pathids = new VlrPathID[pathids_arraysize];
        doParsimArrayUnpacking(b,this->pathids,pathids_arraysize);
    }
    doParsimUnpacking(b,this->src);
    doParsimUnpacking(b,this->rebuild);
    doParsimUnpacking(b,this->dismantled);
}

size_t TeardownInt::getPathidsArraySize() const
{
    return pathids_arraysize;
}

const VlrPathID& TeardownInt::getPathids(size_t k) const
{
    if (k >= pathids_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)pathids_arraysize, (unsigned long)k);
    return this->pathids[k];
}

void TeardownInt::setPathidsArraySize(size_t newSize)
{
    VlrPathID *pathids2 = (newSize==0) ? nullptr : new VlrPathID[newSize];
    size_t minSize = pathids_arraysize < newSize ? pathids_arraysize : newSize;
    for (size_t i = 0; i < minSize; i++)
        pathids2[i] = this->pathids[i];
    delete [] this->pathids;
    this->pathids = pathids2;
    pathids_arraysize = newSize;
}

void TeardownInt::setPathids(size_t k, const VlrPathID& pathids)
{
    if (k >= pathids_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)pathids_arraysize, (unsigned long)k);
    this->pathids[k] = pathids;
}

void TeardownInt::insertPathids(size_t k, const VlrPathID& pathids)
{
    if (k > pathids_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)pathids_arraysize, (unsigned long)k);
    size_t newSize = pathids_arraysize + 1;
    VlrPathID *pathids2 = new VlrPathID[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        pathids2[i] = this->pathids[i];
    pathids2[k] = pathids;
    for (i = k + 1; i < newSize; i++)
        pathids2[i] = this->pathids[i-1];
    delete [] this->pathids;
    this->pathids = pathids2;
    pathids_arraysize = newSize;
}

void TeardownInt::appendPathids(const VlrPathID& pathids)
{
    insertPathids(pathids_arraysize, pathids);
}

void TeardownInt::erasePathids(size_t k)
{
    if (k >= pathids_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)pathids_arraysize, (unsigned long)k);
    size_t newSize = pathids_arraysize - 1;
    VlrPathID *pathids2 = (newSize == 0) ? nullptr : new VlrPathID[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        pathids2[i] = this->pathids[i];
    for (i = k; i < newSize; i++)
        pathids2[i] = this->pathids[i+1];
    delete [] this->pathids;
    this->pathids = pathids2;
    pathids_arraysize = newSize;
}

unsigned int TeardownInt::getSrc() const
{
    return this->src;
}

void TeardownInt::setSrc(unsigned int src)
{
    this->src = src;
}

bool TeardownInt::getRebuild() const
{
    return this->rebuild;
}

void TeardownInt::setRebuild(bool rebuild)
{
    this->rebuild = rebuild;
}

bool TeardownInt::getDismantled() const
{
    return this->dismantled;
}

void TeardownInt::setDismantled(bool dismantled)
{
    this->dismantled = dismantled;
}

class TeardownIntDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_pathids,
        FIELD_src,
        FIELD_rebuild,
        FIELD_dismantled,
    };
  public:
    TeardownIntDescriptor();
    virtual ~TeardownIntDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(TeardownIntDescriptor)

TeardownIntDescriptor::TeardownIntDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(omnetvlr::TeardownInt)), "omnetvlr::VlrIntSetupPacket")
{
    propertyNames = nullptr;
}

TeardownIntDescriptor::~TeardownIntDescriptor()
{
    delete[] propertyNames;
}

bool TeardownIntDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<TeardownInt *>(obj)!=nullptr;
}

const char **TeardownIntDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *TeardownIntDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int TeardownIntDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 4+base->getFieldCount() : 4;
}

unsigned int TeardownIntDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISARRAY | FD_ISRESIZABLE,    // FIELD_pathids
        FD_ISEDITABLE,    // FIELD_src
        FD_ISEDITABLE,    // FIELD_rebuild
        FD_ISEDITABLE,    // FIELD_dismantled
    };
    return (field >= 0 && field < 4) ? fieldTypeFlags[field] : 0;
}

const char *TeardownIntDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "pathids",
        "src",
        "rebuild",
        "dismantled",
    };
    return (field >= 0 && field < 4) ? fieldNames[field] : nullptr;
}

int TeardownIntDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "pathids") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "src") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "rebuild") == 0) return baseIndex + 2;
    if (strcmp(fieldName, "dismantled") == 0) return baseIndex + 3;
    return base ? base->findField(fieldName) : -1;
}

const char *TeardownIntDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "omnetvlr::VlrPathID",    // FIELD_pathids
        "unsigned int",    // FIELD_src
        "bool",    // FIELD_rebuild
        "bool",    // FIELD_dismantled
    };
    return (field >= 0 && field < 4) ? fieldTypeStrings[field] : nullptr;
}

const char **TeardownIntDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *TeardownIntDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int TeardownIntDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    TeardownInt *pp = omnetpp::fromAnyPtr<TeardownInt>(object); (void)pp;
    switch (field) {
        case FIELD_pathids: return pp->getPathidsArraySize();
        default: return 0;
    }
}

void TeardownIntDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    TeardownInt *pp = omnetpp::fromAnyPtr<TeardownInt>(object); (void)pp;
    switch (field) {
        case FIELD_pathids: pp->setPathidsArraySize(size); break;
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'TeardownInt'", field);
    }
}

const char *TeardownIntDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    TeardownInt *pp = omnetpp::fromAnyPtr<TeardownInt>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string TeardownIntDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    TeardownInt *pp = omnetpp::fromAnyPtr<TeardownInt>(object); (void)pp;
    switch (field) {
        case FIELD_pathids: return ulong2string(pp->getPathids(i));
        case FIELD_src: return ulong2string(pp->getSrc());
        case FIELD_rebuild: return bool2string(pp->getRebuild());
        case FIELD_dismantled: return bool2string(pp->getDismantled());
        default: return "";
    }
}

void TeardownIntDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    TeardownInt *pp = omnetpp::fromAnyPtr<TeardownInt>(object); (void)pp;
    switch (field) {
        case FIELD_src: pp->setSrc(string2ulong(value)); break;
        case FIELD_rebuild: pp->setRebuild(string2bool(value)); break;
        case FIELD_dismantled: pp->setDismantled(string2bool(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'TeardownInt'", field);
    }
}

omnetpp::cValue TeardownIntDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    TeardownInt *pp = omnetpp::fromAnyPtr<TeardownInt>(object); (void)pp;
    switch (field) {
        case FIELD_pathids: return omnetpp::toAnyPtr(&pp->getPathids(i)); break;
        case FIELD_src: return (omnetpp::intval_t)(pp->getSrc());
        case FIELD_rebuild: return pp->getRebuild();
        case FIELD_dismantled: return pp->getDismantled();
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'TeardownInt' as cValue -- field index out of range?", field);
    }
}

void TeardownIntDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    TeardownInt *pp = omnetpp::fromAnyPtr<TeardownInt>(object); (void)pp;
    switch (field) {
        case FIELD_src: pp->setSrc(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        case FIELD_rebuild: pp->setRebuild(value.boolValue()); break;
        case FIELD_dismantled: pp->setDismantled(value.boolValue()); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'TeardownInt'", field);
    }
}

const char *TeardownIntDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr TeardownIntDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    TeardownInt *pp = omnetpp::fromAnyPtr<TeardownInt>(object); (void)pp;
    switch (field) {
        case FIELD_pathids: return omnetpp::toAnyPtr(&pp->getPathids(i)); break;
        default: return omnetpp::any_ptr(nullptr);
    }
}

void TeardownIntDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    TeardownInt *pp = omnetpp::fromAnyPtr<TeardownInt>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'TeardownInt'", field);
    }
}

Register_Class(DismantleInt)

DismantleInt::DismantleInt(const char *name) : ::omnetvlr::VlrIntSetupPacket(name)
{
}

DismantleInt::DismantleInt(const DismantleInt& other) : ::omnetvlr::VlrIntSetupPacket(other)
{
    copy(other);
}

DismantleInt::~DismantleInt()
{
    delete [] this->pathids;
}

DismantleInt& DismantleInt::operator=(const DismantleInt& other)
{
    if (this == &other) return *this;
    ::omnetvlr::VlrIntSetupPacket::operator=(other);
    copy(other);
    return *this;
}

void DismantleInt::copy(const DismantleInt& other)
{
    delete [] this->pathids;
    this->pathids = (other.pathids_arraysize==0) ? nullptr : new VlrPathID[other.pathids_arraysize];
    pathids_arraysize = other.pathids_arraysize;
    for (size_t i = 0; i < pathids_arraysize; i++) {
        this->pathids[i] = other.pathids[i];
    }
    this->src = other.src;
}

void DismantleInt::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetvlr::VlrIntSetupPacket::parsimPack(b);
    b->pack(pathids_arraysize);
    doParsimArrayPacking(b,this->pathids,pathids_arraysize);
    doParsimPacking(b,this->src);
}

void DismantleInt::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetvlr::VlrIntSetupPacket::parsimUnpack(b);
    delete [] this->pathids;
    b->unpack(pathids_arraysize);
    if (pathids_arraysize == 0) {
        this->pathids = nullptr;
    } else {
        this->pathids = new VlrPathID[pathids_arraysize];
        doParsimArrayUnpacking(b,this->pathids,pathids_arraysize);
    }
    doParsimUnpacking(b,this->src);
}

size_t DismantleInt::getPathidsArraySize() const
{
    return pathids_arraysize;
}

const VlrPathID& DismantleInt::getPathids(size_t k) const
{
    if (k >= pathids_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)pathids_arraysize, (unsigned long)k);
    return this->pathids[k];
}

void DismantleInt::setPathidsArraySize(size_t newSize)
{
    VlrPathID *pathids2 = (newSize==0) ? nullptr : new VlrPathID[newSize];
    size_t minSize = pathids_arraysize < newSize ? pathids_arraysize : newSize;
    for (size_t i = 0; i < minSize; i++)
        pathids2[i] = this->pathids[i];
    delete [] this->pathids;
    this->pathids = pathids2;
    pathids_arraysize = newSize;
}

void DismantleInt::setPathids(size_t k, const VlrPathID& pathids)
{
    if (k >= pathids_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)pathids_arraysize, (unsigned long)k);
    this->pathids[k] = pathids;
}

void DismantleInt::insertPathids(size_t k, const VlrPathID& pathids)
{
    if (k > pathids_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)pathids_arraysize, (unsigned long)k);
    size_t newSize = pathids_arraysize + 1;
    VlrPathID *pathids2 = new VlrPathID[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        pathids2[i] = this->pathids[i];
    pathids2[k] = pathids;
    for (i = k + 1; i < newSize; i++)
        pathids2[i] = this->pathids[i-1];
    delete [] this->pathids;
    this->pathids = pathids2;
    pathids_arraysize = newSize;
}

void DismantleInt::appendPathids(const VlrPathID& pathids)
{
    insertPathids(pathids_arraysize, pathids);
}

void DismantleInt::erasePathids(size_t k)
{
    if (k >= pathids_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)pathids_arraysize, (unsigned long)k);
    size_t newSize = pathids_arraysize - 1;
    VlrPathID *pathids2 = (newSize == 0) ? nullptr : new VlrPathID[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        pathids2[i] = this->pathids[i];
    for (i = k; i < newSize; i++)
        pathids2[i] = this->pathids[i+1];
    delete [] this->pathids;
    this->pathids = pathids2;
    pathids_arraysize = newSize;
}

unsigned int DismantleInt::getSrc() const
{
    return this->src;
}

void DismantleInt::setSrc(unsigned int src)
{
    this->src = src;
}

class DismantleIntDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_pathids,
        FIELD_src,
    };
  public:
    DismantleIntDescriptor();
    virtual ~DismantleIntDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(DismantleIntDescriptor)

DismantleIntDescriptor::DismantleIntDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(omnetvlr::DismantleInt)), "omnetvlr::VlrIntSetupPacket")
{
    propertyNames = nullptr;
}

DismantleIntDescriptor::~DismantleIntDescriptor()
{
    delete[] propertyNames;
}

bool DismantleIntDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<DismantleInt *>(obj)!=nullptr;
}

const char **DismantleIntDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *DismantleIntDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int DismantleIntDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 2+base->getFieldCount() : 2;
}

unsigned int DismantleIntDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISARRAY | FD_ISRESIZABLE,    // FIELD_pathids
        FD_ISEDITABLE,    // FIELD_src
    };
    return (field >= 0 && field < 2) ? fieldTypeFlags[field] : 0;
}

const char *DismantleIntDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "pathids",
        "src",
    };
    return (field >= 0 && field < 2) ? fieldNames[field] : nullptr;
}

int DismantleIntDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "pathids") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "src") == 0) return baseIndex + 1;
    return base ? base->findField(fieldName) : -1;
}

const char *DismantleIntDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "omnetvlr::VlrPathID",    // FIELD_pathids
        "unsigned int",    // FIELD_src
    };
    return (field >= 0 && field < 2) ? fieldTypeStrings[field] : nullptr;
}

const char **DismantleIntDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *DismantleIntDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int DismantleIntDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    DismantleInt *pp = omnetpp::fromAnyPtr<DismantleInt>(object); (void)pp;
    switch (field) {
        case FIELD_pathids: return pp->getPathidsArraySize();
        default: return 0;
    }
}

void DismantleIntDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    DismantleInt *pp = omnetpp::fromAnyPtr<DismantleInt>(object); (void)pp;
    switch (field) {
        case FIELD_pathids: pp->setPathidsArraySize(size); break;
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'DismantleInt'", field);
    }
}

const char *DismantleIntDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    DismantleInt *pp = omnetpp::fromAnyPtr<DismantleInt>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string DismantleIntDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    DismantleInt *pp = omnetpp::fromAnyPtr<DismantleInt>(object); (void)pp;
    switch (field) {
        case FIELD_pathids: return ulong2string(pp->getPathids(i));
        case FIELD_src: return ulong2string(pp->getSrc());
        default: return "";
    }
}

void DismantleIntDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    DismantleInt *pp = omnetpp::fromAnyPtr<DismantleInt>(object); (void)pp;
    switch (field) {
        case FIELD_src: pp->setSrc(string2ulong(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'DismantleInt'", field);
    }
}

omnetpp::cValue DismantleIntDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    DismantleInt *pp = omnetpp::fromAnyPtr<DismantleInt>(object); (void)pp;
    switch (field) {
        case FIELD_pathids: return omnetpp::toAnyPtr(&pp->getPathids(i)); break;
        case FIELD_src: return (omnetpp::intval_t)(pp->getSrc());
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'DismantleInt' as cValue -- field index out of range?", field);
    }
}

void DismantleIntDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    DismantleInt *pp = omnetpp::fromAnyPtr<DismantleInt>(object); (void)pp;
    switch (field) {
        case FIELD_src: pp->setSrc(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'DismantleInt'", field);
    }
}

const char *DismantleIntDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr DismantleIntDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    DismantleInt *pp = omnetpp::fromAnyPtr<DismantleInt>(object); (void)pp;
    switch (field) {
        case FIELD_pathids: return omnetpp::toAnyPtr(&pp->getPathids(i)); break;
        default: return omnetpp::any_ptr(nullptr);
    }
}

void DismantleIntDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    DismantleInt *pp = omnetpp::fromAnyPtr<DismantleInt>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'DismantleInt'", field);
    }
}

Register_Class(VlrIntTestPacket)

VlrIntTestPacket::VlrIntTestPacket(const char *name) : ::omnetvlr::VlrIntUniPacket(name)
{
}

VlrIntTestPacket::VlrIntTestPacket(const VlrIntTestPacket& other) : ::omnetvlr::VlrIntUniPacket(other)
{
    copy(other);
}

VlrIntTestPacket::~VlrIntTestPacket()
{
}

VlrIntTestPacket& VlrIntTestPacket::operator=(const VlrIntTestPacket& other)
{
    if (this == &other) return *this;
    ::omnetvlr::VlrIntUniPacket::operator=(other);
    copy(other);
    return *this;
}

void VlrIntTestPacket::copy(const VlrIntTestPacket& other)
{
    this->dst = other.dst;
    this->src = other.src;
}

void VlrIntTestPacket::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetvlr::VlrIntUniPacket::parsimPack(b);
    doParsimPacking(b,this->dst);
    doParsimPacking(b,this->src);
}

void VlrIntTestPacket::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetvlr::VlrIntUniPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->dst);
    doParsimUnpacking(b,this->src);
}

unsigned int VlrIntTestPacket::getDst() const
{
    return this->dst;
}

void VlrIntTestPacket::setDst(unsigned int dst)
{
    this->dst = dst;
}

unsigned int VlrIntTestPacket::getSrc() const
{
    return this->src;
}

void VlrIntTestPacket::setSrc(unsigned int src)
{
    this->src = src;
}

class VlrIntTestPacketDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_dst,
        FIELD_src,
    };
  public:
    VlrIntTestPacketDescriptor();
    virtual ~VlrIntTestPacketDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(VlrIntTestPacketDescriptor)

VlrIntTestPacketDescriptor::VlrIntTestPacketDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(omnetvlr::VlrIntTestPacket)), "omnetvlr::VlrIntUniPacket")
{
    propertyNames = nullptr;
}

VlrIntTestPacketDescriptor::~VlrIntTestPacketDescriptor()
{
    delete[] propertyNames;
}

bool VlrIntTestPacketDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<VlrIntTestPacket *>(obj)!=nullptr;
}

const char **VlrIntTestPacketDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *VlrIntTestPacketDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int VlrIntTestPacketDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 2+base->getFieldCount() : 2;
}

unsigned int VlrIntTestPacketDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_dst
        FD_ISEDITABLE,    // FIELD_src
    };
    return (field >= 0 && field < 2) ? fieldTypeFlags[field] : 0;
}

const char *VlrIntTestPacketDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "dst",
        "src",
    };
    return (field >= 0 && field < 2) ? fieldNames[field] : nullptr;
}

int VlrIntTestPacketDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "dst") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "src") == 0) return baseIndex + 1;
    return base ? base->findField(fieldName) : -1;
}

const char *VlrIntTestPacketDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "unsigned int",    // FIELD_dst
        "unsigned int",    // FIELD_src
    };
    return (field >= 0 && field < 2) ? fieldTypeStrings[field] : nullptr;
}

const char **VlrIntTestPacketDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *VlrIntTestPacketDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int VlrIntTestPacketDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    VlrIntTestPacket *pp = omnetpp::fromAnyPtr<VlrIntTestPacket>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void VlrIntTestPacketDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntTestPacket *pp = omnetpp::fromAnyPtr<VlrIntTestPacket>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'VlrIntTestPacket'", field);
    }
}

const char *VlrIntTestPacketDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    VlrIntTestPacket *pp = omnetpp::fromAnyPtr<VlrIntTestPacket>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string VlrIntTestPacketDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    VlrIntTestPacket *pp = omnetpp::fromAnyPtr<VlrIntTestPacket>(object); (void)pp;
    switch (field) {
        case FIELD_dst: return ulong2string(pp->getDst());
        case FIELD_src: return ulong2string(pp->getSrc());
        default: return "";
    }
}

void VlrIntTestPacketDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntTestPacket *pp = omnetpp::fromAnyPtr<VlrIntTestPacket>(object); (void)pp;
    switch (field) {
        case FIELD_dst: pp->setDst(string2ulong(value)); break;
        case FIELD_src: pp->setSrc(string2ulong(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VlrIntTestPacket'", field);
    }
}

omnetpp::cValue VlrIntTestPacketDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    VlrIntTestPacket *pp = omnetpp::fromAnyPtr<VlrIntTestPacket>(object); (void)pp;
    switch (field) {
        case FIELD_dst: return (omnetpp::intval_t)(pp->getDst());
        case FIELD_src: return (omnetpp::intval_t)(pp->getSrc());
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'VlrIntTestPacket' as cValue -- field index out of range?", field);
    }
}

void VlrIntTestPacketDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntTestPacket *pp = omnetpp::fromAnyPtr<VlrIntTestPacket>(object); (void)pp;
    switch (field) {
        case FIELD_dst: pp->setDst(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        case FIELD_src: pp->setSrc(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VlrIntTestPacket'", field);
    }
}

const char *VlrIntTestPacketDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr VlrIntTestPacketDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    VlrIntTestPacket *pp = omnetpp::fromAnyPtr<VlrIntTestPacket>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void VlrIntTestPacketDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    VlrIntTestPacket *pp = omnetpp::fromAnyPtr<VlrIntTestPacket>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VlrIntTestPacket'", field);
    }
}

Register_Class(RepairLinkReqFloodInt)

RepairLinkReqFloodInt::RepairLinkReqFloodInt(const char *name, short kind) : ::omnetpp::cPacket(name, kind)
{
}

RepairLinkReqFloodInt::RepairLinkReqFloodInt(const RepairLinkReqFloodInt& other) : ::omnetpp::cPacket(other)
{
    copy(other);
}

RepairLinkReqFloodInt::~RepairLinkReqFloodInt()
{
}

RepairLinkReqFloodInt& RepairLinkReqFloodInt::operator=(const RepairLinkReqFloodInt& other)
{
    if (this == &other) return *this;
    ::omnetpp::cPacket::operator=(other);
    copy(other);
    return *this;
}

void RepairLinkReqFloodInt::copy(const RepairLinkReqFloodInt& other)
{
    this->dstToPathidsMap = other.dstToPathidsMap;
    this->ttl = other.ttl;
    this->floodSeqnum = other.floodSeqnum;
    this->linkTrace = other.linkTrace;
}

void RepairLinkReqFloodInt::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cPacket::parsimPack(b);
    doParsimPacking(b,this->dstToPathidsMap);
    doParsimPacking(b,this->ttl);
    doParsimPacking(b,this->floodSeqnum);
    doParsimPacking(b,this->linkTrace);
}

void RepairLinkReqFloodInt::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->dstToPathidsMap);
    doParsimUnpacking(b,this->ttl);
    doParsimUnpacking(b,this->floodSeqnum);
    doParsimUnpacking(b,this->linkTrace);
}

const VlrIntVidToPathidSetMap& RepairLinkReqFloodInt::getDstToPathidsMap() const
{
    return this->dstToPathidsMap;
}

void RepairLinkReqFloodInt::setDstToPathidsMap(const VlrIntVidToPathidSetMap& dstToPathidsMap)
{
    this->dstToPathidsMap = dstToPathidsMap;
}

unsigned int RepairLinkReqFloodInt::getTtl() const
{
    return this->ttl;
}

void RepairLinkReqFloodInt::setTtl(unsigned int ttl)
{
    this->ttl = ttl;
}

unsigned int RepairLinkReqFloodInt::getFloodSeqnum() const
{
    return this->floodSeqnum;
}

void RepairLinkReqFloodInt::setFloodSeqnum(unsigned int floodSeqnum)
{
    this->floodSeqnum = floodSeqnum;
}

const VlrIntVidVec& RepairLinkReqFloodInt::getLinkTrace() const
{
    return this->linkTrace;
}

void RepairLinkReqFloodInt::setLinkTrace(const VlrIntVidVec& linkTrace)
{
    this->linkTrace = linkTrace;
}

class RepairLinkReqFloodIntDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_dstToPathidsMap,
        FIELD_ttl,
        FIELD_floodSeqnum,
        FIELD_linkTrace,
    };
  public:
    RepairLinkReqFloodIntDescriptor();
    virtual ~RepairLinkReqFloodIntDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(RepairLinkReqFloodIntDescriptor)

RepairLinkReqFloodIntDescriptor::RepairLinkReqFloodIntDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(omnetvlr::RepairLinkReqFloodInt)), "omnetpp::cPacket")
{
    propertyNames = nullptr;
}

RepairLinkReqFloodIntDescriptor::~RepairLinkReqFloodIntDescriptor()
{
    delete[] propertyNames;
}

bool RepairLinkReqFloodIntDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<RepairLinkReqFloodInt *>(obj)!=nullptr;
}

const char **RepairLinkReqFloodIntDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *RepairLinkReqFloodIntDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int RepairLinkReqFloodIntDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 4+base->getFieldCount() : 4;
}

unsigned int RepairLinkReqFloodIntDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISCOMPOUND,    // FIELD_dstToPathidsMap
        FD_ISEDITABLE,    // FIELD_ttl
        FD_ISEDITABLE,    // FIELD_floodSeqnum
        FD_ISCOMPOUND,    // FIELD_linkTrace
    };
    return (field >= 0 && field < 4) ? fieldTypeFlags[field] : 0;
}

const char *RepairLinkReqFloodIntDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "dstToPathidsMap",
        "ttl",
        "floodSeqnum",
        "linkTrace",
    };
    return (field >= 0 && field < 4) ? fieldNames[field] : nullptr;
}

int RepairLinkReqFloodIntDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "dstToPathidsMap") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "ttl") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "floodSeqnum") == 0) return baseIndex + 2;
    if (strcmp(fieldName, "linkTrace") == 0) return baseIndex + 3;
    return base ? base->findField(fieldName) : -1;
}

const char *RepairLinkReqFloodIntDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "omnetvlr::VlrIntVidToPathidSetMap",    // FIELD_dstToPathidsMap
        "unsigned int",    // FIELD_ttl
        "unsigned int",    // FIELD_floodSeqnum
        "omnetvlr::VlrIntVidVec",    // FIELD_linkTrace
    };
    return (field >= 0 && field < 4) ? fieldTypeStrings[field] : nullptr;
}

const char **RepairLinkReqFloodIntDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *RepairLinkReqFloodIntDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int RepairLinkReqFloodIntDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    RepairLinkReqFloodInt *pp = omnetpp::fromAnyPtr<RepairLinkReqFloodInt>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void RepairLinkReqFloodIntDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    RepairLinkReqFloodInt *pp = omnetpp::fromAnyPtr<RepairLinkReqFloodInt>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'RepairLinkReqFloodInt'", field);
    }
}

const char *RepairLinkReqFloodIntDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    RepairLinkReqFloodInt *pp = omnetpp::fromAnyPtr<RepairLinkReqFloodInt>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string RepairLinkReqFloodIntDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    RepairLinkReqFloodInt *pp = omnetpp::fromAnyPtr<RepairLinkReqFloodInt>(object); (void)pp;
    switch (field) {
        case FIELD_dstToPathidsMap: return "";
        case FIELD_ttl: return ulong2string(pp->getTtl());
        case FIELD_floodSeqnum: return ulong2string(pp->getFloodSeqnum());
        case FIELD_linkTrace: return "";
        default: return "";
    }
}

void RepairLinkReqFloodIntDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    RepairLinkReqFloodInt *pp = omnetpp::fromAnyPtr<RepairLinkReqFloodInt>(object); (void)pp;
    switch (field) {
        case FIELD_ttl: pp->setTtl(string2ulong(value)); break;
        case FIELD_floodSeqnum: pp->setFloodSeqnum(string2ulong(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'RepairLinkReqFloodInt'", field);
    }
}

omnetpp::cValue RepairLinkReqFloodIntDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    RepairLinkReqFloodInt *pp = omnetpp::fromAnyPtr<RepairLinkReqFloodInt>(object); (void)pp;
    switch (field) {
        case FIELD_dstToPathidsMap: return omnetpp::toAnyPtr(&pp->getDstToPathidsMap()); break;
        case FIELD_ttl: return (omnetpp::intval_t)(pp->getTtl());
        case FIELD_floodSeqnum: return (omnetpp::intval_t)(pp->getFloodSeqnum());
        case FIELD_linkTrace: return omnetpp::toAnyPtr(&pp->getLinkTrace()); break;
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'RepairLinkReqFloodInt' as cValue -- field index out of range?", field);
    }
}

void RepairLinkReqFloodIntDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    RepairLinkReqFloodInt *pp = omnetpp::fromAnyPtr<RepairLinkReqFloodInt>(object); (void)pp;
    switch (field) {
        case FIELD_ttl: pp->setTtl(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        case FIELD_floodSeqnum: pp->setFloodSeqnum(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'RepairLinkReqFloodInt'", field);
    }
}

const char *RepairLinkReqFloodIntDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        case FIELD_dstToPathidsMap: return omnetpp::opp_typename(typeid(VlrIntVidToPathidSetMap));
        case FIELD_linkTrace: return omnetpp::opp_typename(typeid(VlrIntVidVec));
        default: return nullptr;
    };
}

omnetpp::any_ptr RepairLinkReqFloodIntDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    RepairLinkReqFloodInt *pp = omnetpp::fromAnyPtr<RepairLinkReqFloodInt>(object); (void)pp;
    switch (field) {
        case FIELD_dstToPathidsMap: return omnetpp::toAnyPtr(&pp->getDstToPathidsMap()); break;
        case FIELD_linkTrace: return omnetpp::toAnyPtr(&pp->getLinkTrace()); break;
        default: return omnetpp::any_ptr(nullptr);
    }
}

void RepairLinkReqFloodIntDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    RepairLinkReqFloodInt *pp = omnetpp::fromAnyPtr<RepairLinkReqFloodInt>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'RepairLinkReqFloodInt'", field);
    }
}

Register_Class(RepairLinkReplyInt)

RepairLinkReplyInt::RepairLinkReplyInt(const char *name) : ::omnetvlr::VlrIntSetupPacket(name)
{
}

RepairLinkReplyInt::RepairLinkReplyInt(const RepairLinkReplyInt& other) : ::omnetvlr::VlrIntSetupPacket(other)
{
    copy(other);
}

RepairLinkReplyInt::~RepairLinkReplyInt()
{
}

RepairLinkReplyInt& RepairLinkReplyInt::operator=(const RepairLinkReplyInt& other)
{
    if (this == &other) return *this;
    ::omnetvlr::VlrIntSetupPacket::operator=(other);
    copy(other);
    return *this;
}

void RepairLinkReplyInt::copy(const RepairLinkReplyInt& other)
{
    this->src = other.src;
    this->brokenPathids = other.brokenPathids;
    this->tempPathid = other.tempPathid;
    this->linkTrace = other.linkTrace;
}

void RepairLinkReplyInt::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetvlr::VlrIntSetupPacket::parsimPack(b);
    doParsimPacking(b,this->src);
    doParsimPacking(b,this->brokenPathids);
    doParsimPacking(b,this->tempPathid);
    doParsimPacking(b,this->linkTrace);
}

void RepairLinkReplyInt::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetvlr::VlrIntSetupPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->src);
    doParsimUnpacking(b,this->brokenPathids);
    doParsimUnpacking(b,this->tempPathid);
    doParsimUnpacking(b,this->linkTrace);
}

unsigned int RepairLinkReplyInt::getSrc() const
{
    return this->src;
}

void RepairLinkReplyInt::setSrc(unsigned int src)
{
    this->src = src;
}

const VlrIntVidVec& RepairLinkReplyInt::getBrokenPathids() const
{
    return this->brokenPathids;
}

void RepairLinkReplyInt::setBrokenPathids(const VlrIntVidVec& brokenPathids)
{
    this->brokenPathids = brokenPathids;
}

const VlrPathID& RepairLinkReplyInt::getTempPathid() const
{
    return this->tempPathid;
}

void RepairLinkReplyInt::setTempPathid(const VlrPathID& tempPathid)
{
    this->tempPathid = tempPathid;
}

const VlrIntVidVec& RepairLinkReplyInt::getLinkTrace() const
{
    return this->linkTrace;
}

void RepairLinkReplyInt::setLinkTrace(const VlrIntVidVec& linkTrace)
{
    this->linkTrace = linkTrace;
}

class RepairLinkReplyIntDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_src,
        FIELD_brokenPathids,
        FIELD_tempPathid,
        FIELD_linkTrace,
    };
  public:
    RepairLinkReplyIntDescriptor();
    virtual ~RepairLinkReplyIntDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(RepairLinkReplyIntDescriptor)

RepairLinkReplyIntDescriptor::RepairLinkReplyIntDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(omnetvlr::RepairLinkReplyInt)), "omnetvlr::VlrIntSetupPacket")
{
    propertyNames = nullptr;
}

RepairLinkReplyIntDescriptor::~RepairLinkReplyIntDescriptor()
{
    delete[] propertyNames;
}

bool RepairLinkReplyIntDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<RepairLinkReplyInt *>(obj)!=nullptr;
}

const char **RepairLinkReplyIntDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *RepairLinkReplyIntDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int RepairLinkReplyIntDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 4+base->getFieldCount() : 4;
}

unsigned int RepairLinkReplyIntDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_src
        FD_ISCOMPOUND,    // FIELD_brokenPathids
        0,    // FIELD_tempPathid
        FD_ISCOMPOUND,    // FIELD_linkTrace
    };
    return (field >= 0 && field < 4) ? fieldTypeFlags[field] : 0;
}

const char *RepairLinkReplyIntDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "src",
        "brokenPathids",
        "tempPathid",
        "linkTrace",
    };
    return (field >= 0 && field < 4) ? fieldNames[field] : nullptr;
}

int RepairLinkReplyIntDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "src") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "brokenPathids") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "tempPathid") == 0) return baseIndex + 2;
    if (strcmp(fieldName, "linkTrace") == 0) return baseIndex + 3;
    return base ? base->findField(fieldName) : -1;
}

const char *RepairLinkReplyIntDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "unsigned int",    // FIELD_src
        "omnetvlr::VlrIntVidVec",    // FIELD_brokenPathids
        "omnetvlr::VlrPathID",    // FIELD_tempPathid
        "omnetvlr::VlrIntVidVec",    // FIELD_linkTrace
    };
    return (field >= 0 && field < 4) ? fieldTypeStrings[field] : nullptr;
}

const char **RepairLinkReplyIntDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *RepairLinkReplyIntDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int RepairLinkReplyIntDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    RepairLinkReplyInt *pp = omnetpp::fromAnyPtr<RepairLinkReplyInt>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void RepairLinkReplyIntDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    RepairLinkReplyInt *pp = omnetpp::fromAnyPtr<RepairLinkReplyInt>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'RepairLinkReplyInt'", field);
    }
}

const char *RepairLinkReplyIntDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    RepairLinkReplyInt *pp = omnetpp::fromAnyPtr<RepairLinkReplyInt>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string RepairLinkReplyIntDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    RepairLinkReplyInt *pp = omnetpp::fromAnyPtr<RepairLinkReplyInt>(object); (void)pp;
    switch (field) {
        case FIELD_src: return ulong2string(pp->getSrc());
        case FIELD_brokenPathids: return "";
        case FIELD_tempPathid: return ulong2string(pp->getTempPathid());
        case FIELD_linkTrace: return "";
        default: return "";
    }
}

void RepairLinkReplyIntDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    RepairLinkReplyInt *pp = omnetpp::fromAnyPtr<RepairLinkReplyInt>(object); (void)pp;
    switch (field) {
        case FIELD_src: pp->setSrc(string2ulong(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'RepairLinkReplyInt'", field);
    }
}

omnetpp::cValue RepairLinkReplyIntDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    RepairLinkReplyInt *pp = omnetpp::fromAnyPtr<RepairLinkReplyInt>(object); (void)pp;
    switch (field) {
        case FIELD_src: return (omnetpp::intval_t)(pp->getSrc());
        case FIELD_brokenPathids: return omnetpp::toAnyPtr(&pp->getBrokenPathids()); break;
        case FIELD_tempPathid: return omnetpp::toAnyPtr(&pp->getTempPathid()); break;
        case FIELD_linkTrace: return omnetpp::toAnyPtr(&pp->getLinkTrace()); break;
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'RepairLinkReplyInt' as cValue -- field index out of range?", field);
    }
}

void RepairLinkReplyIntDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    RepairLinkReplyInt *pp = omnetpp::fromAnyPtr<RepairLinkReplyInt>(object); (void)pp;
    switch (field) {
        case FIELD_src: pp->setSrc(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'RepairLinkReplyInt'", field);
    }
}

const char *RepairLinkReplyIntDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        case FIELD_brokenPathids: return omnetpp::opp_typename(typeid(VlrIntVidVec));
        case FIELD_linkTrace: return omnetpp::opp_typename(typeid(VlrIntVidVec));
        default: return nullptr;
    };
}

omnetpp::any_ptr RepairLinkReplyIntDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    RepairLinkReplyInt *pp = omnetpp::fromAnyPtr<RepairLinkReplyInt>(object); (void)pp;
    switch (field) {
        case FIELD_brokenPathids: return omnetpp::toAnyPtr(&pp->getBrokenPathids()); break;
        case FIELD_tempPathid: return omnetpp::toAnyPtr(&pp->getTempPathid()); break;
        case FIELD_linkTrace: return omnetpp::toAnyPtr(&pp->getLinkTrace()); break;
        default: return omnetpp::any_ptr(nullptr);
    }
}

void RepairLinkReplyIntDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    RepairLinkReplyInt *pp = omnetpp::fromAnyPtr<RepairLinkReplyInt>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'RepairLinkReplyInt'", field);
    }
}

Register_Class(RepairRouteInt)

RepairRouteInt::RepairRouteInt(const char *name) : ::omnetvlr::VlrIntSetupPacket(name)
{
}

RepairRouteInt::RepairRouteInt(const RepairRouteInt& other) : ::omnetvlr::VlrIntSetupPacket(other)
{
    copy(other);
}

RepairRouteInt::~RepairRouteInt()
{
    delete [] this->pathids;
}

RepairRouteInt& RepairRouteInt::operator=(const RepairRouteInt& other)
{
    if (this == &other) return *this;
    ::omnetvlr::VlrIntSetupPacket::operator=(other);
    copy(other);
    return *this;
}

void RepairRouteInt::copy(const RepairRouteInt& other)
{
    delete [] this->pathids;
    this->pathids = (other.pathids_arraysize==0) ? nullptr : new VlrPathID[other.pathids_arraysize];
    pathids_arraysize = other.pathids_arraysize;
    for (size_t i = 0; i < pathids_arraysize; i++) {
        this->pathids[i] = other.pathids[i];
    }
    this->src = other.src;
}

void RepairRouteInt::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetvlr::VlrIntSetupPacket::parsimPack(b);
    b->pack(pathids_arraysize);
    doParsimArrayPacking(b,this->pathids,pathids_arraysize);
    doParsimPacking(b,this->src);
}

void RepairRouteInt::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetvlr::VlrIntSetupPacket::parsimUnpack(b);
    delete [] this->pathids;
    b->unpack(pathids_arraysize);
    if (pathids_arraysize == 0) {
        this->pathids = nullptr;
    } else {
        this->pathids = new VlrPathID[pathids_arraysize];
        doParsimArrayUnpacking(b,this->pathids,pathids_arraysize);
    }
    doParsimUnpacking(b,this->src);
}

size_t RepairRouteInt::getPathidsArraySize() const
{
    return pathids_arraysize;
}

const VlrPathID& RepairRouteInt::getPathids(size_t k) const
{
    if (k >= pathids_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)pathids_arraysize, (unsigned long)k);
    return this->pathids[k];
}

void RepairRouteInt::setPathidsArraySize(size_t newSize)
{
    VlrPathID *pathids2 = (newSize==0) ? nullptr : new VlrPathID[newSize];
    size_t minSize = pathids_arraysize < newSize ? pathids_arraysize : newSize;
    for (size_t i = 0; i < minSize; i++)
        pathids2[i] = this->pathids[i];
    delete [] this->pathids;
    this->pathids = pathids2;
    pathids_arraysize = newSize;
}

void RepairRouteInt::setPathids(size_t k, const VlrPathID& pathids)
{
    if (k >= pathids_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)pathids_arraysize, (unsigned long)k);
    this->pathids[k] = pathids;
}

void RepairRouteInt::insertPathids(size_t k, const VlrPathID& pathids)
{
    if (k > pathids_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)pathids_arraysize, (unsigned long)k);
    size_t newSize = pathids_arraysize + 1;
    VlrPathID *pathids2 = new VlrPathID[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        pathids2[i] = this->pathids[i];
    pathids2[k] = pathids;
    for (i = k + 1; i < newSize; i++)
        pathids2[i] = this->pathids[i-1];
    delete [] this->pathids;
    this->pathids = pathids2;
    pathids_arraysize = newSize;
}

void RepairRouteInt::appendPathids(const VlrPathID& pathids)
{
    insertPathids(pathids_arraysize, pathids);
}

void RepairRouteInt::erasePathids(size_t k)
{
    if (k >= pathids_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)pathids_arraysize, (unsigned long)k);
    size_t newSize = pathids_arraysize - 1;
    VlrPathID *pathids2 = (newSize == 0) ? nullptr : new VlrPathID[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        pathids2[i] = this->pathids[i];
    for (i = k; i < newSize; i++)
        pathids2[i] = this->pathids[i+1];
    delete [] this->pathids;
    this->pathids = pathids2;
    pathids_arraysize = newSize;
}

unsigned int RepairRouteInt::getSrc() const
{
    return this->src;
}

void RepairRouteInt::setSrc(unsigned int src)
{
    this->src = src;
}

class RepairRouteIntDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_pathids,
        FIELD_src,
    };
  public:
    RepairRouteIntDescriptor();
    virtual ~RepairRouteIntDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(RepairRouteIntDescriptor)

RepairRouteIntDescriptor::RepairRouteIntDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(omnetvlr::RepairRouteInt)), "omnetvlr::VlrIntSetupPacket")
{
    propertyNames = nullptr;
}

RepairRouteIntDescriptor::~RepairRouteIntDescriptor()
{
    delete[] propertyNames;
}

bool RepairRouteIntDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<RepairRouteInt *>(obj)!=nullptr;
}

const char **RepairRouteIntDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *RepairRouteIntDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int RepairRouteIntDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 2+base->getFieldCount() : 2;
}

unsigned int RepairRouteIntDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISARRAY | FD_ISRESIZABLE,    // FIELD_pathids
        FD_ISEDITABLE,    // FIELD_src
    };
    return (field >= 0 && field < 2) ? fieldTypeFlags[field] : 0;
}

const char *RepairRouteIntDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "pathids",
        "src",
    };
    return (field >= 0 && field < 2) ? fieldNames[field] : nullptr;
}

int RepairRouteIntDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "pathids") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "src") == 0) return baseIndex + 1;
    return base ? base->findField(fieldName) : -1;
}

const char *RepairRouteIntDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "omnetvlr::VlrPathID",    // FIELD_pathids
        "unsigned int",    // FIELD_src
    };
    return (field >= 0 && field < 2) ? fieldTypeStrings[field] : nullptr;
}

const char **RepairRouteIntDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *RepairRouteIntDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int RepairRouteIntDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    RepairRouteInt *pp = omnetpp::fromAnyPtr<RepairRouteInt>(object); (void)pp;
    switch (field) {
        case FIELD_pathids: return pp->getPathidsArraySize();
        default: return 0;
    }
}

void RepairRouteIntDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    RepairRouteInt *pp = omnetpp::fromAnyPtr<RepairRouteInt>(object); (void)pp;
    switch (field) {
        case FIELD_pathids: pp->setPathidsArraySize(size); break;
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'RepairRouteInt'", field);
    }
}

const char *RepairRouteIntDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    RepairRouteInt *pp = omnetpp::fromAnyPtr<RepairRouteInt>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string RepairRouteIntDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    RepairRouteInt *pp = omnetpp::fromAnyPtr<RepairRouteInt>(object); (void)pp;
    switch (field) {
        case FIELD_pathids: return ulong2string(pp->getPathids(i));
        case FIELD_src: return ulong2string(pp->getSrc());
        default: return "";
    }
}

void RepairRouteIntDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    RepairRouteInt *pp = omnetpp::fromAnyPtr<RepairRouteInt>(object); (void)pp;
    switch (field) {
        case FIELD_src: pp->setSrc(string2ulong(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'RepairRouteInt'", field);
    }
}

omnetpp::cValue RepairRouteIntDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    RepairRouteInt *pp = omnetpp::fromAnyPtr<RepairRouteInt>(object); (void)pp;
    switch (field) {
        case FIELD_pathids: return omnetpp::toAnyPtr(&pp->getPathids(i)); break;
        case FIELD_src: return (omnetpp::intval_t)(pp->getSrc());
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'RepairRouteInt' as cValue -- field index out of range?", field);
    }
}

void RepairRouteIntDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    RepairRouteInt *pp = omnetpp::fromAnyPtr<RepairRouteInt>(object); (void)pp;
    switch (field) {
        case FIELD_src: pp->setSrc(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'RepairRouteInt'", field);
    }
}

const char *RepairRouteIntDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr RepairRouteIntDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    RepairRouteInt *pp = omnetpp::fromAnyPtr<RepairRouteInt>(object); (void)pp;
    switch (field) {
        case FIELD_pathids: return omnetpp::toAnyPtr(&pp->getPathids(i)); break;
        default: return omnetpp::any_ptr(nullptr);
    }
}

void RepairRouteIntDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    RepairRouteInt *pp = omnetpp::fromAnyPtr<RepairRouteInt>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'RepairRouteInt'", field);
    }
}

Register_Class(NotifyVsetInt)

NotifyVsetInt::NotifyVsetInt(const char *name) : ::omnetvlr::VlrIntSetupPacket(name)
{
}

NotifyVsetInt::NotifyVsetInt(const NotifyVsetInt& other) : ::omnetvlr::VlrIntSetupPacket(other)
{
    copy(other);
}

NotifyVsetInt::~NotifyVsetInt()
{
}

NotifyVsetInt& NotifyVsetInt::operator=(const NotifyVsetInt& other)
{
    if (this == &other) return *this;
    ::omnetvlr::VlrIntSetupPacket::operator=(other);
    copy(other);
    return *this;
}

void NotifyVsetInt::copy(const NotifyVsetInt& other)
{
    this->dst = other.dst;
    this->src = other.src;
    this->toVnei = other.toVnei;
}

void NotifyVsetInt::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetvlr::VlrIntSetupPacket::parsimPack(b);
    doParsimPacking(b,this->dst);
    doParsimPacking(b,this->src);
    doParsimPacking(b,this->toVnei);
}

void NotifyVsetInt::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetvlr::VlrIntSetupPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->dst);
    doParsimUnpacking(b,this->src);
    doParsimUnpacking(b,this->toVnei);
}

unsigned int NotifyVsetInt::getDst() const
{
    return this->dst;
}

void NotifyVsetInt::setDst(unsigned int dst)
{
    this->dst = dst;
}

unsigned int NotifyVsetInt::getSrc() const
{
    return this->src;
}

void NotifyVsetInt::setSrc(unsigned int src)
{
    this->src = src;
}

bool NotifyVsetInt::getToVnei() const
{
    return this->toVnei;
}

void NotifyVsetInt::setToVnei(bool toVnei)
{
    this->toVnei = toVnei;
}

class NotifyVsetIntDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_dst,
        FIELD_src,
        FIELD_toVnei,
    };
  public:
    NotifyVsetIntDescriptor();
    virtual ~NotifyVsetIntDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(NotifyVsetIntDescriptor)

NotifyVsetIntDescriptor::NotifyVsetIntDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(omnetvlr::NotifyVsetInt)), "omnetvlr::VlrIntSetupPacket")
{
    propertyNames = nullptr;
}

NotifyVsetIntDescriptor::~NotifyVsetIntDescriptor()
{
    delete[] propertyNames;
}

bool NotifyVsetIntDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<NotifyVsetInt *>(obj)!=nullptr;
}

const char **NotifyVsetIntDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *NotifyVsetIntDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int NotifyVsetIntDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 3+base->getFieldCount() : 3;
}

unsigned int NotifyVsetIntDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_dst
        FD_ISEDITABLE,    // FIELD_src
        FD_ISEDITABLE,    // FIELD_toVnei
    };
    return (field >= 0 && field < 3) ? fieldTypeFlags[field] : 0;
}

const char *NotifyVsetIntDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "dst",
        "src",
        "toVnei",
    };
    return (field >= 0 && field < 3) ? fieldNames[field] : nullptr;
}

int NotifyVsetIntDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "dst") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "src") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "toVnei") == 0) return baseIndex + 2;
    return base ? base->findField(fieldName) : -1;
}

const char *NotifyVsetIntDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "unsigned int",    // FIELD_dst
        "unsigned int",    // FIELD_src
        "bool",    // FIELD_toVnei
    };
    return (field >= 0 && field < 3) ? fieldTypeStrings[field] : nullptr;
}

const char **NotifyVsetIntDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *NotifyVsetIntDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int NotifyVsetIntDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    NotifyVsetInt *pp = omnetpp::fromAnyPtr<NotifyVsetInt>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void NotifyVsetIntDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    NotifyVsetInt *pp = omnetpp::fromAnyPtr<NotifyVsetInt>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'NotifyVsetInt'", field);
    }
}

const char *NotifyVsetIntDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    NotifyVsetInt *pp = omnetpp::fromAnyPtr<NotifyVsetInt>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string NotifyVsetIntDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    NotifyVsetInt *pp = omnetpp::fromAnyPtr<NotifyVsetInt>(object); (void)pp;
    switch (field) {
        case FIELD_dst: return ulong2string(pp->getDst());
        case FIELD_src: return ulong2string(pp->getSrc());
        case FIELD_toVnei: return bool2string(pp->getToVnei());
        default: return "";
    }
}

void NotifyVsetIntDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    NotifyVsetInt *pp = omnetpp::fromAnyPtr<NotifyVsetInt>(object); (void)pp;
    switch (field) {
        case FIELD_dst: pp->setDst(string2ulong(value)); break;
        case FIELD_src: pp->setSrc(string2ulong(value)); break;
        case FIELD_toVnei: pp->setToVnei(string2bool(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'NotifyVsetInt'", field);
    }
}

omnetpp::cValue NotifyVsetIntDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    NotifyVsetInt *pp = omnetpp::fromAnyPtr<NotifyVsetInt>(object); (void)pp;
    switch (field) {
        case FIELD_dst: return (omnetpp::intval_t)(pp->getDst());
        case FIELD_src: return (omnetpp::intval_t)(pp->getSrc());
        case FIELD_toVnei: return pp->getToVnei();
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'NotifyVsetInt' as cValue -- field index out of range?", field);
    }
}

void NotifyVsetIntDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    NotifyVsetInt *pp = omnetpp::fromAnyPtr<NotifyVsetInt>(object); (void)pp;
    switch (field) {
        case FIELD_dst: pp->setDst(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        case FIELD_src: pp->setSrc(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        case FIELD_toVnei: pp->setToVnei(value.boolValue()); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'NotifyVsetInt'", field);
    }
}

const char *NotifyVsetIntDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr NotifyVsetIntDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    NotifyVsetInt *pp = omnetpp::fromAnyPtr<NotifyVsetInt>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void NotifyVsetIntDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    NotifyVsetInt *pp = omnetpp::fromAnyPtr<NotifyVsetInt>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'NotifyVsetInt'", field);
    }
}

Register_Class(RepairLocalReqFloodInt)

RepairLocalReqFloodInt::RepairLocalReqFloodInt(const char *name, short kind) : ::omnetpp::cPacket(name, kind)
{
}

RepairLocalReqFloodInt::RepairLocalReqFloodInt(const RepairLocalReqFloodInt& other) : ::omnetpp::cPacket(other)
{
    copy(other);
}

RepairLocalReqFloodInt::~RepairLocalReqFloodInt()
{
}

RepairLocalReqFloodInt& RepairLocalReqFloodInt::operator=(const RepairLocalReqFloodInt& other)
{
    if (this == &other) return *this;
    ::omnetpp::cPacket::operator=(other);
    copy(other);
    return *this;
}

void RepairLocalReqFloodInt::copy(const RepairLocalReqFloodInt& other)
{
    this->dstToPathidsMap = other.dstToPathidsMap;
    this->brokenPathids = other.brokenPathids;
    this->ttl = other.ttl;
    this->floodSeqnum = other.floodSeqnum;
    this->linkTrace = other.linkTrace;
}

void RepairLocalReqFloodInt::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cPacket::parsimPack(b);
    doParsimPacking(b,this->dstToPathidsMap);
    doParsimPacking(b,this->brokenPathids);
    doParsimPacking(b,this->ttl);
    doParsimPacking(b,this->floodSeqnum);
    doParsimPacking(b,this->linkTrace);
}

void RepairLocalReqFloodInt::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->dstToPathidsMap);
    doParsimUnpacking(b,this->brokenPathids);
    doParsimUnpacking(b,this->ttl);
    doParsimUnpacking(b,this->floodSeqnum);
    doParsimUnpacking(b,this->linkTrace);
}

const VlrIntVidToPathidSetMap& RepairLocalReqFloodInt::getDstToPathidsMap() const
{
    return this->dstToPathidsMap;
}

void RepairLocalReqFloodInt::setDstToPathidsMap(const VlrIntVidToPathidSetMap& dstToPathidsMap)
{
    this->dstToPathidsMap = dstToPathidsMap;
}

const VlrIntVidSet& RepairLocalReqFloodInt::getBrokenPathids() const
{
    return this->brokenPathids;
}

void RepairLocalReqFloodInt::setBrokenPathids(const VlrIntVidSet& brokenPathids)
{
    this->brokenPathids = brokenPathids;
}

unsigned int RepairLocalReqFloodInt::getTtl() const
{
    return this->ttl;
}

void RepairLocalReqFloodInt::setTtl(unsigned int ttl)
{
    this->ttl = ttl;
}

unsigned int RepairLocalReqFloodInt::getFloodSeqnum() const
{
    return this->floodSeqnum;
}

void RepairLocalReqFloodInt::setFloodSeqnum(unsigned int floodSeqnum)
{
    this->floodSeqnum = floodSeqnum;
}

const VlrIntVidVec& RepairLocalReqFloodInt::getLinkTrace() const
{
    return this->linkTrace;
}

void RepairLocalReqFloodInt::setLinkTrace(const VlrIntVidVec& linkTrace)
{
    this->linkTrace = linkTrace;
}

class RepairLocalReqFloodIntDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_dstToPathidsMap,
        FIELD_brokenPathids,
        FIELD_ttl,
        FIELD_floodSeqnum,
        FIELD_linkTrace,
    };
  public:
    RepairLocalReqFloodIntDescriptor();
    virtual ~RepairLocalReqFloodIntDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(RepairLocalReqFloodIntDescriptor)

RepairLocalReqFloodIntDescriptor::RepairLocalReqFloodIntDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(omnetvlr::RepairLocalReqFloodInt)), "omnetpp::cPacket")
{
    propertyNames = nullptr;
}

RepairLocalReqFloodIntDescriptor::~RepairLocalReqFloodIntDescriptor()
{
    delete[] propertyNames;
}

bool RepairLocalReqFloodIntDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<RepairLocalReqFloodInt *>(obj)!=nullptr;
}

const char **RepairLocalReqFloodIntDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *RepairLocalReqFloodIntDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int RepairLocalReqFloodIntDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 5+base->getFieldCount() : 5;
}

unsigned int RepairLocalReqFloodIntDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISCOMPOUND,    // FIELD_dstToPathidsMap
        FD_ISCOMPOUND,    // FIELD_brokenPathids
        FD_ISEDITABLE,    // FIELD_ttl
        FD_ISEDITABLE,    // FIELD_floodSeqnum
        FD_ISCOMPOUND,    // FIELD_linkTrace
    };
    return (field >= 0 && field < 5) ? fieldTypeFlags[field] : 0;
}

const char *RepairLocalReqFloodIntDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "dstToPathidsMap",
        "brokenPathids",
        "ttl",
        "floodSeqnum",
        "linkTrace",
    };
    return (field >= 0 && field < 5) ? fieldNames[field] : nullptr;
}

int RepairLocalReqFloodIntDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "dstToPathidsMap") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "brokenPathids") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "ttl") == 0) return baseIndex + 2;
    if (strcmp(fieldName, "floodSeqnum") == 0) return baseIndex + 3;
    if (strcmp(fieldName, "linkTrace") == 0) return baseIndex + 4;
    return base ? base->findField(fieldName) : -1;
}

const char *RepairLocalReqFloodIntDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "omnetvlr::VlrIntVidToPathidSetMap",    // FIELD_dstToPathidsMap
        "omnetvlr::VlrIntVidSet",    // FIELD_brokenPathids
        "unsigned int",    // FIELD_ttl
        "unsigned int",    // FIELD_floodSeqnum
        "omnetvlr::VlrIntVidVec",    // FIELD_linkTrace
    };
    return (field >= 0 && field < 5) ? fieldTypeStrings[field] : nullptr;
}

const char **RepairLocalReqFloodIntDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *RepairLocalReqFloodIntDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int RepairLocalReqFloodIntDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    RepairLocalReqFloodInt *pp = omnetpp::fromAnyPtr<RepairLocalReqFloodInt>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void RepairLocalReqFloodIntDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    RepairLocalReqFloodInt *pp = omnetpp::fromAnyPtr<RepairLocalReqFloodInt>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'RepairLocalReqFloodInt'", field);
    }
}

const char *RepairLocalReqFloodIntDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    RepairLocalReqFloodInt *pp = omnetpp::fromAnyPtr<RepairLocalReqFloodInt>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string RepairLocalReqFloodIntDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    RepairLocalReqFloodInt *pp = omnetpp::fromAnyPtr<RepairLocalReqFloodInt>(object); (void)pp;
    switch (field) {
        case FIELD_dstToPathidsMap: return "";
        case FIELD_brokenPathids: return "";
        case FIELD_ttl: return ulong2string(pp->getTtl());
        case FIELD_floodSeqnum: return ulong2string(pp->getFloodSeqnum());
        case FIELD_linkTrace: return "";
        default: return "";
    }
}

void RepairLocalReqFloodIntDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    RepairLocalReqFloodInt *pp = omnetpp::fromAnyPtr<RepairLocalReqFloodInt>(object); (void)pp;
    switch (field) {
        case FIELD_ttl: pp->setTtl(string2ulong(value)); break;
        case FIELD_floodSeqnum: pp->setFloodSeqnum(string2ulong(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'RepairLocalReqFloodInt'", field);
    }
}

omnetpp::cValue RepairLocalReqFloodIntDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    RepairLocalReqFloodInt *pp = omnetpp::fromAnyPtr<RepairLocalReqFloodInt>(object); (void)pp;
    switch (field) {
        case FIELD_dstToPathidsMap: return omnetpp::toAnyPtr(&pp->getDstToPathidsMap()); break;
        case FIELD_brokenPathids: return omnetpp::toAnyPtr(&pp->getBrokenPathids()); break;
        case FIELD_ttl: return (omnetpp::intval_t)(pp->getTtl());
        case FIELD_floodSeqnum: return (omnetpp::intval_t)(pp->getFloodSeqnum());
        case FIELD_linkTrace: return omnetpp::toAnyPtr(&pp->getLinkTrace()); break;
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'RepairLocalReqFloodInt' as cValue -- field index out of range?", field);
    }
}

void RepairLocalReqFloodIntDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    RepairLocalReqFloodInt *pp = omnetpp::fromAnyPtr<RepairLocalReqFloodInt>(object); (void)pp;
    switch (field) {
        case FIELD_ttl: pp->setTtl(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        case FIELD_floodSeqnum: pp->setFloodSeqnum(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'RepairLocalReqFloodInt'", field);
    }
}

const char *RepairLocalReqFloodIntDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        case FIELD_dstToPathidsMap: return omnetpp::opp_typename(typeid(VlrIntVidToPathidSetMap));
        case FIELD_brokenPathids: return omnetpp::opp_typename(typeid(VlrIntVidSet));
        case FIELD_linkTrace: return omnetpp::opp_typename(typeid(VlrIntVidVec));
        default: return nullptr;
    };
}

omnetpp::any_ptr RepairLocalReqFloodIntDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    RepairLocalReqFloodInt *pp = omnetpp::fromAnyPtr<RepairLocalReqFloodInt>(object); (void)pp;
    switch (field) {
        case FIELD_dstToPathidsMap: return omnetpp::toAnyPtr(&pp->getDstToPathidsMap()); break;
        case FIELD_brokenPathids: return omnetpp::toAnyPtr(&pp->getBrokenPathids()); break;
        case FIELD_linkTrace: return omnetpp::toAnyPtr(&pp->getLinkTrace()); break;
        default: return omnetpp::any_ptr(nullptr);
    }
}

void RepairLocalReqFloodIntDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    RepairLocalReqFloodInt *pp = omnetpp::fromAnyPtr<RepairLocalReqFloodInt>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'RepairLocalReqFloodInt'", field);
    }
}

Register_Class(RepairLocalReplyInt)

RepairLocalReplyInt::RepairLocalReplyInt(const char *name) : ::omnetvlr::VlrIntSetupPacket(name)
{
}

RepairLocalReplyInt::RepairLocalReplyInt(const RepairLocalReplyInt& other) : ::omnetvlr::VlrIntSetupPacket(other)
{
    copy(other);
}

RepairLocalReplyInt::~RepairLocalReplyInt()
{
}

RepairLocalReplyInt& RepairLocalReplyInt::operator=(const RepairLocalReplyInt& other)
{
    if (this == &other) return *this;
    ::omnetvlr::VlrIntSetupPacket::operator=(other);
    copy(other);
    return *this;
}

void RepairLocalReplyInt::copy(const RepairLocalReplyInt& other)
{
    this->src = other.src;
    this->pathidToPrevhopMap = other.pathidToPrevhopMap;
    this->linkTrace = other.linkTrace;
    this->prevhopVids = other.prevhopVids;
    this->oldestPrevhopIndex = other.oldestPrevhopIndex;
}

void RepairLocalReplyInt::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetvlr::VlrIntSetupPacket::parsimPack(b);
    doParsimPacking(b,this->src);
    doParsimPacking(b,this->pathidToPrevhopMap);
    doParsimPacking(b,this->linkTrace);
    doParsimPacking(b,this->prevhopVids);
    doParsimPacking(b,this->oldestPrevhopIndex);
}

void RepairLocalReplyInt::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetvlr::VlrIntSetupPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->src);
    doParsimUnpacking(b,this->pathidToPrevhopMap);
    doParsimUnpacking(b,this->linkTrace);
    doParsimUnpacking(b,this->prevhopVids);
    doParsimUnpacking(b,this->oldestPrevhopIndex);
}

unsigned int RepairLocalReplyInt::getSrc() const
{
    return this->src;
}

void RepairLocalReplyInt::setSrc(unsigned int src)
{
    this->src = src;
}

const VlrIntPathidToVidVecMap& RepairLocalReplyInt::getPathidToPrevhopMap() const
{
    return this->pathidToPrevhopMap;
}

void RepairLocalReplyInt::setPathidToPrevhopMap(const VlrIntPathidToVidVecMap& pathidToPrevhopMap)
{
    this->pathidToPrevhopMap = pathidToPrevhopMap;
}

const VlrIntVidVec& RepairLocalReplyInt::getLinkTrace() const
{
    return this->linkTrace;
}

void RepairLocalReplyInt::setLinkTrace(const VlrIntVidVec& linkTrace)
{
    this->linkTrace = linkTrace;
}

const VlrIntVidVec& RepairLocalReplyInt::getPrevhopVids() const
{
    return this->prevhopVids;
}

void RepairLocalReplyInt::setPrevhopVids(const VlrIntVidVec& prevhopVids)
{
    this->prevhopVids = prevhopVids;
}

unsigned int RepairLocalReplyInt::getOldestPrevhopIndex() const
{
    return this->oldestPrevhopIndex;
}

void RepairLocalReplyInt::setOldestPrevhopIndex(unsigned int oldestPrevhopIndex)
{
    this->oldestPrevhopIndex = oldestPrevhopIndex;
}

class RepairLocalReplyIntDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_src,
        FIELD_pathidToPrevhopMap,
        FIELD_linkTrace,
        FIELD_prevhopVids,
        FIELD_oldestPrevhopIndex,
    };
  public:
    RepairLocalReplyIntDescriptor();
    virtual ~RepairLocalReplyIntDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(RepairLocalReplyIntDescriptor)

RepairLocalReplyIntDescriptor::RepairLocalReplyIntDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(omnetvlr::RepairLocalReplyInt)), "omnetvlr::VlrIntSetupPacket")
{
    propertyNames = nullptr;
}

RepairLocalReplyIntDescriptor::~RepairLocalReplyIntDescriptor()
{
    delete[] propertyNames;
}

bool RepairLocalReplyIntDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<RepairLocalReplyInt *>(obj)!=nullptr;
}

const char **RepairLocalReplyIntDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *RepairLocalReplyIntDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int RepairLocalReplyIntDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 5+base->getFieldCount() : 5;
}

unsigned int RepairLocalReplyIntDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_src
        FD_ISCOMPOUND,    // FIELD_pathidToPrevhopMap
        FD_ISCOMPOUND,    // FIELD_linkTrace
        FD_ISCOMPOUND,    // FIELD_prevhopVids
        FD_ISEDITABLE,    // FIELD_oldestPrevhopIndex
    };
    return (field >= 0 && field < 5) ? fieldTypeFlags[field] : 0;
}

const char *RepairLocalReplyIntDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "src",
        "pathidToPrevhopMap",
        "linkTrace",
        "prevhopVids",
        "oldestPrevhopIndex",
    };
    return (field >= 0 && field < 5) ? fieldNames[field] : nullptr;
}

int RepairLocalReplyIntDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "src") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "pathidToPrevhopMap") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "linkTrace") == 0) return baseIndex + 2;
    if (strcmp(fieldName, "prevhopVids") == 0) return baseIndex + 3;
    if (strcmp(fieldName, "oldestPrevhopIndex") == 0) return baseIndex + 4;
    return base ? base->findField(fieldName) : -1;
}

const char *RepairLocalReplyIntDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "unsigned int",    // FIELD_src
        "omnetvlr::VlrIntPathidToVidVecMap",    // FIELD_pathidToPrevhopMap
        "omnetvlr::VlrIntVidVec",    // FIELD_linkTrace
        "omnetvlr::VlrIntVidVec",    // FIELD_prevhopVids
        "unsigned int",    // FIELD_oldestPrevhopIndex
    };
    return (field >= 0 && field < 5) ? fieldTypeStrings[field] : nullptr;
}

const char **RepairLocalReplyIntDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *RepairLocalReplyIntDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int RepairLocalReplyIntDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    RepairLocalReplyInt *pp = omnetpp::fromAnyPtr<RepairLocalReplyInt>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void RepairLocalReplyIntDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    RepairLocalReplyInt *pp = omnetpp::fromAnyPtr<RepairLocalReplyInt>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'RepairLocalReplyInt'", field);
    }
}

const char *RepairLocalReplyIntDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    RepairLocalReplyInt *pp = omnetpp::fromAnyPtr<RepairLocalReplyInt>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string RepairLocalReplyIntDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    RepairLocalReplyInt *pp = omnetpp::fromAnyPtr<RepairLocalReplyInt>(object); (void)pp;
    switch (field) {
        case FIELD_src: return ulong2string(pp->getSrc());
        case FIELD_pathidToPrevhopMap: return "";
        case FIELD_linkTrace: return "";
        case FIELD_prevhopVids: return "";
        case FIELD_oldestPrevhopIndex: return ulong2string(pp->getOldestPrevhopIndex());
        default: return "";
    }
}

void RepairLocalReplyIntDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    RepairLocalReplyInt *pp = omnetpp::fromAnyPtr<RepairLocalReplyInt>(object); (void)pp;
    switch (field) {
        case FIELD_src: pp->setSrc(string2ulong(value)); break;
        case FIELD_oldestPrevhopIndex: pp->setOldestPrevhopIndex(string2ulong(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'RepairLocalReplyInt'", field);
    }
}

omnetpp::cValue RepairLocalReplyIntDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    RepairLocalReplyInt *pp = omnetpp::fromAnyPtr<RepairLocalReplyInt>(object); (void)pp;
    switch (field) {
        case FIELD_src: return (omnetpp::intval_t)(pp->getSrc());
        case FIELD_pathidToPrevhopMap: return omnetpp::toAnyPtr(&pp->getPathidToPrevhopMap()); break;
        case FIELD_linkTrace: return omnetpp::toAnyPtr(&pp->getLinkTrace()); break;
        case FIELD_prevhopVids: return omnetpp::toAnyPtr(&pp->getPrevhopVids()); break;
        case FIELD_oldestPrevhopIndex: return (omnetpp::intval_t)(pp->getOldestPrevhopIndex());
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'RepairLocalReplyInt' as cValue -- field index out of range?", field);
    }
}

void RepairLocalReplyIntDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    RepairLocalReplyInt *pp = omnetpp::fromAnyPtr<RepairLocalReplyInt>(object); (void)pp;
    switch (field) {
        case FIELD_src: pp->setSrc(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        case FIELD_oldestPrevhopIndex: pp->setOldestPrevhopIndex(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'RepairLocalReplyInt'", field);
    }
}

const char *RepairLocalReplyIntDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        case FIELD_pathidToPrevhopMap: return omnetpp::opp_typename(typeid(VlrIntPathidToVidVecMap));
        case FIELD_linkTrace: return omnetpp::opp_typename(typeid(VlrIntVidVec));
        case FIELD_prevhopVids: return omnetpp::opp_typename(typeid(VlrIntVidVec));
        default: return nullptr;
    };
}

omnetpp::any_ptr RepairLocalReplyIntDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    RepairLocalReplyInt *pp = omnetpp::fromAnyPtr<RepairLocalReplyInt>(object); (void)pp;
    switch (field) {
        case FIELD_pathidToPrevhopMap: return omnetpp::toAnyPtr(&pp->getPathidToPrevhopMap()); break;
        case FIELD_linkTrace: return omnetpp::toAnyPtr(&pp->getLinkTrace()); break;
        case FIELD_prevhopVids: return omnetpp::toAnyPtr(&pp->getPrevhopVids()); break;
        default: return omnetpp::any_ptr(nullptr);
    }
}

void RepairLocalReplyIntDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    RepairLocalReplyInt *pp = omnetpp::fromAnyPtr<RepairLocalReplyInt>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'RepairLocalReplyInt'", field);
    }
}

Register_Class(RepairLocalPrevInt)

RepairLocalPrevInt::RepairLocalPrevInt(const char *name) : ::omnetvlr::VlrIntSetupPacket(name)
{
}

RepairLocalPrevInt::RepairLocalPrevInt(const RepairLocalPrevInt& other) : ::omnetvlr::VlrIntSetupPacket(other)
{
    copy(other);
}

RepairLocalPrevInt::~RepairLocalPrevInt()
{
}

RepairLocalPrevInt& RepairLocalPrevInt::operator=(const RepairLocalPrevInt& other)
{
    if (this == &other) return *this;
    ::omnetvlr::VlrIntSetupPacket::operator=(other);
    copy(other);
    return *this;
}

void RepairLocalPrevInt::copy(const RepairLocalPrevInt& other)
{
    this->pathidToPrevhopMap = other.pathidToPrevhopMap;
    this->src = other.src;
}

void RepairLocalPrevInt::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetvlr::VlrIntSetupPacket::parsimPack(b);
    doParsimPacking(b,this->pathidToPrevhopMap);
    doParsimPacking(b,this->src);
}

void RepairLocalPrevInt::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetvlr::VlrIntSetupPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->pathidToPrevhopMap);
    doParsimUnpacking(b,this->src);
}

const VlrIntPathidToVidVecMap& RepairLocalPrevInt::getPathidToPrevhopMap() const
{
    return this->pathidToPrevhopMap;
}

void RepairLocalPrevInt::setPathidToPrevhopMap(const VlrIntPathidToVidVecMap& pathidToPrevhopMap)
{
    this->pathidToPrevhopMap = pathidToPrevhopMap;
}

unsigned int RepairLocalPrevInt::getSrc() const
{
    return this->src;
}

void RepairLocalPrevInt::setSrc(unsigned int src)
{
    this->src = src;
}

class RepairLocalPrevIntDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_pathidToPrevhopMap,
        FIELD_src,
    };
  public:
    RepairLocalPrevIntDescriptor();
    virtual ~RepairLocalPrevIntDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(RepairLocalPrevIntDescriptor)

RepairLocalPrevIntDescriptor::RepairLocalPrevIntDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(omnetvlr::RepairLocalPrevInt)), "omnetvlr::VlrIntSetupPacket")
{
    propertyNames = nullptr;
}

RepairLocalPrevIntDescriptor::~RepairLocalPrevIntDescriptor()
{
    delete[] propertyNames;
}

bool RepairLocalPrevIntDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<RepairLocalPrevInt *>(obj)!=nullptr;
}

const char **RepairLocalPrevIntDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *RepairLocalPrevIntDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int RepairLocalPrevIntDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 2+base->getFieldCount() : 2;
}

unsigned int RepairLocalPrevIntDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISCOMPOUND,    // FIELD_pathidToPrevhopMap
        FD_ISEDITABLE,    // FIELD_src
    };
    return (field >= 0 && field < 2) ? fieldTypeFlags[field] : 0;
}

const char *RepairLocalPrevIntDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "pathidToPrevhopMap",
        "src",
    };
    return (field >= 0 && field < 2) ? fieldNames[field] : nullptr;
}

int RepairLocalPrevIntDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "pathidToPrevhopMap") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "src") == 0) return baseIndex + 1;
    return base ? base->findField(fieldName) : -1;
}

const char *RepairLocalPrevIntDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "omnetvlr::VlrIntPathidToVidVecMap",    // FIELD_pathidToPrevhopMap
        "unsigned int",    // FIELD_src
    };
    return (field >= 0 && field < 2) ? fieldTypeStrings[field] : nullptr;
}

const char **RepairLocalPrevIntDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *RepairLocalPrevIntDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int RepairLocalPrevIntDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    RepairLocalPrevInt *pp = omnetpp::fromAnyPtr<RepairLocalPrevInt>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void RepairLocalPrevIntDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    RepairLocalPrevInt *pp = omnetpp::fromAnyPtr<RepairLocalPrevInt>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'RepairLocalPrevInt'", field);
    }
}

const char *RepairLocalPrevIntDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    RepairLocalPrevInt *pp = omnetpp::fromAnyPtr<RepairLocalPrevInt>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string RepairLocalPrevIntDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    RepairLocalPrevInt *pp = omnetpp::fromAnyPtr<RepairLocalPrevInt>(object); (void)pp;
    switch (field) {
        case FIELD_pathidToPrevhopMap: return "";
        case FIELD_src: return ulong2string(pp->getSrc());
        default: return "";
    }
}

void RepairLocalPrevIntDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    RepairLocalPrevInt *pp = omnetpp::fromAnyPtr<RepairLocalPrevInt>(object); (void)pp;
    switch (field) {
        case FIELD_src: pp->setSrc(string2ulong(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'RepairLocalPrevInt'", field);
    }
}

omnetpp::cValue RepairLocalPrevIntDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    RepairLocalPrevInt *pp = omnetpp::fromAnyPtr<RepairLocalPrevInt>(object); (void)pp;
    switch (field) {
        case FIELD_pathidToPrevhopMap: return omnetpp::toAnyPtr(&pp->getPathidToPrevhopMap()); break;
        case FIELD_src: return (omnetpp::intval_t)(pp->getSrc());
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'RepairLocalPrevInt' as cValue -- field index out of range?", field);
    }
}

void RepairLocalPrevIntDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    RepairLocalPrevInt *pp = omnetpp::fromAnyPtr<RepairLocalPrevInt>(object); (void)pp;
    switch (field) {
        case FIELD_src: pp->setSrc(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'RepairLocalPrevInt'", field);
    }
}

const char *RepairLocalPrevIntDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        case FIELD_pathidToPrevhopMap: return omnetpp::opp_typename(typeid(VlrIntPathidToVidVecMap));
        default: return nullptr;
    };
}

omnetpp::any_ptr RepairLocalPrevIntDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    RepairLocalPrevInt *pp = omnetpp::fromAnyPtr<RepairLocalPrevInt>(object); (void)pp;
    switch (field) {
        case FIELD_pathidToPrevhopMap: return omnetpp::toAnyPtr(&pp->getPathidToPrevhopMap()); break;
        default: return omnetpp::any_ptr(nullptr);
    }
}

void RepairLocalPrevIntDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    RepairLocalPrevInt *pp = omnetpp::fromAnyPtr<RepairLocalPrevInt>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'RepairLocalPrevInt'", field);
    }
}

}  // namespace omnetvlr

namespace omnetpp {

}  // namespace omnetpp

