// 
// Copyright (c) 2009 Eric 
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
// $Id$
#ifndef _CONTROLFLOW_HPP
#define _CONTROLFLOW_HPP

#include "desquirr.hpp"
#include "analysis.hpp"
#include "node.hpp"

class Loop
{
public:
    Node_list nodes;
    Node_ptr header;
    Node_ptr tail;
    Node_ptr breakNode;
};

class ControlFlowAnalysis : Analysis/*{{{*/
{

public:
    ControlFlowAnalysis(Node_list &nodes)
            : mNodeList(nodes)
    {}

    /**
     * Analyze a list of nodes
     */
    void AnalyzeNodeList()/*{{{*/
    {
        for (Node_list::iterator n = mNodeList.begin();
             n != mNodeList.end();
             n++)
        {
            SetNode(*n);
            AnalyzeNode();
        }
    }/*}}}*/

    void FindDominators(Node_list &blocks);

    void FindLoops();

    void StructureLoops();

private:
    /**
     * Analyze an individual node (in mNode)
     */
    void AnalyzeNode()/*{{{*/
    {
        //AnalyzeInstructionList();
    }/*}}}*/

    /** Get node */
    Node_list &NodeList()
    { return mNodeList; }

    /** Get node */
    Node_ptr GetNode()
    { return mNode; }

    /** Set node */
    void SetNode(Node_ptr node)
    {
        mNode = node;
        Instructions(&mNode->Instructions());
    }

    Loop *FindNaturalLoopForEdge(Node_ptr header, Node_ptr tail);

    void StructureDoWhileLoop(Loop *loop);

    void StructureWhileLoop(Loop *loop);

    void StructureBreakContinue(Node_list &blocks, Node_ptr continueNode, Node_ptr breakNode);

    void StructureIfs(Node_list &blocks);

    int StructureIfElse(Node_list &blocks, Node_ptr node);

    int StructureIf(Node_list &blocks, Node_ptr node);

private:
    Node_list &mNodeList;
    Node_ptr mNode;
    std::list<Loop *> mLoops;

};/*}}}*/


class BreakContinueAnalysis : public Analysis/*{{{*/
{
public:
    BreakContinueAnalysis(Node_list &nodes, Node_ptr breakNode, Node_ptr continueNode)
            : mNodeList(nodes), mBreakNode(breakNode), mContinueNode(continueNode)
    {}

    /**
     * Analyze a list of nodes
     */
    void AnalyzeNodeList()/*{{{*/
    {
        for (Node_list::iterator n = mNodeList.begin();
             n != mNodeList.end();
             n++)
        {
            SetNode(*n);
            AnalyzeNode();
        }
    }/*}}}*/

private:
    /**
     * Analyze an individual node (in mNode)
     */
    void AnalyzeNode()/*{{{*/
    {
        AnalyzeInstructionList();
    }/*}}}*/

    virtual AnalysisResult OnInstruction()/*{{{*/
    {
//			message("%p\n", Instr()->Address());

        //if (INSTRUCTION_REMOVED == RemoveUnusedDefinition())
        //	return INSTRUCTION_REMOVED;

        //GetFunctionParametersFromStack();
        return CONTINUE;
    }/*}}}*/

    /**
     * Handle an ASSIGNMENT instruction
     */
    virtual void OnAssignment(Assignment *assignment)
    {}

    virtual void OnPush(Push *)
    {}

    virtual void OnPop(Pop *)
    {}

    virtual void OnJump(Jump *insn)
    {
        // replace stuff here.
        Expression_ptr jumpExpr = insn->Operand();
        Addr jumpAddr = static_cast<GlobalVariable *>(jumpExpr.get())->Address();

        msg("Jump checking %a, %a\n", jumpAddr, mBreakNode->Address());

        if (jumpAddr == mBreakNode->Address())
        {
            Instructions().insert(Iterator(), Instruction_ptr(new Break(insn->Address())));
            Erase(Iterator());

            //node->Instructions().remove(insn);
        }
        else if (mContinueNode.use_count() > 0 && jumpAddr == mContinueNode->Address())
        {
            Instructions().insert(Iterator(), Instruction_ptr(new Continue(insn->Address())));
            Erase(Iterator());
        }
    }

    virtual void OnConditionalJump(ConditionalJump *insn)
    {
        Expression_ptr ifExpr = insn->First();
        Expression_ptr jumpExpr = insn->Second();
        Addr jumpAddr = static_cast<GlobalVariable *>(jumpExpr.get())->Address();

        if (jumpAddr == mBreakNode->Address())
        {
            Node_ptr np = Node::CreateFrom(insn->Address());
            Instruction_ptr ip(new Break(insn->Address()));
            np->Instructions().push_back(ip);

            If *ifInstruction = new If(insn->Address(), ifExpr);
            ifInstruction->trueNode = np;

            Instructions().insert(Iterator(), Instruction_ptr(ifInstruction));
            Erase(Iterator());


            //node->Instructions().insert(i, Instruction_ptr(new Break(insn->Address())));
            //node->Instructions().remove(insn);
        }
    }

    /** Get node */
    Node_list &NodeList()
    { return mNodeList; }

    /** Get node */
    Node_ptr GetNode()
    { return mNode; }

    /** Set node */
    void SetNode(Node_ptr node)
    {
        mNode = node;
        Instructions(&mNode->Instructions());
    }

private:
    Node_list &mNodeList;
    Node_ptr mNode;
    Node_ptr mBreakNode;
    Node_ptr mContinueNode;


};/*}}}*/
#endif // _CONTROLFLOW_HPP

