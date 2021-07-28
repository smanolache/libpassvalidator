#ifndef __conf_macros_hdr__h__
#define __conf_macros_hdr__h__

#define METHOD_DECLARATIONS(name, type) \
	void set_##name(type); \
	void merge_##name(type); \
	type name() const; \
	bool dflt_##name() const; \
	bool merged_##name() const; \
	bool specific_##name() const

#define FIELD_DECLARATIONS(name, type) \
	type name##_; \
	bool specific_##name##_; \
	bool merged_##name##_

#define FIELD_INIT(name, value) \
	name##_(value) \
	, specific_##name##_(false) \
	, merged_##name##_(false)

#define FIELD_CPY(other, name) \
	name##_((other).name()) \
	, specific_##name##_((other).specific_##name()) \
	, merged_##name##_((other).merged_##name())

#define FIELD_CPY_ASSIGN(other, name) \
	name##_ = (other).name();			      \
	specific_##name##_ = (other).specific_##name(); \
	merged_##name##_ = (other).merged_##name()

#define FIELD_MOVE_ASSIGN(other, name) \
	name##_ = std::move((other).name##_);	      \
	specific_##name##_ = std::move((other).specific_##name##_);	\
	merged_##name##_ = std::move((other).merged_##name##_)

#define FIELD_MOVE(other, name) \
	name##_(std::move((other).name##_))		\
	, specific_##name##_((other).specific_##name())		\
	, merged_##name##_((other).merged_##name())

#define SET_DEFINITION_IMPL(name, confclass, type, INLINE)		\
	INLINE void confclass::set_##name(type name##__) { name##_ = name##__; specific_##name##_ = true; }
#define MERGE_DEFINITION_IMPL(name, confclass, type, INLINE)		\
	INLINE void confclass::merge_##name(type name##__) { if (dflt_##name()) { name##_ = name##__; merged_##name##_ = true; } }
#define DFLT_DEFINITION_IMPL(name, confclass, type, INLINE)		\
	INLINE bool confclass::dflt_##name() const { return !specific_##name##_ && !merged_##name##_; }
#define MERGED_DEFINITION_IMPL(name, confclass, type, INLINE)		\
	INLINE bool confclass::merged_##name() const { return merged_##name##_; }
#define SPECIFIC_DEFINITION_IMPL(name, confclass, type, INLINE)		\
	INLINE bool confclass::specific_##name() const { return specific_##name##_; }
#define GET_DEFINITION_IMPL(name, confclass, type, INLINE)	\
	INLINE type confclass::name() const { return name##_; }

#define SET_DEFINITION(name, confclass, type) SET_DEFINITION_IMPL(name, confclass, type, inline)
#define MERGE_DEFINITION(name, confclass, type) MERGE_DEFINITION_IMPL(name, confclass, type, inline)
#define DFLT_DEFINITION(name, confclass, type) DFLT_DEFINITION_IMPL(name, confclass, type, inline)
#define MERGED_DEFINITION(name, confclass, type) MERGED_DEFINITION_IMPL(name, confclass, type, inline)
#define SPECIFIC_DEFINITION(name, confclass, type) SPECIFIC_DEFINITION_IMPL(name, confclass, type, inline)
#define GET_DEFINITION(name, confclass, type) GET_DEFINITION_IMPL(name, confclass, type, inline)

#define SET_DEFINITION_NI(name, confclass, type) SET_DEFINITION_IMPL(name, confclass, type, NOARG)
#define MERGE_DEFINITION_NI(name, confclass, type) MERGE_DEFINITION_IMPL(name, confclass, type, NOARG)
#define DFLT_DEFINITION_NI(name, confclass, type) DFLT_DEFINITION_IMPL(name, confclass, type, NOARG)
#define MERGED_DEFINITION_NI(name, confclass, type) MERGED_DEFINITION_IMPL(name, confclass, type, NOARG)
#define SPECIFIC_DEFINITION_NI(name, confclass, type) SPECIFIC_DEFINITION_IMPL(name, confclass, type, NOARG)
#define GET_DEFINITION_NI(name, confclass, type) GET_DEFINITION_IMPL(name, confclass, type, NOARG)

#define METHOD_DEFINITIONS_IMPL(name, confclass, type, INLINE)		\
	SET_DEFINITION_IMPL(name, confclass, type, INLINE)		\
	MERGE_DEFINITION_IMPL(name, confclass, type, INLINE)		\
	DFLT_DEFINITION_IMPL(name, confclass, type, INLINE)		\
	MERGED_DEFINITION_IMPL(name, confclass, type, INLINE)		\
	SPECIFIC_DEFINITION_IMPL(name, confclass, type, INLINE)		\
	GET_DEFINITION_IMPL(name, confclass, type, INLINE)

#define NOARG
#define METHOD_DEFINITIONS(name, confclass, type) METHOD_DEFINITIONS_IMPL(name, confclass, type, inline)
#define METHOD_DEFINITIONS_NI(name, confclass, type) METHOD_DEFINITIONS_IMPL(name, confclass, type, NOARG)

#define METHOD_DELEGATION_IMPL(name, fld, confclass, subconf, type, INLINE) \
	INLINE void confclass::set_##name(type name##__) { (subconf).set_##fld(name##__); } \
	INLINE void confclass::merge_##name(type name##__) { (subconf).merge_##fld(name##__); } \
	INLINE bool confclass::dflt_##name() const { return (subconf).dflt_##fld(); } \
	INLINE bool confclass::merged_##name() const { return (subconf).merged_##fld(); } \
	INLINE bool confclass::specific_##name() const { return (subconf).specific_##fld(); } \
	INLINE type confclass::name() const { return (subconf).fld(); }
#define METHOD_DELEGATION(name, fld, confclass, subconf, type) METHOD_DELEGATION_IMPL(name, fld, confclass, subconf, type, inline)
#define METHOD_DELEGATION_NI(name, fld, confclass, subconf, type) METHOD_DELEGATION_IMPL(name, fld, confclass, subconf, type, NOARG)

#define MERGE_VAR(a, b) if (dflt_##a() && !(b)->dflt_##a()) merge_##a((b)->a())

#endif
