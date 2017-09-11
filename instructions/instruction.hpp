// 
// Copyright (c) 2002 David Eriksson <david@2good.nu>
// 
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
// $Id: instruction.hpp,v 1.9 2007/01/30 09:49:30 wjhengeveld Exp $
#ifndef _INSTRUCTION_HPP
#define _INSTRUCTION_HPP
/*
 *
Instruction   ... ea
    LowLevel  ... insn_t
    Label     ... name
    Case      ... case_value
    Throw     ... exception_expr, datatype
    UnaryInstruction  ... operand
        Push              ... operand
        Pop               ... operand
        Jump              ... target=opnd
        Return            ... returnvalue=opnd
        Switch            ... switchvalue=opnd, ???
---     Call              ... target=opnd
    BinaryInstruction  ... first, second
        ConditionalJump   ... cond=first, target=second
        Assignment        ... dest=first, src=second
 */
#include <sstream>
#include <vector>
#include "desquirr.hpp"

#include "pro.h"
#include "kernwin.hpp"
// Local includes

#include "expression.hpp"

class Expression;

typedef std::list<Expression *> ExpressionList;

std::ostream &operator<<(std::ostream &os, const RegisterToAddress_map &vs);

//typedef std::list<op_t> OperandList;

/**
 * For Defined and Used 
 */
class BoolArray/*{{{*/
{
protected:
    typedef unsigned long BITFIELD;

    BoolArray(BITFIELD bitfield)
            : mBitfield(bitfield)
    {}

public:
    enum
    {
        SIZE = 22   // XXX: the size is just a nice number
    };

    BoolArray()
    {
        Clear();
    }

    BoolArray(const BoolArray &other)
            : mBitfield(other.mBitfield)
    {
    }


    BoolArray operator~() const
    {
        return BoolArray(~mBitfield);
    }

    bool operator!=(const BoolArray &other) const
    {
        return other.mBitfield != mBitfield;
    }

    void operator|=(const BoolArray &other)
    {
        mBitfield |= other.mBitfield;
    }

    BoolArray operator|(const BoolArray &other) const
    {
        return BoolArray(mBitfield | other.mBitfield);
    }

    BoolArray operator&(const BoolArray &other) const
    {
        return BoolArray(mBitfield & other.mBitfield);
    }

    bool Get(int i) const
    {
        if (i >= 0 && i < SIZE)
            return 0 != (mBitfield & POWER_OF_2[i]);
        else
            return false;
    }

    void Set(int i)
    {
        if (i >= 0 && i < SIZE)
            mBitfield |= POWER_OF_2[i];
    }

    void Clear(int i)
    {
        mBitfield &= ~POWER_OF_2[i];
    }

    void Clear()
    {
        mBitfield = 0;
    }

    void Or(BoolArray &other)
    {
        mBitfield |= other.mBitfield;
    }

    int CountSet() const
    {
        int count = 0;
        for (int i = 0; i < SIZE; i++)
            if (mBitfield & POWER_OF_2[i])
                count++;
        return count;
    }

    friend std::ostream &operator<<(std::ostream &os, const BoolArray &ba)
    {
        bool first = true;
        os << '{';
        for (int i = 0; i < SIZE; i++)
        {
            if (ba.Get(i))
            {
                if (first)
                    first = false;
                else
                    os << ", ";

                os << Register::Name(i);
            }
        }
        os << '}';
        return os;
    }

private:
    BITFIELD mBitfield;

    static const int POWER_OF_2[SIZE];
};/*}}}*/

class Assignment;

class Case;

class ConditionalJump;

class Jump;

class Label;

class LowLevel;

class Push;

class Pop;

class Return;

class Switch;

class DoWhile;

class While;

class iff;

class Break;

class Continue;

class Throw;

/**
 * Abstract base class for Instruction visitors
 */
class InstructionVisitor
{
public:
    virtual ~InstructionVisitor()
    {}

    virtual void Visit(Assignment &)      = 0;

    virtual void Visit(Case &)            = 0;

    virtual void Visit(ConditionalJump &) = 0;

    virtual void Visit(Jump &)            = 0;

    virtual void Visit(Label &)           = 0;

    virtual void Visit(LowLevel &)        = 0;

    virtual void Visit(Push &)            = 0;

    virtual void Visit(Pop &)             = 0;

    virtual void Visit(Return &)          = 0;

    virtual void Visit(Switch &)          = 0;

    virtual void Visit(DoWhile &)         = 0;

    virtual void Visit(While &)           = 0;

    virtual void Visit(iff &)              = 0;

    virtual void Visit(Break &)           = 0;

    virtual void Visit(Continue &)        = 0;

    virtual void Visit(Throw &)           = 0;

    // Helper functions when accepting node lists
    virtual void NodeBegin(Node_ptr)
    {}

    virtual void NodeEnd()
    {}
};

void Accept(Node_list &nodes, InstructionVisitor &visitor);

void Accept(Instruction_list &instructions, InstructionVisitor &visitor);


/**
 * an instruction
 */
class Instruction/*{{{*/
{
public:
    enum InstructionType
    {
        ASSIGNMENT,
        CALL,
        CASE,
        CONDITIONAL_JUMP,
        JUMP,
        LABEL,
        LOW_LEVEL,
        PUSH,
        POP,
        RETURN,
        SWITCH,
        THROW,
        DO_WHILE,
        WHILE,
        IF,
        FOR,
        BREAK,
        CONTINUE,
        TO_BE_DELETED
    };

    enum OperandTypeValue
    {
        INVALID,
        DEFINITION,
        USE,
        USE_AND_DEFINITION
    };

    virtual ~Instruction()
    {}

    virtual Instruction_ptr Copy()
    {
        return Instruction_ptr();
    }

#if 1

    virtual int OperandCount()
    {
        // Default implementation
        return 0;
    }

    virtual Expression_ptr Operand(int index)
    {
        // Default implementation
        Expression_ptr result;
        msg("ERROR: default implementation for Instruction::Operand called\n");
        return result;
    }

    virtual void Operand(int index, Expression_ptr e)
    {
        // Default implementation
    }

    virtual OperandTypeValue OperandType(int index)
    {
        return INVALID;
    }

#endif

    virtual Addr Address() const
    { return mAddress; }

    virtual InstructionType Type() const
    { return mType; }

    virtual bool IsType(InstructionType type)
    {
        return Type() == type;
    }

    virtual void Accept(InstructionVisitor &visitor) = 0;

    /**
     * Return true if the whole instruction can be removed
     */
    virtual bool RemoveDefinition(unsigned short reg)
    {
        // Do nothing by default
        return false;
    }

    BoolArray &Definitions()
    { return mDefinitions; }

    BoolArray &Uses()
    { return mUses; }

    BoolArray &LastDefinitions()
    { return mLastDefinitions; }

    BoolArray &FlagDefinitions()
    { return mFlagDefinitions; }

    RegisterToAddress_map &DuChain()
    { return mDuChain; }

    void AddToDuChain(unsigned short reg, Addr address)
    {
        mDuChain.insert(
                RegisterToAddress_pair(reg, address)
        );
    }

    bool DefinitionHasNoUses(unsigned short reg)
    {
        return mDuChain.count(reg) == 0;
    }

    void SetLastDefinition(unsigned short reg)
    {
        mLastDefinitions.Set(reg);
    }

    bool IsLastDefinition(unsigned short reg)
    {
        return mLastDefinitions.Get(reg);
    }

    bool MarkForDeletion()/*{{{*/
    {
        if (TO_BE_DELETED == mType)
        {
            return false;
        }
        else
        {
            mType = TO_BE_DELETED;
            return true;
        }
    }/*}}}*/

    static void FindDefintionUseChains(Instruction_list &instructions);

    static void DumpInstructionList(Instruction_list &insns);

    friend std::ostream &operator<<(std::ostream &os, Instruction &insn)
    {
        insn.print(os);
        return os;
    }

    virtual void print(std::ostream &os)
    {
        os << boost::format("   insn %08lx")
              % Address();
        os << " use=" << Uses();
        os << " def=" << Definitions();
        os << " last=" << LastDefinitions();
        os << " flag=" << FlagDefinitions();
        os << " chain=" << DuChain();
    }

    Node_list &Statements()
    {
        return mStatements;
    }

protected:
    Instruction(InstructionType type, Addr ea)
            : mType(type), mAddress(ea)
    {}

private:
    InstructionType mType;
    Addr mAddress;
    BoolArray mDefinitions;
    BoolArray mUses;
    BoolArray mLastDefinitions;
    BoolArray mFlagDefinitions;
    RegisterToAddress_map mDuChain;
    Node_list mStatements;

};/*}}}*/

/*
 * No-operand instructions
 */






/*
 * Single-operand instructions
 */









/*
 * Double-operand instructions
 */






/*
 * Switch/case instructions
 */



















class ErasePool/*{{{*/
{
private:
    typedef std::list<Instruction_list::iterator> IteratorList;

    IteratorList mIterators;
    Instruction_list &mInstructions;

    struct EraseHelper
    {
        EraseHelper(Instruction_list &instructions)
                : mInstructions(instructions)
        {}

        Instruction_list &mInstructions;

        void operator()(Instruction_list::iterator item)
        {
            mInstructions.erase(item);
        }
    };

public:
    ErasePool(Instruction_list &instructions)
            : mInstructions(instructions)
    {}

    ~ErasePool()
    {
        for_each(mIterators.begin(), mIterators.end(), EraseHelper(mInstructions));
    }

    void Erase(Instruction_list::iterator item)
    {
        if ((**item).MarkForDeletion())
        {
            mIterators.push_back(item);
        }
    }
};/*}}}*/

#endif // _INSTRUCTION_HPP

