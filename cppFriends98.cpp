#include <boost/fusion/container/vector.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

// C++98にはstatic_assertもstd::is_sameもない
// Boostのを使うこともできますが
class TestCpp98BoostFusionVector : public ::testing::Test{};

TEST_F(TestCpp98BoostFusionVector, Numbered) {
    EXPECT_NE(typeid(boost::fusion::vector<int,int>), typeid(boost::fusion::vector2<int,int>));
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
