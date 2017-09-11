//
// Created by efry on 11/09/2017.
//

#ifndef DESQUIRR_POP_HPP
#define DESQUIRR_POP_HPP

#include "unaryinstruction.hpp"

class Pop : public UnaryInstruction/*{{{*/
{
public:
    Pop(Addr ea, Expression_ptr operand)
            : UnaryInstruction(POP, ea, operand)
    {}

    virtual Instruction_ptr Copy()
    {
        return Instruction_ptr(new Pop(Address(), Operand(0)->Copy()));
    }

    virtual void print(std::ostream &os)
    {
        Instruction::print(os);
        os << "POP " << *Operand(0) << "\n";
    }

    virtual void Accept(InstructionVisitor &visitor)
    {
        visitor.Visit(*this);
    }

    virtual OperandTypeValue OperandType(int index)
    {
        return DEFINITION;
    }

    virtual bool RemoveDefinition(unsigned short reg)
    {
        if (!Operand().get())
        {
            message("%p Error: no operand!\n", Address());
            return false;
        }

        if (!Operand()->IsType(Expression::REGISTER))
        {
            message("Error: trying to remove non-register defintion in POP\n");
            return false;
        }

        Register *expression = static_cast<Register *>(Operand().get());

        if (reg != expression->Index())
        {
            message("Error: trying to remove a non-existing defintion in POP\n");
            return false;
        }

        Operand(Dummy::Create());

        Definitions().Clear(reg);

        return false;
    }
};/*}}}*/

#endif //DESQUIRR_POP_HPP
