//
// Created by efry on 11/09/2017.
//

#ifndef DESQUIRR_PUSH_HPP
#define DESQUIRR_PUSH_HPP

#include "unaryinstruction.hpp"

class Push : public UnaryInstruction/*{{{*/
{
public:
    Push(Addr ea, Expression_ptr operand)
            : UnaryInstruction(PUSH, ea, operand)
    {}

    virtual Instruction_ptr Copy()
    {
        return Instruction_ptr(new Push(Address(), Operand(0)->Copy()));
    }

    virtual void print(std::ostream &os)
    {
        Instruction::print(os);
        os << "PUSH " << *Operand(0) << "\n";
    }

    virtual void Accept(InstructionVisitor &visitor)
    {
        visitor.Visit(*this);
    }

    virtual OperandTypeValue OperandType(int index)
    {
        return USE;
    }
};/*}}}*/

#endif //DESQUIRR_PUSH_HPP
