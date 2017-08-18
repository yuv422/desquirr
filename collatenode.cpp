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

#include "collatenode.hpp"
#include "node.hpp"

int CollateNode::Run()
{
    int nMatched = 0;

    for (Node_list::iterator n = mNodeList.begin();
         n != mNodeList.end(); n++)
    {
        Node_ptr node = *n;

        if ((node->Type() == Node::FALL_THROUGH || node->Type() == Node::JUMP)
            && (node->Successor(0)->Type() == Node::FALL_THROUGH || node->Successor(0)->Type() == Node::JUMP ||
                node->Successor(0)->Type() == Node::CONDITIONAL_JUMP)
            && node->Successor(0)->PredecessorCount() == 1) //node1 dominates it's follower node
        {

            msg("can join %s node to %s node!  %a %a :-)\n", node->TypeString(), node->Successor(0)->TypeString(),
                node->Address(), node->Successor(0)->Address());
            Node_ptr follower = node->Successor(0);

            Node_ptr new_node = Node_ptr();

            if (follower->Type() == Node::FALL_THROUGH)
                new_node = Node_ptr(new FallThroughNode(follower->Successor(0)->Address(), node->Instructions().begin(),
                                                        node->Instructions().end()));
            else if (follower->Type() == Node::JUMP)
                new_node = Node_ptr(new JumpNode(follower->Successor(0)->Address(), node->Instructions().begin(),
                                                 node->Instructions().end()));
            else if (follower->Type() == Node::CONDITIONAL_JUMP)
                new_node = Node_ptr(
                        new ConditionalJumpNode(follower->Successor(0)->Address(), follower->Successor(1)->Address(),
                                                node->Instructions().begin(), node->Instructions().end()));

            //Node_list::const_iterator it = find(mNodeList.begin(), mNodeList.end(), node);
            Node_list::iterator it = find(mNodeList.begin(), mNodeList.end(), node);
            new_node->ConnectSuccessor(0, follower->Successor(0));

            if (follower->Type() == Node::CONDITIONAL_JUMP)
                new_node->ConnectSuccessor(1, follower->Successor(1));

            mNodeList.insert(it, new_node);

            new_node->Instructions().insert(new_node->Instructions().end(), follower->Instructions().begin(),
                                            follower->Instructions().end());

            new_node->Cleanup(false); //clean up goto's and labels. Leave first label and last goto.

            //node->ReconnectSuccessor(follower, follower->Successor(0));

            follower->MarkForDeletion();
            node->MarkForDeletion();

            for (Node_list::iterator p = node->mPreds.begin();
                 p != node->mPreds.end();
                 p++)
            {
                Node_ptr predNode = *p;
                predNode->ReconnectSuccessor(node, new_node); //reconnect successors
            }

            //nMatched++;

            Node::RemoveDeletedNodes(mNodeList);
            return 1;
        }

        //remove single jump nodes if they are dominated by their predecessor.
        if (node->Type() == Node::FALL_THROUGH && node->DominatesNode(node->Successor(0)) &&
            node->Successor(0)->PredecessorCount() == 1 && node->Successor(0)->Type() == Node::JUMP &&
            node->Successor(0)->IsSingleJumpNode())
        {
            msg("removed single jump node at %a\n", node->Successor(0)->Address());
            Node_ptr follower = node->Successor(0);
            node->ReconnectSuccessor(follower, follower->Successor(0));
            follower->MarkForDeletion();

            Node::RemoveDeletedNodes(mNodeList);
            return 1;
        }

        if (node->Type() == Node::CONDITIONAL_JUMP && node->DominatesNode(node->Successor(1)) &&
            node->Successor(1)->PredecessorCount() == 1 && node->Successor(1)->Type() == Node::JUMP &&
            node->Successor(1)->IsSingleJumpNode())
        {
            msg("removed single jump node at %a\n", node->Successor(1)->Address());
            Node_ptr follower = node->Successor(1);
            node->ReconnectSuccessor(follower, follower->Successor(0));
            follower->MarkForDeletion();

            Node::RemoveDeletedNodes(mNodeList);
            return 1;
        }

    }
/*
	if(nMatched > 0)
	{
		//remove deleted nodes.

		Node::RemoveDeletedNodes(mNodeList);
	}

	return nMatched;
*/
    return 0;
}
