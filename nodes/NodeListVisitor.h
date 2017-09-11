#ifndef DESQUIRR_NODELISTVISITOR_H
#define DESQUIRR_NODELISTVISITOR_H


#include <desquirr.hpp>

class NodeListVisitor
{
public:
    virtual bool visit(Node_list &nodes) = 0;

    bool accept(Instruction *instruction, NodeListVisitor &visitor)
    {
        bool status = false;

    }
};


#endif //DESQUIRR_NODELISTVISITOR_H
