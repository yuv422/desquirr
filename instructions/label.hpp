//
// Created by efry on 11/09/2017.
//

#ifndef DESQUIRR_LABEL_HPP
#define DESQUIRR_LABEL_HPP

#include "instruction.hpp"

class Label : public Instruction/*{{{*/
{
public:
    Label(Addr ea, const char *name)
            : Instruction(LABEL, ea), mName(name)
    {}

    virtual Instruction_ptr Copy()
    {
        return Instruction_ptr(new Label(Address(), Name().c_str()));
    }

    virtual void print(std::ostream &os)
    {
        Instruction::print(os);
        os << boost::format("LABEL %s\n")
              % Name();
    }

    const std::string &Name() const
    { return mName; }

    virtual void Accept(InstructionVisitor &visitor)
    {
        visitor.Visit(*this);
    }

private:
    std::string mName;
};/*}}}*/

#endif //DESQUIRR_LABEL_HPP
