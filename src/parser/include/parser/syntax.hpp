#ifndef PICOHASKELL_SYNTAX_HPP
#define PICOHASKELL_SYNTAX_HPP

class SyntaxTreeNode {
public:
    virtual ~SyntaxTreeNode() = default;
};
class Literal : public SyntaxTreeNode {

};
class Constructor : public SyntaxTreeNode {

};
class Variable : public SyntaxTreeNode {

};

#endif //PICOHASKELL_SYNTAX_HPP
