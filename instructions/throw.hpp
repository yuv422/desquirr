//
// Created by efry on 11/09/2017.
//

#ifndef DESQUIRR_THROW_HPP
#define DESQUIRR_THROW_HPP

#include "instruction.hpp"

/**
 * A throw instruction can have either zero or one parameters
 */
class Throw : public Instruction/*{{{*/
{
public:
    Throw(Addr ea, Expression_ptr exception, const std::string &dataType)
            : Instruction(THROW, ea), mException(exception), mDataType(dataType)
    {}

    virtual Instruction_ptr Copy()
    {
        if (IsRethrow())
            return Instruction_ptr(new Throw(Address()));

        return Instruction_ptr(new Throw(Address(), mException->Copy(), DataType()));
    }

    virtual void print(std::ostream &os)
    {
        Instruction::print(os);
        os << "THROW " << mDataType << " " << *mException << "\n";
    }

    Throw(Addr ea)
            : Instruction(THROW, ea)
    {}

    virtual void Accept(InstructionVisitor &visitor)
    {
        visitor.Visit(*this);
    }

    virtual int OperandCount()
    {
        return IsRethrow() ? 0 : 1;
    }

    virtual Expression_ptr Operand(int index)
    {
        Expression_ptr result;
        if (0 == index)
            result = mException;
        else
            msg("ERROR: Throw(%d) -> NULL\n", index);
        return result;
    }

    virtual void Operand(int index, Expression_ptr e)
    {
        if (0 == index)
            mException = e;
        else
            msg("ERROR: Throw(%d, %08lx)\n", index, e.get());
    }

    bool IsRethrow()
    {
        return NULL == mException.get();
    }

    Expression_ptr Exception()
    {
        return mException;
    }

    std::string DataType()
    { return mDataType; }

private:
    Expression_ptr mException;
    std::string mDataType;
};/*}}}*/

#endif //DESQUIRR_THROW_HPP
