//
// Created by efry on 11/09/2017.
//

#ifndef DESQUIRR_JUMP_HPP
#define DESQUIRR_JUMP_HPP

#include "unaryinstruction.hpp"

class Jump : public UnaryInstruction/*{{{*/
{
public:
    Jump(Addr ea, Expression_ptr destination)
            : UnaryInstruction(JUMP, ea, destination)
    {}

    virtual Instruction_ptr Copy()
    {
        return Instruction_ptr(new Jump(Address(), Operand(0)->Copy()));
    }

    virtual void print(std::ostream &os)
    {
        Instruction::print(os);
        os << "JUMP " << *Operand(0) << "\n";
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

#endif //DESQUIRR_JUMP_HPP
