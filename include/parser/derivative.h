#pragma once
/**
 * @file derivative.h
 * @brief Contains functions to find derivatives in and of postfix expressions.
 * 
 * @author: Dhairya Patel 
*/

#pragma once

#include "parser/expression.h"

namespace parser 
{

/**
 * @brief Computes all derivatives in given postfix expression.
 * 
 * @param postfix Postfix expression.
 * @tparam T floating point type used by expression.
 * 
 * @return Postfix expression with all derivatives computed.
*/
template<std::floating_point T>
auto all_derivatives(const expr<T>& e) -> expr<T>;

/**
 * @brief Differentiates given postfix expression.
 * 
 * @param postfix Postfix expression.
 * @tparam T floating point type used by expression.
 * 
 * @return Derivative of postfix expression.
*/
template<std::floating_point T>
auto differentiate(const expr<T>& postfix) -> expr<T>
{
    return differentiate<T>(postfix.cbegin(), postfix.cend());
}

/**
 * @brief Differentiates given postfix expression within specified bounds.
 * 
 * @tparam T Floating point type used by expression.
 * @tparam iter Bidirectional iterator.
 * 
 * @param begin Iterator to first element of expression to differentiate.
 * @param end Iterator to one after the last element of expression to 
 * differentiate.
 * 
 * @warning Bidirectional iterator should point to type token<T>. 
 * 
 * @return Derivative of postfix subexpression.
*/
template<std::floating_point T, std::bidirectional_iterator iter>
auto differentiate(iter begin, iter end) -> expr<T>
{
    // If expression to differentiate is a single variable, the derivative is 1
    if (std::prev(end)->type == VAR)
    {
        return {{CONST, NO_OP, 1.0}};
    }
    // If expression to differentiate is a constant, the derivative is 1
    else if (std::prev(end)->type == CONST)
    {
        return {{CONST, NO_OP, 0.0}};
    }
    // If expression to differentiate is a function of one variable, we call a
    // separate function to deal with that case
    else if (std::prev(end)->type == FUNC)
    {
        return differentiate_func<T>(begin, end);
    }
    // If expression to differentiate is a binary operation, we call a separate
    // function to deal with that case
    else if (std::prev(end)->type == BIN_OP)
    {
        return differentiate_bin_op<T>(begin, end);
    }
    else
    {
        throw std::invalid_argument("Unrecognized token to differentiate.");
    }
}

/**
 * @brief Differentiates given postfix expression within specified bounds that
 * can be represented as f(g(z)) where f corresponds to an operation with type
 * FUNC and g(z) is any function of z represented in postfix. 
 * 
 * @tparam T Floating point type used by expression.
 * @tparam iter Bidirectional iterator.
 * 
 * @param begin Iterator to first element of expression to differentiate.
 * @param end Iterator to one after the last element of expression to 
 * differentiate.
 * 
 * @return Derivative of postfix subexpression.
 * 
 * @warning Bidirectional iterator should point to type token<T>. 
 * 
 * @throw invalid_argument if prev(end) is not of type FUNC.
*/
template<std::floating_point T, std::bidirectional_iterator iter>
auto differentiate_func(iter begin, iter end) -> expr<T>
{
    auto f = std::prev(end);
    if (f->type != FUNC)
    {
        throw std::invalid_argument("Expression to differentiate is not a function of one variable.");
    }

    // Input postfix [begin : end] looks like [[g] f], where [g] is itself
    // a subexpression, postfix[begin : end - 1] = postfix[begin : f]. The 
    // derivative of postfix[begin : end] is equivalent to [[g'] [g] [f'] *] by
    // the chain rule, where [f'] and [g'] are the postfix expressions 
    // representing the symbolic derivatives of f and [g], respectively.

    // Deal with special case where g(z) = c (so f'(g(z)) * g'(z) = 0) and when
    // g(z) = z so f'(g(z)) * g'(z) = f'(z). We do not necessarily need a separate
    // case, but without this special case, the final derivative will have 
    // expressions multiplied by 1 and 0, which adds unnecessary computations.
    auto g_end = std::prev(f);
    if (begin == g_end)
    {
        // If the argument of the function is constant, the derivative is 0.
        if (begin->type == CONST)
        {
            return expr<T>{{CONST, NO_OP, 0.0}};
        }
        // If the argument of the function is z, the derivative is [z [f']].
        else if (begin->type == VAR)
        {
            auto derivative = get_deriv(*f);
            derivative.emplace_front(VAR, NO_OP);

            return derivative;
        }
        else
        {
            throw std::logic_error("Unexpected token encountered.");
        }
    }
    // In general, the derivative is [[g'] [g] [f'] *]
    else
    {
        auto derivative = differentiate<T>(begin, f);
        derivative.insert(derivative.end(), begin, f);
        auto f_deriv = get_deriv(*f);
        derivative.insert(derivative.end(), std::make_move_iterator(f_deriv.begin()), std::make_move_iterator(f_deriv.end()));
        derivative.emplace_back(BIN_OP, MUL);

        return derivative;
    }
}

/**
 * @brief Differentiates given postfix expression within specified bounds that
 * can be represented as f(z) · g(z) where · is one of +, -, *, / or ^, and 
 * g(z) and h(z) are any expressions. 
 * 
 * @tparam T Floating point type used by expression.
 * @tparam iter Bidirectional iterator.
 * 
 * @param begin Iterator to first element of expression to differentiate.
 * @param end Iterator to one after the last element of expression to 
 * differentiate.
 * 
 * @return Derivative of postfix subexpression.
 * 
 * @warning Bidirectional iterator should point to type token<T>. 
 * 
 * @throw invalid_argument if prev(end) is not of type BIN_OP.
*/
template<std::floating_point T, std::bidirectional_iterator iter>
auto differentiate_bin_op(iter begin, iter end) -> expr<T>
{
    auto bin_op = std::prev(end);

    if (bin_op->type != BIN_OP)
    {
        throw std::invalid_argument("Expression to differentiate is not a binary operation.");
    }

    if (bin_op->op == ADD || bin_op->op == SUB)
    {
        return differentiate_bin_op_add_sub<T>(begin, end);
    }
    else if (bin_op->op == MUL)
    {
        return differentiate_bin_op_mul<T>(begin, end);
    }
    else if (bin_op->op == DIV)
    {
        return differentiate_bin_op_div<T>(begin, end);
    }
    else if (bin_op->op == POW)
    {
        return differentiate_bin_op_pow<T>(begin, end);
    }
    else
    {
        throw std::invalid_argument("Unrecognized/unimplemented binary operator.");
    }
}

/**
 * @brief Differentiates given postfix expression within specified bounds when
 * the expression ends with an ADD or SUB. Between begin and end 
 * (not inclusive), the postfix should look like [[f] [g] ·], where [f] and [g]
 * are themselves postfix expressions and · is either + or -.
 * 
 * @tparam T Floating point type used by expression.
 * @tparam iter Bidirectional iterator.
 * 
 * @param begin Iterator to first element of expression to differentiate.
 * @param end Iterator to one after the last element of expression to 
 * differentiate.
 * 
 * @return Derivative of postfix subexpression.
 * 
 * @warning Bidirectional iterator should point to type token<T>. 
 * 
 * @throw invalid_argument if prev(end) is not ADD or SUB.
*/
template<std::floating_point T, std::bidirectional_iterator iter>
auto differentiate_bin_op_add_sub(iter begin, iter end) -> expr<T>
{
    // Decrement iterator to point to the binary operator.
    end--;
    if (end->op != ADD && end->op != SUB)
    {
        throw std::invalid_argument("Expression to differentiate is not an addition or subtraction.");
    }

    // With end decremented, postfix[begin:end] looks like [[f] [g]], where [f]
    // and [g] are subexpressions. We want the middle index that points to the
    // first element of [g]/ one after last element of [f] to isolate the two
    // subexpressions [f] and [g].
    auto mid = expr<T>::subexpr_begin(end);
    // [g] = postfix[mid:end]
    // [f] = postfix[begin:mid]

    // First we deal with special cases where [f'] or [g'] are constants. This
    // is not strictly necessary, but without the special cases, we get zero
    // additions in final expression, which are unnecessary.

    // In case [f] is a constant, we can simply return [g'] instead if · is +
    // and [- [g']] if · is -.
    if (begin == std::prev(mid) && begin->type == CONST)
    {
        auto derivative = differentiate<T>(mid, end);
        // Insert negation if the binary operation was subtraction
        if (end->op == SUB)
        {
            // If [g'] is also a constant, can just compute [-g']
            if (derivative.size() == 1 && derivative.front().type == CONST)
            {
                derivative.front().val = - derivative.front().val;
            }
            // Otherwise, a negative operation is appended [[g'] -]
            else
            {
                derivative.emplace_back(FUNC, NEG);
            }
        }

        return derivative;
    }
    // In case [g] is a constant, we can simply return [f'] instead.
    else if (mid == std::prev(end) && mid->type == CONST)
    {
        return differentiate<T>(begin, mid);   // [[f']]
    }
    // In general, d(f · g)/dz = df/dz · dg/dz when · is + or -. So, the 
    // derivative of [[f] [g] ·] is [[f'] [g'] ·].
    else
    {
        auto derivative = differentiate<T>(begin, mid);
        auto g_deriv = differentiate<T>(mid, end);
        derivative.insert(derivative.end(), std::make_move_iterator(g_deriv.begin()), std::make_move_iterator(g_deriv.end()));
        derivative.push_back(*end);

        return derivative;
    }
}

/**
 * @brief Differentiates given postfix expression within specified bounds when
 * the expression ends with a MUL. Between begin and end (not inclusive), the
 * postfix should look like [[f] [g] *].
 * 
 * @tparam T Floating point type used by expression.
 * @tparam iter Bidirectional iterator.
 * 
 * @param begin Iterator to first element of expression to differentiate.
 * @param end Iterator to one after the last element of expression to 
 * differentiate.
 * 
 * @return Derivative of postfix subexpression.
 * 
 * @warning Bidirectional iterator should point to type token<T>. 
 * 
 * @throw invalid_argument if prev(end) is not MUL.
*/
template<std::floating_point T, std::bidirectional_iterator iter>
auto differentiate_bin_op_mul(iter begin, iter end) -> expr<T>
{
    // Decrement iterator to point to the binary operator.
    end--;
    if (end->op != MUL)
    {
        throw std::invalid_argument("Expression to differentiate is not an addition or subtraction.");
    }

    // With end decremented, postfix[begin:end] looks like [[f] [g]], where [f]
    // and [g] are subexpressions. We want the middle index that points to the
    // first element of [g]/ one after last element of [f] to isolate the two
    // subexpressions [f] and [g].
    auto mid = expr<T>::subexpr_begin(end);
    // [g] = postfix[mid:end]
    // [f] = postfix[begin:mid]

    // In the general case, the derivative is [[f'] [g] * [g'] [f] * +].
    // Let [p_1] be the expression [[f'] [g] *] 
    // Let [p_2] be the expression [[g'] [f] *]
    // The derivative is then [[p_1] [p_2] +]

    expr<T> p_1;
    // If [f] is a constant or a variable, [p_1] is simpler
    if (begin == std::prev(mid))
    {
        // [f] is a constant, so [p_1] is [0]
        if (begin->type == CONST)
        {
            p_1 = {{CONST, NO_OP, 0}};
        }
        // [f] is a variable, so [p_1] is [[g]]
        else if (begin->type == VAR)
        {
            p_1 = expr<T>(mid, end);
        }
        else
        {
            std::invalid_argument("Unexpected token encountered.");
        }
    }
    // In general, the[p_1] is [[f'] [g] *]
    else
    {
        p_1 = differentiate<T>(begin, mid);
        p_1.insert(p_1.end(), mid, end);
        p_1.emplace_back(BIN_OP, MUL);
    }

    expr<T> p_2;
    // If [g] is a constant or a variable, [p_2] is simpler
    if (mid == std::prev(mid))
    {
        // [g] is a constant, so [p_2] is [0]
        if (mid->type == CONST)
        {
            p_2 = {{CONST, NO_OP, 0}};
        }
        // [g] is a variable, so [p_2] is [[f]]
        else if (mid->type == VAR)
        {
            p_2 = expr<T>(begin, mid);
        }
        else
        {
            std::invalid_argument("Unexpected token encountered.");
        }
    }
    // In general, [p_2] is [[g'] [f] *]
    else
    {
        p_2 = differentiate<T>(mid, end);
        p_2.insert(p_1.end(), begin, mid);
        p_2.emplace_back(BIN_OP, MUL);
    }

    // [0 [p_2] +] is equal to [[p_2]]
    if (p_1.size() == 1 && p_1.front().type == CONST && p_1.front().val == std::complex<T>(0, 0))
    {
        return p_2;
    }
    // [[p_1] 0 +] is equal to [[p_1]]
    else if (p_2.size() == 1 && p_2.front().type == CONST && p_2.front().val == std::complex<T>(0, 0))
    {
        return p_1;
    }
    // In general, return [[p_1] [p_2] +]
    else
    {
        p_1.insert(p_1.end(), std::make_move_iterator(p_2.begin()), std::make_move_iterator(p_2.end()));
        p_1.emplace_back(BIN_OP, ADD);

        return p_1;
    }
}

/**
 * @brief Differentiates given postfix expression within specified bounds when
 * the expression ends with a MUL. Between begin and end (not inclusive), the
 * postfix should look like [[f] [g] /].
 * 
 * @tparam T Floating point type used by expression.
 * @tparam iter Bidirectional iterator.
 * 
 * @param begin Iterator to first element of expression to differentiate.
 * @param end Iterator to one after the last element of expression to 
 * differentiate.
 * 
 * @return Derivative of postfix subexpression.
 * 
 * @warning Bidirectional iterator should point to type token<T>. 
 * 
 * @throw invalid_argument if prev(end) is not DIV.
*/
template<std::floating_point T, std::bidirectional_iterator iter>
auto differentiate_bin_op_div(iter begin, iter end) -> expr<T>
{
    // Decrement iterator to point to the binary operator.
    end--;
    if (end->op != DIV)
    {
        throw std::invalid_argument("Expression to differentiate is not an addition or subtraction.");
    }

    // With end decremented, postfix[begin:end] looks like [[f] [g]], where [f]
    // and [g] are subexpressions. We want the middle index that points to the
    // first element of [g]/ one after last element of [f] to isolate the two
    // subexpressions [f] and [g].
    auto mid = expr<T>::subexpr_begin(end);
    // [g] = postfix[mid:end]
    // [f] = postfix[begin:mid]

    // In the general case, the derivative is [[f'] [g] * [g'] [f] * - [g] [g] * /].
    // Let [p_1] be the expression [[f'] [g] *] 
    // Let [p_2] be the expression [[g'] [f] *]
    // Let [p_3] be the expression [[g] [g] *]
    // The derivative is then [[p_1] [p_2] - [p_3] /]

    expr<T> p_2, p_3;
    // If [g] is a constant or a variable, [p_2] are [p_3] can be simpler
    if (mid == std::prev(end))
    {
        // If [g] is a constant, the derivative is [[f'] [1 / g] *]
        if (mid->type == CONST)
        {
            auto derivative = differentiate<T>(begin, mid);
            derivative.emplace_back(CONST, NO_OP, (T) 1.0 / mid->val);
            derivative.emplace_back(BIN_OP, MUL);

            return derivative;
        }
        // If [g] is a variable, so [p_2] is [[f]] and [p_3] is [z z *]
        else if (mid->type == VAR)
        {
            p_2 = expr<T>(begin, mid);
            p_3 = {{VAR, NO_OP}, {VAR, NO_OP}, {BIN_OP, MUL}};
        }
        else
        {
            std::invalid_argument("Unexpected token encountered.");
        }
    }
    // In general, [p_1] is [[g'] [f] *]
    // In general, [p_3] is [[g] [g] *]
    else
    {
        p_2 = differentiate<T>(mid, end);
        p_2.insert(p_2.begin(), begin, mid);
        p_2.emplace_back(BIN_OP, MUL);
        
        p_3 = expr<T>(mid, end);
        p_3.insert(p_3.end(), mid, end);
        p_3.emplace_back(BIN_OP, MUL);
    }
    
    expr<T> p_1;
    // If [f] is a constant or a variable, [p_1] is simpler
    if (begin == std::prev(mid))
    {
        // [f] is a constant, so [p_1] is [0]
        if (begin->type == CONST)
        {
            p_1 = {{CONST, NO_OP, 0}};
        }
        // [f] is a variable, so [p_1] is [[g]]
        else if (begin->type == VAR)
        {
            p_1 = expr<T>(mid, end);
        }
        else
        {
            std::invalid_argument("Unexpected token encountered.");
        }
    }
    // In general, [p_1] is [[f'] [g] *]
    else
    {
        p_1 = differentiate<T>(begin, mid);
        p_1.insert(p_1.end(), mid, end);
        p_1.emplace_back(BIN_OP, MUL);
    }
    
    expr<T> derivative;
    // [0 [p_2] -] is equal to [[p_2] -]
    if (p_1.size() == 1 && p_1.front().type == CONST && p_1.front().val == std::complex<T>(0, 0))
    {
        derivative = std::move(p_2);
        derivative.emplace_back(FUNC, NEG);
    }
    // In general, the numerator of the derivative is [[p_1] [p_2] -]
    else
    {
        p_1.insert(p_1.end(), std::make_move_iterator(p_2.begin()), std::make_move_iterator(p_2.end()));
        p_1.emplace_back(BIN_OP, SUB);

        derivative = std::move(p_1);
    }

    derivative.insert(derivative.end(), std::make_move_iterator(p_3.begin()), std::make_move_iterator(p_3.end()));
    derivative.emplace_back(BIN_OP, DIV);
    return derivative;
}

/**
 * @brief Differentiates given postfix expression within specified bounds when
 * the expression ends with a MUL. Between begin and end (not inclusive), the
 * postfix should look like [[f] [g] ^].
 * 
 * @tparam T Floating point type used by expression.
 * @tparam iter Bidirectional iterator.
 * 
 * @param begin Iterator to first element of expression to differentiate.
 * @param end Iterator to one after the last element of expression to 
 * differentiate.
 * 
 * @return Derivative of postfix subexpression.
 * 
 * @warning Bidirectional iterator should point to type token<T>. 
 * 
 * @throw invalid_argument if prev(end) is not POW.
*/
template<std::floating_point T, std::bidirectional_iterator iter>
auto differentiate_bin_op_pow(iter begin, iter end) -> expr<T>
{
    // Decrement iterator to point to the binary operator.
    end--;
    if (end->op != POW)
    {
        throw std::invalid_argument("Expression to differentiate is not an addition or subtraction.");
    }

    // With end decremented, postfix[begin:end] looks like [[f] [g]], where [f]
    // and [g] are subexpressions. We want the middle index that points to the
    // first element of [g]/ one after last element of [f] to isolate the two
    // subexpressions [f] and [g].
    auto mid = expr<T>::subexpr_begin(end);
    // [g] = postfix[mid:end]
    // [f] = postfix[begin:mid]
  
    // d(f ^ g)/dz = g * f^{g - 1} * f' + f^g * g' * ln(f)
    // In the general case, the derivative is (absolute cringe incoming)
    // [[f'] [g] [f] [g] 1 - ^ * * [f] ln [g'] [f] [g] ^ * * +]
    // Let [p_1] be the expression [[f'] [g] [f] [g] 1 - ^ * *]
    // Let [p_2] be the expression [[g'] [f] ln [f] [g] ^ * *]
    // The derivative is then [[p_1] [p_2] +]

    expr<T> p_1;
    // If [f] is a constant or a variable
    if (begin == std::prev(mid))
    {
        // If [f] is a constant, [p_1] is [0]
        if (begin->type == CONST)
        {
            p_1 = {{CONST, NO_OP, 0}};

            // If [f] is exactly 0, we will simply return the derivative as [0]
            if (begin->val == std::complex<T>(0, 0))
            {
                return p_1;
            }
        }
        // If [f] is a variable, [p_1] is [[g] [f] [g] 1 - ^ *] 
        else if (begin->type == VAR)
        {
            p_1 = expr<T>(mid, end);
            p_1.insert(p_1.end(), begin, mid);
            // If [g] is a constant, the [[g] 1 -] portion can be simplified to
            // the constant [[g - 1]]
            if (mid == std::prev(end) && mid->type == CONST)
            {
                p_1.emplace_back(CONST, NO_OP, mid->val - (T) 1.0);
            }
            // If [g] is not a constant, push back [[g] 1 -]
            else
            {
                p_1.insert(p_1.end(), mid, end);
                p_1.emplace_back(CONST, NO_OP, 1.0);
                p_1.emplace_back(BIN_OP, SUB);
            }
            p_1.emplace_back(BIN_OP, POW);
            p_1.emplace_back(BIN_OP, MUL);
        }
        else
        {
            std::invalid_argument("Unexpected token encountered.");
        }
    }
    // In the general case, [p_1] is  [[f'] [g] [f] [g] 1 - ^ * *]
    else
    {
        p_1 = differentiate<T>(begin, mid);
        p_1.insert(p_1.end(), mid, end);
        p_1.insert(p_1.end(), begin, mid);
        p_1.insert(p_1.end(), mid, end);
        p_1.emplace_back(CONST, NO_OP, 1.0);
        p_1.emplace_back(BIN_OP, SUB);
        p_1.emplace_back(BIN_OP, POW);
        p_1.emplace_back(BIN_OP, MUL);
        p_1.emplace_back(BIN_OP, MUL);
    }

    expr<T> p_2;
    // If [g] is a constant or a variable
    if (mid == std::prev(end))
    {
        // If [g] is a constant, [p_2] is [0]
        if (mid->type == CONST)
        {
            p_2 = {{CONST, NO_OP, 0.0}};
        }
        // If [g] is a variable, [p_2] is [[f] ln [f] [g] ^ *]
        else if (mid->type == VAR)
        {
            p_2 = expr<T>(begin, mid);
            p_2.emplace_back(FUNC, LOG);
            p_2.insert(p_2.end(), begin, mid);
            p_2.insert(p_2.end(), mid, end);
            p_2.emplace_back(BIN_OP, POW);
            p_2.emplace_back(BIN_OP, MUL);
        }
        else
        {
            std::invalid_argument("Unexpected token encountered.");
        }
    }
    // In the general case, [p_1] is  [[g'] [f] ln [f] [g] ^ * *]
    else
    {
        p_2 = differentiate<T>(mid, end);
        p_2.insert(p_2.end(), begin, mid);
        p_2.emplace_back(FUNC, LOG);
        p_2.insert(p_2.end(), begin, mid);
        p_2.insert(p_2.end(), mid, end);
        p_2.emplace_back(BIN_OP, POW);
        p_2.emplace_back(BIN_OP, MUL);
        p_2.emplace_back(BIN_OP, MUL);
    }
    
    // [0 [p_2] +] is equal to [[p_2]]
    if (p_1.size() == 1 && p_1.front().type == CONST && p_1.front().val == std::complex<T>(0, 0))
    {
        return p_2;
    }
    // [[p_1] 0 +] is equal to [[p_1]]
    else if (p_2.size() == 1 && p_2.front().type == CONST && p_2.front().val == std::complex<T>(0, 0))
    {
        return p_1;
    }
    // In general, return [[p_1] [p_2] +]
    else
    {
        p_1.insert(p_1.end(), std::make_move_iterator(p_2.begin()), std::make_move_iterator(p_2.end()));
        p_1.emplace_back(BIN_OP, ADD);

        return p_1;
    }
}

/**
 * @brief Map a token representing a single argument function to a vector of 
 * tokens in a postfix expression representing its derivative.
 * 
 * @param t Token with type function.
 * @return Vector of tokens representing the derivative of input token.
 * @throw invalid_argument if derivative of input token not found.
*/
template<std::floating_point T>
auto get_deriv(token<T> t) -> expr<T>
{
    static std::unordered_map<operation, expr<T>> const table = {
        {operation::SIN, { {FUNC, COS} }},
        {operation::COS, { {FUNC, SIN} , {FUNC, NEG} }}
    };

    auto it = table.find(t.op);
    if (it != table.end())
    {
        return it->second;
    }
    else
    {
        throw std::invalid_argument("Derivative not found.");
    }
}

};