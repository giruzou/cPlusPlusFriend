// やめるのだフェネックで学ぶC++の実証コード(ライブラリの使い方)
#include <cctype>
#include <ctime>
#include <codecvt>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <locale>
#include <random>
#include <regex>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>
#include <boost/any.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/time_facet.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/locale.hpp>
#include <boost/math/constants/constants.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/multiprecision/miller_rabin.hpp>
#include <boost/random.hpp>
#include <boost/random/random_device.hpp>
#include <boost/regex.hpp>
#include <boost/type_traits/function_traits.hpp>
#include <gtest/gtest.h>
#include "cFriendsCommon.h"
#include "cppFriends.hpp"

// C++98では違う型、C++11は同じ型
static_assert(std::is_same<boost::fusion::vector<int,int>, boost::fusion::vector2<int,int>>::value, "Different");

class TestRegex : public ::testing::Test {
protected:
    static const std::string pattern_;
    static const std::string input_;
    static const std::string expected_;

    void createStdRegex(std::regex_constants::syntax_option_type t) {
        // 再帰正規表現はサポートしていない
        std::regex expr(pattern_, t);
    }
};

// ()の入れ子を、最も外側の()で分ける、再帰正規表現
const std::string TestRegex::pattern_  {"((?>[^\\s(]+|(\\((?>[^()]+|(?-1))*\\))))"};
const std::string TestRegex::input_    {" (a) ((b)) (((c))) (d) "};
const std::string TestRegex::expected_ {"(a)::((b))::(((c)))::(d)::"};

TEST_F(TestRegex, StdTypes1) {
    const auto typeSet = {std::regex_constants::ECMAScript, std::regex_constants::extended,
                          std::regex_constants::awk, std::regex_constants::egrep};

    for(auto t : typeSet) {
        ASSERT_ANY_THROW(createStdRegex(t));
    }
}

TEST_F(TestRegex, StdTypes2) {
    const auto typeSet = {std::regex_constants::basic, std::regex_constants::grep};

    for(auto t : typeSet) {
        using Iter = std::string::const_iterator;
        Iter startI = input_.begin();
        Iter endI = input_.end();
        std::match_results<Iter> results;
        std::regex expr(pattern_, t);
        std::regex_constants::match_flag_type flags = std::regex_constants::match_default;

        std::ostringstream os;
        while(std::regex_search(startI, endI, results, expr, flags)) {
            auto& head = results[0];
            const std::string substr(head.first, head.second);
            os << substr << "::";
            startI = head.second;
            flags |= std::regex_constants::match_prev_avail;
        }
        EXPECT_TRUE(os.str().empty());
    }
}

TEST_F(TestRegex, Boost) {
    using Iter = std::string::const_iterator;
    Iter startI = input_.begin();
    Iter endI = input_.end();
    boost::match_results<Iter> results;
    boost::regex expr(pattern_);
    boost::match_flag_type flags = boost::match_default;

    std::ostringstream osSearch;
    while(boost::regex_search(startI, endI, results, expr, flags)) {
        auto& head = results[0];
        const std::string substr(head.first, head.second);
        osSearch << substr << "::";
        startI = head.second;
        flags |= boost::match_prev_avail;
        flags |= boost::match_not_bob;
    }
    EXPECT_EQ(expected_, osSearch.str());

    std::ostringstream osIter;
    boost::sregex_token_iterator i {input_.begin(), input_.end(), expr, 1};
    boost::sregex_token_iterator e;
    while(i != e) {
        osIter << *i << "::";
        ++i;
    }
    EXPECT_EQ(expected_, osIter.str());
}

namespace {
    void parseComplexRegex(void) {
        // https://www.checkmarx.com/wp-content/uploads/2015/03/ReDoS-Attacks.pdf
        std::regex expr("^[a-zA-Z]+(([\\'\\,\\.\\- ][a-zA-Z ])?[a-zA-Z]*)*$");
        std::smatch match;
        std::string str = "aaaaaaaaaaaaaaaaaaaaaaaaaaaa!";
        ASSERT_TRUE(std::regex_match(str, match, expr));
    }
}

TEST_F(TestRegex, ReDos) {
    ASSERT_ANY_THROW(parseComplexRegex());
}

namespace {
    static_assert(sizeof(char) == 1, "Expect sizeof(char) == 1");
    static_assert(sizeof('a') == 1, "Expect sizeof(char) == 1");
    static_assert(sizeof(g_arrayForTestingSize) == 1, "g_arrayForTestingSize must have one element");
    static_assert(sizeof(TestingOuterStruct1) > sizeof(TestingOuterStruct1::member), "Unexpected struct size");
    static_assert(sizeof(TestingEmptyStruct) == 1, "Expect sizeof(empty struct) == 1");
    __attribute__((unused)) void funcTakesByte(uint8_t e) {}

    /* これら上とを同時には定義できない
    using BYTETYPE = uint8_t;
    void funcTakesByte(unsigned char e) {}
    void funcTakesByte(BYTETYPE e) {}
    */

    using Paragraph = std::vector<std::string>;
    const std::string MyJoinStrings(const Paragraph& paragraph) {
        std::ostringstream os;
        auto size = paragraph.size();

        for(decltype(size) i=0; i<size; ++i) {
            const auto& str = paragraph.at(i);
            os << str;
            if (((i + 1) < size) && !str.empty() && ::isascii(*(str.rbegin()))) {
                // 正規のUTF-8を仮定し、MSBが1でなければUS-ASCIIとみなす
                os << " ";
            }
        }

        const std::string result = os.str();
        return result;
    }
}

// 行を結合して一行にする
class TestJoinStrings : public ::testing::Test {
protected:
    static const Paragraph sentence1_;
    static const Paragraph sentence2_;
    static const std::string expectedNoSpaces_;
    static const std::string expectedWithSpaces1_;
    static const std::string expectedWithSpaces2_;

    static const Paragraph sentenceJ1_;
    static const Paragraph sentenceJ2_;
    static const std::string expectedJapanese_;
    static const std::string expectedWithSpacesJ1_;
    static const std::string expectedWithSpacesJ2_;
};

// https://www.rain.org/~mkummel/stumpers/15feb02a.html
const Paragraph TestJoinStrings::sentence1_ {"Now", "here", "is", "water"};
const Paragraph TestJoinStrings::sentence2_ {"Nowhere", "is", "water"};
const std::string TestJoinStrings::expectedNoSpaces_ = "Nowhereiswater";
const std::string TestJoinStrings::expectedWithSpaces1_ = "Now here is water";
const std::string TestJoinStrings::expectedWithSpaces2_ = "Nowhere is water";

const Paragraph TestJoinStrings::sentenceJ1_ {"弁慶が", "なぎなたを振り回し"};
const Paragraph TestJoinStrings::sentenceJ2_ {"弁慶がな", "ぎなたを振り回し"};
const std::string TestJoinStrings::expectedJapanese_ = "弁慶がなぎなたを振り回し";
const std::string TestJoinStrings::expectedWithSpacesJ1_ = "弁慶が なぎなたを振り回し";
const std::string TestJoinStrings::expectedWithSpacesJ2_ = "弁慶がな ぎなたを振り回し";

TEST_F(TestJoinStrings, English) {
    const auto actualNoSpaces1 = boost::algorithm::join(sentence1_, "");
    const auto actualNoSpaces2 = boost::algorithm::join(sentence2_, "");
    const auto actualWithSpaces1 = boost::algorithm::join(sentence1_, " ");
    const auto actualWithSpaces2 = boost::algorithm::join(sentence2_, " ");
    EXPECT_EQ(expectedNoSpaces_, actualNoSpaces1);
    EXPECT_EQ(expectedNoSpaces_, actualNoSpaces2);
    EXPECT_EQ(expectedWithSpaces1_, actualWithSpaces1);
    EXPECT_EQ(expectedWithSpaces2_, actualWithSpaces2);

    const auto actual1 = MyJoinStrings(sentence1_);
    const auto actual2 = MyJoinStrings(sentence2_);
    EXPECT_EQ(expectedWithSpaces1_, actual1);
    EXPECT_EQ(expectedWithSpaces2_, actual2);
}

TEST_F(TestJoinStrings, NonAlpha) {
    const Paragraph vec {"1", "2 ", "", "3!", "$"};
    const std::string expected = "1 2  3! $";
    const auto actual = MyJoinStrings(vec);
    EXPECT_EQ(expected, actual);
}

TEST_F(TestJoinStrings, Japanese) {
    const auto actualWithSpaces1 = boost::algorithm::join(sentenceJ1_, " ");
    const auto actualWithSpaces2 = boost::algorithm::join(sentenceJ2_, " ");
    EXPECT_EQ(expectedWithSpacesJ1_, actualWithSpaces1);
    EXPECT_EQ(expectedWithSpacesJ2_, actualWithSpaces2);

    const auto actual1 = MyJoinStrings(sentenceJ1_);
    const auto actual2 = MyJoinStrings(sentenceJ2_);
    EXPECT_EQ(expectedJapanese_, actual1);
    EXPECT_EQ(expectedJapanese_, actual2);
}

// UTF-8を解釈する
class TestUtfCharCounter : public ::testing::Test{};

TEST_F(TestUtfCharCounter, Well) {
    // http://en.cppreference.com/w/cpp/locale/wstring_convert/from_bytes
    // の例で、utf8.data()を使っているが、utf8.data()は
    // C++11以前では null terminateされているとは限らない(C++11ではされている)
    std::string utf8jp = "かばんちゃん急に何を言い出すの";
    std::u16string utf16jp = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(utf8jp);
    EXPECT_EQ(15, utf16jp.size());

    // ラッキービーストは複数いるはず
    std::string utf8 = "I'm a lucky beast";
    std::u16string utf16 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(utf8);
    EXPECT_EQ(17, utf16.size());

    constexpr size_t length = 5;
    std::vector<char> vec(length, ' ');
    vec.push_back(0);
    std::u16string utf16sp = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(vec.data());
    EXPECT_EQ(length, utf16sp.size());
}

TEST_F(TestUtfCharCounter, ByteOrderMark) {
    // BOM + 半角空白
    const std::vector<uint8_t> elements {0xef, 0xbb, 0xbf, 0x20};

    std::vector<char> vec;
    for(auto e : elements) {
        vec.push_back(static_cast<char>(e));
    }
    vec.push_back(0);

    bool thrown = false;
    try {
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(vec.data());
    } catch(std::range_error& e) {
        thrown = true;
    }
    EXPECT_FALSE(thrown);
}

TEST_F(TestUtfCharCounter, Ill) {
    // 半角空白 = 00100000 をわざと冗長なUTF-8で表現する
    // 11100000 10000000 10100000
    const std::vector<uint8_t> elements {0xe0, 0x80, 0xa0};

    constexpr size_t length = 5;
    std::vector<char> vec;
    for(size_t i=0; i<length; ++i) {
        for(auto e : elements) {
            vec.push_back(static_cast<char>(e));
        }
    }
    vec.push_back(0);

    bool thrown = false;
    try {
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(vec.data());
    } catch(std::range_error& e) {
        thrown = true;
    }
    EXPECT_TRUE(thrown);
}

// ファイル出力
class TestFileStream : public ::testing::Test {};

TEST_F(TestFileStream, Close) {
    EXPECT_FALSE(errno);
    {
        std::ofstream fs1;
        fs1.exceptions(std::ifstream::failbit);
        // ファイルに関連付けられていなければcloseに失敗する
        ASSERT_ANY_THROW(fs1.close());
    }
    EXPECT_FALSE(errno);

    {
        std::ofstream fs2;
        fs2.exceptions(std::ifstream::failbit);
        fs2 << "test";
        // ofstreamのデストラクタは失敗しても何も教えてくれない
    }
    EXPECT_FALSE(errno);
}

// 整数型が持てる最大桁数
static_assert(std::numeric_limits<uint64_t>::digits10 == 19, "");
static_assert(std::numeric_limits<int64_t>::digits10 == 18, "");

// 敢えて自作する
template <typename T>
constexpr int MyNumericLimits(T a, int digits) {
    auto n = a * 10 + 9;
    return (n > a) ? MyNumericLimits(n, digits + 1) : digits;
}

template <typename T>
constexpr int MyNumericLimits(void) {
    return MyNumericLimits<T>(9,1);
}

static_assert(MyNumericLimits<uint64_t>() == 19, "");
static_assert(MyNumericLimits<int64_t>() == 18, "");

constexpr int MyIntMinExplicit(int l, int r) {
    return (l < r) ? l : r;
}

template <typename T>
constexpr T MyMin(T l, T r) {
    return (l < r) ? l : r;
}
// typedefは使えない
auto const MyIntMinAlias = &MyMin<int>;

// Cではこうするが、C++ではfunction traitsが使えない
#define CPPFRIENDS_MY_MACRO_MIN(l, r) ((l < r) ? l : r)

static_assert(std::is_same<int, decltype(MyIntMinExplicit(0,0))>::value, "");
static_assert(std::is_same<int, decltype(MyIntMinAlias(0,0))>::value, "");
static_assert(std::is_same<int, boost::function_traits<decltype(MyIntMinExplicit)>::arg1_type>::value, "");
static_assert(std::is_same<int, boost::function_traits<decltype(*MyIntMinExplicit)>::arg1_type>::value, "");
static_assert(std::is_same<int, boost::function_traits<decltype(*MyIntMinAlias)>::arg1_type>::value, "");
// こうは書けない
// static_assert(std::is_same<int, boost::function_traits<decltype(MyIntMinAlias)>::arg1_type>::value, "");
// static_assert(std::is_same<int, boost::function_traits<decltype(CPPFRIENDS_MY_MACRO_MIN)>::arg1_type>::value, "");

// #define MY_MACRO_POW(base, exp) ((exp) ? (base * MY_MACRO_POW(base, exp - 1)) : 1)

// 桁あふれは考慮していない
template <typename T>
constexpr T MyIntegerPow(T base, T exp) {
    return (exp) ? (base * MyIntegerPow(base, exp - 1)) : 1;
}

class TestCppMacro : public ::testing::Test {};

TEST_F(TestCppMacro, Plain) {
    EXPECT_EQ(-1, MyIntMinExplicit(-1, 0));
    EXPECT_EQ(-1, MyIntMinExplicit(0, -1));
    EXPECT_EQ(-1, MyIntMinAlias(-1, 0));
    EXPECT_EQ(-1, MyIntMinAlias(0, -1));
    EXPECT_EQ(-1, CPPFRIENDS_MY_MACRO_MIN(-1, 0));
    EXPECT_EQ(-1, CPPFRIENDS_MY_MACRO_MIN(0, -1));
}

TEST_F(TestCppMacro, Recursive) {
    using Data = unsigned int;
    Data base = 2;
    for(Data i=0; i<10; ++i) {
        EXPECT_EQ(1 << i, MyIntegerPow(base, i));
    }

    base = 3;
    Data expected = 1;
    for(Data i=0; i<5; ++i) {
        EXPECT_EQ(expected, MyIntegerPow(base, i));
        expected *= 3;
    }

    // こうは書けない
    // std::cout << MY_MACRO_POW(2,0);
}

class TestPrimalityTesting : public ::testing::Test{};

TEST_F(TestPrimalityTesting, MersenneNumber) {
    boost::random::random_device seed;
    std::mt19937 gen(seed);

    const unsigned int index[] = {3,5,7,13,17,19,31,61,89,107,127,521,607};
    using BigNumber = boost::multiprecision::uint1024_t;
    BigNumber base = 2;
    for(auto i : index) {
        BigNumber n = boost::multiprecision::pow(base, i);
        n -= 1;
        EXPECT_TRUE(boost::multiprecision::miller_rabin_test(n, 25, gen));
    }
}

// Googleの看板"{first 10-digit prime found in consecutive digits of e}.com"を解く
TEST_F(TestPrimalityTesting, QuizBoard) {
    using LongFloat = boost::multiprecision::number<boost::multiprecision::cpp_dec_float<200>>;
    // const std::string にすると、770行のエラーメッセージが出る
    // decltype(str) = const std::string なので変換できない
    // std::string str = boost::math::constants::e<LongFloat>().convert_to<decltype(str)>();
    const auto str = boost::math::constants::e<LongFloat>().convert_to<std::string>();

    boost::random::random_device seed;
    std::mt19937 gen(seed);
    long long int digits = 0;
    decltype(digits) solution = 0;
    constexpr decltype(digits) expected = 7427466391ll;
    constexpr decltype(digits) base = 10000000000ll;  // 10桁
    static_assert(std::numeric_limits<decltype(digits)>::max() > base, "Too small");

    size_t i = 0;
    char digit[] = {'\0', '\0'};
    for(auto c : str) {
        ++i;
        if (!::isdigit(c)) {
            continue;
        }

        digits *= 10;
        digit[0] = c;
        digits += boost::lexical_cast<decltype(digits)>(digit);
        digits %= base;
        if ((i >= 10) && (boost::multiprecision::miller_rabin_test(digits, 25, gen))) {
            solution = digits;
            break;
        }
    }

    EXPECT_EQ(expected, solution);
}

// 時刻と時差を扱う
namespace {
    std::string convertLocalTimeToUTC(const std::string& timeStr, const std::string& localeStr) {
        auto* input_facet = new boost::posix_time::time_input_facet("%d/%m/%Y %H:%M:%S %ZP");
        std::istringstream is(timeStr);
        boost::locale::generator gen;
        if (!localeStr.empty()) {
            is.imbue(gen(localeStr));
        }
        is.imbue(std::locale(is.getloc(), input_facet));
        boost::local_time::local_date_time lt(boost::posix_time::not_a_date_time);
        is >> lt;

        std::ostringstream os;
        os << lt.utc_time();
        return os.str();
    }
}

class TestDateFormat : public ::testing::Test{};

TEST_F(TestDateFormat, LeapSecond) {
    const char* dataDormat = "%Y-%m-%d %H:%M:%S";
    std::tm t = {};

    {
        std::istringstream is("2017-01-01 08:59:59");
        is >> std::get_time(&t, dataDormat);
        ASSERT_FALSE(is.fail());
        EXPECT_EQ(1483228799, std::mktime(&t));
    }

    {
        std::istringstream is("2017-01-01 08:59:60");
        is >> std::get_time(&t, dataDormat);
        ASSERT_FALSE(is.fail());
        EXPECT_EQ(1483228800, std::mktime(&t));

    }

    {
        std::istringstream is("2017-01-01 08:59:61");
        is >> std::get_time(&t, dataDormat);
        ASSERT_TRUE(is.fail());
    }
}

TEST_F(TestDateFormat, RepeatTime) {
    EXPECT_EQ("2017-Oct-29 00:30:00", convertLocalTimeToUTC("29/10/2017 01:30:00 BST+1", "en_GB.UTF-8"));
    EXPECT_EQ("2017-Oct-29 01:30:00", convertLocalTimeToUTC("29/10/2017 01:30:00 GMT+0", "en_GB.UTF-8"));
    EXPECT_EQ("2017-Oct-28 16:30:00", convertLocalTimeToUTC("29/10/2017 01:30:00 JST+9", "ja_JP.UTF-8"));
}

namespace JapariPark {
    class Animal {
    public:
        Animal(void) {
            setLine("animal");
        }
        virtual ~Animal(void) = default;

        const std::string& IntroduceSelf(void) const {
            return line_;
        }

    protected:
        Animal(const std::string& name) {
            setLine(name);
        }

    private:
        void setLine(const std::string& name) {
            line_ = "I'm ";
            // 本当はこれらの文字で始まっても母音でないこともあるし(uniqueなど)
            // これ以外の文字で始まっても母音のこともある(euroなど)
            line_ += (name.find_first_of("aeiou") == 0) ? "an" : "a";
            line_ += " ";
            line_ += name;
        }
        std::string line_;
    };

    class Cat : public Animal {
    public:
        Cat(void) : Animal("cat") {}
        virtual ~Cat(void) = default;
    protected:
        Cat(const std::string& name) : Animal(name) {}
    };

    class Serval : public Cat {
    public:
        Serval(void) : Cat("serval") {}
        virtual ~Serval(void) = default;
    };

    template <typename T>
    const std::string FindByKonoha(const T& obj) {
        std::string line;

        auto& tid = typeid(obj);
        if (tid == typeid(Animal)) {
            line = "She is an animal";
        } else if (tid == typeid(Cat)) {
            line = "She is a cat";
        } else if (tid == typeid(Serval)) {
            line = "She is a serval";
        } else {
            line = "Give me a second helping, please";
        }
        return line;
    }

    template <typename T>
    const std::string FindByKonohaHashCode(const T& obj) {
        std::string line;

        auto tcode = typeid(obj).hash_code();
        if (tcode == typeid(Animal).hash_code()) {
            line = "She is an animal.";
        } else if (tcode == typeid(Cat).hash_code()) {
            line = "She is a cat.";
        } else if (tcode == typeid(Serval).hash_code()) {
            line = "She is a serval.";
        } else {
            line = "Give me a second helping, please.";
        }
        return line;
    }

    template <typename T>
    const std::string FindByKonohaSwitchCase(const T& obj) {
        std::string line;
#if 0
        // typeid()が整数でないのでコンパイルできない
        switch(typeid(obj)) {
        case typeid(Animal):
            line = "She is an animal.";
            break;
        default:
            line = "Give me a second helping, please.";
            break;
        }

        switch(typeid(obj).hash_code()) {
        // caseがnon-constexprなのでコンパイルできない
        case typeid(Animal).hash_code():
            line = "She is an animal.";
            break;
        default:
            line = "Give me a second helping, please.";
            break;
        }
#endif
        return line;
    }

    const std::string FindByKonohaAny(const boost::any& obj) {
        std::string line;

        // constの有無はtypeidに影響しない
        if (obj.type() == typeid(Animal)) {
            line = "She is an animal!";
        } else if (obj.type() == typeid(Cat)) {
            line = "She is a cat!";
        } else if (obj.type() == typeid(Serval)) {
            line = "She is a serval!";
        } else {
            line = "Give me a second helping, please!";
        }
        return line;
    }

    time_t g_currentTimestamp;
    class ConstMemFn {
    public:
        virtual ~ConstMemFn(void) = default;
        void MakeSideEffect(void) const {
            g_currentTimestamp = ::time(nullptr);
            if (g_currentTimestamp) {
                throw std::runtime_error("Non zero timestamp");
            }
            return;
        }
    };
}

class TestSwitchCase : public ::testing::Test {};

TEST_F(TestSwitchCase, Polymorphism) {
    JapariPark::Animal animal;
    EXPECT_EQ("I'm an animal", animal.IntroduceSelf());
    JapariPark::Cat cat;
    EXPECT_EQ("I'm a cat", cat.IntroduceSelf());
    JapariPark::Serval serval;
    EXPECT_EQ("I'm a serval", serval.IntroduceSelf());
}

TEST_F(TestSwitchCase, Procedural) {
    JapariPark::Animal animal;
    EXPECT_EQ("She is an animal", JapariPark::FindByKonoha(animal));
    JapariPark::Cat cat;
    EXPECT_EQ("She is a cat", JapariPark::FindByKonoha(cat));
    JapariPark::Serval serval;
    EXPECT_EQ("She is a serval", JapariPark::FindByKonoha(serval));
    std::string str;
    EXPECT_EQ("Give me a second helping, please", JapariPark::FindByKonoha(str));
}

TEST_F(TestSwitchCase, HashCode) {
    JapariPark::Animal animal;
    EXPECT_EQ("She is an animal.", JapariPark::FindByKonohaHashCode(animal));
    JapariPark::Cat cat;
    EXPECT_EQ("She is a cat.", JapariPark::FindByKonohaHashCode(cat));
    JapariPark::Serval serval;
    EXPECT_EQ("She is a serval.", JapariPark::FindByKonohaHashCode(serval));
    std::string str;
    EXPECT_EQ("Give me a second helping, please.", JapariPark::FindByKonohaHashCode(str));
}

TEST_F(TestSwitchCase, SwitchCase) {
    std::string str;
    EXPECT_EQ("", JapariPark::FindByKonohaSwitchCase(str));
}

TEST_F(TestSwitchCase, Any) {
    JapariPark::Animal animal;
    const JapariPark::Animal animalC;

    boost::any obj = animal;
    EXPECT_EQ("She is an animal!", JapariPark::FindByKonohaAny(obj));
    obj = animalC;
    EXPECT_EQ("She is an animal!", JapariPark::FindByKonohaAny(obj));

    JapariPark::Cat cat;
    const JapariPark::Cat catC;
    obj = cat;
    EXPECT_EQ("She is a cat!", JapariPark::FindByKonohaAny(obj));
    obj = catC;
    EXPECT_EQ("She is a cat!", JapariPark::FindByKonohaAny(obj));

    JapariPark::Serval serval;
    const JapariPark::Serval servalC;
    obj = serval;
    EXPECT_EQ("She is a serval!", JapariPark::FindByKonohaAny(obj));
    obj = servalC;
    EXPECT_EQ("She is a serval!", JapariPark::FindByKonohaAny(obj));

    std::string str;
    obj = str;
    EXPECT_EQ("Give me a second helping, please!", JapariPark::FindByKonohaAny(obj));
}

TEST_F(TestSwitchCase, ConstMemFn) {
    JapariPark::g_currentTimestamp = 0;
    JapariPark::ConstMemFn obj;
    ASSERT_ANY_THROW(obj.MakeSideEffect());
    EXPECT_TRUE(JapariPark::g_currentTimestamp);
}

// ものすごくコンパイルが遅い
template <typename Result>
class BaseDelayedFunction {
public:
    virtual ~BaseDelayedFunction(void) = default;
    virtual Result Exec(void) = 0;
    virtual const std::string& GetName(void) = 0;
    virtual const std::string& GetPrettyName(void) = 0;
};

template <typename Result, typename Enable = void, typename ... ArgTypes>
class DelayedFunction;

template <typename Result, typename ... ArgTypes>
class DelayedFunction<Result,
                      typename std::enable_if_t<std::is_pod<Result>::value, void>, ArgTypes...>
    : public BaseDelayedFunction<Result> {
public:
    using Function = Result(&)(ArgTypes...);
    DelayedFunction(const Function f, ArgTypes... args) : func_(std::bind(f, args...)) {}
    virtual ~DelayedFunction(void) = default;
    virtual Result Exec(void) override { return func_(); }
    virtual const std::string& GetName(void) override {
        name_ = __FUNCTION__;
        return name_;
    }
    virtual const std::string& GetPrettyName(void) override {
        prettyName_ = __PRETTY_FUNCTION__;
        return prettyName_;
    }
private:
    std::function<Result()> func_;
    std::string name_;
    std::string prettyName_;
};

template <typename Result, typename ... ArgTypes>
class DelayedFunction<Result,
                      typename std::enable_if_t<!std::is_pod<Result>::value, void>, ArgTypes...>
    : public BaseDelayedFunction<Result> {
public:
    using Function = Result(&)(ArgTypes...);
    DelayedFunction(const Function f, ArgTypes... args) : func_(std::bind(f, args...)) {}
    virtual ~DelayedFunction(void) = default;
    virtual Result Exec(void) override { return func_(); }
    virtual const std::string& GetName(void) override {
        name_ = __FUNCTION__;
        return name_;
    }
    virtual const std::string& GetPrettyName(void) override {
        prettyName_ = __PRETTY_FUNCTION__;
        return prettyName_;
    }
private:
    std::function<Result()> func_;
    std::string name_;
    std::string prettyName_;
};

// && 対応は別途
namespace {
    template <typename Result, typename ... ArgTypes>
    auto CreateDelayedFunction(Result(&f)(ArgTypes...), ArgTypes... args) {
        std::unique_ptr<BaseDelayedFunction<Result>> func(new DelayedFunction<Result, void, ArgTypes...>(f, args...));
        return func;
    }

    int intSum3(int a, int b, int c) {
        return a + b + c;
    }

    int intProduce5(int a, int b, int c, int d, int e) {
        return a * b * c * d * e;
    }

    using ClassInteger = boost::multiprecision::int1024_t;
    ClassInteger intNegate(ClassInteger a) {
        return -a;
    }
}

class TestFunctionName : public ::testing::Test {};

TEST_F(TestFunctionName, IntSum) {
    int a = 2;
    int b = 5;
    int c = 7;
    auto f = CreateDelayedFunction(intSum3, a, b, c);
    EXPECT_EQ(14, f->Exec());
    // GetName
    // const string& DelayedFunction<Result, typename std::enable_if<std::is_pod<_Tp>::value, void>::type, ArgTypes ...>::GetPrettyName() [with Result = int; ArgTypes = {int, int, int}; typename std::enable_if<std::is_pod<_Tp>::value, void>::type = void; std::string = std::basic_string<char>]
    std::cout << f->GetName() << "\n" << f->GetPrettyName()<< "\n";
}

TEST_F(TestFunctionName, IntProduct) {
    auto f = CreateDelayedFunction(intProduce5, 2, 3, 5, 7, 11);
    EXPECT_EQ(2310, f->Exec());
    // GetName
    // const string& DelayedFunction<Result, typename std::enable_if<std::is_pod<_Tp>::value, void>::type, ArgTypes ...>::GetPrettyName() [with Result = int; ArgTypes = {int, int, int, int, int}; typename std::enable_if<std::is_pod<_Tp>::value, void>::type = void; std::string = std::basic_string<char>]
    std::cout << f->GetName() << "\n" << f->GetPrettyName()<< "\n";
}

TEST_F(TestFunctionName, CppInt) {
    ClassInteger c = -1;
    auto f = CreateDelayedFunction(intNegate, c);
    EXPECT_EQ(1, f->Exec());
    // GetName
    // const string& DelayedFunction<Result, typename std::enable_if<(! std::is_pod<_Tp>::value), void>::type, ArgTypes ...>::GetPrettyName() [with Result = boost::multiprecision::number<boost::multiprecision::backends::cpp_int_backend<1024u, 1024u, (boost::multiprecision::cpp_integer_type)1u, (boost::multiprecision::cpp_int_check_type)0u, void> >; ArgTypes = {boost::multiprecision::number<boost::multiprecision::backends::cpp_int_backend<1024u, 1024u, (boost::multiprecision::cpp_integer_type)1u, (boost::multiprecision::cpp_int_check_type)0u, void>, (boost::multiprecision::expression_template_option)0u>}; typename std::enable_if<(! std::is_pod<_Tp>::value), void>::type = void; std::string = std::basic_string<char>]
    std::cout << f->GetName() << "\n" << f->GetPrettyName()<< "\n";
}

namespace {
    struct MyConstBoolObject {
        constexpr MyConstBoolObject(bool b) : b_{b} {}
        operator bool() const { return b_; }
        const bool b_ {false};
    };

    bool g_assertFlag = false;

    void SetAssertFlag(void) {
        g_assertFlag = true;
    }

    bool SetAndClearAssertFlag(void) {
        bool flag = false;
        std::swap(flag, g_assertFlag);
        return flag;
    }

    void MyAssertImpl(void) {
        SetAssertFlag();
    }

    template <typename T>
    void MyAssert(T&& cond) {
        if (cond) {
            return;
        }
        MyAssertImpl();
    }
}

class TestTemplateAssert : public ::testing::Test {
    virtual void SetUp() override {
        SetAndClearAssertFlag();
    }
};

TEST_F(TestTemplateAssert, TemplateTrue) {
    // lvalue
    bool b = true;
    MyAssert(b);
    EXPECT_FALSE(SetAndClearAssertFlag());

    int c = 1;
    MyAssert(c);
    EXPECT_FALSE(SetAndClearAssertFlag());

    MyConstBoolObject obj(true);
    MyAssert(obj);
    EXPECT_FALSE(SetAndClearAssertFlag());

    auto p = &c;
    MyAssert(p);
    EXPECT_FALSE(SetAndClearAssertFlag());

    // rvalue
    MyAssert(2);
    EXPECT_FALSE(SetAndClearAssertFlag());

    MyAssert(MyConstBoolObject(true));
    EXPECT_FALSE(SetAndClearAssertFlag());

    MyAssert(&c);
    EXPECT_FALSE(SetAndClearAssertFlag());

    // 式
    MyAssert(MyConstBoolObject(c < 2));
    EXPECT_FALSE(SetAndClearAssertFlag());
}

TEST_F(TestTemplateAssert, ConstTrue) {
    const bool b = true;
    MyAssert(b);
    EXPECT_FALSE(SetAndClearAssertFlag());

    const int c = 1;
    MyAssert(c);
    EXPECT_FALSE(SetAndClearAssertFlag());

    const MyConstBoolObject obj(true);
    MyAssert(obj);
    EXPECT_FALSE(SetAndClearAssertFlag());
}

TEST_F(TestTemplateAssert, TemplateFalse) {
    bool b = false;
    MyAssert(b);
    EXPECT_TRUE(SetAndClearAssertFlag());

    int c = 0;
    MyAssert(c);
    EXPECT_TRUE(SetAndClearAssertFlag());

    MyConstBoolObject obj(false);
    MyAssert(obj);
    EXPECT_TRUE(SetAndClearAssertFlag());

    void* p = nullptr;
    MyAssert(p);
    EXPECT_TRUE(SetAndClearAssertFlag());

    MyAssert(0);
    EXPECT_TRUE(SetAndClearAssertFlag());

    MyAssert(MyConstBoolObject(false));
    EXPECT_TRUE(SetAndClearAssertFlag());

    MyAssert(nullptr);
    EXPECT_TRUE(SetAndClearAssertFlag());

    // 式
    MyAssert(MyConstBoolObject(c > 0));
    EXPECT_TRUE(SetAndClearAssertFlag());
}

TEST_F(TestTemplateAssert, ConstFalse) {
    const bool b = false;
    MyAssert(b);
    EXPECT_TRUE(SetAndClearAssertFlag());

    const int c = 0;
    MyAssert(c);
    EXPECT_TRUE(SetAndClearAssertFlag());

    const MyConstBoolObject obj(false);
    MyAssert(obj);
    EXPECT_TRUE(SetAndClearAssertFlag());
}

namespace {
    void MyAssertByBool(bool cond) {
        if (cond) {
            return;
        }
        MyAssertImpl();
    }
}

TEST_F(TestTemplateAssert, BoolTrue) {
    // lvalue
    bool b = true;
    MyAssertByBool(b);
    EXPECT_FALSE(SetAndClearAssertFlag());

    int c = 1;
    MyAssertByBool(c);
    EXPECT_FALSE(SetAndClearAssertFlag());

    MyConstBoolObject obj(true);
    MyAssertByBool(obj);
    EXPECT_FALSE(SetAndClearAssertFlag());

    auto p = &c;
    MyAssertByBool(p);
    EXPECT_FALSE(SetAndClearAssertFlag());

    // rvalue
    MyAssertByBool(2);
    EXPECT_FALSE(SetAndClearAssertFlag());

    MyAssertByBool(MyConstBoolObject(true));
    EXPECT_FALSE(SetAndClearAssertFlag());

    // &vが常にtrueと評価されると警告が出る
    // MyAssertByBool(&c);
    // EXPECT_FALSE(SetAndClearAssertFlag());

    // 式
    MyAssert(MyConstBoolObject(c < 2));
    EXPECT_FALSE(SetAndClearAssertFlag());
}

TEST_F(TestTemplateAssert, BoolFalse) {
    bool b = false;
    MyAssertByBool(b);
    EXPECT_TRUE(SetAndClearAssertFlag());

    int c = 0;
    MyAssertByBool(c);
    EXPECT_TRUE(SetAndClearAssertFlag());

    MyConstBoolObject obj(false);
    MyAssertByBool(obj);
    EXPECT_TRUE(SetAndClearAssertFlag());

    // boolに変換できる
    void* p = nullptr;
    MyAssertByBool(p);
    EXPECT_TRUE(SetAndClearAssertFlag());

    MyAssertByBool(0);
    EXPECT_TRUE(SetAndClearAssertFlag());

    MyAssertByBool(MyConstBoolObject(false));
    EXPECT_TRUE(SetAndClearAssertFlag());

    // nullptrをboolに変換できないのでエラーになる
    // MyAssertByBool(nullptr);

    // 式
    MyAssert(MyConstBoolObject(c > 0));
    EXPECT_TRUE(SetAndClearAssertFlag());
}

namespace {
    void MyAssertByInt(int cond) {
        if (cond) {
            return;
        }
        MyAssertImpl();
    }
}

TEST_F(TestTemplateAssert, Int) {
    bool b = true;
    MyAssertByInt(b);
    EXPECT_FALSE(SetAndClearAssertFlag());

    MyConstBoolObject objBool(true);
    MyAssertByInt(objBool);
    EXPECT_FALSE(SetAndClearAssertFlag());

    int c = 1;
    MyAssertByInt(c);
    EXPECT_FALSE(SetAndClearAssertFlag());
    MyAssertByInt(1);
    EXPECT_FALSE(SetAndClearAssertFlag());

    // intに変換できないのでコンパイルエラーになる
    // MyAssertByInt(&c);
    // void* p = nullptr;
    // MyAssertByInt(p);
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
