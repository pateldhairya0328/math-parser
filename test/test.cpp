#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "parser/parser.h"
#include "parser/print.h"

TEST(test, test)
{
    auto infix = parser::expr("\\sin(z + \\cos(z - 2))");
    auto postfix = infix.postfix();
    std::cout << postfix << std::endl;
}