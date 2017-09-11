//
// Created by efry on 11/09/2017.
//

#ifndef DESQUIRR_IF_H
#define DESQUIRR_IF_H

#include "instruction.hpp"
#include "unaryinstruction.hpp"

    class If : public UnaryInstruction/*{{{*/
    {
    public:
        iff(Addr ea, Expression_ptr value)
                : UnaryInstruction(IF, ea, value)
        {}

        virtual Instruction_ptr Copy()
        {
            return Instruction_ptr(new iff(Address(), Operand(0)->Copy()));
        }

        virtual void print(std::ostream &os)
        {
            Instruction::print(os);
            os << "IF " << *Operand(0) << "\n";
        }

        virtual void Accept(InstructionVisitor &visitor)
        {
            visitor.Visit(*this);
        }

        virtual OperandTypeValue OperandType(int index)
        {
            return USE;
        }

        Node_ptr trueNode;
        Node_ptr falseNode;

    private:


    };/*}}}*/



#endif //DESQUIRR_IF_H
