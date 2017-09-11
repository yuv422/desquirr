//
// Created by efry on 11/09/2017.
//

#ifndef DESQUIRR_CONTINUE_HPP
#define DESQUIRR_CONTINUE_HPP

#include "instruction.hpp"

class Continue : public Instruction/*{{{*/
{
public:
    Continue(Addr ea)
            : Instruction(CONTINUE, ea)
    {}

    virtual Instruction_ptr Copy()
    {
        return Instruction_ptr(new Continue(Address()));
    }

    virtual void print(std::ostream &os)
    {
        Instruction::print(os);
        os << "CONTINUE\n";
    }

    virtual void Accept(InstructionVisitor &visitor)
    {
        visitor.Visit(*this);
    }

private:

};/*}}}*/

#endif //DESQUIRR_CONTINUE_HPP
