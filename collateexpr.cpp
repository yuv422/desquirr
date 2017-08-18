// 
// Copyright (c) 2009 Eric Fry 
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

#include <ida.hpp>

#include "collateexpr.hpp"
#include "idapro.hpp"
#include "node.hpp"

int CollateExpr::Run()
{
    int matched = 0;

    for (Node_list::iterator n = mNodeList.begin();
         n != mNodeList.end() && !matched;)
    {
        Node_ptr node = *n;

        if (node->Type() == Node::CONDITIONAL_JUMP && node->Successor(1)->Type() == Node::CONDITIONAL_JUMP
            && node->Successor(0) == node->Successor(1)->Successor(0) //both nodes target the same jump
            //&& node->DominatesNode(node->Successor(1)) //node1 dominates it's follower node
            && node->Successor(1)->PredecessorCount() == 1

            //&& node->Successor(1)->InstructionCount() == 1) //the follower node only contains a jump instruction.
            && node->Successor(1)->IsSingleJumpNode())
        {
            msg("can reduce node! %a %a :-)\n", node->Address(), node->SuccessorAddress(1));
            Node_ptr jumpTarget = node->Successor(0);
            Node_ptr follower = node->Successor(1);

            follower->MarkForDeletion();

            static_cast<ConditionalJumpNode *>(node.get())->ReconnectFollower(follower->Successor(1));

            Instruction_ptr node_insn = static_cast<ConditionalJumpNode *>(node.get())->FindJumpInstruction();
            Expression_ptr expr1 = static_cast<BinaryInstruction *>(node_insn.get())->Operand(0);

            Instruction_ptr follower_insn = static_cast<ConditionalJumpNode *>(follower.get())->FindJumpInstruction();
            Expression_ptr expr2 = static_cast<BinaryInstruction *>(follower_insn.get())->Operand(0);

            Expression_ptr newExpr(new BinaryExpression(expr1, "||", expr2));
            static_cast<BinaryInstruction *>(node_insn.get())->Operand(0, newExpr);
            matched++;
            break;
        }
        else if (node->Type() == Node::CONDITIONAL_JUMP && node->Successor(1)->Type() == Node::JUMP &&
                 node->Successor(0)->Type() == Node::CONDITIONAL_JUMP
                 && node->DominatesNode(node->Successor(0)) && node->DominatesNode(node->Successor(1))
                 && node->Successor(0)->Successor(0) == node->Successor(1)->Successor(0)

                 && node->Successor(1)->IsSingleJumpNode()
                 && node->Successor(0)->IsSingleJumpNode())
            //&& node->Successor(1)->InstructionCount() == 1 //N2 only contains a jump instruction.
            //&& node->Successor(0)->InstructionCount() == 1) //FIXME need to handle labels etc.

            //&& node->Successor(0)->Successor(1)->Type() == Node::FALL_THROUGH)
        {
            msg("can reduce  if expr (!N1 or N3) %a\n", node->Address());

            Node_ptr jumpTarget = node->Successor(0);
            Node_ptr follower = node->Successor(1);

            follower->MarkForDeletion();
            jumpTarget->MarkForDeletion();

            static_cast<ConditionalJumpNode *>(node.get())->ReconnectFollower(jumpTarget->Successor(1));
            node->ReconnectSuccessor(jumpTarget, jumpTarget->Successor(0));

            Instruction_ptr node_insn = static_cast<ConditionalJumpNode *>(node.get())->FindJumpInstruction();
            Expression_ptr expr1 = static_cast<BinaryInstruction *>(node_insn.get())->Operand(0);
            Expression_ptr expr_jump = static_cast<BinaryInstruction *>(node_insn.get())->Operand(1);
            Expression_ptr new_loc = CreateLocalCodeReference(jumpTarget->Successor(0)->Address());

            Instruction_ptr jumpTarget_insn = static_cast<ConditionalJumpNode *>(jumpTarget.get())->FindJumpInstruction();
            Expression_ptr expr2 = static_cast<BinaryInstruction *>(jumpTarget_insn.get())->Operand(0);

            ExpressionNegationHelper negExpr;
            negExpr.negateExpression(expr1);

            Expression_ptr newExpr(new BinaryExpression(expr1, "||", expr2));
            static_cast<BinaryInstruction *>(node_insn.get())->Operand(0, newExpr);
            static_cast<BinaryInstruction *>(node_insn.get())->Operand(1, new_loc);

            matched++;
            break;
        }
        else if (node->Type() == Node::CONDITIONAL_JUMP && node->Successor(1)->Type() == Node::CONDITIONAL_JUMP
                 //&& node->DominatesNode(node->Successor(1)) && node->DominatesNode(node->Successor(1)->Successor(1))
                 && node->Successor(1)->IsSingleJumpNode()
                 && node->Successor(1)->PredecessorCount() == 1 &&
                 node->Successor(1)->Successor(1)->PredecessorCount() == 2
                 && node->Successor(0) == node->Successor(1)->Successor(1))
        {
//			msg("can reduce  if expr (!N1 or N2) via fallthrough %a\n", node->Address());
            msg("can reduce  if expr (!N1 and N2) via fallthrough %a\n", node->Address());

            Node_ptr jumpTarget = node->Successor(0);
            Node_ptr follower = node->Successor(1);

            follower->MarkForDeletion();

            node->ReconnectSuccessor(jumpTarget, follower->Successor(0));
            static_cast<ConditionalJumpNode *>(node.get())->ReconnectFollower(follower->Successor(1));


            Instruction_ptr node_insn = static_cast<ConditionalJumpNode *>(node.get())->FindJumpInstruction();
            Expression_ptr expr1 = static_cast<BinaryInstruction *>(node_insn.get())->Operand(0);

            Instruction_ptr follower_insn = static_cast<ConditionalJumpNode *>(follower.get())->FindJumpInstruction();
            Expression_ptr expr2 = static_cast<BinaryInstruction *>(follower_insn.get())->Operand(0);
            Expression_ptr new_loc = CreateLocalCodeReference(follower->Successor(0)->Address());

            ExpressionNegationHelper negExpr;
            negExpr.negateExpression(expr1);

            Expression_ptr newExpr(new BinaryExpression(expr1, "&&", expr2));
            static_cast<BinaryInstruction *>(node_insn.get())->Operand(0, newExpr);
            static_cast<BinaryInstruction *>(node_insn.get())->Operand(1, new_loc);

            matched++;
            break;
        }
        else if (node->Type() == Node::CONDITIONAL_JUMP && node->Successor(0)->Type() == Node::CONDITIONAL_JUMP
                 && node->Successor(0)->IsSingleJumpNode()
                 && node->Successor(0)->PredecessorCount() == 1
                 && node->Successor(1) == node->Successor(0)->Successor(0))
        {
            /* node1 jump node2 follower node3
               node2 jump node3
            */

            msg("can reduce  if expr (!N1 or N2) via jump %a\n", node->Address());

            Node_ptr jumpTarget = node->Successor(0);
            Node_ptr follower = node->Successor(1);

            jumpTarget->MarkForDeletion();

            node->ReconnectSuccessor(follower, jumpTarget->Successor(1));
            node->ReconnectSuccessor(jumpTarget, jumpTarget->Successor(0));

            Instruction_ptr node_insn = static_cast<ConditionalJumpNode *>(node.get())->FindJumpInstruction();
            Expression_ptr expr1 = static_cast<BinaryInstruction *>(node_insn.get())->Operand(0);

            Instruction_ptr jump_insn = static_cast<ConditionalJumpNode *>(jumpTarget.get())->FindJumpInstruction();
            Expression_ptr expr2 = static_cast<BinaryInstruction *>(jump_insn.get())->Operand(0);
            Expression_ptr new_loc = CreateLocalCodeReference(jumpTarget->Successor(0)->Address());

            ExpressionNegationHelper negExpr;
            negExpr.negateExpression(expr1);

            Expression_ptr newExpr(new BinaryExpression(expr1, "||", expr2));
            static_cast<BinaryInstruction *>(node_insn.get())->Operand(0, newExpr);
            static_cast<BinaryInstruction *>(node_insn.get())->Operand(1, new_loc);

            matched++;
            break;
        }
        else if (node->Type() == Node::CONDITIONAL_JUMP && node->Successor(0)->Type() == Node::CONDITIONAL_JUMP
                 && node->Successor(0)->IsSingleJumpNode()
                 && node->Successor(0)->PredecessorCount() == 1 //node->DominatesNode(node->Successor(0))
                 && node->Successor(1) == node->Successor(0)->Successor(1))
        {
            msg("can reduce  if expr (N1 and N2) %a\n", node->Address());

            Node_ptr jumpTarget = node->Successor(0);

            jumpTarget->MarkForDeletion();

            node->ReconnectSuccessor(jumpTarget, jumpTarget->Successor(0));

            Instruction_ptr node_insn = static_cast<ConditionalJumpNode *>(node.get())->FindJumpInstruction();
            Expression_ptr expr1 = static_cast<BinaryInstruction *>(node_insn.get())->Operand(0);

            Instruction_ptr jump_insn = static_cast<ConditionalJumpNode *>(jumpTarget.get())->FindJumpInstruction();
            Expression_ptr expr2 = static_cast<BinaryInstruction *>(jump_insn.get())->Operand(0);

            Expression_ptr new_loc = CreateLocalCodeReference(jumpTarget->Successor(0)->Address());

            Expression_ptr newExpr(new BinaryExpression(expr1, "&&", expr2));

            static_cast<BinaryInstruction *>(node_insn.get())->Operand(0, newExpr);
            static_cast<BinaryInstruction *>(node_insn.get())->Operand(1, new_loc);

            matched++;
            break;
        }

        if (!matched)
            n++;
    }

    if (matched)
    {
        //remove deleted nodes.
        Node::RemoveDeletedNodes(mNodeList);
    }

    return matched;
}