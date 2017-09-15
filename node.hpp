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
// $Id: node.hpp,v 1.7 2007/01/30 09:49:41 wjhengeveld Exp $
#ifndef _NODE_HPP
#define _NODE_HPP

/*
 *  class hierarchy:

Node  ... begin-end
    ReturnNode   ! when last=ret
    OneWayNode   ... successor
       FallThroughNode    ! when succ=label
       JumpNode           ! when last=jump
    TwoWayNode   ... succA, succB
       ConditinalJumpNode ! when last=jcond
       CallNode           ! when last=call
    N_WayNode     ... list! when last=load PC with expression.
	DoWhileNode
*/

#include <boost/dynamic_bitset.hpp>

//
// Local includes
// 
#include "desquirr.hpp"
#include "instruction.hpp"

class Node/*{{{*/
{
public:
    enum NodeType
    {
        CALL,                                // XXX: not yet implemented
        CONDITIONAL_JUMP,
        FALL_THROUGH,
        JUMP,
        N_WAY,                            // XXX: not yet implemented
        RETURN,
        DO_WHILE,
        LOOP_CONTINUE,
        LOOP_BREAK,
        TO_BE_DELETED
    };

    virtual Node_ptr Copy()
    {
        return Node_ptr();
    }

    static Node_ptr CreateFrom(Addr address); //ERIC hack for my new IF instruction.

    static void RemoveDeletedNodes(Node_list &nodes);

    Instruction_list &Instructions()
    { return mInstructions; }

    int InstructionCount()
    { return Instructions().size(); }

    Addr Address() const
    { return mAddress; }

    void Address(Addr address)
    {
        mAddress = address;
    }

    NodeType Type() const
    { return mType; }

    const char *TypeString()
    {
        switch (mType)
        {
            case CALL : return "CALL";
                break;
            case CONDITIONAL_JUMP : return "CONDITIONAL_JUMP";
                break;
            case FALL_THROUGH : return "FALL_THROUGH";
                break;
            case JUMP : return "JUMP";
                break;
            case N_WAY : return "N_WAY";
                break;
            case RETURN : return "RETURN";
                break;
            case DO_WHILE : return "DO_WHILE";
                break;
            case LOOP_CONTINUE : return "LOOP_CONTINUE";
                break;
            case LOOP_BREAK : return "LOOP_BREAK";
                break;
            case TO_BE_DELETED : return "TO_BE_DELETED";
                break;
            default : return "UNKNOWN";
                break;
        }

        return "";
    }

    BoolArray &Definitions()
    { return mDefinitions; }

    BoolArray &Uses()
    { return mUses; }

    BoolArray &LiveIn()
    { return mLiveIn; }

    BoolArray &LiveOut()
    { return mLiveOut; }

    bool InLiveOut(short int reg)
    {
        return mLiveOut.Get(reg);
    }

    virtual int SuccessorCount()
    {
        // default implementation
        return 0;
    }

    int PredecessorCount()
    {
        return (int)mPreds.size();
    }

    virtual Addr SuccessorAddress(int index)
    {
        // default implementation
        return INVALID_ADDR;
    }

    virtual Node_ptr Successor(int index)
    {
        Node_ptr result;
        msg("ERROR: Node::Successor called\n");
        return result;
    }

    virtual bool ConnectSuccessor(int index, Node_ptr successor)
    {
        // default implementation
        return false;
    }

    virtual bool ReconnectSuccessor(Node_ptr old_successor, Node_ptr new_successor)
    {
        // default implementation
        return false;
    }

    void ConnectPredecessor(Node_ptr predecessor)
    {
        mPreds.push_back(predecessor);
    }

    virtual void Cleanup(bool cleanAll)
    {
        return;
    }

    bool IsSingleJumpNode()
    {
        int i = InstructionCount();
        if (i == 0)
            return false;

        if (Instructions().front()->Type() == Instruction::LABEL)
            i--;

        if ((mType == JUMP || mType == CONDITIONAL_JUMP) && i == 1)
            return true;

        return false;
    }

    bool IsMarkedForDeletion()
    {
        return Type()==TO_BE_DELETED;
    }

    friend std::ostream &operator<<(std::ostream &os, Node &n)
    {
        n.print(os);
        printlist(os, n.Instructions());
        return os;
    }

    virtual void print(std::ostream &os)
    {
        os << boost::format("node %08lx-%08lx #insn=%d")
              % Address()
              % (Instructions().size() ? Instructions().back()->Address() : 0)
              % Instructions().size();
        os << " use=" << Uses();
        os << " def=" << Definitions();
        os << " in=" << LiveIn();
        os << " out=" << LiveOut();

        os << " pred={";
        for (Node_list::iterator p = mPreds.begin();
             p != mPreds.end();
             p++)
        {
            Node_ptr pred = *p;
            os << boost::format(" %X") % pred->Address();
        }

        os << "} ";

    }

    static void CreateList(Instruction_list &instructions,
                           Node_list &nodes);

    static void ConnectSuccessors(Node_list &nodes);

    static void CreateSharedNodes(Node_list &nodes);

    static void FindDefintionUseChains(Node_list &nodes);

    static void LiveRegisterAnalysis(Node_list &nodes);

    template<class T>
    void SetDominators(T const &d)
    {
        mDominators = d;
    }

protected:
    Node(NodeType type,
         Instruction_list::iterator begin,
         Instruction_list::iterator end)
            : mAddress(INVALID_ADDR), mType(type)
    {
        for (Instruction_list::iterator item = begin;
             item != end;
             item++)
        {
            mInstructions.push_back(*item);
        }

        if (mInstructions.size())
        {
            mAddress = (**mInstructions.begin()).Address();
        }
        else
        {
            message("Warning! Empty node of type %i created!\n", mType);
        }
    }


public:
    Node(NodeType type, Addr address)
            : mAddress(address), mType(type)
    {
    }

    virtual ~Node()
    { msg("Node destructor at %a\n", mAddress); }

    void CopySuccessors(Node_ptr node);

    boost::dynamic_bitset<> mDominators;
    boost::dynamic_bitset<> mPostDominators;

    //test if this node dominates node 'n'
    bool DominatesNode(Node_ptr n)
    {
        return n->mDominators.test(domId);
    }

    void MarkForDeletion()
    {
        mType = TO_BE_DELETED;
    }

    int domId;
    Node_list mPreds;
protected:


private:
    Addr mAddress;
    NodeType mType;
    Instruction_list mInstructions;

    BoolArray mUses;
    BoolArray mDefinitions;
    BoolArray mLiveIn;
    BoolArray mLiveOut;


    static const std::vector<Addr> GetSwitchExpression(Addr address);
};/*}}}*/

class OneWayNode : public Node/*{{{*/
{
protected:
    OneWayNode(NodeType type, Addr successor,
               Instruction_list::iterator begin,
               Instruction_list::iterator end)
            : Node(type, begin, end), mSuccessorAddress(successor)
    {}

    virtual int SuccessorCount()
    {
        return 1;
    }

    virtual Addr SuccessorAddress(int index)
    {
        return 0 == index ? mSuccessorAddress : INVALID_ADDR;
    }

    virtual Node_ptr Successor(int index)
    {
        Node_ptr result;
        if (0 == index)
            result = mSuccessor;
        else
            msg("ERROR: OneWayNode::Successor(%d) called\n", index);
        return result;
    }

    virtual bool ConnectSuccessor(int index, Node_ptr successor)
    {
        if (0 == index && successor->Address() == mSuccessorAddress)
        {
            mSuccessor = successor;
            return true;
        }
        else
            return false;
    }

    virtual bool ReconnectSuccessor(Node_ptr old_successor, Node_ptr new_successor)
    {
        if (old_successor == mSuccessor)
        {
            mSuccessor = new_successor;
            mSuccessorAddress = new_successor->Address();
            return true;
        }

        return false;
    }

private:
    Addr mSuccessorAddress;
    Node_ptr mSuccessor;
};/*}}}*/

class TwoWayNode : public Node/*{{{*/
{
protected:
    TwoWayNode(NodeType type, Addr successorA, Addr successorB,
               Instruction_list::iterator begin,
               Instruction_list::iterator end)
            : Node(type, begin, end)
    {
        mSuccessorAddress[0] = successorA;
        mSuccessorAddress[1] = successorB;
    }

    virtual int SuccessorCount()
    {
        return 2;
    }

    virtual Addr SuccessorAddress(int index)
    {
        switch (index)
        {
            case 0:
            case 1: return mSuccessorAddress[index];
            default: return INVALID_ADDR;
        }
    }

    virtual Node_ptr Successor(int index)
    {
        Node_ptr result;
        switch (index)
        {
            case 0:
            case 1: result = mSuccessor[index];
                break;
            default: msg("ERROR: TwoWayNode::Successor(%d) called\n", index);
        }
        return result;
    }

    virtual bool ConnectSuccessor(int index, Node_ptr successor)
    {
        switch (index)
        {
            case 0:
            case 1:
                if (successor->Address() == mSuccessorAddress[index])
                {
                    mSuccessor[index] = successor;
                    return true;
                }
                // fall through

            default: return false;
        }
    }

    virtual bool ReconnectSuccessor(Node_ptr old_successor, Node_ptr new_successor)
    {
        bool matched = false;
        if (old_successor == mSuccessor[0])
        {
            mSuccessor[0] = new_successor;
            mSuccessorAddress[0] = new_successor->Address();
            matched = true;
        }

        if (old_successor == mSuccessor[1])
        {
            mSuccessor[1] = new_successor;
            mSuccessorAddress[1] = new_successor->Address();
            matched = true;
        }

        return matched;
    }

    void ConnectSuccessorAddress(int index, Addr a)
    {
        switch (index)
        {
            case 0:
            case 1:

                mSuccessorAddress[index] = a;
            default: break;
        }
    }

private:
    Addr mSuccessorAddress[2];
    Node_ptr mSuccessor[2];
};/*}}}*/

class N_WayNode : public Node/*{{{*/
{
    public:
        N_WayNode(const std::vector<Addr> &successor_list,
                Instruction_list::iterator begin,
                Instruction_list::iterator end)
            : Node(N_WAY, begin, end)
        {
            mSuccessorAddress = successor_list;
            mSuccessor.resize(successor_list.size());
        }

        virtual int SuccessorCount()
        {
            return mSuccessorAddress.size();
        }

        virtual Addr SuccessorAddress(int index)
        {
            if (index<0 || index>=mSuccessorAddress.size())
                return INVALID_ADDR;
            return mSuccessorAddress[index];
        }

        virtual Node_ptr Successor(int index)
        {
            Node_ptr result;
            if (index<0 || index>=mSuccessor.size()) {
                msg("ERROR: N_WayNode::Successor(%d) called\n", index);
                return result;
            }

            return mSuccessor[index];
        }

        virtual bool ConnectSuccessor(int index, Node_ptr successor)
        {
            if (index<0 || index>=mSuccessorAddress.size())
                return false;

            if (successor->Address() == mSuccessorAddress[index])
            {
                mSuccessor[index] = successor;
                return true;
            }
        }

        virtual bool ReconnectSuccessor(Node_ptr old_successor, Node_ptr new_successor)
        {
            bool matched = false;
            for(int i=0; i < SuccessorCount(); i++)
            {
                if (old_successor == mSuccessor[i])
                {
                    mSuccessor[i] = new_successor;
                    mSuccessorAddress[i] = new_successor->Address();
                    matched = true;
                }
            }

            return matched;
        }

    private:
        std::vector<Addr> mSuccessorAddress;
        std::vector<Node_ptr> mSuccessor;
};/*}}}*/

class JumpNode : public OneWayNode/*{{{*/
{
public:
    JumpNode(Addr destination,
             Instruction_list::iterator begin,
             Instruction_list::iterator end)
            : OneWayNode(JUMP, destination, begin, end)
    {}

    virtual Node_ptr Copy()
    {
        Instruction_list list;

        for (Instruction_list::iterator i = Instructions().begin();
             i != Instructions().end();
             i++)
        {
            Instruction_ptr ptr = *i;
            list.push_back(ptr->Copy());
        }

        Node_ptr np = Node_ptr(new JumpNode(SuccessorAddress(0), list.begin(), list.end()));
        //np->CopySuccessors(Node_ptr(this));

        return np;
    }

    virtual void print(std::ostream &os)
    {
        Node::print(os);
        os << boost::format("JUMP target=%08lx\n")
              % SuccessorAddress(0);
    }

    static Node_ptr CreateFrom(Instruction_ptr i,
                               Instruction_list::iterator begin,
                               Instruction_list::iterator end);

    virtual void Cleanup(bool cleanAll);
};/*}}}*/

class ConditionalJumpNode : public TwoWayNode /*{{{*/
{
public:
    ConditionalJumpNode(
            Addr destination, Addr follower,
            Instruction_list::iterator begin,
            Instruction_list::iterator end)
            : TwoWayNode(CONDITIONAL_JUMP, destination, follower, begin, end)
    {}

    virtual Node_ptr Copy()
    {
        Instruction_list list;

        for (Instruction_list::iterator i = Instructions().begin();
             i != Instructions().end();
             i++)
        {
            Instruction_ptr ptr = *i;
            list.push_back(ptr->Copy());
        }

        Node_ptr np = Node_ptr(
                new ConditionalJumpNode(SuccessorAddress(0), SuccessorAddress(1), list.begin(), list.end()));
        //np->CopySuccessors(Node_ptr(this));

        return np;
    }

    virtual void print(std::ostream &os)
    {
        Node::print(os);
        os << boost::format("CONDJUMP target=%08lx follow=%08lx\n")
              % SuccessorAddress(0)
              % SuccessorAddress(1);
    }

    void ReconnectFollower(Node_ptr follower)
    {
        ConnectSuccessorAddress(1, follower->Address());
        ConnectSuccessor(1, follower);
    }

    Instruction_ptr FindJumpInstruction()
    {
        for (Instruction_list::iterator i = Instructions().begin();
             i != Instructions().end();
             i++)
        {
            Instruction_ptr ptr = *i;
            if (ptr->Type() == Instruction::CONDITIONAL_JUMP)
                return ptr;
        }

        return Instruction_ptr();
    }

    static Node_ptr CreateFrom(Instruction_ptr i,
                               Addr follower,
                               Instruction_list::iterator begin,
                               Instruction_list::iterator end);

    virtual void Cleanup(bool cleanAll); //remove labels and gotos.
};/*}}}*/

class FallThroughNode : public OneWayNode/*{{{*/
{
public:
    FallThroughNode(
            Addr follower,
            Instruction_list::iterator begin,
            Instruction_list::iterator end)
            : OneWayNode(FALL_THROUGH, follower, begin, end)
    {}

    virtual Node_ptr Copy()
    {
        Instruction_list list;

        for (Instruction_list::iterator i = Instructions().begin();
             i != Instructions().end();
             i++)
        {
            Instruction_ptr ptr = *i;
            list.push_back(ptr->Copy());
        }

        Node_ptr np = Node_ptr(new FallThroughNode(SuccessorAddress(0), list.begin(), list.end()));
        //np->CopySuccessors(Node_ptr(this));
        return np;
    }

    virtual void print(std::ostream &os)
    {
        Node::print(os);
        os << boost::format("FALLTHROUGH follow=%08lx\n")
              % SuccessorAddress(0);
    }

    virtual void Cleanup(bool cleanAll); //remove labels and gotos.

};/*}}}*/

class ReturnNode : public Node/*{{{*/
{
public:
    ReturnNode(
            Instruction_list::iterator begin,
            Instruction_list::iterator end)
            : Node(RETURN, begin, end)
    {}

    virtual Node_ptr Copy()
    {
        Instruction_list list;

        for (Instruction_list::iterator i = Instructions().begin();
             i != Instructions().end();
             i++)
        {
            Instruction_ptr ptr = *i;
            list.push_back(ptr->Copy());
        }

        return Node_ptr(new ReturnNode(list.begin(), list.end()));
    }

    virtual void print(std::ostream &os)
    {
        Node::print(os);
        os << boost::format("RETURN\n");
    }
};/*}}}*/

class CallNode : public TwoWayNode
{
public:
    CallNode(
            Addr calladdr,
            Addr follower,
            Instruction_list::iterator begin,
            Instruction_list::iterator end)
            : TwoWayNode(CALL, calladdr, follower, begin, end)
    {}

    virtual void print(std::ostream &os)
    {
        Node::print(os);
        os << boost::format("CALL target=%08lx follow=%08lx\n")
              % SuccessorAddress(0)
              % SuccessorAddress(1);
    }
};

class DoWhileNode : public Node/*{{{*/
{
public:
    DoWhileNode(Addr address)
            : Node(DO_WHILE, address)
    {}

    virtual void print(std::ostream &os)
    {
        Node::print(os);
        os << boost::format("Do While follow=%08lx\n")
              % SuccessorAddress(0);
    }

public:
    Instruction_ptr expr;
    Node_list mNodes;
    Addr breakAddr;
    Addr continueAddr;

};/*}}}*/
#endif

