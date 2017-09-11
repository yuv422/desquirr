//
// Created by efry on 11/09/2017.
//

#ifndef DESQUIRR_CASE_HPP
#define DESQUIRR_CASE_HPP

#include "instruction.hpp"

class Case : public Instruction/*{{{*/
{
public:
    Case(Addr ea, std::vector<std::string> &values)
            : Instruction(CASE, ea), mValues(values)
    {}

    virtual Instruction_ptr Copy()
    {
        return Instruction_ptr(new Case(Address(), Values()));
    }

    virtual void print(std::ostream &os)
    {
        Instruction::print(os);
        os << "CASE FIXME addValueHere"; //FIXME boost::format("CASE %08lx\n") % Values();
    }

    std::vector<std::string> &Values()
    { return mValues; }

    virtual void Accept(InstructionVisitor &visitor)
    {
        visitor.Visit(*this);
    }

private:
    std::vector<std::string> mValues;
};/*}}}*/

#endif //DESQUIRR_CASE_HPP
