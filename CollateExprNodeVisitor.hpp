//
// Created by Eric Fry on 15/09/2017.
//

#ifndef DESQUIRR_COLLATEEXPRNODEVISITOR_HPP
#define DESQUIRR_COLLATEEXPRNODEVISITOR_HPP


#include <nodes/NodeVisitor.hpp>

class CollateExprNodeVisitor : public NodeVisitor {
public:
    bool visit(Node_ptr node) override;
};


#endif //DESQUIRR_COLLATEEXPRNODEVISITOR_HPP
