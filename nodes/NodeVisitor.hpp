//
// Created by efry on 12/09/2017.
//

#ifndef DESQUIRR_NODELISTVISITOR_HPP
#define DESQUIRR_NODELISTVISITOR_HPP

#include <desquirr.hpp>

class NodeVisitor
{
public:
    NodeVisitor() {
        didWorkFlag = false;
    }
    bool didWork() { return didWorkFlag; }
    void reset() { didWorkFlag = false; }

    virtual bool visit(Node_ptr node) = 0;

protected:
    bool didWorkFlag;
};


#endif //DESQUIRR_NODELISTVISITOR_HPP
