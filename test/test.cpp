#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "parser/derivative.h"
#include "parser/parser.h"
#include "parser/print.h"

TEST(test, test)
{
    auto infix = parser::expr<float>("4.5^z");
    std::cout << infix << std::endl;

    auto postfix = infix.postfix();
    std::cout << postfix << std::endl;

    auto deriv = parser::differentiate(postfix);
    std::cout << deriv << std::endl;
}