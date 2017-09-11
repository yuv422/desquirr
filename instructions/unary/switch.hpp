//
// Created by efry on 11/09/2017.
//

#ifndef DESQUIRR_SWITCH_HPP
#define DESQUIRR_SWITCH_HPP

#include "unaryinstruction.hpp"

class Switch : public UnaryInstruction/*{{{*/
{
public:
    Switch(Addr ea, Expression_ptr value/*, switch_info_t& si*/)
            : UnaryInstruction(SWITCH, ea, value)//, mSwitchInfo(si)
    {}

    virtual Instruction_ptr Copy()
    {
        return Instruction_ptr(new Switch(Address(), Operand(0)->Copy()));
    }

    virtual void print(std::ostream &os)
    {
        Instruction::print(os);
        os << "SWITCH " << *Operand(0) << "\n";
    }

    virtual void Accept(InstructionVisitor &visitor)
    {
        visitor.Visit(*this);
    }

    virtual OperandTypeValue OperandType(int index)
    {
        return USE;
    }

private:
    //switch_info_t mSwitchInfo;
};/*}}}*/

#endif //DESQUIRR_SWITCH_HPP
