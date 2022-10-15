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
    // End points to one after the last element
    auto last = end;
    last--;

    // If expression to differentiate is a single variable, the derivative is 1
    if (last->type == VAR)
    {
        return expr<T>{{CONST, NO_OP, 1.0}};
    }
    // If expression to differentiate is a constant, the derivative is 1
    else if (last->type == CONST)
    {
        return expr<T>{{CONST, NO_OP, 0.0}};
    }
    // If expression to differentiate is a function of one variable, we call a
    // separate function to deal with that case
    else if (last->type == FUNC)
    {
        return differentiate_func<T>(begin, end);
    }
    // If expression to differentiate is a binary operation, we call a separate
    // function to deal with that case
    else if (last->type == BIN_OP)
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
 * @param begin Iterator to first element of expression to differentiate.
 * @param end Iterator to one after the last element of expression to 
 * 
 * @return Derivative of postfix subexpression.
 * 
 * @warning Bidirectional iterator should point to type token<T>. 
 * 
 * @throw invalid_argument if end-- is not of type FUNC.
*/
template<std::floating_point T, std::bidirectional_iterator iter>
auto differentiate_func(iter begin, iter end) -> expr<T>
{
    auto f = end;
    f--;

    if (f->type != FUNC)
    {
        throw std::invalid_argument("Expression to differentiate is not a function of one variable.");
    }

    // Input postfix [begin : end] looks like [[g] f], where [g] is itself
    // a subexpression, postfix[start : end - 1]. The derivative of 
    // postfix[start : end] is equivalent to [[g'] [g] [f'] *] by the chain rule,
    // where [f'] and [g'] are the postfix expressions representing the 
    // symbolic derivatives of f and [g], respectively.

    // Deal with special case where g(z) = c (so f'(g(z)) * g'(z) = 0) and when
    // g(z) = z so f'(g(z)) * g'(z) = f'(z). We do not necessarily need a separate
    // case, but without this special case, the final derivative will have 
    // expressions multiplied by 1 and 0, which adds unnecessary computations.
    auto g_end = f;
    g_end--;
    if (begin == g_end)
    {
        if (begin->type == CONST)
        {
            return expr<T>{{CONST, NO_OP, 1.0}};
        }
        else if (begin->type == VAR)
        {
            auto derivative = get_deriv(*f);        // [[f']]
            derivative.emplace_front(VAR, NO_OP);   // [z [f']]

            return derivative;
        }
        else
        {
            throw std::logic_error("Unexpected token encountered.");
        }
    }
    else
    {
        auto f_deriv = get_deriv(*f);
        auto g_deriv = differentiate<T>(begin, f);
        
        auto derivative = std::move(g_deriv);                                   // [[g']]
        derivative.insert(derivative.end(), begin, f);                          // [[g'] [g]]
        derivative.insert(derivative.end(), f_deriv.begin(), f_deriv.end());    // [[g'] [g] [f']]
        derivative.emplace_back(BIN_OP, MUL);                              // [[g'] [g] [f'] *]

        return derivative;
    }
}

/**
 * @brief Differentiates given postfix expression within specified bounds that
 * can be represented as f(z) · g(z) where · is one of +, -, *, / or ^, and 
 * g(z) and h(z) are any expressions. 
 * 
 * @param begin Iterator to first element of expression to differentiate.
 * @param end Iterator to one after the last element of expression to 
 * 
 * @return Derivative of postfix subexpression.
 * 
 * @warning Bidirectional iterator should point to type token<T>. 
 * 
 * @throw invalid_argument if end-- is not of type BIN_OP.
*/
template<std::floating_point T, std::bidirectional_iterator iter>
auto differentiate_bin_op(iter begin, iter end) -> expr<T>
{
    auto bin_op = end;
    bin_op--;

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
 * postfix[end - 1] is ADD or SUB. Between begin and end (not inclusive), 
 * the postfix should look like [[f] [g] ·], where [f] and [g] are themselves
 * postfix expressions and · is either + or -.
 * 
 * @param begin Iterator to first element of expression to differentiate.
 * @param end Iterator to one after the last element of expression to 
 * 
 * @return Derivative of postfix subexpression.
 * 
 * @warning Bidirectional iterator should point to type token<T>. 
 * 
 * @throw invalid_argument if postfix[end - 1] is not ADD or SUB.
*/
template<std::floating_point T, std::bidirectional_iterator iter>
auto differentiate_bin_op_add_sub(iter begin, iter end) -> expr<T>
{
    // Iterator pointing to the binary operator.
    auto op = end;
    op--;
    if (op->type != ADD && op->type != SUB)
    {
        throw std::invalid_argument("Expression to differentiate is not an addition or subtraction.");
    }



    throw std::logic_error("Not yet implemented.");
}

/**
 * @brief Differentiates given postfix expression within specified bounds when
 * postfix[end - 1] is MUL.
 * 
 * @param postfix Postfix expression. Between start and end (not inclusive), 
 * the postfix should look like [[f] [g] *], where [f] and [g] are themselves
 * postfix expressions.
 * @param start Index of first token of subexpression to differentiate.
 * @param end Index after last token of subexpression to differentiate.
 * 
 * @return Derivative of postfix subexpression.
 * 
 * @throw invalid_argument if postfix[end - 1] is not MUL.
*/
template<std::floating_point T, std::bidirectional_iterator iter>
auto differentiate_bin_op_mul(iter begin, iter end) -> expr<T>
{
    throw std::logic_error("Not yet implemented.");
}

/**
 * @brief Differentiates given postfix expression within specified bounds when
 * postfix[end - 1] is DIV.
 * 
 * @param postfix Postfix expression. Between start and end (not inclusive), 
 * the postfix should look like [[f] [g] /], where [f] and [g] are themselves
 * postfix expressions.
 * @param start Index of first token of subexpression to differentiate.
 * @param end Index after last token of subexpression to differentiate.
 * 
 * @return Derivative of postfix subexpression.
 * 
 * @throw invalid_argument if postfix[end - 1] is not DIV.
*/
template<std::floating_point T, std::bidirectional_iterator iter>
auto differentiate_bin_op_div(iter begin, iter end) -> expr<T>
{
    throw std::logic_error("Not yet implemented.");
}

/**
 * @brief Differentiates given postfix expression within specified bounds when
 * postfix[end - 1] is POW.
 * 
 * @param postfix Postfix expression. Between start and end (not inclusive), 
 * the postfix should look like [[f] [g] ^], where [f] and [g] are themselves
 * postfix expressions.
 * @param start Index of first token of subexpression to differentiate.
 * @param end Index after last token of subexpression to differentiate.
 * 
 * @return Derivative of postfix subexpression.
 * 
 * @throw invalid_argument if postfix[end - 1] is not POW.
*/
template<std::floating_point T, std::bidirectional_iterator iter>
auto differentiate_bin_op_pow(iter begin, iter end) -> expr<T>
{
    throw std::logic_error("Not yet implemented.");
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