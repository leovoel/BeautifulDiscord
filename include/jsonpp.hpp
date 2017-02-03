// The MIT License (MIT)

// Copyright (c) 2014-2016 Danny Y.

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// This file was generated with a script.
// Generated 2017-01-28 12:46:21.475000 UTC
// This header was generated with jsonpp v0.9.0 (revision 4a7ef3d)
// https://github.com/Rapptz/jsonpp

#ifndef JSONPP_SINGLE_INCLUDE_HPP
#define JSONPP_SINGLE_INCLUDE_HPP

#if !defined(JSONPP_NOEXCEPT)
#if defined(_MSC_VER) && _MSC_VER <= 1800
#define JSONPP_NOEXCEPT throw()
#else
#define JSONPP_NOEXCEPT noexcept
#endif
#endif

#if defined(_MSC_VER)
#include <ciso646>
#endif

#if !defined(JSONPP_ASSERT)
#include <cassert>
#define JSONPP_ASSERT(condition, message) assert((condition) && (message))
#endif // JSONPP_ASSERT

#include <cstddef>
#include <string>
#include <exception>

namespace json {
struct error: std::exception {
    const char* what() const JSONPP_NOEXCEPT override {
        return "json::error";
    }
};

class parser_error : public error {
private:
    std::string error;
public:
    parser_error(const std::string& str, std::size_t line, std::size_t column):
        error("stdin:" + std::to_string(line) + ':' + std::to_string(column) + ": error: " + str) {}

    const char* what() const JSONPP_NOEXCEPT override {
        return error.c_str();
    }
};

class from_json_error : public error {
public:
    std::string message;

    explicit from_json_error(std::string message):
        message(std::move(message)) {}

    const char* what() const JSONPP_NOEXCEPT override {
        return message.c_str();
    }
};

class canonical_from_json_error : public from_json_error {
public:
    explicit canonical_from_json_error(std::string message):
        from_json_error(std::move(message)) {}
};
} // json

#ifndef JSON_TYPE_TRAITS_HPP
#define JSON_TYPE_TRAITS_HPP

#include <type_traits>
#include <iterator>

namespace json {
template<typename T>
struct identity {
    using type = T;
};

template<typename T>
using Identity = typename identity<T>::type;

template<typename T>
using Unqualified = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

template<bool B>
using Bool = std::integral_constant<bool, B>;

template<typename T>
using Not = Bool<!T::value>;

template<typename Condition, typename Then, typename Else>
using If = typename std::conditional<Condition::value, Then, Else>::type;

template<typename... Args>
struct And : Bool<true> {};

template<typename T, typename... Args>
struct And<T, Args...> : If<T, And<Args...>, Bool<false>> {};

template<typename... Args>
struct Or : Bool<false> {};

template<typename T, typename... Args>
struct Or<T, Args...> : If<T, Bool<true>, Or<Args...>> {};

template<typename... Args>
using EnableIf = typename std::enable_if<And<Args...>::value, int>::type;

template<typename... Args>
using DisableIf = typename std::enable_if<Not<And<Args...>>::value, int>::type;

template<typename T, typename... Dummies>
struct depend_on {
    using type = T;
};

template<typename T, typename... Dummies>
using DependOn = typename depend_on<T, Dummies...>::type;

template<typename...>
struct dependent_false : std::false_type {};

template<typename T>
struct is_bool : std::is_same<T, bool> {};

template<typename T>
struct is_number : And<std::is_arithmetic<T>, Not<is_bool<T>>> {};

using null = decltype(nullptr);
using boolean = bool;
using string = std::string;
using number = double;

template<typename T>
struct is_null : std::is_same<T, null> {};

class value;

template<typename T>
struct is_value : std::is_same<T, value> {};

template<typename T, typename U = typename std::decay<T>::type>
struct is_string : Or<std::is_same<U, std::string>, std::is_same<U, const char*>, std::is_same<U, char*>> {};

struct has_to_json_impl {
    template<typename T, typename U = decltype(to_json(std::declval<T&>()))>
    static is_value<U> test(int);

    template<typename...>
    static std::false_type test(...);
};

struct has_to_json_key_impl {
    template<typename T, typename X = decltype(to_json_key(std::declval<T&>()))>
    static is_string<X> test(int);
    template<typename...>
    static std::false_type test(...);
};

template<typename T>
struct has_to_json_key : decltype(has_to_json_key_impl::test<T>(0)) {};

template<typename Type>
struct canonical_schema {
    using unspecialized = Type;
};

namespace detail {
using std::end;
using std::begin;

struct has_iterators_impl {
    template<typename T, typename B = decltype(begin(std::declval<T&>())),
                         typename E = decltype(end(std::declval<T&>()))>
    static std::true_type test(int);
    template<typename...>
    static std::false_type test(...);
};

struct is_array_impl {
    template<typename T, typename U = Unqualified<T>,
                         typename V = typename U::value_type,
                         typename S = decltype(std::declval<U&>().shrink_to_fit()),
                         typename R = decltype(std::declval<U&>().reserve(0))>
    static std::true_type test(int);
    template<typename...>
    static std::false_type test(...);
};

struct is_object_impl {
    template<typename T, typename U = Unqualified<T>,
                         typename K = typename U::key_type,
                         typename V = typename U::mapped_type,
                         typename C = typename U::key_compare,
                         typename P = typename U::value_type,
                         typename F = decltype(std::declval<P&>().first),
                         typename S = decltype(std::declval<P&>().second)>
    static std::true_type test(int);
    template<typename...>
    static std::false_type test(...);
};
} // detail

template<typename T>
struct has_iterators : decltype(detail::has_iterators_impl::test<T>(0)) {};

template<typename T>
struct is_array_like : And<Or<decltype(detail::is_array_impl::test<T>(0)), std::is_array<T>>, has_iterators<T>, Not<is_string<T>>> {};

template<typename T>
struct is_object_like : And<decltype(detail::is_object_impl::test<T>(0)), has_iterators<T>> {};

template<typename T>
struct is_object : std::is_same<T, typename DependOn<value, T>::object> {};

template<typename T>
struct is_array : std::is_same<T, typename DependOn<value, T>::array> {};

template<typename T>
struct is_json : Or<is_null<T>, is_bool<T>, is_number<T>, is_string<T>, is_array<T>, is_object<T>> {};

template<typename T>
struct has_to_json : decltype(has_to_json_impl::test<T>(0)) {};

enum class type {
    null, string, boolean, number, array, object
};

template<unsigned Flags, unsigned Flag>
struct has_extension : Bool<(Flags & Flag) == Flag> {};

template<typename T, typename U = Unqualified<T>>
struct is_regular_serialisable : Or<has_to_json<U>, is_json<U>, is_value<U>> {};

struct has_canonical_schema_impl {
    template<typename T, typename X = typename canonical_schema<T>::unspecialized>
    static std::false_type test(int);
    template<typename...>
    static std::true_type test(...);
};

template<typename T>
struct has_canonical_schema : decltype(has_canonical_schema_impl::test<T>(0)) {};

template<typename Type>
struct json_schema {
    using unspecialized = Type;
};

struct has_json_schema_impl {
    template<typename T, typename X = typename json_schema<T>::unspecialized>
    static std::false_type test(int);
    template<typename...>
    static std::true_type test(...);
};

template<typename T>
struct has_json_schema : decltype(has_json_schema_impl::test<T>(0)) {};

template<typename T>
struct is_serialisable : Or<is_regular_serialisable<T>, has_canonical_schema<T>, has_json_schema<T>> {};

template<typename T>
struct is_deserialisable : Or<is_json<T>, Not<std::is_array<T>>,
                              has_canonical_schema<T>, is_object_like<T>,
                              is_array_like<T>, has_json_schema<T>> {};

template<typename T>
struct lazy_value_type {
    using type = typename T::value_type;
};

template<typename T>
struct lazy_array_type {};

template<typename T>
struct lazy_array_type<T[]> {
    static_assert(dependent_false<T>::value, "The array type must have extent.");
};

template<typename T, unsigned N>
struct lazy_array_type<T[N]> {
    using type = T;
};

template<typename T>
using array_value_type = If<std::is_array<T>, lazy_array_type<T>, lazy_value_type<T>>;
} // json

#endif // JSON_TYPE_TRAITS_HPP
#if defined(_WIN32)
#if !defined(JSONPP_NO_WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN 1
#endif // WIN32_LEAN_AND_MEAN
#if !defined(JSONPP_NO_NOMINMAX)
#define NOMINMAX 1
#endif // NOMINMAX
#include <windows.h>
#endif
#include <stdexcept>
#include <system_error>
#include <cerrno>

namespace json {
namespace detail {
#if defined(_WIN32)
inline std::u16string utf8_to_utf16(const std::string& utf8) {
    static_assert(sizeof(wchar_t) == sizeof(char16_t), "wchar_t must be 16 bits");
    std::wstring temp;
    auto required_size = ::MultiByteToWideChar(CP_UTF8, 0, &utf8[0], static_cast<int>(utf8.size()), nullptr, 0);
    temp.resize(required_size);
    if(::MultiByteToWideChar(CP_UTF8, 0, &utf8[0], static_cast<int>(utf8.size()), &temp[0], required_size) == 0) {
        throw std::runtime_error("unable to convert from UTF-8 to UTF-16");
    }
    return { temp.begin(), temp.end() };
}
#else
inline std::u16string utf8_to_utf16(const std::string& utf8) {
    std::u16string result;
    using size_type = decltype(utf8.size());
    std::invalid_argument invalid_utf8("Invalid UTF-8 string given");
    size_type i = 0;
    while(i < utf8.size()) {
        char32_t codepoint;
        size_type iterations = 0;
        unsigned char byte = utf8[i++];
        if(byte <= 0x7F) {
            codepoint = byte;
        }
        else if(byte <= 0xBF) {
            throw invalid_utf8;
        }
        else if(byte <= 0xDF) {
            codepoint = byte & 0x1F;
            iterations = 1;
        }
        else if(byte <= 0xEF) {
            codepoint = byte & 0x0F;
            iterations = 2;
        }
        else if(byte <= 0xF7) {
            codepoint = byte & 0x07;
            iterations = 3;
        }
        else {
            throw invalid_utf8;
        }

        for(size_type j = 0; j < iterations; ++j) {
            if(i == utf8.size()) {
                throw invalid_utf8;
            }
            unsigned char next_byte = utf8[i++];
            if(next_byte < 0x80 || next_byte > 0xBF) {
                throw invalid_utf8;
            }

            codepoint = (codepoint << 6) + (next_byte & 0x3F);
        }
        if(codepoint > 0x10FFFF || (codepoint >= 0xD800 && codepoint <= 0xDFFF)) {
            throw invalid_utf8;
        }

        if(codepoint <= 0xFFFF) {
            result.push_back(codepoint);
        }
        else {
            codepoint -= 0x10000;
            result.push_back((codepoint >> 10) + 0xD800);
            result.push_back((codepoint & 0x3FF) + 0xDC00);
        }
    }

    return result;
}
#endif
} // detail
} // json

namespace json {
template<unsigned I>
struct choice : choice<I + 1> {};

template<>
struct choice<8> {};

struct select_overload : choice<0> {};

struct otherwise {
    otherwise(...) {}
};
} // json

namespace json {
namespace detail {
struct to_json_key_impl {
private:
    template<typename T, typename U = Unqualified<T>, EnableIf<has_to_json_key<U>> = 0>
    auto impl(choice<0>, T&& value) const -> decltype(to_json_key(std::forward<T>(value))) {
        return to_json_key(std::forward<T>(value));
    }

    template<typename T, typename U = Unqualified<T>, EnableIf<is_string<U>> = 0>
    auto impl(choice<1>, T&& value) const -> decltype(std::forward<T>(value)) {
        return std::forward<T>(value);
    }

    template<typename T, typename U = Unqualified<T>, EnableIf<is_bool<U>> = 0>
    std::string impl(choice<2>, T&& value) const {
        return std::forward<T>(value) ? "true" : "false";
    }

    template<typename T, typename U = Unqualified<T>, EnableIf<std::is_same<U, char>> = 0>
    std::string impl(choice<3>, T&& value) const {
        return std::string(1, std::forward<T>(value));
    }

    template<typename T, typename U = Unqualified<T>, EnableIf<std::is_arithmetic<U>> = 0>
    auto impl(choice<4>, T&& value) const -> decltype(std::to_string(std::forward<T>(value))) {
        return std::to_string(std::forward<T>(value));
    }
public:
    template<typename T>
    auto operator()(T&& value) const -> decltype(impl(select_overload{}, std::forward<T>(value))) {
        return impl(select_overload{}, std::forward<T>(value));
    }
};
} // detail

constexpr detail::to_json_key_impl to_json_key{};
} // json

#include <sstream>
#include <iosfwd>
#include <cmath>

namespace json {
struct format_options {
    enum : int {
        none = 0,
        allow_nan_inf = 1 << 0,
        minify = 1 << 1,
        escape_multi_byte = 1 << 2,
        scientific = 1 << 3,
        fixed = 1 << 4,
        defaultfloat = scientific | fixed
    };

    format_options() JSONPP_NOEXCEPT {};
    format_options(int indent, int flags = none, int precision = 6) JSONPP_NOEXCEPT: flags(flags), indent(indent), precision(precision) {}

    int flags = none;
    int indent = 4;
    int precision = 6;
    int depth = 0;
};

namespace detail {
template<typename OStream>
inline void indent(OStream& out, const format_options& opt) {
    out << '\n';
    for(int i = 0; i < (opt.indent * opt.depth); ++i) {
        out << ' ';
    }
}
} // detail

template<typename OStream, typename T, EnableIf<is_null<T>> = 0>
inline OStream& dump(OStream& out, const T&, format_options = {}) {
    out << "null";
    return out;
}

template<typename OStream, typename T, EnableIf<is_bool<T>> = 0>
inline OStream& dump(OStream& out, const T& t, format_options = {}) {
    out << (t ? "true" : "false");
    return out;
}

template<typename OStream, typename T, EnableIf<is_number<T>> = 0>
inline OStream& dump(OStream& out, const T& t, format_options opt = {}) {
    if((opt.flags & opt.allow_nan_inf) != opt.allow_nan_inf && (std::isnan(t) || std::isinf(t))) {
        // stream null instead if nan is found
        out << "null";
        return out;
    }
    auto precision = out.precision();
    auto flags = out.flags();

    if((opt.flags & opt.defaultfloat) == opt.defaultfloat) {
        out.unsetf(out.floatfield);
    }
    else if((opt.flags & opt.scientific) == opt.scientific) {
        out.setf(out.scientific, out.floatfield);
    }
    else if((opt.flags & opt.fixed) == opt.fixed) {
        out.setf(out.fixed, out.floatfield);
    }
    else {
        out.unsetf(out.floatfield);
    }

    out.precision(opt.precision);
    out << t;
    out.precision(precision);
    out.flags(flags);
    return out;
}

namespace detail {
template<typename OStream>
inline bool escape_control(OStream& out, char control) {
    switch(control) {
    case '"':
        out << "\\\"";
        return true;
    case '\\':
        out << "\\\\";
        return true;
    case '/':
        out << "\\/";
        return true;
    case '\b':
        out << "\\b";
        return true;
    case '\f':
        out << "\\f";
        return true;
    case '\n':
        out << "\\n";
        return true;
    case '\r':
        out << "\\r";
        return true;
    case '\t':
        out << "\\t";
        return true;
    default:
        return false;
    }
}

template<typename OStream>
inline OStream& escape_str(OStream& out, const std::u16string& utf16) {
    auto fill = out.fill();
    auto width = out.width();
    auto flags = out.flags();
    out << '"';
    for(auto&& c : utf16) {
        if(c <= 0x7F) {
            if(escape_control(out, static_cast<char>(c))) {
                continue;
            }
            out << static_cast<char>(c);
        }
        else {
            // prepare stream
            out << "\\u";
            out.width(4);
            out.fill('0');
            out.setf(out.hex, out.basefield);
            out << c;
            out.width(width);
            out.fill(fill);
            out.flags(flags);
        }
    }
    out << '"';
    return out;
}
} // detail

template<typename OStream, typename T, EnableIf<is_string<T>> = 0>
inline OStream& dump(OStream& out, const T& t, format_options opt = {}) {
    bool escape = (opt.flags & opt.escape_multi_byte) == opt.escape_multi_byte;
    if(escape) {
        return detail::escape_str(out, detail::utf8_to_utf16(t));
    }
    out << '"';
    for(auto&& c : t) {
        if(detail::escape_control(out, c)) {
            continue;
        }
        else {
            out << c;
        }
    }
    out << '"';
    return out;
}

template<typename OStream, typename T, EnableIf<is_array_like<T>> = 0>
inline OStream& dump(OStream& out, const T& t, format_options opt = {}) {
    using value_type = typename array_value_type<T>::type;
    static_assert(is_regular_serialisable<value_type>::value, "The array's value type must be JSON serialisable.");

    bool prettify = (opt.flags & opt.minify) != opt.minify;
    opt.depth += prettify;
    out << '[';

    using std::begin;
    using std::end;
    auto&& first = begin(t);
    auto&& last  = end(t);
    bool first_pass = true;
    for(; first != last; ++first) {
        if(not first_pass) {
            out << ',';
        }

        if(prettify) {
            detail::indent(out, opt);
        }

        dump(out, *first, opt);
        first_pass = false;
    }

    if(prettify) {
        --opt.depth;
        if(not first_pass) {
            detail::indent(out, opt);
        }
    }

    out << ']';
    return out;
}

template<typename OStream, typename T, EnableIf<is_object_like<T>> = 0>
inline OStream& dump(OStream& out, const T& t, format_options opt = {}) {
    using value_type = typename T::mapped_type;
    static_assert(is_regular_serialisable<value_type>::value, "The map's mapped type must be JSON serialisable.");
    bool prettify = (opt.flags & format_options::minify) != format_options::minify;
    opt.depth += prettify;
    out << '{';

    using std::begin;
    using std::endl;

    auto&& first = begin(t);
    auto&& last  = end(t);
    bool first_pass = true;

    for(; first != last; ++first) {

        auto&& elem = *first;
        if(not first_pass) {
            out << ',';
        }

        if(prettify) {
            detail::indent(out, opt);
        }

        dump(out, ::json::to_json_key(elem.first), opt);
        out << ':';

        if(prettify) {
            out << ' ';
        }

        dump(out, elem.second, opt);
        first_pass = false;
    }

    if(prettify) {
        --opt.depth;

        if(not first_pass) {
            detail::indent(out, opt);
        }
    }

    out << '}';
    return out;
}

template<typename OStream, typename T, EnableIf<has_to_json<T>, Not<is_json<T>>> = 0>
inline OStream& dump(OStream& out, const T& t, format_options opt = {}) {
    return dump(out, to_json(t), opt);
}

template<typename T>
inline std::string dump_string(const T& value, format_options opt = {}) {
    std::ostringstream ss;
    dump(ss, value, opt);
    return ss.str();
}
} // json

#include <map>
#include <vector>
#include <cstdint>
#include <memory>

namespace json {
class value {
public:
    using object = std::map<std::string, value>;
    using array  = std::vector<value>;
private:
    template<typename T> struct type_tag {};

    template<typename T>
    using is_valid_getter = Or<std::is_same<T, double>, std::is_same<T, std::string>,
                               std::is_same<T, array>, std::is_same<T, object>,
                               std::is_same<T, bool>>;

    union storage_t {
        double number;
        bool boolean;
        std::string* str;
        array* arr;
        object* obj;
    } storage;
    type storage_type;

    void copy(const value& other) {
        switch(other.storage_type) {
        case type::array:
            storage.arr = new array(*(other.storage.arr));
            break;
        case type::string:
            storage.str = new std::string(*(other.storage.str));
            break;
        case type::object:
            storage.obj = new object(*(other.storage.obj));
            break;
        case type::number:
            storage.number = other.storage.number;
            break;
        case type::boolean:
            storage.boolean = other.storage.boolean;
            break;
        default:
            break;
        }
        storage_type = other.storage_type;
    }

    const double& getter(type_tag<double>) const {
        return storage.number;
    }

    double& getter(type_tag<double>) {
        return storage.number;
    }

    const std::string& getter(type_tag<std::string>) const {
        return *(storage.str);
    }

    std::string& getter(type_tag<std::string>) {
        return *(storage.str);
    }

    const bool& getter(type_tag<bool>) const {
        return storage.boolean;
    }

    bool& getter(type_tag<bool>) {
        return storage.boolean;
    }

    const array& getter(type_tag<array>) const {
        return *(storage.arr);
    }

    array& getter(type_tag<array>) {
        return *(storage.arr);
    }

    const object& getter(type_tag<object>) const {
        return *(storage.obj);
    }

    object& getter(type_tag<object>) {
        return *(storage.obj);
    }
public:
    value() JSONPP_NOEXCEPT: storage_type(type::null) {}

    // to make sure that 0 and pointer types don't become valid candidates
    // for the conversion of a json value, we template it.
    template<typename T, EnableIf<is_null<T>> = 0>
    value(T) JSONPP_NOEXCEPT: storage_type(type::null) {}

    ~value() {
        clear();
    }

    value(double v) JSONPP_NOEXCEPT: storage_type(type::number) {
        storage.number = v;
    }

    template<typename T, EnableIf<is_bool<T>, Not<is_string<T>>> = 0>
    value(const T& b) JSONPP_NOEXCEPT: storage_type(type::boolean) {
        storage.boolean = b;
    }

    template<typename T, typename U = Unqualified<T>, EnableIf<is_string<U>, Not<is_bool<U>>> = 0>
    value(T&& value): storage_type(type::string) {
        storage.str = new std::string(std::forward<T>(value));
    }

    value(const char* str, std::size_t sz): storage_type(type::string) {
        storage.str = new std::string(str, sz);
    }

    value(const char* str, const char* strend): storage_type(type::string) {
        storage.str = new std::string(str, strend);
    }

    template<typename T, EnableIf<has_to_json<T>, Not<is_string<T>>, Not<is_bool<T>>> = 0>
    value(const T& t): value(to_json(t)) {}

    value(array arr): storage_type(type::array) {
        storage.arr = new array(std::move(arr));
    }

    value(object obj): storage_type(type::object) {
        storage.obj = new object(std::move(obj));
    }

    value(std::initializer_list<array::value_type> l): storage_type(type::array) {
        storage.arr = new array(l.begin(), l.end());
    }

    value(const value& other) {
        copy(other);
    }

    value(value&& other) JSONPP_NOEXCEPT {
        switch(other.storage_type) {
        case type::array:
            storage.arr = other.storage.arr;
            other.storage.arr = nullptr;
            break;
        case type::string:
            storage.str = other.storage.str;
            other.storage.str = nullptr;
            break;
        case type::object:
            storage.obj = other.storage.obj;
            other.storage.obj = nullptr;
            break;
        case type::boolean:
            storage.boolean = other.storage.boolean;
            break;
        case type::number:
            storage.number = other.storage.number;
            break;
        default:
            break;
        }

        storage_type = other.storage_type;
        other.storage_type = type::null;
    }

    template<typename T, EnableIf<has_to_json<T>, Not<is_string<T>>, Not<is_bool<T>>> = 0>
    value& operator=(const T& t) {
        *this = to_json(t);
        return *this;
    }

    value& operator=(const value& other) {
        clear();
        copy(other);
        return *this;
    }

    value& operator=(value&& other) JSONPP_NOEXCEPT {
        clear();
        switch(other.storage_type) {
        case type::array:
            storage.arr = other.storage.arr;
            other.storage.arr = nullptr;
            break;
        case type::string:
            storage.str = other.storage.str;
            other.storage.str = nullptr;
            break;
        case type::object:
            storage.obj = other.storage.obj;
            other.storage.obj = nullptr;
            break;
        case type::boolean:
            storage.boolean = other.storage.boolean;
            break;
        case type::number:
            storage.number = other.storage.number;
            break;
        default:
            break;
        }

        storage_type = other.storage_type;
        other.storage_type = type::null;
        return *this;
    }

    std::string type_name() const {
        switch(storage_type) {
        case type::array:
            return "array";
        case type::string:
            return "string";
        case type::object:
            return "object";
        case type::number:
            return "number";
        case type::boolean:
            return "boolean";
        case type::null:
            return "null";
        default:
            return "unknown";
        }
    }

    void clear() JSONPP_NOEXCEPT {
        switch(storage_type) {
        case type::array:
            delete storage.arr;
            break;
        case type::string:
            delete storage.str;
            break;
        case type::object:
            delete storage.obj;
            break;
        default:
            break;
        }
        storage_type = type::null;
    }

    template<typename T, EnableIf<is_string<T>> = 0>
    bool is() const JSONPP_NOEXCEPT {
        return storage_type == type::string;
    }

    template<typename T, EnableIf<is_null<T>> = 0>
    bool is() const JSONPP_NOEXCEPT {
        return storage_type == type::null;
    }

    template<typename T, EnableIf<is_number<T>> = 0>
    bool is() const JSONPP_NOEXCEPT {
        return storage_type == type::number;
    }

    template<typename T, EnableIf<is_bool<T>> = 0>
    bool is() const JSONPP_NOEXCEPT {
        return storage_type == type::boolean;
    }

    template<typename T, EnableIf<is_object<T>> = 0>
    bool is() const JSONPP_NOEXCEPT {
        return storage_type == type::object;
    }

    template<typename T, EnableIf<is_array<T>> = 0>
    bool is() const JSONPP_NOEXCEPT {
        return storage_type == type::array;
    }

    template<typename T, DisableIf<is_json<T>> = 0>
    bool is() const JSONPP_NOEXCEPT {
        return false;
    }

    template<typename T, EnableIf<std::is_same<T, const char*>> = 0>
    T as() const {
        JSONPP_ASSERT(is<T>(), "called as<T> with type mismatch");
        return storage.str->c_str();
    }

    template<typename T, EnableIf<std::is_same<T, std::string>> = 0>
    T as() const {
        JSONPP_ASSERT(is<T>(), "called as<T> with type mismatch");
        return *(storage.str);
    }

    template<typename T, EnableIf<is_null<T>> = 0>
    T as() const {
        JSONPP_ASSERT(is<T>(), "called as<T> with type mismatch");
        return {};
    }

    template<typename T, EnableIf<is_bool<T>> = 0>
    T as() const {
        JSONPP_ASSERT(is<T>(), "called as<T> with type mismatch");
        return storage.boolean;
    }

    template<typename T, EnableIf<is_number<T>> = 0>
    T as() const {
        JSONPP_ASSERT(is<T>(), "called as<T> with type mismatch");
        return storage.number;
    }

    template<typename T, EnableIf<is_object<T>> = 0>
    T as() const {
        JSONPP_ASSERT(is<T>(), "called as<T> with type mismatch");
        return *(storage.obj);
    }

    template<typename T, EnableIf<is_array<T>> = 0>
    T as() const {
        JSONPP_ASSERT(is<T>(), "called as<T> with type mismatch");
        return *(storage.arr);
    }

    template<typename T, DisableIf<is_json<T>> = 0>
    T as() const {
        static_assert(dependent_false<T>::value, "calling value::as<T>() on an invalid type (use a json type instead)");
        return {};
    }

    template<typename T>
    T as(Identity<T>&& def) const {
        return is<T>() ? as<T>() : std::forward<T>(def);
    }

    template<typename T>
    auto get() const -> decltype(getter(type_tag<T>{})) {
        static_assert(is_valid_getter<T>::value, "Invalid getter passed. Must be boolean, number, array, object or string.");
        JSONPP_ASSERT(is<T>(), "called get<T> with type mismatch");
        return getter(type_tag<T>{});
    }

    template<typename T>
    auto get() -> decltype(getter(type_tag<T>{})) {
        static_assert(is_valid_getter<T>::value, "Invalid getter passed. Must be boolean, number, array, object or string.");
        JSONPP_ASSERT(is<T>(), "called get<T> with type mismatch");
        return getter(type_tag<T>{});
    }

    template<typename T, EnableIf<is_string<T>> = 0>
    value operator[](const T& str) const {
        if(!is<object>()) {
            return {};
        }

        auto it = storage.obj->find(str);
        if(it != storage.obj->end()) {
            return it->second;
        }
        return {};
    }

    template<typename T, EnableIf<is_number<T>> = 0>
    value operator[](const T& index) const {
        if(!is<array>()) {
            return {};
        }

        auto&& arr = *storage.arr;

        if(static_cast<size_t>(index) < arr.size()) {
            return arr[index];
        }
        return {};
    }

    template<typename OStream>
    friend OStream& dump(OStream& out, const value& val, format_options opt = {}) {
        switch(val.storage_type) {
        case type::array:
            return dump(out, *val.storage.arr, opt);
        case type::string:
            return dump(out, *val.storage.str, opt);
        case type::object:
            return dump(out, *val.storage.obj, opt);
        case type::boolean:
            return dump(out, val.storage.boolean, opt);
        case type::number:
            return dump(out, val.storage.number, opt);
        case type::null:
            return dump(out, nullptr, opt);
        default:
            return out;
        }
    }

    friend bool operator==(const value& lhs, const value& rhs) JSONPP_NOEXCEPT {
        if(lhs.storage_type != rhs.storage_type) {
            return false;
        }

        switch(lhs.storage_type) {
        case type::array:
            return *lhs.storage.arr == *rhs.storage.arr;
        case type::object:
            return *lhs.storage.obj == *rhs.storage.obj;
        case type::string:
            return *lhs.storage.str == *rhs.storage.str;
        case type::boolean:
            return lhs.storage.boolean == rhs.storage.boolean;
        case type::number:
            return lhs.storage.number == rhs.storage.number;
        default:
            return true;
        }
    }
};

inline bool operator!=(const value& lhs, const value& rhs) JSONPP_NOEXCEPT {
    return !(lhs == rhs);
}

template<typename OStream>
inline OStream& dump(OStream& out, const value& val, format_options opt);

using array  = value::array;
using object = value::object;

template<typename T>
inline auto value_cast(const value& v) -> decltype(v.as<T>()) {
    return v.as<T>();
}

template<typename T>
inline auto value_cast(const value& v, T&& def) -> decltype(v.as<Unqualified<T>>(std::forward<T>(def))) {
    return v.as<Unqualified<T>>(std::forward<T>(def));
}
} // json

#include <cstring>

namespace json {
namespace detail {
inline bool is_space(char ch) {
    switch(ch) {
    case 0x0D: // carriage return
    case 0x09: // tab
    case 0x0A: // line feed
    case 0x20: // space
        return true;
    default:
        return false;
    }
}

inline bool is_float(char ch) {
    switch(ch) {
    case 0x30: // 0
    case 0x31: // 1
    case 0x32: // 2
    case 0x33: // 3
    case 0x34: // 4
    case 0x35: // 5
    case 0x36: // 6
    case 0x37: // 7
    case 0x38: // 8
    case 0x39: // 9
    case 0x2B: // +
    case 0x2D: // -
    case 0x2E: // .
    case 0x45: // E
    case 0x65: // e
        return true;
    default:
        return false;
    }
}
} // detail

struct extensions {
    enum : unsigned {
        none = 0,
        comments = 1 << 1,
        trailing_comma = 1 << 2,
        all = comments | trailing_comma
    };
};

template<unsigned Flags>
struct parser {
private:
    std::size_t line = 1;
    std::size_t column = 1;
    const char* str;

    template<unsigned X = Flags, EnableIf<has_extension<X, extensions::comments>> = 0>
    void skip_comments() {
        if(*str != '/') {
            return;
        }
        const char* copy = str + 1;
        switch(*copy) {
        case '/':
            for(++copy; *copy != '\0' && *copy != 0x0A; ++copy) {
                  ++column;
            }
            break;
        case '*':
            ++copy;
            if (*copy == '\0')
                return;
            for(const char* prev = copy++; true; ++prev, ++copy) {
                if (*copy == '/' && *prev == '*') {
                    ++copy;
                    break;
                }
                if (*copy == '\0')
                    throw parser_error("expected */, received EOF instead", line, column);
                if(*str == 0x0A) {
                    ++line;
                    column = 0;
                }
                ++column;
            }
            break;
        case '\0':
        default:
            return;
        }
        str = copy;
    }

    template<unsigned X = Flags, DisableIf<has_extension<X, extensions::comments>> = 0>
    void skip_comments() {}

    void skip_white_space() {
        while(*str != '\0') {
            skip_comments();
            if (!detail::is_space(*str)) {
                return;
            }
            if(*str == 0x0A) {
                ++line;
                column = 0;
            }
            ++str;
            ++column;
        }
    }

    template<unsigned X = Flags, EnableIf<has_extension<X, extensions::trailing_comma>> = 0>
    void check_trailing_comma(bool) {}

    template<unsigned X = Flags, DisableIf<has_extension<X, extensions::trailing_comma>> = 0>
    void check_trailing_comma(bool has_trailing_comma) {
        if(has_trailing_comma) {
            throw parser_error("extraneous comma found", line, column);
        }
    }

    void parse_null(value& v) {
        static const char null_str[] = "null";
        if(*str == '\0') {
            throw parser_error("expected null, received EOF instead", line, column);
        }

        if(std::strncmp(str, null_str, sizeof(null_str) - 1) != 0) {
            throw parser_error("expected null not found", line, column);
        }

        v = nullptr;
        str = str + sizeof(null_str) - 1;
        column += sizeof(null_str);
    }

    void parse_number(value& v) {
        const char* begin = str;
        if(*begin == '\0') {
            throw parser_error("expected number, received EOF instead", line, column);
        }

        if(*str == '-') {
            ++str; // started with -
        }

        if(*str == '0') {
            // starting with 0
            ++str;
            if(*str >= '0' && *str <= '9') {
                // leading zero
                auto offset = column + (str - begin);
                throw parser_error("numbers cannot start with a zero", line, offset);
            }
        }

        while(detail::is_float(*str)) {
            ++str;
        }

        auto size = static_cast<size_t>(str - begin);
        double val = 0.0;
        size_t idx;
        try {
            std::string temp(begin, str);
            val = std::stod(temp, &idx);
            column += temp.size() + 1;
        }
        catch(const std::exception&) {
            throw parser_error("number could not be parsed properly", line, column);
        }

        if(idx != size) {
            throw parser_error("number could not be parsed properly", line, column);
        }

        v = val;
    }

    int get_codepoint(const char*& copy) {
        int codepoint = 0;
        for(unsigned i = 0; i < 4; ++i) {
            char hex = *copy;
            if(hex <= 0x1F) {
                throw parser_error("incomplete codepoint provided", line, column);
            }

            // convert the hex character to its integral representation
            // e.g., 'F' -> 15
            if(hex >= '0' && hex <= '9') {
                hex -= '0';
            }
            else if(hex >= 'A' && hex <= 'F') {
                hex -= 'A' - 0xA;
            }
            else if(hex >= 'a' && hex <= 'f') {
                hex -= 'a' - 0xA;
            }
            else {
                throw parser_error("invalid codepoint provided", line, column);
            }

            codepoint = codepoint * 16 + hex;
            ++column;
            ++copy;
        }
        return codepoint;
    }

    void parse_codepoint(const char*& copy, std::string& result) {
        // parse the hex characters
        // in order to do so, we have to increment by one to get these digits.
        // ergo, *copy == 'u', ++copy = codepoint
        ++copy;
        ++column;
        int codepoint = get_codepoint(copy);

        // a regular ASCII code point
        if(codepoint < 0x80) {
            result.push_back(codepoint);
            return;
        }

        // handle surrogate pairs
        if(codepoint >= 0xD800 && codepoint <= 0xDFFF) {
            if(codepoint >= 0xDC00) {
                throw parser_error("low surrogate pair found but high surrogate pair expected", line, column);
            }

            // get the  low surrogate pair
            if(*copy != '\\' && *(copy + 1) != 'u') {
                throw parser_error("low surrogate pair expected but not found", line, column);
            }

            copy += 2;
            int low_surrogate = get_codepoint(copy);
            if(low_surrogate < 0xDC00 || low_surrogate > 0xDFFF) {
                throw parser_error("low surrogate out of range [\\uDC000, \\uDFFF]", line, column);
            }

            codepoint = 0x10000 + (((codepoint - 0xD800) << 10) | ((low_surrogate - 0xDC00) & 0x3FF));
        }

        if(codepoint < 0x800) {
            result.push_back(0xC0 | (codepoint >> 6));
        }
        else if(codepoint < 0x10000) {
            result.push_back(0xE0 | (codepoint >> 12));
        }
        else {
            result.push_back(0xF0 | (codepoint >> 18));
            result.push_back(0x80 | ((codepoint >> 12) & 0x3F));
        }
        result.push_back(0x80 | ((codepoint >> 6) & 0x3F));
        result.push_back(0x80 | (codepoint & 0x3F));
    }

    template<typename Value>
    void parse_string(Value& v) {
        const char* copy = str + 1;
        if(*copy == '\0') {
            throw parser_error("expected string, received EOF instead", line, column);
        }

        std::string result;

        // give an initial buffer of 64 to allow constant reallocation
        result.reserve(64);

        // begin parsing
        while(true) {
            ++column;
            auto byte = static_cast<unsigned char>(*copy);

            if(byte <= 0x1F) {
                throw parser_error("invalid characters found in string or string is incomplete", line, column);
            }

            // end of string found
            if(*copy == '"') {
                break;
            }

            // non-escape character is good to go
            if(*copy != '\\') {
                result.push_back(*copy++);
                continue;
            }

            // at this point *copy == '\\'
            // so increment it to check the next character
            ++copy;
            switch(*copy) {
            case '/':
                result.push_back('/');
                ++copy;
                break;
            case '\\':
                result.push_back('\\');
                ++copy;
                break;
            case '"':
                result.push_back('\"');
                ++copy;
                break;
            case 'f':
                result.push_back('\f');
                ++copy;
                break;
            case 'n':
                result.push_back('\n');
                ++copy;
                break;
            case 'r':
                result.push_back('\r');
                ++copy;
                break;
            case 't':
                result.push_back('\t');
                ++copy;
                break;
            case 'b':
                result.push_back('\b');
                ++copy;
                break;
            case 'u':
                parse_codepoint(copy, result);
                break;
            default:
                throw parser_error("improper or incomplete escape character found", line, column);
            }
        }

        result.shrink_to_fit();
        v = std::move(result);
        ++copy;
        str = copy;
    }

    void parse_bool(value& v) {
        if(*str == '\0') {
            throw parser_error("expected boolean, received EOF instead", line, column);
        }

        bool expected_true = *str == 't';
        const char* boolean = expected_true ? "true" : "false";
        const size_t len = expected_true ? 4 : 5;

        if(std::strncmp(str, boolean, len) != 0) {
            throw parser_error("expected boolean not found", line, column);
        }

        v = expected_true;
        str = str + len;
        column += len;
    }

    void parse_array(value& v) {
        ++str;
        array arr;
        skip_white_space();

        if(*str == '\0') {
            throw parser_error("expected value, received EOF instead", line, column);
        }

        while (*str && *str != ']') {
            value elem;
            parse_value(elem);
            if(*str != ',') {
                if(*str != ']') {
                    throw parser_error("missing comma", line, column);
                }
            }
            else if(*str == ',') {
                ++str;

                // skip whitespace
                skip_white_space();

                // handle missing input
                bool is_trailing_comma = *str && *str == ']';
                check_trailing_comma(is_trailing_comma);
            }

            arr.push_back(std::move(elem));
        }

        v = std::move(arr);
        if(*str == ']') {
            ++str;
        }
    }

    void parse_object(value& v) {
        ++str;
        object obj;
        bool last_is_comma = false;

        skip_white_space();

        if(*str == '\0') {
            throw parser_error("expected string key, received EOF instead", line, column);
        }

        while(*str) {
            std::string key;
            value elem;

            skip_white_space();

            // empty object
            if(*str == '}') {
                check_trailing_comma(last_is_comma);
                break;
            }

            last_is_comma = false;

            if(*str != '"') {
                throw parser_error("expected string as key not found", line, column);
            }
            parse_string(key);
            skip_white_space();

            if(*str != ':') {
                throw parser_error("missing semicolon", line, column);
            }
            ++str;
            parse_value(elem);

            if(*str != ',') {
                if(*str != '}') {
                    throw parser_error("missing comma", line, column);
                }
            }
            else if(*str == ',') {
                last_is_comma = true;
                ++column;
                ++str;
            }
            obj.emplace(std::move(key), std::move(elem));
        }

        v = std::move(obj);
        if(*str == '}') {
            ++str;
            ++column;
        }
        else {
            throw parser_error("expected closing brace", line, column);
        }
    }

    void parse_value(value& v) {
        skip_white_space();
        if(*str == '\0') {
            throw parser_error("unexpected EOF found", line, column);
        }

        if(isdigit(*str) || *str == '+' || *str == '-') {
            parse_number(v);
        }
        else {
            switch(*str) {
            case 'n':
                parse_null(v);
                break;
            case '"':
                parse_string(v);
                break;
            case 't':
            case 'f':
                parse_bool(v);
                break;
            case '[':
                parse_array(v);
                break;
            case '{':
                parse_object(v);
                break;
            default:
                throw parser_error("unexpected token found", line, column);
                break;
            }
        }

        skip_white_space();
    }
public:
    parser(const char* str) JSONPP_NOEXCEPT: str(str) {}

    void parse(value& v) {
        parse_value(v);
        if(*str != '\0') {
            throw parser_error("unexpected token found", line, column);
        }
    }
};

template<unsigned Flags = extensions::none>
inline void parse(const std::string& str, value& v) {
    parser<Flags> js(str.c_str());
    js.parse(v);
}

template<unsigned Flags = extensions::none, typename IStream, DisableIf<is_string<IStream>> = 0>
inline void parse(IStream& in, value& v) {
    static_assert(std::is_base_of<std::istream, IStream>::value, "Input stream passed must inherit from std::istream");
    if(in) {
        std::ostringstream ss;
        ss << in.rdbuf();
        parse<Flags>(ss.str(), v);
    }
}

template<unsigned Flags = extensions::none, typename T>
inline value parse(T&& t) {
    value v;
    parse<Flags>(std::forward<T>(t), v);
    return v;
}
} // json

#ifndef JSON_CANONICAL_HPP
#define JSON_CANONICAL_HPP

namespace json {
template<typename T, typename Sfinae = int>
struct type_name;

template<typename T>
struct type_name<T, EnableIf<is_null<T>>> {
    static constexpr const char* value = "null";
};

template<typename T>
constexpr const char* type_name<T, EnableIf<is_null<T>>>::value;

template<typename T>
struct type_name<T, EnableIf<is_bool<T>>> {
    static constexpr const char* value = "boolean";
};

template<typename T>
constexpr const char* type_name<T, EnableIf<is_bool<T>>>::value;

template<typename T>
struct type_name<T, EnableIf<is_number<T>>> {
    static constexpr const char* value = "number";
};

template<typename T>
constexpr const char* type_name<T, EnableIf<is_number<T>>>::value;

template<typename T>
struct type_name<T, EnableIf<is_string<T>>> {
    static constexpr const char* value = "string";
};

template<typename T>
constexpr const char* type_name<T, EnableIf<is_string<T>>>::value;

template<typename T>
struct type_name<T, EnableIf<is_object<T>>> {
    static constexpr const char* value = "object";
};

template<typename T>
constexpr const char* type_name<T, EnableIf<is_object<T>>>::value;

template<typename T>
struct type_name<T, EnableIf<is_array<T>>> {
    static constexpr const char* value = "array";
};

template<typename T>
constexpr const char* type_name<T, EnableIf<is_array<T>>>::value;
} // json

namespace json {
namespace detail {
struct canonical_to_json_algo;

struct canonical_to_json_type {
private:
    template<typename Source, EnableIf<is_json<Source>> = 0>
    static value impl(Source const& source, int)
    { return source; }

    template<typename Source, DisableIf<is_json<Source>> = 0>
    static value impl(Source const& source, long)
    {
        DependOn<canonical_to_json_algo, Source> algo {};
        canonical_schema<Source> {}(algo, source);
        return std::move(algo).result;
    }

public:
    template<typename Source>
    value operator()(Source const& source) const
    { return impl(source, 0); }
};

} // detail

constexpr detail::canonical_to_json_type canonical_to_json {};

namespace detail {

struct canonical_to_json_algo {
    object result;

    template<typename Source>
    void member(const char* name, Source const& source)
    {
        result.insert({ name, canonical_to_json(source) });
    }
};

struct canonical_from_json_algo;

template<typename Dest>
struct canonical_from_json_type {
private:
    template<typename Dep = Dest, EnableIf<is_json<Dep>> = 0>
    static void impl(value const& v, Dest& result, int)
    {
        if(!v.is<Dest>()) {
            std::ostringstream fmt;
            fmt << "expected a(n) " << type_name<Dest>::value << ", received a(n) " << v.type_name() << " instead";
            throw canonical_from_json_error { std::move(fmt).str() };
        }
        result = v.as<Dest>();
    }

    template<typename Dep = Dest, DisableIf<is_json<Dep>> = 0>
    static void impl(value const& v, Dest& result, long)
    {
        if(!v.is<object>()) {
            std::ostringstream fmt;
            fmt << "expected an object, received a(n) " << v.type_name() << " instead";
            throw canonical_from_json_error { std::move(fmt).str() };
        }

        auto&& obj = v.as<object>();
        DependOn<canonical_from_json_algo, Dep> algo { obj };
        canonical_schema<Dest> {}(algo, result);
    }

public:
    Dest operator()(value const& v) const
    {
        Dest result;
        impl(v, result, 0);
        return result;
    }

    void operator()(value const& v, Dest& result) const
    {
        impl(v, result, 0);
    }
};

} // detail

template<typename Dest>
inline Dest canonical_from_json(value const& v)
{
    static constexpr detail::canonical_from_json_type<Dest> call;
    return call(v);
}

template<typename Dest>
inline void canonical_from_json(value const& v, Dest& result)
{
    static constexpr detail::canonical_from_json_type<Dest> call;
    call(v, result);
}

namespace detail {

struct canonical_from_json_algo {
    object obj;

    template<typename Value>
    void member(const char* name, Value& value) const
    {
        auto it = obj.find(name);
        if(it == obj.end()) {
            std::ostringstream fmt;
            fmt << "missing member '" << name << '\'';
            throw canonical_from_json_error { std::move(fmt).str() };
        }

        try {
            canonical_from_json<Value>(it->second, value);
        } catch(canonical_from_json_error& e) {
            std::ostringstream fmt;
            fmt << "bad member '" << name << "': " << std::move(e).message;
            e.message = std::move(fmt).str();
            throw;
        }
    }
};

} // detail

} // json

#endif // JSON_CANONICAL_HPP
#include <utility>

namespace json {
namespace detail {
struct to_json_algo;

struct to_json_impl {
private:
    template<typename T, typename U = Unqualified<T>, EnableIf<is_regular_serialisable<U>> = 0>
    value impl(choice<0>, T&& value) const {
        return ::json::value(std::forward<T>(value));
    }

    template<typename T, typename U = Unqualified<T>, EnableIf<is_array_like<U>> = 0>
    value impl(choice<1>, T&& value) const {
        using value_type = typename array_value_type<U>::type;
        static_assert(is_serialisable<value_type>::value, "The array value type must be JSON serialisable.");
        array ret;
        for(auto&& x : std::forward<T>(value)) {
            ret.push_back(impl(select_overload{}, x));
        }
        return ret;
    }

    template<typename T, typename U = Unqualified<T>, EnableIf<is_object_like<U>> = 0>
    value impl(choice<2>, T&& value) const {
        using value_type = typename U::mapped_type;
        static_assert(is_serialisable<value_type>::value, "The map's mapped type must be JSON serialisable.");
        object obj;
        for(auto&& p : std::forward<T>(value)) {
            obj.emplace(to_json_key(p.first), impl(select_overload{}, p.second));
        }
        return obj;
    }

    template<typename T, typename U = Unqualified<T>>
    value impl(otherwise, T&& value) const {
        DependOn<to_json_algo, U> algo {};
        json_schema<U> {}(algo, std::forward<T>(value));
        return std::move(algo).result;
    }
public:
    template<typename T>
    auto operator()(T&& value) const -> decltype(impl(select_overload{}, std::forward<T>(value))) {
        return impl(select_overload{}, std::forward<T>(value));
    }
};

struct from_json_algo;

struct from_json_impl {
private:
    template<typename T, EnableIf<is_json<T>> = 0>
    void impl(choice<0>, const value& v, T& dest) const {
        if(!v.is<T>()) {
            std::ostringstream fmt;
            fmt << "expected " << type_name<T>::value << ", received " << v.type_name() << " instead";
            throw from_json_error(std::move(fmt).str());
        }

        dest = v.as<T>();
    }

    void impl(choice<1>, const value& v, value& dest) const {
        dest = v;
    }

    template<typename T, EnableIf<is_array_like<T>, Not<std::is_array<T>>> = 0>
    void impl(choice<2>, const value& v, T& dest) const {
        using value_type = typename T::value_type;
        static_assert(is_deserialisable<value_type>::value, "The array's value type must be JSON deserialisable.");
        auto&& arr = v.as<array>();
        dest.clear();
        for(array::size_type i = 0; i < arr.size(); ++i) {
            try {
                dest.emplace_back(call<value_type>(arr[i]));
            }
            catch(from_json_error& e) {
                std::ostringstream fmt;
                fmt << "at array element " << i << ": " << std::move(e).message;
                e.message = std::move(fmt).str();
                throw;
            }
        }
    }

    template<typename T, EnableIf<is_object_like<T>> = 0>
    void impl(choice<3>, const value& v, T& dest) const {
        using mapped_type = typename T::mapped_type;
        using key_type = typename T::key_type;
        static_assert(is_deserialisable<mapped_type>::value, "The map's mapped type must be JSON deserialisable.");
        static_assert(is_string<key_type>::value, "The map's key must be a string.");
        dest.clear();
        auto&& obj = v.as<object>();
        for(auto&& p : obj) {
            try {
                dest.emplace(p.first, call<mapped_type>(p.second));
            }
            catch(from_json_error& e) {
                std::ostringstream fmt;
                fmt << "at object key \"" << p.first << "\": " << std::move(e).message;
                e.message = std::move(fmt).str();
                throw;
            }
        }
    }

    template<typename T>
    void impl(otherwise, const value& v, T& dest) const {
        if(!v.is<object>()) {
            throw from_json_error("expected object, received " + v.type_name() + " instead");
        }

        DependOn<from_json_algo, T> algo { v.get<object>() };
        json_schema<T>{}(algo, dest);
    }
public:
    template<typename T>
    T call(const value& v) const {
        T result;
        impl(select_overload{}, v, result);
        return result;
    }

    template<typename T>
    void call(const value& v, T& result) const {
        impl(select_overload{}, v, result);
    }
};
} // detail

constexpr detail::to_json_impl to_json{};
constexpr detail::from_json_impl from_json_caller{};

template<typename T>
inline void from_json(const value& v, T& result) {
    from_json_caller.call(v, result);
}

template<typename T>
inline auto from_json(const value& v) -> decltype(from_json_caller.call<T>(v)) {
    return from_json_caller.call<T>(v);
}

namespace detail {
struct to_json_algo {
    object result;

    template<typename Source>
    void member(const char* name, const Source& source) {
        result.emplace(name, to_json(source));
    }
};

struct from_json_algo {
    const object& obj;

    template<typename Value>
    void member(const char* name, Value& value) const {
        auto&& js = value_at(name);
        try {
            from_json(js, value);
        }
        catch(from_json_error& e) {
            std::ostringstream fmt;
            fmt << "at key '" << name << "': " << std::move(e).message;
            e.message = std::move(fmt).str();
            throw;
        }
    }

    const value& value_at(const char* name) const {
        auto it = obj.find(name);
        if(it == obj.end()) {
            std::ostringstream fmt;
            fmt << "missing key '" << name << '\'';
            throw from_json_error{ std::move(fmt).str() };
        }
        return it->second;
    }

    bool has_key(const char* name) const {
        return obj.count(name);
    }
};
} // detail
} // json

#endif // JSONPP_SINGLE_INCLUDE_HPP
