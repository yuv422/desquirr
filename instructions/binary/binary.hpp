//
// Created by efry on 11/09/2017.
//

#ifndef DESQUIRR_BINARY_HPP
#define DESQUIRR_BINARY_HPP

#include "instruction.hpp"

/**
 * Abstract base class for double-operand instructions
 */
class BinaryInstruction : public Instruction/*{{{*/
{
public:
    void First(Expression_ptr first)
    { mFirst = first; }

    Expression_ptr First()
    { return mFirst; }

    void Second(Expression_ptr second)
    { mSecond = second; }

    Expression_ptr Second()
    { return mSecond; }

#if 1

    virtual int OperandCount()
    {
        return 2;
    }

    virtual Expression_ptr Operand(int index)
    {
        Expression_ptr result;
        if (0 == index)
            result = mFirst;
        else if (1 == index)
            result = mSecond;
        else
            msg("ERROR: BinaryInstruction::Operand(%d) -> NULL\n", index);
        return result;
    }

    virtual void Operand(int index, Expression_ptr e)
    {
        if (0 == index)
            mFirst = e;
        else if (1 == index)
            mSecond = e;
        else
            msg("ERROR: BinaryInstruction(%d, %08lx)\n", index, e.get());
    }

#endif

protected:
    BinaryInstruction(InstructionType type, Addr ea,
                      Expression_ptr first, Expression_ptr second)
            : Instruction(type, ea), mFirst(first), mSecond(second)
    {}

private:
    Expression_ptr mFirst;
    Expression_ptr mSecond;
};/*}}}*/

#endif //DESQUIRR_BINARY_HPP
