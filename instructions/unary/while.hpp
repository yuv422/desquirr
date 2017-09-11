//
// Created by efry on 11/09/2017.
//

#ifndef DESQUIRR_WHILE_HPP
#define DESQUIRR_WHILE_HPP

#include "unaryinstruction.hpp"

class While : public UnaryInstruction/*{{{*/
{
public:
    While(Addr ea, Expression_ptr value)
            : UnaryInstruction(WHILE, ea, value)
    { mStatements = new Node_list(); }

    virtual Instruction_ptr Copy()
    {
        While *w = new While(Address(), Operand(0)->Copy());

        //FIXME need to copy loop nodes here. If we ever need to copy a While Instruction.

        return Instruction_ptr(w);
    }

    virtual void print(std::ostream &os)
    {
        Instruction::print(os);
        os << "WHILE " << *Operand(0) << "\n";
    }

    virtual void Accept(InstructionVisitor &visitor)
    {
        visitor.Visit(*this);
    }

    virtual OperandTypeValue OperandType(int index)
    {
        return USE;
    }

    void AddLoopNode(Node_ptr n)
    {
        Node_ptr np(n);
        Statements().push_back(np);
    }

private:

};/*}}}*/

#endif //DESQUIRR_WHILE_HPP
