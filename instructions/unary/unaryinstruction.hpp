//
// Created by efry on 11/09/2017.
//

#ifndef DESQUIRR_UNARYINSTRUCTION_H
#define DESQUIRR_UNARYINSTRUCTION_H

#include "instruction.hpp"

/**
 * Abstract base class for single-operand instructions
 */
class UnaryInstruction : public Instruction/*{{{*/
{
public:
    void Operand(Expression_ptr operand)
    { mOperand = operand; }

    Expression_ptr Operand()
    { return mOperand; }


#if 1

    virtual int OperandCount()
    {
        return 1;
    }

    virtual Expression_ptr Operand(int index)
    {
        Expression_ptr result;
        if (0 == index)
            result = mOperand;
        else
            msg("ERROR: UnaryInstruction::Operand(%d) -> NULL\n", index);
        return result;
    }

    virtual void Operand(int index, Expression_ptr e)
    {
        if (0 == index)
            mOperand = e;
        else
            msg("ERROR: UnaryInstruction(%d, %08lx)\n", index, e.get());
    }

#endif

protected:
    UnaryInstruction(InstructionType type, Addr ea, Expression_ptr operand)
            : Instruction(type, ea), mOperand(operand)
    {}

private:
    Expression_ptr mOperand;
};/*}}}*/


#endif //DESQUIRR_UNARYINSTRUCTION_H
