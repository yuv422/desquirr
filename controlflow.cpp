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
#include <boost/dynamic_bitset.hpp>
#include <stack>
#include <CollateNodeVisitor.hpp>
#include <set>

#include "idainternal.hpp"
#include "controlflow.hpp"
#include "ida-x86.hpp"
#include "CollateExprNodeVisitor.hpp"

//work goes here. ;-)

void ControlFlowAnalysis::FindDominators(Node_list &blocks)
{
    int nNodes = blocks.size();
    int i = 0;
    for (Node_list::iterator n = blocks.begin();
         n != blocks.end();
         n++)
    {
        Node_ptr node = *n;
        //boost::dynamic_bitset<> b(nNodes);
        //b.set();
        //node->SetDominators(b);
        node->mDominators.resize(nNodes);
        node->mDominators.set();
        node->mPostDominators.resize(nNodes);
        node->mPostDominators.set();
        node->domId = i++;

        node->mPreds.clear();

    }

    //wireup predecessors.
    for (Node_list::iterator n = blocks.begin();
         n != blocks.end();
         n++)
    {
        Node_ptr node = *n;

        for (i = 0; i < node->SuccessorCount(); i++)
        {
            Node_ptr succ = node->Successor(i);
            succ->ConnectPredecessor(node);
        }
    }

    Node_ptr n = *blocks.begin();

    n->mDominators.reset();
    n->mDominators.set(n->domId);

    boost::dynamic_bitset<> t(nNodes);

    bool changed;
    do
    {
        changed = false;
        for (Node_list::iterator it = blocks.begin();
             it != blocks.end();
             it++)
        {
            Node_ptr node = *it;
            if (it == blocks.begin()) //hack need to store entry node.
                continue;

            for (Node_list::iterator p = node->mPreds.begin();
                 p != node->mPreds.end();
                 p++)
            {
                Node_ptr pred = *p;
                t.reset();
                t |= node->mDominators;
                node->mDominators &= pred->mDominators;
                node->mDominators.set(node->domId);

                if (node->mDominators != t)
                    changed = true;
            }
        }
    } while (changed);

//Post Dominators

    n = blocks.back();

    n->mPostDominators.reset();
    n->mPostDominators.set(n->domId);

    do
    {
        changed = false;
        for (Node_list::iterator it = blocks.begin();
             it != blocks.end();
             it++)
        {
            Node_ptr node = *it;
            if (node == blocks.back()) //this should be the exit node. FIXME need better way of finding exit block.
                continue;

            t.set();
            //t |= node->mPostDominators;

            for (i = 0; i < node->SuccessorCount(); i++)
            {
                Node_ptr succ = node->Successor(i);
                t &= succ->mPostDominators;
            }

            t.set(node->domId);

            if (node->mPostDominators != t)
            {
                changed = true;
                node->mPostDominators = t;
            }

        }
    } while (changed);
}

bool list_sort_helper(Node_ptr n, Node_ptr n1)
{
    return (n->Address() < n1->Address());
}

bool loop_compare(Loop *loop, Loop *loop1)
{
    Node_list n1 = loop->nodes;
    Node_list n2 = loop1->nodes;

    if (n2.size() > n1.size())
        return false;

    n1.sort(list_sort_helper);
    n2.sort(list_sort_helper);

    return includes(n1.begin(), n1.end(), n2.begin(), n2.end(), list_sort_helper);
}

void ControlFlowAnalysis::FindLoops()
{
    for (Node_list::iterator n = mNodeList.begin();
         n != mNodeList.end();
         n++)
    {
        if (n == mNodeList.begin()) //hack need to store entry node.
            continue;

        Node_ptr node = *n;

        for (int i = 0; i < node->SuccessorCount(); i++)
        {
            Node_ptr succ = node->Successor(i);

            if (node->mDominators.test(succ->domId))
                mLoops.push_back(FindNaturalLoopForEdge(succ, node));
        }
    }

    if (!mLoops.empty())
    {
        mLoops.sort(loop_compare);
        mLoops.reverse();
    }

}

Loop *ControlFlowAnalysis::FindNaturalLoopForEdge(Node_ptr header, Node_ptr tail)
{
    msg("Loop (%a,%a)\n", header->Address(), tail->Address());
    std::stack<Node_ptr> workList;

    Loop *loop = new Loop();

    loop->header = header;
    loop->tail = tail;
    loop->nodes.push_back(header);

    if (header != tail)
    {
        Node_ptr np(tail);
        loop->nodes.push_back(np);
        Node_ptr wp(tail);
        workList.push(wp);
        msg("loop tail node added at %a\n", tail->Address());
    }

    while (!workList.empty())
    {
        Node_ptr node = workList.top();
        workList.pop();
        for (Node_list::iterator p = node->mPreds.begin();
             p != node->mPreds.end();
             p++)
        {
            Node_ptr pred = *p;
            //msg("pred = %a\n", pred->Address());
            Node_list::iterator it = find(loop->nodes.begin(), loop->nodes.end(), pred);
            if (it == loop->nodes.end())
            {
                Node_ptr np(pred);
                loop->nodes.push_back(np);
                Node_ptr wp(pred);
                workList.push(wp);
                msg("#loop node added at %a\n", pred->Address());
            }
        }
    }

    //loop->nodes.sort(list_sort_helper);
    msg("loop count = %d\n", loop->nodes.size());
    return loop;
}

Node_ptr find_node(Node_list &list, Node_ptr node)
{
    Node_list::iterator n = find(list.begin(), list.end(), node);

    return n != list.end() ? *n : Node_ptr();
}

Node_ptr find_node_by_domId(Node_list &list, int domId)
{
    for (Node_list::iterator n = list.begin();
         n != list.end();
         n++)
    {
        Node_ptr node = *n;

        if (node->domId == domId)
            return node;
    }

    return Node_ptr();
}

Node_ptr find_imm_post_dominator(Node_list &list, Node_list &loop_list, Node_ptr node)
{
    Node_list plist;

    for (int i = 0; i < node->mPostDominators.size(); i++)
    {
        if (node->mPostDominators[i])
        {
            Node_ptr postdom_node = find_node_by_domId(list, i);
            if (postdom_node.use_count() > 0)
            {
                //Node_ptr np(postdom_node);
                plist.push_back(postdom_node);//np);
                msg("node at %a post dominates node at %a\n", postdom_node->Address(), node->Address());
            }
        }

    }

    if (plist.size() != 0)
    {
        Node_list q;
        Node_list visited;

        //Node_ptr np(node);
        //q.push_back(np);
        q.push_back(node);
        for (; !q.empty();)
        {
            Node_ptr ptr = q.front();
            q.pop_front();

            Node_list::iterator it;

            it = find(visited.begin(), visited.end(), ptr);
            if (it != visited.end())
                continue; //already visited this node must be a loop.

            if (ptr != node)
            {
                it = find(plist.begin(), plist.end(), ptr);
                if (it != plist.end())
                {
                    it = find(loop_list.begin(), loop_list.end(), ptr);
                    if (it != loop_list.end())
                    {
                        return ptr;
                    }
                    /*
                    it = find(loop_list.begin(), loop_list.end(), ptr); //post dominator cannot be a loop node.
                    if(it == loop_list.end())
                    {
                        msg("found imm post dominator of %a at %a\n", node->Address(), ptr->Address());
                        return ptr; //found immediate post-dominator.
                    }
                    */
                }
            }

            Node_ptr vnp(ptr);
            visited.push_back(vnp);

            for (int i = 0; i < ptr->SuccessorCount(); i++)
            {
                Node_ptr snp(ptr->Successor(i));
                msg("pushback %a\n", snp->Address());
                q.push_back(ptr->Successor(i)); //snp);
            }
        }
    }

    return Node_ptr();
}

void collect_stray_loop_nodes(Loop *loop)
{
    Node_list q;
    Node_list visited;

    Node_ptr np(loop->header);
    q.push_back(np);
    for (; !q.empty();)
    {
        Node_ptr ptr = q.front();
        q.pop_front();
        msg("ptr usage_count = %d\n", ptr.use_count());

        if (ptr == loop->breakNode) //we've hit the loop post dominator
            continue;

        Node_list::iterator it;

        it = find(visited.begin(), visited.end(), ptr);
        if (it != visited.end())
            continue;

        Node_ptr vlnp(ptr);
        visited.push_back(vlnp);

        it = find(loop->nodes.begin(), loop->nodes.end(), ptr);
        if (it == loop->nodes.end() && loop->header->DominatesNode(ptr))
        {
            Node_ptr vnp(ptr);
            loop->nodes.push_back(vnp);
            msg("Adding another node to loop at %a\n", ptr->Address());
        }

        for (int i = 0; i < ptr->SuccessorCount(); i++)
        {
            Node_ptr snp(ptr->Successor(i));
            q.push_back(snp);
        }
    }
}

void ControlFlowAnalysis::StructureLoops()
{
    for (std::list<Loop *>::iterator l = mLoops.begin();
         l != mLoops.end();
         l++)
    {
        Loop *loop = *l;

        loop->breakNode = find_imm_post_dominator(mNodeList, loop->nodes, loop->header);

        //if(breakNode.use_count() > 0)
        //	msg("break node at %a\n", breakNode->Address());

        //doWhileNode->continueAddr = loop->nodes.back()->Address();

        msg("loop header = %a, block count = %d continue address = %a\n", loop->header->Address(), loop->nodes.size(),
            loop->nodes.back()->Address());

        for (Node_list::iterator n = loop->nodes.begin();
             n != loop->nodes.end();
             n++)
        {
            Node_ptr node = *n;
            msg("LoopNode: %a %s\n", node->Address(), node->TypeString());
        }

        msg("END\n");


        //FIXME.. need to get last compare of last loop block.
        //loop->nodes.back()->Instructions().pop_back();

//		msg("removing node at %a\n", loop->nodes.back()->Address());
        //mNodeList.remove(loop->nodes.back());
        //loop->nodes.pop_back(); //get rid of loop goto.


        Instruction::InstructionType loopType = Instruction::DO_WHILE;

        if (loop->header->SuccessorCount() == 2)
        {
            for (int i = 0; i < loop->header->SuccessorCount(); i++)
            {
                if (loop->breakNode.use_count() > 0 && loop->header->Successor(i)->Address() == loop->breakNode->Address() ||
                    !loop->header->DominatesNode(loop->header->Successor(i)))
                {
                    loopType = Instruction::WHILE;
                    //loop->breakNode = find_imm_post_dominator(mNodeList, loop->nodes, loop->header->Successor(i==0?1:0));
                }
            }
        }
        if (loop->breakNode.use_count() > 0)
            msg("break node at %a\n", loop->breakNode->Address());

        if (loopType == Instruction::DO_WHILE)
            StructureDoWhileLoop(loop);
        else
            StructureWhileLoop(loop);

    }

    StructureIfs(mNodeList);
}

void ControlFlowAnalysis::StructureWhileLoop(Loop *loop)
{
    msg("while loop\n");
    if (loop->nodes.size() == 0)
        return;

    Instruction_ptr expr;
    //expr = loop->nodes.front()->Instructions().back(); //while
    Node_ptr first_node = loop->nodes.front();

    if (first_node->Type() != Node::CONDITIONAL_JUMP)
    {
        msg("Error: first loop node not a conditional jump! %a\n", first_node->Address());
        return;
    }
    expr = static_cast<ConditionalJumpNode *>(loop->nodes.front().get())->FindJumpInstruction();


    if (loop->breakNode.use_count() == 0)
    {
        msg("Error: while loop without breakNode!");
        return;
    }

    ExpressionNegationHelper negExpr;

    //test
    Expression_ptr ep(expr->Operand(0));

    Expression_ptr jumpExpr(expr->Operand(1));

    Addr jumpAddr = static_cast<GlobalVariable *>(jumpExpr.get())->Address();
    if (jumpAddr == loop->breakNode->Address()) //negate expression if the jump exits the loop.
        negExpr.negateExpression(ep);

    While *whileInstruction = new While(loop->header->Address(), ep);

    //removed unneeded label instruction from header block.
    //if(loop->header->Instructions().front()->Type() == Instruction::InstructionType::LABEL)
    //	loop->header->Instructions().pop_front();

    loop->header->Instructions().insert(
            loop->header->Instructions().end(),
            Instruction_ptr(whileInstruction)
    );

    msg("loop header. %s\n", loop->header->TypeString());

    loop->nodes.front()->Instructions().remove(expr);

    //convert the header node into a fallthrough node.
    Node_ptr while_node = Node_ptr(new FallThroughNode(loop->breakNode->Address(), loop->header->Instructions().begin(),
                                                       loop->header->Instructions().end()));
    while_node->ConnectSuccessor(0, loop->breakNode);


    loop->header->MarkForDeletion();

//		msg("removing node at %a\n", loop->nodes.back()->Address());
    //mNodeList.remove(loop->nodes.back());
    //loop->nodes.pop_back(); //get rid of loop expr.

    collect_stray_loop_nodes(loop);

    Node_list::iterator it = find(mNodeList.begin(), mNodeList.end(), loop->header);
    mNodeList.insert(it, while_node);

    //helper stub nodes.
    Node_ptr loop_break = Node_ptr(new Node(Node::LOOP_BREAK, loop->breakNode->Address()));
    Node_ptr loop_continue = Node_ptr(new Node(Node::LOOP_CONTINUE, loop->header->Address()));

    for (Node_list::iterator n = loop->nodes.begin();
         n != loop->nodes.end();
         n++)
    {
        Node_ptr node = *n;

        if (node == loop->header)
        {
            //doWhileNode->mNodes.push_back(node);
            //doWhile->AddLoopNode(node);
            continue;
        }

        Node_ptr replace_node = find_node(mNodeList, node);
        if (replace_node.use_count() > 0)
        {
            msg("replacing node of type %d at %a\n", replace_node->Type(), replace_node->Address());
            whileInstruction->AddLoopNode(replace_node);

            //connect up new break/continue stub nodes. These are used when finding dominators.
            for (int i = 0; i < replace_node->SuccessorCount(); i++)
            {
                Node_ptr s = replace_node->Successor(i);

                if (s == loop->header)
                {
                    replace_node->ReconnectSuccessor(loop->header, loop_continue);
                }

                if (s == loop->breakNode)
                {
                    replace_node->ReconnectSuccessor(loop->breakNode, loop_break);
                }
            }

            mNodeList.remove(replace_node);

        }
    }

    whileInstruction->AddLoopNode(loop_continue);
    whileInstruction->AddLoopNode(loop_break);

    for (Node_list::iterator n = loop->header->mPreds.begin();
         n != loop->header->mPreds.end();
         n++)
    {
        Node_ptr predNode = *n;
        Node_list::const_iterator loop_iter = find(loop->nodes.begin(), loop->nodes.end(), predNode);
        if (loop_iter == loop->nodes.end()) //only link to nodes that are not part of the loop.
        {
            predNode->ReconnectSuccessor(loop->header, while_node); //reconnect successors
        }

    }


    if (whileInstruction->Statements().front()->Instructions().empty() == false)
    {
        //removed unneeded label instruction from first loop block.
        if (whileInstruction->Statements().front()->Instructions().front()->Type() == Instruction::LABEL)
            whileInstruction->Statements().front()->Instructions().pop_front();
    }

    //BreakContinueAnalysis bca(whileInstruction->Statements(), loop->breakNode, Node_ptr());
    //bca.AnalyzeNodeList();

    Node::RemoveDeletedNodes(mNodeList);
    FindDominators(mNodeList);

    StructureIfs(whileInstruction->Statements());

    whileInstruction->Statements().sort(list_sort_helper);
}

void ControlFlowAnalysis::StructureDoWhileLoop(Loop *loop)
{
    msg("Do while loop\n");
    Instruction_ptr expr;

//    Node_list::iterator n = loop->nodes.end();
//
//    n--;
//    for (; (*n)->Type() != Node::CONDITIONAL_JUMP;)
//    {
//        msg("dowhile searching for header. skipping %s at %a\n", (*n)->TypeString(), (*n)->Address());
//        n--;
//    }

//		n--;  //FIXME big hack! dropping last fall through node.
//				msg("dowhile exprNode = %a %s\n", (*n)->Address(), (*n)->TypeString());
//		n--;

    //expr = (*n)->Instructions().back(); //do while
    expr = loop->tail->Instructions().back();

    msg("dowhile insn = %a\n",  expr->Address());

    if (loop->breakNode.get() == NULL)
    {
        msg("ERROR: doWhile loop with no break node! Header at %a", loop->header->Address());
        return;
    }

    //test
    Expression_ptr ep(expr->Operand(0));
    if (loop->tail->Successor(0) != loop->header) //if the jump exits the loop then negate the expression.
    {
        ExpressionNegationHelper negExpr;
        negExpr.negateExpression(ep);
    }

    DoWhile *doWhileInstruction = new DoWhile(loop->header->Address(), ep);

    /*
    loop->header->Instructions().insert(
                loop->header->Instructions().begin(),
                Instruction_ptr(doWhileInstruction)
                );
    */
    //loop->nodes.back()->Instructions().remove(expr);
    //(*n)->Instructions().remove(expr);
    //FIXME loop->tail->Instructions().remove(expr);

    Instruction_list tmpInstructions;
    tmpInstructions.push_back(Instruction_ptr(doWhileInstruction));

    //convert the header node into a fallthrough node.
    Node_ptr while_node = Node_ptr(
            new FallThroughNode(loop->breakNode->Address(), tmpInstructions.begin(), tmpInstructions.end()));
    while_node->ConnectSuccessor(0, find_node(mNodeList, loop->breakNode));

    //loop->header->MarkForDeletion();

    collect_stray_loop_nodes(loop);

    Node_list::iterator it = find(mNodeList.begin(), mNodeList.end(), loop->header);
    mNodeList.insert(it, while_node);

    //helper stub nodes.
    Node_ptr loop_break = Node_ptr(new Node(Node::LOOP_BREAK, loop->breakNode->Address()));
    Node_ptr loop_continue = Node_ptr(new Node(Node::LOOP_CONTINUE, loop->header->Address()));

    for (Node_list::iterator n = loop->nodes.begin();
         n != loop->nodes.end();
         n++)
    {
        Node_ptr node = *n;
/*
			if(node == loop->header)
			{
				//doWhileNode->mNodes.push_back(node);
				//doWhile->AddLoopNode(node);
				continue;
			}
*/
        Node_ptr replace_node = find_node(mNodeList, node);
        if (node ==
            loop->header) //if this is the case then we have probable selected the while_node which is wrong. We need the original header node.
            replace_node = loop->header;

        if (replace_node.use_count() > 0)
        {
            msg("replacing node of type %s at %a\n", replace_node->TypeString(), replace_node->Address());
            doWhileInstruction->AddLoopNode(replace_node);

            //connect up new break/continue stub nodes. These are used when finding dominators.
            for (int i = 0; i < replace_node->SuccessorCount(); i++)
            {
                Node_ptr s = replace_node->Successor(i);

                if (s == loop->header)
                {
                    replace_node->ReconnectSuccessor(loop->header, loop_continue);
                }

                else if (s == loop->breakNode)
                {
                    replace_node->ReconnectSuccessor(loop->breakNode, loop_break);
                }
                else
                {
                    Node_list::const_iterator loop_iter = find(loop->nodes.begin(), loop->nodes.end(), s);
                    if (loop_iter == loop->nodes.end())
                    {
                        replace_node->ReconnectSuccessor(s, loop_break);
                    }
                }
            }

            mNodeList.remove(replace_node);

        }
    }

    doWhileInstruction->AddLoopNode(loop_continue);
    doWhileInstruction->AddLoopNode(loop_break);

    for (Node_list::iterator n = loop->header->mPreds.begin();
         n != loop->header->mPreds.end();
         n++)
    {
        Node_ptr predNode = *n;
        Node_list::const_iterator loop_iter = find(loop->nodes.begin(), loop->nodes.end(), predNode);
        if (loop_iter == loop->nodes.end()) //only link to nodes that are not part of the loop.
        {
            predNode->ReconnectSuccessor(loop->header, while_node); //reconnect successors
        }

    }

    doWhileInstruction->Statements().sort(list_sort_helper);

    if (doWhileInstruction->Statements().front()->Instructions().empty() == false)
    {
        //removed unneeded label instruction from first loop block.
        if (doWhileInstruction->Statements().front()->Instructions().front()->Type() == Instruction::LABEL)
            doWhileInstruction->Statements().front()->Instructions().pop_front();
    }

    //BreakContinueAnalysis bca(whileInstruction->Statements(), loop->breakNode, Node_ptr());
    //bca.AnalyzeNodeList();

    Node::RemoveDeletedNodes(mNodeList);
    FindDominators(mNodeList);

    StructureIfs(doWhileInstruction->Statements());

    doWhileInstruction->Statements().sort(list_sort_helper);
}

void ControlFlowAnalysis::StructureBreakContinue(Node_list &blocks, Node_ptr continueNode, Node_ptr breakNode)
{
    for (Node_list::iterator n = blocks.begin();
         n != blocks.end();
         n++)
    {
        Node_ptr node = *n;

        for (Instruction_list::iterator i = node->Instructions().begin();
             i != node->Instructions().end();
             i++)
        {
            Instruction_ptr insn = *i;

            if (insn->Type() == Instruction::JUMP)
            {
                // replace stuff here.
                Expression_ptr jumpExpr = static_cast<Jump *>(insn.get())->Operand();
                Addr jumpAddr = static_cast<GlobalVariable *>(jumpExpr.get())->Address();

                if (jumpAddr == breakNode->Address())
                {
                    node->Instructions().insert(i, Instruction_ptr(new Break(insn->Address())));
                    //node->Instructions().remove(insn);
                }
                else if (continueNode.use_count() > 0 && jumpAddr == continueNode->Address())
                {
                    node->Instructions().insert(i, Instruction_ptr(new Continue(insn->Address())));
                    //node->Instructions().remove(insn);
                }

            }
            else if (insn->Type() == Instruction::CONDITIONAL_JUMP)
            {
                Expression_ptr jumpExpr = static_cast<ConditionalJump *>(insn.get())->Second();
                Addr jumpAddr = static_cast<GlobalVariable *>(jumpExpr.get())->Address();

                if (jumpAddr == breakNode->Address())
                {
                    node->Instructions().insert(i, Instruction_ptr(new Break(insn->Address())));
                    //node->Instructions().remove(insn);
                }
                //replace stuff here.
            }
        }
    }
}


void ControlFlowAnalysis::StructureIfs(Node_list &blocks)
{
    int i = 0;

    FindDominators(blocks);
    do
    {
        i = 0;
        msg("performing if analysis pass..\n");

        CollateExprNodeVisitor collateExprNodeVisitor;
        AcceptNodeVisitor(blocks, collateExprNodeVisitor);
        i = collateExprNodeVisitor.didWork() ? 1 : 0;

        FindDominators(blocks);

        for (Node_list::iterator n = blocks.begin();
             n != blocks.end();
             n++)
        {
            Node_ptr node = *n;
            if (node->Type() == Node::TO_BE_DELETED)
            {
                msg("skipping deleted node at %a\n", node->Address());
                continue;
            }
            if (StructureIfElse(blocks, node) > 0 || StructureIf(blocks, node) > 0)
            {
                i++;
                break;
            }
        }

        Node::RemoveDeletedNodes(blocks);

        FindDominators(blocks);

        CollateNodeVisitor collateNodeVisitor;
        AcceptNodeVisitor(blocks, collateNodeVisitor);
        if (collateNodeVisitor.didWork())
            i++;

        FindDominators(blocks);

    } while (i > 0);

    return;
}

int ControlFlowAnalysis::StructureIfElse(Node_list &blocks, Node_ptr node)
{
    if (node->Type() == Node::TO_BE_DELETED || node->SuccessorCount() != 2)
        return 0;

    Node_ptr trueNode = node->Successor(0);
    Node_ptr falseNode = node->Successor(1);

    if (trueNode->SuccessorCount() != 1 || falseNode->SuccessorCount() != 1 || !node->DominatesNode(trueNode) ||
        !node->DominatesNode(falseNode))
        return 0;

    if (falseNode->Successor(0) != trueNode->Successor(0))
        return 0;

    if (trueNode->PredecessorCount() != 1 || falseNode->PredecessorCount() != 1)
        return 0;

    msg("found if/else at %a\n", node->Address());

    Instruction_ptr expr = node->Instructions().back(); //FIXME find actual jump instruction

    Expression_ptr ep(expr->Operand(0));

    trueNode->Cleanup(true); //cleanup all goto and label instructions.
    falseNode->Cleanup(true); //cleanup all goto and label instructions.

    If *ifInstruction = new If(expr->Address(), ep);
    ifInstruction->trueNode = trueNode;
    ifInstruction->falseNode = falseNode;

    if (trueNode->Instructions().front()->Type() == Instruction::LABEL)
        trueNode->Instructions().pop_front();

    node->Instructions().insert(
            node->Instructions().end(),
            Instruction_ptr(ifInstruction)
    );

    node->Instructions().remove(expr);

    Node_ptr new_node = Node_ptr(new FallThroughNode(trueNode->Successor(0)->Address(), node->Instructions().begin(),
                                                     node->Instructions().end()));
    Node_list::iterator it = find(blocks.begin(), blocks.end(), node);

    blocks.insert(it, new_node);

    //blocks.remove(trueNode);
    //blocks.remove(falseNode);
    //blocks.remove(node);

    trueNode->MarkForDeletion();
    falseNode->MarkForDeletion();
    node->MarkForDeletion();

    new_node->ConnectSuccessor(0, trueNode->Successor(0));

    //fixup preds
    Node_ptr followerNode = trueNode->Successor(0);

    for (Node_list::iterator n = node->mPreds.begin();
         n != node->mPreds.end();
         n++)
    {
        Node_ptr predNode = *n;
        predNode->ReconnectSuccessor(node, new_node); //reconnect successors
    }

    return 1;
}

int ControlFlowAnalysis::StructureIf(Node_list &blocks, Node_ptr node)
{
    if (node->Type() == Node::TO_BE_DELETED || node->SuccessorCount() != 2)
        return 0;

    Node_ptr jumpNode = node->Successor(0);
    Node_ptr followerNode = node->Successor(1);

    if (node->DominatesNode(jumpNode) && jumpNode->PredecessorCount() == 1 && jumpNode->SuccessorCount() == 1 &&
        jumpNode->Successor(0) == followerNode)
    {

        msg("found if at %a\n", node->Address());

        Instruction_ptr expr = node->Instructions().back();

        Expression_ptr ep(expr->Operand(0));

        jumpNode->Cleanup(true); //cleanup all goto and label instructions.

        If *ifInstruction = new If(expr->Address(), ep);
        ifInstruction->trueNode = jumpNode;

        node->Instructions().insert(
                node->Instructions().end(),
                Instruction_ptr(ifInstruction)
        );

        node->Instructions().remove(expr);
        Node_ptr new_node = Node_ptr(
                new FallThroughNode(followerNode->Address(), node->Instructions().begin(), node->Instructions().end()));
        Node_list::iterator it = find(blocks.begin(), blocks.end(), node);

        blocks.insert(it, new_node);

        //blocks.remove(jumpNode);
        //blocks.remove(node);
        jumpNode->MarkForDeletion();
        node->MarkForDeletion();

        new_node->ConnectSuccessor(0, followerNode);


        //fixup preds


        for (Node_list::iterator n = node->mPreds.begin();
             n != node->mPreds.end();
             n++)
        {
            Node_ptr predNode = *n;
            predNode->ReconnectSuccessor(node, new_node); //reconnect successors
        }

        return 1;
    }

    else if (node->DominatesNode(followerNode) && followerNode->PredecessorCount() == 1 &&
             followerNode->SuccessorCount() == 1 && followerNode->Successor(0)->Type() == Node::RETURN)
    {

        msg("found if reverse return at %a\n", node->Address());

        Instruction_ptr expr = node->Instructions().back();

        Expression_ptr ep(expr->Operand(0));
        ExpressionNegationHelper negExpr;
        negExpr.negateExpression(ep);

        followerNode->Cleanup(true); //cleanup all goto and label instructions.

        //FIXME should be configurable with D:NoReturnValue
        //Add Return ax;
        followerNode->Instructions().push_back(
                Instruction_ptr(new Return(node->Address(), Expression_ptr(new Register(REG_AX)))));

        If *ifInstruction = new If(expr->Address(), ep);
        ifInstruction->trueNode = followerNode;

        node->Instructions().insert(
                node->Instructions().end(),
                Instruction_ptr(ifInstruction)
        );

        node->Instructions().remove(expr);
        Node_ptr new_node = Node_ptr(
                new FallThroughNode(jumpNode->Address(), node->Instructions().begin(), node->Instructions().end()));
        Node_list::iterator it = find(blocks.begin(), blocks.end(), node);

        blocks.insert(it, new_node);

        //blocks.remove(jumpNode);
        //blocks.remove(node);
        followerNode->MarkForDeletion();
        node->MarkForDeletion();

        new_node->ConnectSuccessor(0, jumpNode);


        //fixup preds


        for (Node_list::iterator n = node->mPreds.begin();
             n != node->mPreds.end();
             n++)
        {
            Node_ptr predNode = *n;
            predNode->ReconnectSuccessor(node, new_node); //reconnect successors
        }

        return 1;
    }

    else if (node->DominatesNode(followerNode) && followerNode->PredecessorCount() == 1 &&
             (followerNode->Type() == Node::FALL_THROUGH || followerNode->Type() == Node::JUMP) &&
             jumpNode->Address() == followerNode->Successor(0)->Address())
    {
        msg("found reverse if at %a\n", node->Address());
        Instruction_ptr expr = static_cast<ConditionalJumpNode *>(node.get())->FindJumpInstruction();
        Expression_ptr ep(expr->Operand(0));

        ExpressionNegationHelper negExpr;
        negExpr.negateExpression(ep);
        If *ifInstruction = new If(expr->Address(), ep);
        ifInstruction->trueNode = followerNode;

        node->Instructions().insert(
                node->Instructions().end(),
                Instruction_ptr(ifInstruction)
        );

        node->Instructions().remove(expr);

        Node_ptr new_node = Node_ptr(
                new FallThroughNode(jumpNode->Address(), node->Instructions().begin(), node->Instructions().end()));
        Node_list::iterator it = find(blocks.begin(), blocks.end(), node);

        blocks.insert(it, new_node);

        //blocks.remove(followerNode);
        //blocks.remove(node);
        followerNode->MarkForDeletion();
        node->MarkForDeletion();

        new_node->ConnectSuccessor(0, jumpNode);

        //fixup preds

        for (Node_list::iterator n = node->mPreds.begin();
             n != node->mPreds.end();
             n++)
        {
            Node_ptr predNode = *n;
            predNode->ReconnectSuccessor(node, new_node); //reconnect successors
        }

        return 1;
    }


    return 0;
}

void ControlFlowAnalysis::StructureSwitches(Node_list &blocks)
{
    FindDominators(blocks);
    Node_list nodesToRemove;

    for (Node_list::iterator n = blocks.begin();
         n != blocks.end();
         n++)
    {
        Node_ptr node = *n;
        if(node->Type() == Node::N_WAY && node->Instructions().back()->Type() == Instruction::SWITCH) {
            std::set<Node_ptr> exitNodes = findSwitchExitNodes(node, blocks);
            Switch *switchInsn = static_cast<Switch *>(node->Instructions().back().get());
            N_WayNode *switchNode = static_cast<N_WayNode *>(node.get());

            if (exitNodes.size() > 1)
            {
                msg("WARN: %d exit nodes found in switch case nodes.\n", exitNodes.size());
                continue;
            }
            Node_ptr exitNode = *exitNodes.begin();
            Node_ptr breakNodeStub = Node_ptr(new Node(Node::LOOP_BREAK, exitNode->Address())); //FIXME need to rename this to BREAK
            for (Node_list::iterator n1 = blocks.begin();
                 n1 != blocks.end();
                 n1++) {
                if (*n1 != *n && node->DominatesNode(*n1) && *n1 != exitNode) {
                    Node_ptr dominatedNode = *n1;
                    msg("Adding Node %a to switch statement\n", dominatedNode->Address());
                    if (dominatedNode->HasPredecessor(node))
                    {
                        switchNode->RemoveSuccessor(dominatedNode);
                    }
                    //disconnect successors that aren't dominated by switch node
                    switchInsn->AddStatementNode(dominatedNode);

                    if (dominatedNode->HasSuccessor(exitNode))
                    {
                        //If this node exits the case statement block then remove jump instruction/label and add break instruction.
                        dominatedNode->ReconnectSuccessor(exitNode, breakNodeStub);
                        dominatedNode->Cleanup(true);
                        dominatedNode->Instructions().push_back(Instruction_ptr(new Break(exitNode->Address()))); //FIXME what should this address be?
                    }

                    nodesToRemove.push_back(dominatedNode);
                }
            }

            for (Node_list::iterator n1 = nodesToRemove.begin();
                 n1 != nodesToRemove.end();
                 n1++)
            {
                blocks.remove(*n1);
            }
        }
    }
}

std::set<Node_ptr> ControlFlowAnalysis::findSwitchExitNodes(Node_ptr switchNode, Node_list &blocks)
{
    std::set<Node_ptr> exitNodes;

    for (Node_list::iterator n = blocks.begin();
         n != blocks.end();
         n++)
    {
        Node_ptr node = *n;
        if (node != switchNode && switchNode->DominatesNode(node))
        {
            if (node->PostDominatesNode(switchNode))
            {
                exitNodes.insert(node);
                msg("found post dominating exit node for switch %a\n", node->Address());
                return exitNodes;
            }

            for (int i = 0; i < node->SuccessorCount(); i++)
            {
                Node_ptr successor = node->Successor(i);
                if (!switchNode->DominatesNode(successor))
                {
                    exitNodes.insert(successor);
                    msg("found exit node for switch %a\n", successor->Address());
                }
            }
        }
    }

    return exitNodes;
}

