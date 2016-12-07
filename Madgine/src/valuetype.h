#pragma once

namespace Engine {


class MADGINE_EXPORT ValueType {
public:
    ValueType();
    ValueType(const ValueType &other);
    ValueType(ValueType &&other);

	ValueType(const Ogre::Vector2 &v);
    ValueType(const Ogre::Vector3 &v);
    ValueType(Scripting::Scope *e);
    ValueType(const std::string &s);
    ValueType(const char *s);
    ValueType(bool b);
    ValueType(int i);
    ValueType(float f);
	ValueType(InvPtr s);
	ValueType(size_t s);

    ~ValueType();

    void clear();

    void operator=(const ValueType &);
    void operator=(Scripting::Scope *);
    void operator=(const std::string &);
    void operator=(bool);
    void operator=(int);
    void operator=(float);
    void operator=(const char *);
	void operator=(InvPtr);
	void operator=(size_t);

    bool operator==(const ValueType &) const;
	bool operator!=(const ValueType &) const;
    bool operator <(const ValueType &) const;
    bool operator >(const ValueType &) const;

    void operator+=(const ValueType &);
	ValueType operator+(const ValueType &) const;

	void operator-=(const ValueType &);
	ValueType operator-(const ValueType &) const;

	void operator/=(const ValueType &);
	ValueType operator/(const ValueType &) const;

	void operator*=(const ValueType &);
	ValueType operator*(const ValueType &) const;

	const Ogre::Vector2 &asVector2() const;
	const Ogre::Vector2 &asVector2(const Ogre::Vector2 &v);
    const Ogre::Vector3 &asVector3() const;
    const Ogre::Vector3 &asVector3(const Ogre::Vector3 &v);
    bool isScope() const;
	Scripting::Scope *asScope() const;
	Scripting::Scope *asScope(const Scripting::Scope *s);
    bool isInvScope() const;
    InvPtr asInvScope() const;
	InvPtr asInvScope(InvPtr s);
    bool isString() const;
    const std::string &asString() const;
    const std::string &asString(const std::string &s);
    bool isBool() const;
    bool asBool() const;
    bool asBool(bool b);
	bool isUInt() const;
	size_t asUInt() const;
	size_t asUInt(size_t s);
    bool isInt() const;
    int asInt() const;
    int asInt(int i);
    bool isFloat() const;
    float asFloat() const;
    float asFloat(float f);
    bool isNull() const;
	const ValueType &asDefault(const ValueType &default);

    std::string toString() const;

	std::string getTypeString() const;

	void readState(Serialize::SerializeInStream &in);
	void writeState(Serialize::SerializeOutStream &out) const;
	void writeCreationData(Serialize::SerializeOutStream &out) const;

	bool peek(Serialize::SerializeInStream &in);

public:
	template <class T>
	struct isValueType {
		constexpr static bool value = false;
	};

	template <>
	struct isValueType<int> {
		constexpr static bool value = true;
	};

	template <>
	struct isValueType<std::string> {
		constexpr static bool value = true;
	};

	template <>
	struct isValueType<Ogre::Vector3> {
		constexpr static bool value = true;
	};

	template <>
	struct isValueType<Ogre::Vector2> {
		constexpr static bool value = true;
	};

	template <>
	struct isValueType<float> {
		constexpr static bool value = true;
	};

	template <>
	struct isValueType<bool> {
		constexpr static bool value = true;
	};

	template <>
	struct isValueType<size_t> {
		constexpr static bool value = true;
	};

	template <>
	struct isValueType<InvPtr> {
		constexpr static bool value = true;
	};

	template <>
	struct isValueType<Scripting::Scope *> {
		constexpr static bool value = true;
	};

	template <>
	struct isValueType<ValueType> {
		constexpr static bool value = true;
	};


	template <class T>
	using enableValueType = std::enable_if_t<isValueType<std::remove_const_t<std::remove_reference_t<T>>>::value, T>;

    template <class T>
    std::enable_if_t<!std::is_class<T>::value, enableValueType<T>> as() const;

    template <class T>
    std::enable_if_t<std::is_class<T>::value, enableValueType<const T&>> as() const;

    template <class T>
	std::enable_if_t<!std::is_class<T>::value, enableValueType<T>> asDefault(T defaultValue);

    template <class T>
	std::enable_if_t<std::is_class<T>::value, enableValueType<const T &>> asDefault(const T &defaultValue);

private:
	enum class Type : unsigned char {
		BoolValue,
		ScopeValue,
		InvPtrValue,
		IntValue,
		UIntValue,
        FloatValue,
        StringValue,
        Vector3Value,
		Vector2Value,
        NullValue
    } mType;

    union {
        bool mBool;
		Scripting::Scope *mScope;
        InvPtr mInvPtr;
        int mInt;
		size_t mUInt;
        float mFloat;
        std::string *mString;
        Ogre::Vector3 *mVector3;
		Ogre::Vector2 *mVector2;
    } mUnion;

    bool setType(Type t);

    void decRef();
    void incRef();

};



}

std::ostream &operator <<(std::ostream &stream,
                          const Engine::ValueType &v);

