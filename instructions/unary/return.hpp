//
// Created by efry on 11/09/2017.
//

#ifndef DESQUIRR_RETURN_HPP
#define DESQUIRR_RETURN_HPP

#include "unaryinstruction.hpp"

class Return : public UnaryInstruction/*{{{*/
{
public:
    Return(Addr ea, Expression_ptr value)
            : UnaryInstruction(RETURN, ea, value)
    {}

    virtual Instruction_ptr Copy()
    {
        return Instruction_ptr(new Return(Address(), Operand(0)->Copy()));
    }

    virtual void print(std::ostream &os)
    {
        Instruction::print(os);
        os << "RETURN " << *Operand(0) << "\n";
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

#endif //DESQUIRR_RETURN_HPP
