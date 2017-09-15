//
// Created by Eric Fry on 13/09/2017.
//

#include "node.hpp"
#include "CollateNodeVisitor.hpp"

bool CollateNodeVisitor::visit(Node_ptr node) {
    if ((node->Type() == Node::FALL_THROUGH || node->Type() == Node::JUMP)
        && (node->Successor(0)->Type() == Node::FALL_THROUGH || node->Successor(0)->Type() == Node::JUMP ||
            node->Successor(0)->Type() == Node::CONDITIONAL_JUMP)
        && node->Successor(0)->PredecessorCount() == 1) //node1 dominates it's follower node
    {

        msg("can join %s node to %s node!  %a %a :-)\n", node->TypeString(), node->Successor(0)->TypeString(),
            node->Address(), node->Successor(0)->Address());
        Node_ptr follower = node->Successor(0);

        node->MarkForDeletion();

        follower->Address(node->Address());

        //Add node's instructions to front of follower node's instruction list.
        follower->Instructions().insert(follower->Instructions().begin(), node->Instructions().begin(), node->Instructions().end());

        follower->Cleanup(false); //clean up goto's and labels. Leave first label and last goto.

        follower->mPreds.clear();

        for (Node_list::iterator p = node->mPreds.begin();
             p != node->mPreds.end();
             p++)
        {
            Node_ptr predNode = *p;
            follower->ConnectPredecessor(predNode);
            predNode->ReconnectSuccessor(node, follower); //reconnect successors
        }
/*
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
*/
        //nMatched++;
        didWorkFlag = true;
        return true;
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

        didWorkFlag = true;
        return true;
    }

    if (node->Type() == Node::CONDITIONAL_JUMP && node->DominatesNode(node->Successor(1)) &&
        node->Successor(1)->PredecessorCount() == 1 && node->Successor(1)->Type() == Node::JUMP &&
        node->Successor(1)->IsSingleJumpNode())
    {
        msg("removed single jump node at %a\n", node->Successor(1)->Address());
        Node_ptr follower = node->Successor(1);
        node->ReconnectSuccessor(follower, follower->Successor(0));
        follower->MarkForDeletion();

        didWorkFlag = true;
        return true;
    }

    return false;
}
