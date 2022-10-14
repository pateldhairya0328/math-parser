/**
 * @file expression.h
 * @brief Class to represent math expression.
 * 
 * @author Dhairya Patel
*/

#pragma once

#include <complex>
#include <concepts>
#include <iterator>
#include <list>
#include <string>

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
 * @brief Wrapper around std::list<token> that represents a math expression as 
 * a sequence of parser::token, each which represent a value or an operation.
 * 
 * @tparam T The floating point type (float, double or long double) to use in 
 * the storing and parsing of values in the expression. Defaults to double.
*/
template<std::floating_point T = double>
class expr
{
private:
    /**
     * Use a list instead of a vector to represent the expression since random
     * access into an expression is not important, the expressions would be
     * parsed by traversing the tokens one by one. But, expressions will be
     * modified repeatedly with insertions and deletions anywhere in the
     * expression, where lists outperform vectors.
    */
    std::list<token<T>> m_expr;
    
    // True if expression in postfix, False if expression in infix
    bool m_postfix;
public:
    /**
     * @brief Initialize infix list of tokens from a string representing an 
     *        infix expression.
     * 
     * @param infix String representing an infix math expression. Note that any
     *        spaces in the string are ignored.
     * 
     * @return expr instance representing infix math expression.
    */
    expr(const std::string& infix)
    {
        throw std::logic_error("Not yet implemented.");
    }

    /**
     * @brief Constructs expression by copying list of tokens.
     * 
     * @param other List of tokens to copy.
     * @param postfix Indicates whether init represents a postfix expression.
     * Defaults to true.
     * 
     * @return expr instance representing math expression.
    */
    expr(const std::list<token<T>>& other, bool postfix = true):
        m_expr(other),
        m_postfix(postfix)
    {}
    
    /**
     * @brief Constructs expression by moving list of tokens.
     * 
     * @param other List of tokens to move.
     * @param postfix Indicates whether init represents a postfix expression.
     * Defaults to true.
     * 
     * @return expr instance representing math expression.
    */
    expr(std::list<token<T>>&& other, bool postfix = true):
        m_expr(other),
        m_postfix(postfix)
    {}

    /**
     * @brief Initialize expr from initializer list of tokens.
     * 
     * @param init Initializer list of tokens representing a math expression.
     * @param postfix Indicates whether init represents a postfix expression.
     * Defaults to true.
     * 
     * @return Expression representing an infix math expression.
    */
    expr(std::initializer_list<token<T>> init, bool postfix = true):
        m_expr(init),
        m_postfix(postfix)
    {}

    /**
     * @brief Initialize expr from a pair of input iterators. The list 
     * generated contains the elements [begin, end), where begin is the element
     * pointed to by the first iterator and end is the element pointed to by
     * the second iterator.
     * 
     * @param begin Input iterator pointing to the first element in the list.
     * @param end Input iterator pointing to one after last element in the
     * list.
     * @param postfix Indicates whether init represents a postfix expression.
     * Defaults to true.
     * 
     * @return expr instance representing math expression.
    */
    template<typename input_iterator,
             typename = std::_RequireInputIter<input_iterator>>
    expr(input_iterator begin, input_iterator end, bool postfix = true):
        m_expr(begin, end),
        m_postfix(postfix)
    {}

    /**
     * @brief Copy constructor.
     * 
     * @param other Other instance of expr.
     * 
     * @return Copied instance of expr.
    */
    expr(const expr& other) :
        m_expr(other.m_expr),
        m_postfix(other.m_postfix)
    {}

    /**
     * @brief Copy assignment operator.
     * 
     * @param other Other instance of expr.
     * 
     * @return Copied instance of expr.
    */
    expr& operator=(const expr& other)
    {
        return *this = expr(other);
    }

    /**
     * @brief Move constructor.
     * 
     * @param other Other instance of expr.
     * 
     * @return Moved instance of expr.
    */
    expr(expr&& other) :
        m_expr(std::exchange(other.m_expr, 0)),
        m_postfix(std::exchange(other.postfix, 0))
    {}

    /**
     * @brief Move assignment operator.
     * 
     * @param other Other instance of expr.
     * 
     * @return Moved instance of expr.
    */
    expr& operator=(expr&& other)
    {
        std::swap(m_expr, other.m_expr);
        std::swap(m_postfix, other.m_postfix);

        return *this;
    }

    /**
     * @brief Destructor.
    */
    ~expr();

    // Add iterators and necessary iterator functions so the expr class
    // essentially behaves like a wrapper around std::list<token>
    using iterator = std::list<token<T>>::iterator;

    using const_iterator = std::list<token<T>>::const_iterator;

    iterator begin();

    iterator end();

    const_iterator cbegin() const;
    
    const_iterator cend() const;
};

};