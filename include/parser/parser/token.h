/**
 * @file token.h
 * @brief Contains struct to represent a single element of a math expression,
 * as well as helper functions associated with working with a token and its 
 * member fields.
 * 
 * @author Dhairya Patel
*/

#pragma once

#include <complex>
#include <unordered_map>

namespace parser
{

// Type of token; VAR = variable, CONST = constant value, BIN_OP = binary
// operation, FUNC = function, OTHER_TYPE = everything else
enum token_type { VAR, CONST, BIN_OP, FUNC, OTHER_TYPE };

// Name of operation
enum operation { L_BRACKET, R_BRACKET, ADD, SUB, NEG, MUL, DIV, POW, RE, IM, ABS, ARG, CONJ, EXP, LOG, COS, SIN, TAN, SEC, CSC, COT, ACOS, ASIN, ATAN, COSH, SINH, TANH, ACOSH, ASINH, ATANH, DERIV, NO_OP };

/**
 * @brief Struct representing a single token of a math expression. A token can
 * be any single component in a math expression: a variable, a constant, a 
 * function, a symbol, etc.
 * 
 * @tparam The floating point type (float, double or long double) to use in 
 * the storing of values (if necessary) in the token. Defaults to double
*/
template<std::floating_point T = double>
struct token 
{
    token_type type;     // Must always be set
    operation op;        // NO_OP if token_type = VAR, CONST
    std::complex<T> val; // Set only if token_type = VAR, CONST
};

/**
 * @brief Map an enum for a binary operation to a function pointer for a 
 * function that evaluates the function specified by the enum.
 * 
 * @param op Enum specifying binary operation.
 * @return Function pointer for a function of two complex values to one complex 
 * value.
 * @throw invalid_argument if op is not one of ADD, SUB, MUL, DIV or POW
*/
template<std::floating_point T = double>
auto (*get_bin_op(operation op))(std::complex<T> z_1, std::complex<T> z_2) -> std::complex<T>
{
    static std::unordered_map<operation, std::complex<T>(*)(std::complex<T>, std::complex<T>)> const table = {
        {operation::ADD, [](std::complex<T> z_1, std::complex<T> z_2) { return z_1 + z_2; }},
        {operation::SUB, [](std::complex<T> z_1, std::complex<T> z_2) { return z_1 - z_2; }},
        {operation::MUL, [](std::complex<T> z_1, std::complex<T> z_2) { return z_1 * z_2; }},
        {operation::DIV, [](std::complex<T> z_1, std::complex<T> z_2) { return z_1 / z_2; }},
        {operation::POW, [](std::complex<T> z_1, std::complex<T> z_2) { return std::pow(z_1, z_2); }}
    };

    auto it = table.find(op);
    if (it != table.end())
    {
        return it->second;
    }
    else
    {
        throw std::invalid_argument("Binary operation not found.");
    }
}

/**
 * @brief Map an enum for a function to a function pointer for a function 
 * that evaluates the function specified by the enum.
 * 
 * @param op Enum specifying function.
 * @return Function pointer for a function of one complex value to one complex 
 * value.
 * @throw invalid_argument if op does not correspond to type FUNC.
*/
template<std::floating_point T = double>
auto (*get_func(operation op))(std::complex<T> z) -> std::complex<T>
{
    static std::unordered_map<operation, std::complex<T>(*)(std::complex<T>)> const table = {
        {operation::NEG,   [](std::complex<T> z) { return - z; }},
        {operation::RE,    [](std::complex<T> z) { return (std::complex<T>) z.real(); }},
        {operation::IM,    [](std::complex<T> z) { return (std::complex<T>) z.imag(); }},
        {operation::ABS,   [](std::complex<T> z) { return (std::complex<T>) std::abs(z); }},
        {operation::ARG,   [](std::complex<T> z) { return (std::complex<T>) std::arg(z); }},
        {operation::CONJ,  [](std::complex<T> z) { return (std::complex<T>) std::conj(z); }},
        {operation::EXP,   [](std::complex<T> z) { return std::exp(z); }},
        {operation::LOG,   [](std::complex<T> z) { return std::log(z); }},
        {operation::COS,   [](std::complex<T> z) { return std::cos(z); }},
        {operation::SIN,   [](std::complex<T> z) { return std::sin(z); }},
        {operation::TAN,   [](std::complex<T> z) { return std::tan(z); }},
        {operation::SEC,   [](std::complex<T> z) { return 1.0 / std::cos(z); }},
        {operation::CSC,   [](std::complex<T> z) { return 1.0 / std::sin(z); }},
        {operation::COT,   [](std::complex<T> z) { return 1.0 / std::tan(z); }},
        {operation::ACOS,  [](std::complex<T> z) { return std::acos(z); }},
        {operation::ASIN,  [](std::complex<T> z) { return std::asin(z); }},
        {operation::ATAN,  [](std::complex<T> z) { return std::atan(z); }},
        {operation::COSH,  [](std::complex<T> z) { return std::cosh(z); }},
        {operation::SINH,  [](std::complex<T> z) { return std::sinh(z); }},
        {operation::TANH,  [](std::complex<T> z) { return std::tanh(z); }},
        {operation::ACOSH, [](std::complex<T> z) { return std::acosh(z); }},
        {operation::ASINH, [](std::complex<T> z) { return std::asinh(z); }},
        {operation::ATANH, [](std::complex<T> z) { return std::atanh(z); }},
        {operation::DERIV, [](std::complex<T> z) { return std::complex<T>(0, 0); }},
    };

    auto it = table.find(op);
    if (it != table.end())
    {
        return it->second;
    }
    else
    {
        throw std::invalid_argument("Function not found.");
    }
}

/**
 * @brief Map string name of operation to the enum for the operation
 * 
 * @param op String (in all lower case) representing an operation
 * @return Enum for the corresponding operation
 * @throw invalid_argument if string does not correspond to any defined 
 * operation.
*/
auto get_operation(const std::string& op) -> operation
{
    static std::unordered_map<std::string, operation> const table = {
        {"{",     operation::L_BRACKET},
        {"}",     operation::R_BRACKET},
        {"(",     operation::L_BRACKET},
        {")",     operation::R_BRACKET},
        {"+",     operation::ADD},
        {"-",     operation::SUB},
        {"*",     operation::MUL},
        {"/",     operation::DIV},
        {"^",     operation::POW},
        {"re",    operation::RE},
        {"im",    operation::IM},
        {"abs",   operation::ABS},
        {"arg",   operation::ARG},
        {"conj",  operation::CONJ},
        {"exp",   operation::EXP},
        {"log",   operation::LOG},
        {"cos",   operation::COS},
        {"sin",   operation::SIN},
        {"tan",   operation::TAN},
        {"sec",   operation::TAN},
        {"csc",   operation::TAN},
        {"cot",   operation::TAN},
        {"acos",  operation::TAN},
        {"asin",  operation::TAN},
        {"atan",  operation::TAN},
        {"cosh",  operation::TAN},
        {"sinh",  operation::TAN},
        {"tanh",  operation::TAN},
        {"acosh", operation::TAN},
        {"asinh", operation::TAN},
        {"atanh", operation::TAN},
        {"deriv", operation::DERIV}
    };

    auto it = table.find(op);
    if (it != table.end())
    {
        return it->second;
    }
    else
    {
        throw std::invalid_argument("Operation not found.");
    }
}

/**
 * @brief Maps operator to its precendence level.
 * 
 * @param op Enum specifying operation.
 * @return Non-negative integer representing precendence level of operation. 
 * @throw invalid_argument for invalid operation enum.
*/
auto get_precedence(operation op) -> size_t
{
    static std::unordered_map<operation, size_t> const table = {
        {operation::L_BRACKET, 4},
        {operation::R_BRACKET, 4},
        {operation::ADD, 0},
        {operation::SUB, 0},
        {operation::NEG, 1},
        {operation::MUL, 1},
        {operation::DIV, 1},
        {operation::POW, 2},
        {operation::RE, 3},
        {operation::IM, 3},
        {operation::ABS, 3},
        {operation::ARG, 3},
        {operation::CONJ, 3},
        {operation::EXP, 3},
        {operation::LOG, 3},
        {operation::COS, 3},
        {operation::SIN, 3},
        {operation::TAN, 3},
        {operation::SEC, 3},
        {operation::CSC, 3},
        {operation::COT, 3},
        {operation::ACOS, 3},
        {operation::ASIN, 3},
        {operation::ATAN, 3},
        {operation::COSH, 3},
        {operation::SINH, 3},
        {operation::TANH, 3},
        {operation::ACOSH, 3},
        {operation::ASINH, 3},
        {operation::ATANH, 3},
        {operation::DERIV, 3}
    };

    auto it = table.find(op);
    if (it != table.end())
    {
        return it->second;
    }
    else
    {
        throw std::invalid_argument("Type not found.");
    }
}

/**
 * @brief Maps operator to its token type.
 * 
 * @param op Enum specifying operation.
 * @return Enum specifying token type.
 * @throw invalid_argument for invalid operation enum.
*/
auto get_token_type(operation op) -> token_type
{
    static std::unordered_map<operation, token_type> const table = {
        {operation::L_BRACKET, token_type::OTHER_TYPE},
        {operation::R_BRACKET, token_type::OTHER_TYPE},
        {operation::ADD, token_type::BIN_OP},
        {operation::SUB, token_type::BIN_OP},
        {operation::NEG, token_type::FUNC},
        {operation::MUL, token_type::BIN_OP},
        {operation::DIV, token_type::BIN_OP},
        {operation::POW, token_type::BIN_OP},
        {operation::RE, token_type::FUNC},
        {operation::IM, token_type::FUNC},
        {operation::ABS, token_type::FUNC},
        {operation::ARG, token_type::FUNC},
        {operation::CONJ, token_type::FUNC},
        {operation::EXP, token_type::FUNC},
        {operation::LOG, token_type::FUNC},
        {operation::COS, token_type::FUNC},
        {operation::SIN, token_type::FUNC},
        {operation::TAN, token_type::FUNC},
        {operation::SEC, token_type::FUNC},
        {operation::CSC, token_type::FUNC},
        {operation::COT, token_type::FUNC},
        {operation::ACOS, token_type::FUNC},
        {operation::ASIN, token_type::FUNC},
        {operation::ATAN, token_type::FUNC},
        {operation::COSH, token_type::FUNC},
        {operation::SINH, token_type::FUNC},
        {operation::TANH, token_type::FUNC},
        {operation::ACOSH, token_type::FUNC},
        {operation::ASINH, token_type::FUNC},
        {operation::ATANH, token_type::FUNC},
        {operation::DERIV, token_type::FUNC},
        {operation::NO_OP, token_type::OTHER_TYPE}
    };

    auto it = table.find(op);
    if (it != table.end())
    {
        return it->second;
    }
    else
    {
        throw std::invalid_argument("Type not found.");
    }
}

};