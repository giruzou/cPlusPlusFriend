#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <algorithm>
#include <array>
#include <atomic>
#include <fstream>
#include <future>
#include <iostream>
#include <random>
#include <regex>
#include <string>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/any.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/io/ios_state.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/type_traits/function_traits.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <windows.h>

namespace {
    void TrimAndExpand(std::string& str) {
        boost::replace_all(str, "\r\n", " ");
        boost::replace_all(str, "\r", " ");
        boost::replace_all(str, "\n", " ");
        boost::trim(str);
        return;
    }
}

class Train {
    // すごーい! シリアライザはクラスを永続化できるフレンズなんだね
    friend std::ostream& operator <<(std::ostream& os, const Train& train);
    friend std::istream& operator >>(std::istream& is, Train& train);
    friend boost::serialization::access;

    // +演算子は列車を連結できるフレンズなんだね
    friend Train operator +(const Train& left, const Train& right) {
        Train train(left.name_ + "-" + right.name_, left.type_);
        return train;
    }

    // std::swapはメンバ変数を一括置換できるフレンズなんだね。おもしろーい！
    template<typename T>
    friend void std::swap(T& a, T& b);

    // ユニットテストはprivateメンバを読めるフレンズなんだね。たーのしー!
    FRIEND_TEST(TestSerialization, Initialize);
    FRIEND_TEST(TestSerialization, Concatenate);
    FRIEND_TEST(TestSerialization, ToString);
    FRIEND_TEST(TestSerialization, Std);
    FRIEND_TEST(TestSerialization, Boost);
    FRIEND_TEST(TestSerialization, StdInvalid);
    FRIEND_TEST(TestSerialization, BoostInvalidOut);
    FRIEND_TEST(TestSerialization, BoostInvalidIn);

public:
    // 君ははまなす廃止後、定期運行されている急行がないと知っているフレンズなんだね
    enum class Type { Local, Rapid, Limited_Express };
    Train(void) = default;
    Train(const std::string& name, Type type) : name_(name), type_(type) {
        TrimAndExpand(name_);
    }

    virtual ~Train(void) = default;  // {}ではない

    std::string ToString(void) const {
        std::string str;
        auto i = std::find_if(descriptions_.begin(), descriptions_.end(),
                              [=](auto& s) { return (s.first == type_); });
        std::string desc = (i != descriptions_.end()) ? i->second : "??";
        str = name_ + "は" + desc + "で走る列車なんだね";
        return str;
    }

private:
    // Effective Modern C++ 項目17を参照
    Train(const Train&) = default;
    Train& operator =(const Train&) = default;
    Train(Train&&) = default;
    Train& operator =(Train&&) = default;

    class InvalidValue : public std::ios_base::failure {
    public:
        explicit InvalidValue(const std::string& message) : std::ios_base::failure(message) {}
    };

    // 例外は、返り値が得られないことを教えてくれるフレンズなんだね
    // boost::optionalという手もありますけど
    static Type toType(std::underlying_type<Type>::type type) {
        if (std::none_of(
                descriptions_.begin(), descriptions_.end(),
                [=](auto& s) {
                    return (static_cast<decltype(type)>(s.first) == type);
                })) {
            std::string message = "Invalid number "
                + boost::lexical_cast<decltype(message)>(type);

            throw InvalidValue(message);
        }

        return static_cast<Type>(type);
    };

    template<typename T>
    void serialize(T& archive, const unsigned int version) {
        auto original = *this;
        archive & name_;

        try {
            // すごーい! Boost Serializationはenumをキャストなしで読み書きできるフレンズなんだね
            // stdはstd::underlying_typeにしないと、>>で読み書きしたり、
            // unorderedのキーにしたりできないみたいです
            archive & type_;
            toType(static_cast<boost::function_traits<decltype(toType)>::arg1_type>(type_));
        } catch(InvalidValue& e) {
            // type_が得られないときはロールバックする
            std::swap(*this, original);
            throw e;
            // 読み込み失敗による例外は、そのまま呼び出し元に伝える
        }

        return;
    }

    // メンバがすべてmove assignableかつmove constructibleだから、このクラスはswappable
    std::string name_;
    Type type_ {Type::Local};
    // 種別を読めるようにする
    static const std::vector<std::pair<Type, std::string>> descriptions_;
};

const std::vector<std::pair<Train::Type, std::string>> Train::descriptions_ {
    {Train::Type::Local, "普通"},
    {Train::Type::Rapid, "快速"},
    {Train::Type::Limited_Express, "特急"}};

std::ostream& operator <<(std::ostream& os, const Train& train) {
    // osが例外を出すよう設定するが後で戻す
    boost::io::ios_flags_saver saver(os);
    os.exceptions(std::ios::failbit | std::ios::badbit);

    using IntType = std::underlying_type<decltype(train.type_)>::type;
    // 列車名に改行がないと想定しているが、あったら空白に置き換える
    std::string name = train.name_;
    TrimAndExpand(name);
    os << name << "\n";
    os << static_cast<IntType>(train.type_);
    return os;
}

std::istream& operator >>(std::istream& is, Train& train) {
    // isが例外を出すよう設定するが後で戻す
    boost::io::ios_flags_saver saver(is);
    is.exceptions(std::ios::failbit | std::ios::badbit);

    Train t;
    // 改行コードの違うプラットフォーム超えてデータを運ぶことは想定していない
    std::getline(is, t.name_);

    // ストリームから読み込んだ内容は戻さないので、基本例外安全とはいえても
    // 強い例外安全とは言えない
    std::underlying_type<decltype(train.type_)>::type type;
    is >> type;
    t.type_ = Train::toType(type);

    // std::swapはno-throw保証なので、同時に強い例外安全でもある
    std::swap(train, t);
    // 君は全メンバの読み込みに成功したオブジェクトを返すフレンズなんだね
    return is;
}

template<typename Derived>
class RandomNumber {
public:
    RandomNumber(void) = default;
    uint_fast32_t Get(void) {
        return static_cast<Derived*>(this)->impl();
    }

    RandomNumber(const RandomNumber&) = delete;
    RandomNumber& operator =(const RandomNumber&) = delete;
};

class SoftwareRand : public RandomNumber<SoftwareRand> {
public:
    SoftwareRand(void) : gen(rd()) {}
private:
    // すごーい! CRTPは静的ポリモーフィズムができるフレンズなんだね
    friend class RandomNumber<SoftwareRand>;
    std::result_of<decltype(
        &RandomNumber<SoftwareRand>::Get)(RandomNumber<SoftwareRand>)>::type impl() {
        return gen();
    }

    std::random_device rd;
    std::mt19937 gen;
};

class HardwareRand : public RandomNumber<HardwareRand> {
public:
    HardwareRand(void) {}
private:
    friend class RandomNumber<HardwareRand>;
    std::result_of<decltype(
        &RandomNumber<SoftwareRand>::Get)(RandomNumber<SoftwareRand>)>::type impl() {
        uint32_t number = 0;
        asm volatile (
            "rdrand %0 \n\t"
            :"=r"(number)::);
        return number;
    }
};

class TestSerialization : public ::testing::Test{};

TEST_F(TestSerialization, Initialize) {
    Train train;
    EXPECT_TRUE(train.name_.empty());
    EXPECT_EQ(Train::Type::Local, train.type_);

    const std::string rapidName = "Acty";
    constexpr Train::Type rapidType = Train::Type::Rapid;
    Train rapid(rapidName, rapidType);
    EXPECT_EQ(rapidName, rapid.name_);
    EXPECT_EQ(rapidType, rapid.type_);

    const std::string ltdExpressName = " フラノラベンダー\nエクスプレス ";
    constexpr Train::Type ltdExpressType = Train::Type::Limited_Express;
    Train ltdExpress(ltdExpressName, ltdExpressType);
    EXPECT_EQ("フラノラベンダー エクスプレス", ltdExpress.name_);
    EXPECT_EQ(ltdExpressType, ltdExpress.type_);
}

TEST_F(TestSerialization, Concatenate) {
    constexpr Train::Type type = Train::Type::Limited_Express;
    const std::string leftName =  "Sunrise Seto";
    const std::string rightName = "Sunrise Izumo";
    const Train left(leftName, type);
    const Train right(rightName, type);

    const Train ltdExpress = left + right;
    EXPECT_EQ("Sunrise Seto-Sunrise Izumo", ltdExpress.name_);
    EXPECT_EQ(type, ltdExpress.type_);
}

TEST_F(TestSerialization, ToString) {
    const Train local("いさぶろう", Train::Type::Local);
    const Train rapid("ラビット", Train::Type::Rapid);
    const Train ltdExpress("サンダーバード", Train::Type::Limited_Express);

    EXPECT_EQ("いさぶろうは普通で走る列車なんだね", local.ToString());
    EXPECT_EQ("ラビットは快速で走る列車なんだね", rapid.ToString());
    EXPECT_EQ("サンダーバードは特急で走る列車なんだね", ltdExpress.ToString());

    Train outOfService("回送", Train::Type::Local);
    outOfService.type_ = static_cast<Train::Type>(3);
    EXPECT_EQ("回送は??で走る列車なんだね", outOfService.ToString());
}

TEST_F(TestSerialization, Std) {
    {
        const std::string name = "Sunrise Seto";
        const Train::Type type = Train::Type::Limited_Express;

        const Train original(name, type);
        std::stringstream ss;
        ss << original;

        Train restored;
        ss >> restored;
        EXPECT_EQ(name, restored.name_);
        EXPECT_EQ(type, restored.type_);
    }

    {
        const std::string name = " Tottori\nLiner ";
        constexpr Train::Type type = Train::Type::Rapid;

        const Train original(name, type);
        std::stringstream ss;
        ss << original;

        Train restored;
        ss >> restored;
        EXPECT_EQ("Tottori Liner", restored.name_);
        EXPECT_EQ(type, restored.type_);
    }
}

TEST_F(TestSerialization, Boost) {
    const std::string name = "Sunrise Izumo";
    constexpr Train::Type type = Train::Type::Limited_Express;

    const Train original(name, type);
    std::stringstream ss;
    boost::archive::text_oarchive oarch(ss);
    oarch << original;

    Train restored;
    boost::archive::text_iarchive iarch(ss);
    iarch >> restored;
    EXPECT_EQ(name, restored.name_);
    EXPECT_EQ(type, restored.type_);
}

TEST_F(TestSerialization, StdInvalid) {
    const std::string name = "Unnamed";
    {
        const std::string arg = name + "\n3";
        std::istringstream is(arg);
        Train notRestored;
        std::string actual;

        try {
            is >> notRestored;
        } catch(Train::InvalidValue& e) {
            // 入力を適切にenumへとキャストできなかった
            actual = e.what();
        }

        const std::string expected = "Invalid number 3";
        EXPECT_EQ(expected, actual);
    }

    {
        std::istringstream is(name);
        Train notRestored;
        std::string actual;

        try {
            is >> notRestored;
        } catch(std::exception& e) {
            // 入力が短すぎて読めなかった
            actual = e.what();
        }

        EXPECT_FALSE(actual.empty());
    }
}

TEST_F(TestSerialization, BoostInvalidOut) {
    const std::string name = "Unnamed";
    const Train::Type type = static_cast<Train::Type>(3);
    const Train original(name, type);
    std::ostringstream os;
    std::string actual;

    try {
        boost::archive::text_oarchive oarch(os);
        oarch << original;
    } catch(Train::InvalidValue& e) {
        // 出力値が壊れている
        actual = e.what();
    }

    const std::string expected = "Invalid number 3";
    EXPECT_EQ(expected, actual);
}

TEST_F(TestSerialization, BoostInvalidIn) {
    const std::string name = "Unnamed";
    const Train::Type type = Train::Type::Local;
    const Train original(name, type);
    std::ostringstream os;
    boost::archive::text_oarchive oarch(os);
    oarch << original;

    auto baseText = os.str();
    baseText.pop_back();

    const std::vector<std::pair<std::string, std::string>> testCases {
        {"3", "Invalid number 3"}, {"A", "input stream error"}};

    for(auto& test : testCases) {
        const std::string text = baseText + test.first;
        std::istringstream is(text);
        std::string actual;
        Train notRestored;

        try {
            boost::archive::text_iarchive iarch(is);
            iarch >> notRestored;
        } catch(std::exception& e) {
            // 入力が壊れている
            actual = e.what();
        }

        EXPECT_EQ(test.second, actual);
    }
}

class TestRandomNumber : public ::testing::Test{};

namespace {
    using Count = size_t;
    template<typename T,
             typename R = typename std::result_of<decltype(&T::Get)(T)>::type>
    void CountRandomNumber(T& randomNumber, std::ostream& os) {
        std::unordered_map<R, bool> board;
        Count count=0;
        for(count=0; count<10; ++count) {
            os << randomNumber.Get() << ":";
        }
        os << "\n";

        for(count=0; count<1000000; ++count) {
            board[randomNumber.Get()] = true;
        }

        std::cout << board.size() << "/" << count << " numbers are found \n";
        EXPECT_LE((count / 100) * 99, count);
        return;
    }
}

TEST_F(TestRandomNumber, List) {
    SoftwareRand sr;
    HardwareRand hr;
    CountRandomNumber(sr, std::cout);
    CountRandomNumber(hr, std::cout);
}

namespace {
    int arg1(int a) {
        return a + 2;
    }

    int arg2(int a, int b) {
        return a * b + 2;
    }

    using Arg1Type  = int(*)(int);
    using Arg2Type  = int(*)(int, int);
}

class TestFuncAnyCast : public ::testing::Test{};

TEST_F(TestFuncAnyCast, Initialize) {
    boost::any func = arg1;
    auto result1 = (boost::any_cast<Arg1Type>(func))(10);
    EXPECT_EQ(12, result1);

    func = arg2;
    auto result2 = (boost::any_cast<Arg2Type>(func))(3, 5);
    EXPECT_EQ(17, result2);
}

class MyCounter {
public:
    using Number = int;
    MyCounter(Number count) : count_(count) {}
    virtual ~MyCounter(void) = default;

    void Run(void) {
        // コンパイラが最適化すれば、足した結果をまとめてnonVolatileCounter_に格納する可能性がある
        for(Number i = 0; i < count_; ++i) {
            ++nonVolatileCounter_;
        }

        // volatileはatomicではないので競合する可能性がある
        for(Number i = 0; i < count_; ++i) {
            ++volatileCounter_;
            ++atomicCounter_;
        }
    }

    static void Reset(void) {
        nonVolatileCounter_ = 0;
        volatileCounter_ = 0;
        atomicCounter_ = 0;
    }

    static Number GetValue(void) {
        return nonVolatileCounter_;
    }

    static Number GetVolatileValue(void) {
        return volatileCounter_;
    }

    static Number GetAtomicValue(void) {
        return atomicCounter_;
    }

private:
    Number count_ {0};
    static Number nonVolatileCounter_;
    static volatile Number volatileCounter_;
    static std::atomic<Number> atomicCounter_;
};

MyCounter::Number MyCounter::nonVolatileCounter_ {0};
volatile MyCounter::Number MyCounter::volatileCounter_ {0};
std::atomic<MyCounter::Number> MyCounter::atomicCounter_ {0};

class TestMyCounter : public ::testing::Test {
protected:
    using SizeOfThreads = int;

    virtual void SetUp() override {
        MyCounter::Reset();
        hardwareConcurrency_ = static_cast<decltype(hardwareConcurrency_)>(std::thread::hardware_concurrency());
        processAffinityMask_ = 0;
        systemAffinityMask_ = 0;

        // https://msdn.microsoft.com/ja-jp/library/cc429135.aspx
        // https://msdn.microsoft.com/ja-jp/library/windows/desktop/ms683213(v=vs.85).aspx
        // で引数の型が異なる。下が正しい。
        if (!GetProcessAffinityMask(GetCurrentProcess(), &processAffinityMask_, &systemAffinityMask_)) {
            // 取得に失敗したらシングルコアとみなす
            std::cout << "GetProcessAffinityMask failed\n";
            processAffinityMask_ = 1;
            systemAffinityMask_ = 1;
       }
    }

    virtual void TearDown() override {
        if (!SetProcessAffinityMask(GetCurrentProcess(), processAffinityMask_)) {
            std::cout << "Restoring ProcessAffinityMask failed\n";
        } else {
            std::cout << "ProcessAffinityMask restored\n";
        }
    }

    void runCounters(SizeOfThreads sizeOfThreads, MyCounter::Number count) {
        std::vector<std::unique_ptr<MyCounter>> counterSet;
        std::vector<std::future<void>> futureSet;

        // 並行処理を作って、後で実行できるようにする
        for(decltype(sizeOfThreads) index = 0; index < sizeOfThreads; ++index) {
            std::unique_ptr<MyCounter> pCounter(new MyCounter(count));
            MyCounter* pRawCounter = pCounter.get();
            futureSet.push_back(std::async(std::launch::async, [=](void) -> void { pRawCounter->Run(); }));
            counterSet.push_back(std::move(pCounter));
        }

        // 並行して評価して、結果がそろうのを待つ
        for(auto& f : futureSet) {
            f.get();
        }

        std::cout << "MyCounter::GetValue() = " << MyCounter::GetValue() << "\n";
        std::cout << "MyCounter::GetVolatileValue() = " << MyCounter::GetVolatileValue() << "\n";
        std::cout << "MyCounter::GetAtomicValue() = " << MyCounter::GetAtomicValue() << "\n";
        return;
    }

    class IntBox {
    public:
        using Data = int;
        IntBox(Data n) : var_(n), mvar_(n), cvar_(n) {}
        virtual ~IntBox() = default;

        Data Get(void) const { return var_; }
        Data GetMutableVar(void) const { return mvar_; }
        Data GetConstVar(void) const { return cvar_; }
        void Set(Data n) { var_ = n; }
        void SetMutableVar(Data n) const { mvar_ = n; }

        // コンパイラはメンバ変数が不変だとは断定できないので、動作するだろう
        // Exceptional C++ Style 項目24を参照
        void Overwrite1(Data n) const {
            const_cast<IntBox*>(this)->var_ = n;
        }

        void Overwrite2(Data n) const {
            *(const_cast<Data*>(&var_)) = n;
            *(const_cast<Data*>(&cvar_)) = n;
        }

    private:
        Data var_;
        mutable Data mvar_;
        const Data cvar_;
    };

    SizeOfThreads hardwareConcurrency_ {0};

private:
    DWORD_PTR processAffinityMask_ {1};
    DWORD_PTR systemAffinityMask_  {1};
};

TEST_F(TestMyCounter, MultiCore) {
    if (hardwareConcurrency_ <= 1) {
        return;
    }

    const MyCounter::Number count = 1000000;  // 十分大きくする
    SizeOfThreads sizeOfThreads = hardwareConcurrency_;
    std::cout << "Run on multi threading\n";
    runCounters(sizeOfThreads, count);

    MyCounter::Number expected = static_cast<decltype(expected)>(sizeOfThreads) * count;
    // 最適化しなければこうはならない
#ifdef CPPFRIENDS_NO_OPTIMIZATION
    EXPECT_GT(expected, MyCounter::GetValue());
#else
    EXPECT_EQ(expected, MyCounter::GetValue());
#endif
    // マルチコアなら競合が起きるので成り立つはずだが、条件次第では競合しない可能性もある
    EXPECT_GT(expected, MyCounter::GetVolatileValue());
    // これは常に成り立つはず
    EXPECT_EQ(expected, MyCounter::GetAtomicValue());
}

TEST_F(TestMyCounter, SingleCore) {
    /* 使うCPUを1個に固定する */
    DWORD_PTR procMask = 1;
    if (!SetProcessAffinityMask(GetCurrentProcess(), procMask)) {
        std::cout << "SetProcessAffinityMask failed\n";
    }

    SizeOfThreads sizeOfThreads = std::max(hardwareConcurrency_, 4);
    const MyCounter::Number count = 1000000;
    std::cout << "Run on a single thread\n";
    runCounters(sizeOfThreads, count);

    MyCounter::Number expected = static_cast<decltype(expected)>(sizeOfThreads) * count;
    EXPECT_EQ(expected, MyCounter::GetValue());
    // シングルコアなら競合しない
    EXPECT_EQ(expected, MyCounter::GetVolatileValue());
    // これは常に成り立つはず
    EXPECT_EQ(expected, MyCounter::GetAtomicValue());
}

TEST_F(TestMyCounter, ConstMemberFunction) {
    IntBox::Data n = 1;
    IntBox box {n};
    EXPECT_EQ(n, box.Get());
    EXPECT_EQ(n, box.GetMutableVar());
    EXPECT_EQ(n, box.GetConstVar());

    ++n;
    box.Set(n);
    EXPECT_EQ(n, box.Get());

    ++n;
    box.SetMutableVar(n);
    EXPECT_EQ(n, box.GetMutableVar());

    ++n;
    box.Overwrite1(n);
    EXPECT_EQ(n, box.Get());

    ++n;
    box.Overwrite2(n);
    EXPECT_EQ(n, box.Get());
    EXPECT_EQ(n, box.GetConstVar());
}

class NaiveCopy {
public:
    NaiveCopy(void) = default;
    virtual ~NaiveCopy(void) = default;

    void Add(int e) {
        array_.push_back(e);
    }

    int At(size_t i) {
        return array_.at(i);
    }

    void CopyTo(NaiveCopy& other) const {
        // this == otherのときは、otherつまりthisの要素を消してしまう!
        // メンバ関数宣言にconstがあっても無駄である
        other.array_.clear();

        // otherを消してしまったのなら、何もコピーされない
        for(const auto& e : array_) {
            other.array_.push_back(e);
        }
    }

    void CopyTo_s(NaiveCopy& other) const {
        if (this == &other) {
            return;
        }

        for(const auto& e : array_) {
            other.array_.push_back(e);
        }
    }

private:
    std::vector<int> array_;
};

class SubNaiveCopy : public NaiveCopy {};

class TestNaiveCopy : public ::testing::Test {};

TEST_F(TestNaiveCopy, CopyTo) {
    NaiveCopy original;
    original.Add(1);
    original.Add(2);
    EXPECT_EQ(2, original.At(1));

    NaiveCopy other;
    original.CopyTo(other);
    EXPECT_EQ(2, original.At(1));
    EXPECT_EQ(2, other.At(1));

    original.CopyTo(original);
    ASSERT_ANY_THROW(original.At(1));
}

TEST_F(TestNaiveCopy, CopyTo_s) {
    NaiveCopy original;
    original.Add(1);
    original.Add(2);
    EXPECT_EQ(2, original.At(1));
    original.CopyTo_s(original);
    EXPECT_EQ(2, original.At(1));
}

class TestTypeId : public ::testing::Test {};

TEST_F(TestTypeId, Compare) {
    EXPECT_EQ(typeid(NaiveCopy), typeid(NaiveCopy));
    EXPECT_EQ(typeid(const NaiveCopy), typeid(NaiveCopy));
    EXPECT_NE(typeid(SubNaiveCopy), typeid(NaiveCopy));
}

class TestThreads : public ::testing::Test {};

TEST_F(TestThreads, All) {
    struct ThreadEnv {
        int value;
    };

    ThreadEnv env1 {1};
    ThreadEnv env2 {2};
    {
        std::thread thr1([&env1](void) { std::cout << env1.value; });
        std::thread thr2([&env2](void) { std::cout << env2.value; });
        thr1.join();
        thr2.join();
    }

    std::cout << "\n";
}

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

class TestZeroInitialize : public ::testing::Test {
protected:
    virtual void SetUp() override {
        // リセット
        decltype(os_) os;
        os_.swap(os);
    }

    // PODの配列
    template <typename T, size_t S>
    typename std::enable_if_t<std::is_pod<T>::value, void>
    ZeroInitialize(T (&argArray)[S]) {
        // どのパターンにマッチングしたか記録を残す
        os_ << "array(" << sizeof(argArray) << "),";
        // 配列の先頭要素だけでなく、配列全体のサイズ
        ::memset(argArray, 0, sizeof(argArray));
        return;
    }

    // nullptr
    template <typename T>
    typename std::enable_if_t<std::is_null_pointer<T>::value, void>
    ZeroInitialize(T) {
        os_ << "nullptr,";
        return;
    }

    // PODを指すポインタ
    template <typename T>
    typename std::enable_if_t<std::is_pointer<T>::value && std::is_pod<std::remove_pointer_t<T>>::value, void>
    ZeroInitialize(T pObject) {
        if (pObject) {
            os_ << "pointer(" << sizeof(*pObject) << "),";
            ::memset(pObject, 0, sizeof(*pObject));
        } else {
            os_ << "null-pointer,";
        }
        return;
    }

    // PODへの参照
    template <typename T>
    typename std::enable_if_t<!std::is_pointer<T>::value && std::is_pod<T>::value, void>
               ZeroInitialize(T& object) {
        os_ << "reference(" << sizeof(object) << "),";
        ::memset(&object, 0, sizeof(object));
        return;
    }

    // NULL=0は何もしない
    // 0以外の整数も無意味なので何もしない(本当はコンパイルエラーにしたい)
    template <typename T>
    typename std::enable_if_t<std::is_integral<T>::value, void>
    ZeroInitialize(const T&) {
        os_ << "integral,";
        return;
    }

    // 呼び出しログ
    std::ostringstream os_;

    class Inner {
    public:
        Inner(void) = default;
        Inner(int a) : a_(a) {}
        virtual ~Inner(void) = default;
        virtual int Get(void) const { return a_; }
    private:
        int a_ {0};
    };
};

TEST_F(TestZeroInitialize, All) {
    std::string expected;

    short primitive = 1;
    {
        EXPECT_TRUE(primitive);
        ZeroInitialize(primitive);
        EXPECT_FALSE(primitive);
        expected += "reference(2),";

        primitive = 2;
        EXPECT_TRUE(primitive);
        ZeroInitialize(&primitive);
        EXPECT_FALSE(primitive);
        expected += "pointer(2),";
    }

    constexpr size_t arraySize = 5;
    {
        int array1[arraySize] {2,3,4,5,6};
        ZeroInitialize(array1);
        for(const auto e : array1) {
            EXPECT_FALSE(e);
        }
        expected += "array(20),";
    }
    {
        int array2[arraySize] {2,3,4,5,6};
        ZeroInitialize(&array2[0]);
        EXPECT_FALSE(array2[0]);
        for(size_t i = 1; i < arraySize; ++i) {
            EXPECT_TRUE(array2[i]);
        }
        expected += "pointer(4),";

        ZeroInitialize(array2[arraySize - 1]);
        EXPECT_FALSE(array2[arraySize - 1]);
        expected += "reference(4),";
    }
    {
        int v = 0;
        int* arrayP[arraySize] {&v,&v,&v,&v,&v};
        ZeroInitialize(arrayP);
        for(const auto e : arrayP) {
            EXPECT_FALSE(e);
        }
        // 32bit環境では異なる
        expected += "array(40),";
    }

    struct Data {
        int m1;
        int* p;
        int m2;
    };
    {
        Data data1 {1, nullptr, 2};
        data1.p = &data1.m1;
        ZeroInitialize(data1);
        EXPECT_FALSE(data1.m1);
        EXPECT_FALSE(data1.p);
        EXPECT_FALSE(data1.m2);
        // 32bit環境では異なる
        expected += "reference(24),";
    }
    {
        Data data2 {3, nullptr, 4};
        data2.p = &data2.m1;
        ZeroInitialize(&data2);
        EXPECT_FALSE(data2.m1);
        EXPECT_FALSE(data2.p);
        EXPECT_FALSE(data2.m2);
        expected += "pointer(24),";
    }
    {
        Data dataArray[] {{1, nullptr, 2}, {3, nullptr, 4}};
        ZeroInitialize(dataArray);
        for(const auto& e : dataArray) {
            EXPECT_FALSE(e.m1);
            EXPECT_FALSE(e.p);
            EXPECT_FALSE(e.m2);
        }
        expected += "array(48),";
    }

    static_assert(!std::is_pointer<decltype(nullptr)>::value, "nullptr is a pointer");
    static_assert(std::is_integral<decltype(NULL)>::value, "NULL is a number");
    static_assert(!std::is_pod<Inner>::value, "Inner is a POD type");

    ZeroInitialize(nullptr);
    expected += "nullptr,";
    Data* pNull = nullptr;
    ZeroInitialize(pNull);
    expected += "null-pointer,";

    ZeroInitialize(NULL);
    ZeroInitialize(0);
    ZeroInitialize(1);
    ZeroInitialize(0x123456789ull);
    expected += "integral,integral,integral,integral,";

    // vtableへのポインタをクリアすると異常になるので、コンパイルエラーにする
#if 0
    Inner inner(1);
    Inner innerArray[2];
    ZeroInitialize(inner);
    ZeroInitialize(&inner);
    ZeroInitialize(innerArray);
#endif

    EXPECT_EQ(expected, os_.str());
}

// strをosoutとoserrの両方に書き出す
#define ILL_DEBUG_PRINT(osout, oserr, str) \
    osout << str << "@" << __PRETTY_FUNCTION__ << ":"; \
    oserr << str << "@" << __PRETTY_FUNCTION__ << ":"; \

#define WELL_DEBUG_PRINT(osout, oserr, str) \
    do { \
        osout << str << "@" << __PRETTY_FUNCTION__ << ":"; \
        oserr << str << "@" << __PRETTY_FUNCTION__ << ":"; \
    } while(0) \

class TestMacroExpansion : public ::testing::Test {};

TEST_F(TestMacroExpansion, Ill) {
    std::ostringstream out;
    std::ostringstream err;

    std::string message {"Event"};
    std::string log = message;
    log += "@";
    log += __PRETTY_FUNCTION__;
    log += ":";

    EXPECT_FALSE(out.tellp());
    ILL_DEBUG_PRINT(out, err, message);
    EXPECT_EQ(log, out.str());
    EXPECT_EQ(log, err.str());

    // outには書いたので、条件はfalse
    if (!out.tellp()) ILL_DEBUG_PRINT(out, err, message);
    // outにはログが書かれないが、errには書かれてしまう
    EXPECT_EQ(log, out.str());
    EXPECT_NE(log, err.str());
}

TEST_F(TestMacroExpansion, Well) {
    std::ostringstream out;
    std::ostringstream err;

    std::string message {"Event"};
    std::string log = message;
    log += "@";
    log += __PRETTY_FUNCTION__;
    log += ":";

    WELL_DEBUG_PRINT(out, err, message);
    EXPECT_EQ(log, out.str());
    EXPECT_EQ(log, err.str());

    if (!out.tellp()) WELL_DEBUG_PRINT(out, err, message);
    EXPECT_EQ(log, out.str());
    EXPECT_EQ(log, err.str());
}

namespace {
    static_assert(sizeof(char) == 1, "Expect sizeof(char) == 1");
    static_assert(sizeof('a') == 1, "Expect sizeof(char) == 1");
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

// std::functionによる遅延実行を行うオブジェクトをコンテナに入れるための
// 処理に共通なクラス
class BaseCommand {
protected:
    BaseCommand(void) = default;
    virtual void exec(void) = 0;
public:
    virtual ~BaseCommand(void) = default;
    void Exec(void) { exec(); }
};

// &関数名, std::function, ラムダ式, operator() をマッチングする
template <typename Functor>
class ConcreteCommand : public BaseCommand {
public:
    ConcreteCommand(const Functor& f) : BaseCommand(), f_(f) {}
    virtual ~ConcreteCommand(void) = default;
    virtual void exec(void) override { f_(); }
private:
    Functor f_;
};

// 関数名をマッチングする
template <typename Result, typename ... ArgTypes>
class ConcreteCommandF : public BaseCommand {
    using Function = Result(&)(ArgTypes...);
public:
    ConcreteCommandF(const Function f) : BaseCommand(), f_(f) {}
    virtual ~ConcreteCommandF(void) = default;
    virtual void exec(void) override { f_(); }
private:
    Function f_;
};

template <typename T>
auto CreateConcreteCommand(const T& f) {
    std::unique_ptr<BaseCommand> command(new ConcreteCommand<T>(f));
    return command;
}

template <typename Result, typename ... ArgTypes>
auto CreateConcreteCommand(Result(&f)(ArgTypes...)) {
    std::unique_ptr<BaseCommand> command(new ConcreteCommandF<Result, ArgTypes...>(f));
    return command;
}

namespace {
    int g_anGlobalVariable = 0;
    void setVarToOne(void) {
        g_anGlobalVariable = 1;
    }

    void doubleVar(void) {
        g_anGlobalVariable *= 2;
    }

    struct TripleVar {
        void operator()(void) { g_anGlobalVariable *= 3; }
        void addFour(void) { g_anGlobalVariable += 4; }
    };

    struct ClassFunction {
        static void decrement(void) { --g_anGlobalVariable; }
    };

    void squareVar(void) {
        g_anGlobalVariable = g_anGlobalVariable * g_anGlobalVariable;
    }
}

class TestCommandPattern : public ::testing::Test{};

TEST_F(TestCommandPattern, All) {
    std::vector<std::unique_ptr<BaseCommand>> commands;

    g_anGlobalVariable = 0;
    commands.push_back(CreateConcreteCommand(&setVarToOne));
    std::function<decltype(doubleVar)> func = doubleVar;
    commands.push_back(CreateConcreteCommand(func));
    commands.push_back(CreateConcreteCommand([=](void){ g_anGlobalVariable += 5;}));
    TripleVar op;
    commands.push_back(CreateConcreteCommand(op));
    commands.push_back(CreateConcreteCommand(&ClassFunction::decrement));
    commands.push_back(CreateConcreteCommand(squareVar));
    commands.push_back(CreateConcreteCommand(ClassFunction::decrement));

    for(auto& command : commands) {
        command->Exec();
    }

    // (((1 * 2 + 5) * 3) - 1) ^ 2 - 1
    EXPECT_EQ(399, g_anGlobalVariable);
}

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
