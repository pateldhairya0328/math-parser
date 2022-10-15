/**
 * @file expression.h
 * @brief Class to represent math expression.
 * 
 * @author Dhairya Patel
*/

#pragma once

#include <concepts>
#include <iterator>
#include <list>
#include <stack>
#include <string>

#include "token.h"

namespace parser
{

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

    /**
     * @brief Helper function in interpreting input string. Finds end of 
     * function name escaped using a \. 
     * 
     * @param input Input string to find end of function name in.
     * @param n Index of where function name starts.
     * @return Index of where function name ends.
     * @throw If end index not found.
    */
    auto get_op_end_index(const std::string& input, size_t n) -> size_t
    {
        for (size_t i = n + 1; i < input.size(); i++)
        {
            if (input[i] == '\\' || input[i] == '-' || input[i] == '+' || input[i] == '*' || input[i] == '/' || input[i] == '^' || input[i] == '{' || input[i] == '(' || input[i] == '[') 
            {
                return i - 1;
            }
        }
        
        throw std::invalid_argument("Operation end index not found.");
    }

public:
    /**
     * @brief Default constructor.
    */
    expr() {};

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
        // Input with spaces removed
        auto cleaned = infix;
        std::string::iterator end_pos = std::remove(cleaned.begin(), cleaned.end(), ' ');
        cleaned.erase(end_pos, cleaned.end());

        for (size_t i = 0; i < cleaned.size(); i++)
        {
            // pre-defined operation escaped by \ found, so the token for the operation is made
            if (cleaned[i] == '\\')
            {
                auto j = get_op_end_index(cleaned, i);
                auto op_str = cleaned.substr(i + 1, j - i);
                auto op = get_operation(op_str);
                auto type = get_token_type(op);
                m_expr.push_back({type, op});
                i = j;
            }
            // negative sign found at start of expression, so NEG token pushed instead of SUB
            else if (i == 0 && cleaned[i] == '-')
            {
                m_expr.push_back({FUNC, NEG});
            }
            // negative sign found immediately after bracket, so NEG token pushed instead of SUB
            else if (i > 0 && cleaned[i] == '-' && (cleaned[i - 1] == '{' || cleaned[i - 1] == '('))
            {
                m_expr.push_back({FUNC, NEG});
            }
            // a number is found, so we look for the end of the number (i.e., the first non 0-9/. character)
            else if (std::isdigit(cleaned[i]) || cleaned[i] == '.') {
                bool period_found = false;
                auto j = i;

                while (j < cleaned.size() && (std::isdigit(cleaned[j]) || cleaned[j] == '.'))
                {
                    if (cleaned[j] == '.')
                    {
                        if (period_found)
                        {
                            throw std::invalid_argument("Invalid number formatting detected.");
                        }
                        else
                        {
                            period_found = true;
                        }
                    }
                    j++;
                }

                auto num = std::stod(cleaned.substr(i, j - i));

                if (j < cleaned.size() && cleaned[j] == 'i')
                {
                    m_expr.push_back({CONST, NO_OP, std::complex<T>(0, num)});
                    j++;
                }
                else
                {
                    m_expr.push_back({CONST, NO_OP, num});
                }

                i = j - 1;
            }
            // a complex number [a,b] is found
            else if (cleaned[i] == '[')
            {
                auto j = i + 1;

                while (j < cleaned.size() && cleaned[j] != ',')
                {
                    j++;
                }

                auto re = std::stod(cleaned.substr(i + 1, j - i - 1));
                auto k = j + 1;
                
                while (k < cleaned.size() && cleaned[k] != ']')
                {
                    k++;
                }

                auto im = std::stod(cleaned.substr(j + 1, k - j - 1));

                m_expr.push_back({CONST, NO_OP, std::complex<T>(re, im)});

                i = k;
            }
            // imaginary constant i found
            else if (cleaned[i] == 'i')
            {
                m_expr.push_back({CONST, NO_OP, std::complex<T>(0, 1)});
            }
            // Euler's number i found
            else if (cleaned[i] == 'e')
            {
                m_expr.push_back({CONST, NO_OP, std::complex<T>(2.71828182845904523536, 0)});
            }
            // pi found
            else if (cleaned.substr(i, 2) == "pi")
            {
                m_expr.push_back({CONST, NO_OP, std::complex<T>(3.14159265358979323846, 0)});
                i++;
            }
            // z found
            else if (cleaned[i] == 'z')
            {
                m_expr.push_back({VAR, NO_OP, 0});
            }
            // +, -, *, /, (, ), {, } found
            else 
            {
                auto op_str = cleaned.substr(i, 1);
                auto op = get_operation(op_str);
                auto type = get_token_type(op);
                m_expr.push_back({type, op});
            }
        }
    
        m_postfix = false;
    }

    /**
     * @brief Constructs expression by copying list of m_expr.
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
     * @brief Constructs expression by moving list of m_expr.
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
     * @brief Initialize expr from initializer list of m_expr.
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
    auto operator=(const expr& other) -> expr&
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
        m_expr(std::move(other.m_expr)),
        m_postfix(std::exchange(other.m_postfix, 0))
    {}

    /**
     * @brief Move assignment operator.
     * 
     * @param other Other instance of expr.
     * 
     * @return Moved instance of expr.
    */
    auto operator=(expr&& other) -> expr&
    {
        std::swap(m_expr, other.m_expr);
        std::swap(m_postfix, other.m_postfix);

        return *this;
    }

    /**
     * @brief Destructor.
    */
    ~expr() = default;

    /**
     * @brief Evaluates a postfix expression.
     * 
     * @param postfix Vector of tokens representing a postfix math expression.
     * @param z Value to evaluate expression at.
    */
    auto evaluate(std::complex<T> z) -> std::complex<T>
    {
        std::stack<std::complex<T>> eval_stack;
        std::complex<T> temp1, temp2;

        for (auto it = m_expr.begin(); it != m_expr.end(); it++)
        {
            if (it->type == CONST)
            {
                eval_stack.push(it->val);
            }
            else if (it->type == VAR)
            {
                eval_stack.push(z);
            }
            else if (it->type == FUNC)
            {
                temp1 = eval_stack.top();
                eval_stack.pop();
                eval_stack.push(get_func(it->op)(temp1));
            }
            else if (it->type == BIN_OP)
            {
                temp1 = eval_stack.top();
                eval_stack.pop();
                temp2 = eval_stack.top();
                eval_stack.pop();
                eval_stack.push(get_bin_op(it->op)(temp2, temp1));
            }
        }

        temp1 = eval_stack.top();
        eval_stack.pop();
        return temp1;
    }

    /**
     * @brief Returns an equivalent postfix expression.
     * 
     * @return expr equivalent in postfix.
    */
    auto postfix() const -> expr
    {
        // If expr already in postfix form, return it
        if (m_postfix)
        {
            return *this;
        }
        
        // Convert infix to postfix
        std::list<token<T>> postfix;
        std::stack<token<T>> stack;

        for (auto& t: m_expr)
        {
            if (t.type == VAR || t.type == CONST)
            {
                postfix.push_back(t);
            }
            else if (t.type == FUNC)
            {
                stack.push(t);
            }
            else if (t.type == BIN_OP)
            {
                auto t_precedence = get_precedence(t.op);

                while (!stack.empty() && stack.top().op != L_BRACKET && get_precedence(stack.top().op) >= t_precedence)
                {
                    postfix.push_back(stack.top());
                    stack.pop();
                }

                stack.push(t);
            }
            else if (t.op == L_BRACKET)
            {
                stack.push(t);
            }
            else if (t.op == R_BRACKET)
            {
                while (stack.top().op != L_BRACKET)
                {
                    if (stack.empty())
                    {
                        throw std::invalid_argument("Mismatched brackets in infix expression.");
                    }

                    postfix.push_back(stack.top());
                    stack.pop();
                }

                stack.pop();

                if (stack.top().type == FUNC)
                {
                    postfix.push_back(stack.top());
                    stack.pop();
                }
            }
        }

        while (!stack.empty())
        {
            if (stack.top().op == L_BRACKET)
            {
                throw std::invalid_argument("Mismatched brackets in infix expression.");
            }

            postfix.push_back(stack.top());
            stack.pop();
        }

        return postfix;
    }

    /**
     * @brief Extracts the smallest legal subexpression of input postfix 
     * expression that ends at one before element pointed to by @p end.
     * 
     * The smallest legal subexpression is one that can be evaluated as a 
     * correct mathematical expression by itself. For example [3 4 *] (equal to
     * the infix [3 * 4]) is a legal subexpression of [5 3 4 * -] (equal to the
     * infix [5 - (3 * 4)]). But, [4 *] is not, since its corresponding infix 
     * [* 4] is not complete.
     * 
     * @tparam T Floating point type used by expression.
     * @tparam iter Bidirectional iterator.
     * 
     * @param end Iterator to one after the last token of subexpression.
     * 
     * @return Bidirectional iterator pointing to first token of smallest legal
     * subexpression that ends at @p end.
     * 
     * @warning Bidirectional iterator should point to type token<T>. 
     * 
    */
    template<std::bidirectional_iterator iter>
    static auto subexpr_begin(iter end)
    {
        // The tokens for the subexpression in postfix must lie immediately to
        // the left of the end token. A CONST or VAR token form a complete
        // subexpression themselves. A FUNC token requires another 
        // token to its left to complete it. A BIN_OP token requires two more
        // tokens lying to its left to complete it. k tracks how many more
        // tokens we need to complete the full subexpression. start tracks the
        // tentative start iterator and decrements until k = 0, i.e., no
        // more tokens needed to complete the subexpression.
        auto start = end;
        size_t k = 1;
        while (k > 0)
        {
            // Decrement iterator to go to next token down
            start--;

            // If we encounter function, we need one more token to complete
            // inside function
            if (start->type == FUNC)
            {
                k += 1;
            }
            // If we encounter binary operation, we need two more tokens to
            // complete inside function
            else if (start->type == BIN_OP)
            {
                k += 2;
            }

            // Subtract token we just read
            k--;
        }

        return start;
    }

    // The following are simply wrapper functions around the contained list. See
    // https://en.cppreference.com/w/cpp/container/list for documentaion of the 
    // underlying functions.

    // Element access

    auto front()
    {
        return m_expr.front();
    }

    auto back()
    {
        return m_expr.back();
    }

    auto front() const
    {
        return m_expr.front();
    }

    auto back() const
    {
        return m_expr.back();
    }

    // Iterators

    auto begin() noexcept
    {
        return m_expr.begin();
    }

    auto end() noexcept
    {
        return m_expr.end();
    }

    auto cbegin() const noexcept
    {
        return m_expr.cbegin();
    }
    
    auto cend() const noexcept
    {
        return m_expr.cend();
    }
    
    auto rbegin() noexcept
    {
        return m_expr.rbegin();
    }

    auto rend() noexcept
    {
        return m_expr.rend();
    }

    auto crbegin() const noexcept
    {
        return m_expr.crbegin();
    }
    
    auto crend() const noexcept
    {
        return m_expr.crend();
    }

    // Capacity

    [[nodiscard]] auto empty() const noexcept
    {
        return m_expr.empty();
    }

    auto size() const noexcept
    {
        return m_expr.size();
    }

    // Modifiers

    auto clear() noexcept
    {
        m_expr.clear();
    }

    auto insert(auto pos, const auto& value)
    {
        return m_expr.insert(pos, value);
    }
    
    auto insert(auto pos, auto&& value)
    {
        return m_expr.insert(pos, value);
    }
    
    auto insert(auto pos, auto count, const auto& value)
    {
        return m_expr.insert(pos, count, value);
    }
    
    template<std::input_iterator input_iter>
    auto insert(auto pos, input_iter first, input_iter second)
    {
        return m_expr.insert(pos, first, second);
    }
    
    auto insert(auto pos, auto ilist)
    {
        return m_expr.insert(pos, ilist);
    }

    auto emplace(auto pos, auto&&... args)
    {
        return m_expr.emplace(pos, args...);
    }
    
    auto push_back(const auto& value)
    {
        m_expr.push_back(value);
    }
    
    auto push_back(auto&& value)
    {
        m_expr.push_back(value);
    }

    auto emplace_back(auto&&... args)
    {
        return m_expr.emplace_back(args...);
    }

    auto pop_back()
    {
        m_expr.pop_back();
    }

    auto push_front(const auto& value)
    {
        m_expr.push_front(value);
    }
    
    auto push_front(auto&& value)
    {
        m_expr.push_front(value);
    }

    auto emplace_front(auto&&... args)
    {
        return m_expr.emplace_front(args...);
    }

    auto pop_front()
    {
        m_expr.pop_front();
    }
};

};