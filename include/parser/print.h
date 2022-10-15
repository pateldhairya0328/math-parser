/**
 * @file print.h
 * @brief Contains operator overloads to print tokens and expressions.
 * 
 * @author Dhairya Patel
*/

#pragma once

#include <iostream>

#include "parser/expression.h"
#include "parser/token.h"

namespace parser
{

/**
 * @brief Prints token.
 * 
 * @param os Output stream to print to.
 * @param t Token to print.
*/
template<std::floating_point T>
auto operator<<(std::ostream& os, const token<T>& t) -> std::ostream&
{
    static std::unordered_map<operation, std::string> const table = {
        {L_BRACKET, "("},
        {R_BRACKET, ")"},
        {ADD, "+"},
        {SUB, "-"},
        {NEG, "~"},
        {MUL, "*"},
        {DIV, "/"},
        {POW, "^"},
        {RE, "re"},
        {IM, "im"},
        {ABS, "abs"},
        {ARG, "arg"},
        {CONJ, "conj"},
        {EXP, "exp"},
        {LOG, "log"},
        {COS, "cos"},
        {SIN, "sin"},
        {TAN, "tan"},
        {SEC, "sec"},
        {CSC, "csc"},
        {COT, "cot"},
        {ACOS, "acos"},
        {ASIN, "asin"},
        {ATAN, "atan"},
        {COSH, "cosh"},
        {SINH, "sinh"},
        {TANH, "tanh"},
        {ACOSH, "acosh"},
        {ASINH, "asinh"},
        {ATANH, "atanh"},
        {DERIV, "deriv"},
        {NO_OP, ""}
    };

    if (t.type == BIN_OP || t.type == FUNC || t.type == OTHER_TYPE)
    {
        os << table.find(t.op)->second;
    }
    else if (t.type == CONST)
    {
        os << t.val;
    }
    else
    {
        os << "z";
    }

    return os;
}

/**
 * @brief Prints vector of tokens.
 * 
 * @param os Output stream to print to.
 * @param ts Vector of tokens to print.
*/
template<std::floating_point T>
auto operator<<(std::ostream& os, const expr<T>& e) -> std::ostream&
{
    os << "[";

    for (auto it = e.cbegin(); it != std::prev(e.cend()); it++)
    {
        os << *it << ' ';
    }

    os << e.back() << "]";

    return os;
}

};