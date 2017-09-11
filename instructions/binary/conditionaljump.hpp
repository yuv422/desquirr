//
// Created by efry on 11/09/2017.
//

#ifndef DESQUIRR_CONDITIONALJUMP_HPP
#define DESQUIRR_CONDITIONALJUMP_HPP

#include "binary.hpp"

class ConditionalJump : public BinaryInstruction/*{{{*/
{
public:
    ConditionalJump(Addr ea,
                    Expression_ptr condition, Expression_ptr destination)
            : BinaryInstruction(CONDITIONAL_JUMP, ea, condition, destination)
    {}

    virtual Instruction_ptr Copy()
    {
        return Instruction_ptr(new ConditionalJump(Address(), Operand(0)->Copy(), Operand(1)->Copy()));
    }

    virtual void print(std::ostream &os)
    {
        Instruction::print(os);
        os << "CONDITIONAL (" << *Operand(0) << ")  goto " << *Operand(1) << "\n";
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

#endif //DESQUIRR_CONDITIONALJUMP_HPP
