//
// Created by efry on 11/09/2017.
//

#ifndef DESQUIRR_BREAK_HPP
#define DESQUIRR_BREAK_HPP

#include "instruction.hpp"

class Break : public Instruction/*{{{*/
{
public:
    Break(Addr ea)
            : Instruction(BREAK, ea)
    {}

    virtual Instruction_ptr Copy()
    {
        return Instruction_ptr(new Break(Address()));
    }

    virtual void print(std::ostream &os)
    {
        Instruction::print(os);
        os << "BREAK\n";
    }

    virtual void Accept(InstructionVisitor &visitor)
    {
        visitor.Visit(*this);
    }

private:

};/*}}}*/

#endif //DESQUIRR_BREAK_HPP
