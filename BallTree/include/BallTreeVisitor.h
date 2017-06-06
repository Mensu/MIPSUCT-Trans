#ifndef __BALL_TREE_VISITOR
#define __BALL_TREE_VISITOR
/**
 * visitor pattern for multiple dispatch
 */
struct BallTreeBranch;
struct BallTreeLeaf;

class BallTreeVisitor {
  public:
    virtual void Visit(const BallTreeBranch*) = 0;
    virtual void Visit(const BallTreeLeaf*) = 0;
};

#endif  //__BALL_TREE_VISITOR