#ifndef __BALL_TREE_VISITOR
#define __BALL_TREE_VISITOR
/**
 * visitor pattern for multiple dispatch
 */
struct ExBallTreeBranch;
struct BallTreeLeaf;

class BallTreeVisitor {
  public:
    virtual void Visit(const ExBallTreeBranch*) = 0;
    virtual void Visit(const BallTreeLeaf*) = 0;
};

#endif  //__BALL_TREE_VISITOR