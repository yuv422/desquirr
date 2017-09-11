//
// Created by efry on 11/09/2017.
//

#ifndef DESQUIRR_ASSIGNMENT_HPP
#define DESQUIRR_ASSIGNMENT_HPP

#include "binary.hpp"

class Assignment : public BinaryInstruction/*{{{*/
{
public:
    Assignment(Addr ea, Expression_ptr destination, Expression_ptr source)
            : BinaryInstruction(ASSIGNMENT, ea, destination, source)
    {}

    virtual Instruction_ptr Copy()
    {
        return Instruction_ptr(new Assignment(Address(), Operand(0)->Copy(), Operand(1)->Copy()));
    }

    virtual void print(std::ostream &os)
    {
        Instruction::print(os);
        os << "ASSIGN " << *Operand(0) << " := " << *Operand(1) << "\n";
    }

    bool IsCall()
    {
        // TODO: make proper implementation
        return Second()->IsType(Expression::CALL);
    }

    virtual void Accept(InstructionVisitor &visitor)
    {
        visitor.Visit(*this);
    }

    virtual OperandTypeValue OperandType(int index)
    {
        if (0 == index)
        {
            if (Operand(0)->IsType(Expression::UNARY_EXPRESSION))
            {
                // TODO: verify that the Operand is UnaryExpression("*", Register())
                // This is an indirect store, so we use the operand, not define it!
                return USE;
            }

            return DEFINITION;
        }
        else
            return USE;
    }

    virtual bool RemoveDefinition(unsigned short reg)
    {
        if (!First()->IsType(Expression::REGISTER))
        {
            message("%p Error: trying to remove non-register defintion in assignment\n",
                    Address());
            return false;
        }

        Register *expression = static_cast<Register *>(First().get());

        if (reg != expression->Index())
        {
            message("%p Error: trying to remove a non-existing defintion: %d/%d\n", Address(), reg,
                    expression->Index());
            return false;
        }

        // Replace defintion with dummy instruction
        First(Dummy::Create());
        Definitions().Clear(reg);

        // TODO: if the second operand does not contain a CALL expression we can
        // return true here
        return !IsCall();
    }
};/*}}}*/

#endif //DESQUIRR_ASSIGNMENT_HPP
