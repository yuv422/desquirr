//
// Created by efry on 11/09/2017.
//

#ifndef DESQUIRR_DOWHILE_HPP
#define DESQUIRR_DOWHILE_HPP

#include "unaryinstruction.hpp"

class DoWhile : public UnaryInstruction/*{{{*/
{
public:
    DoWhile(Addr ea, Expression_ptr value)
            : UnaryInstruction(DO_WHILE, ea, value)
    { mStatements = new Node_list(); }

    virtual Instruction_ptr Copy()
    {
        DoWhile *dw = new DoWhile(Address(), Operand(0)->Copy());

        //FIXME need to copy loop nodes here. If we ever need to copy a DoWhile Instruction.

        return Instruction_ptr(dw);
    }

    virtual void print(std::ostream &os)
    {
        Instruction::print(os);
        os << "DO_WHILE " << *Operand(0) << "\n";
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

#endif //DESQUIRR_DOWHILE_HPP
