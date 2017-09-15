//
// Created by Eric Fry on 13/09/2017.
//

#ifndef DESQUIRR_COLLATENODEVISITOR_HPP
#define DESQUIRR_COLLATENODEVISITOR_HPP

#include "nodes/NodeVisitor.hpp"

class CollateNodeVisitor  : public NodeVisitor {
    bool visit(Node_ptr node) override;
};


#endif //DESQUIRR_COLLATENODEVISITOR_HPP
