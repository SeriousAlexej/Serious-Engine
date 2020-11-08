
#ifndef SE_INCL_LISTITERATOR_INL
#define SE_INCL_LISTITERATOR_INL
#ifdef PRAGMA_ONCE
  #pragma once
#endif

/* simple list iterator: all functions are inline */
template<class Cbase>
class CListIter {
public:
  CListNode *li_CurrentNode;
public:
  /* default constructor - no list attached */
  CListIter(void) {
    li_CurrentNode = NULL;
  };
  /* constructor with list attaching */
  CListIter(const CListHead &lhHead) {
    li_CurrentNode = &lhHead.IterationHead();
  };
  /* constructor to start from given node */
  CListIter(CListNode &lnNode) {
    ASSERT(lnNode.IsLinked());
    li_CurrentNode = &lnNode;
  };
  /* start iterating */
  void Reset(const CListHead &lhHead) {
    li_CurrentNode = &lhHead.IterationHead();
  };
  /* move to next node */
  void MoveToNext(void) {
    li_CurrentNode = &li_CurrentNode->IterationSucc();
  };
  /* move to previous node */
  void MoveToPrev(void) {
    li_CurrentNode = &li_CurrentNode->IterationPred();
  };
  /* check if finished */
  BOOL IsPastEnd(void) {
    return li_CurrentNode->IsTailMarker();
  };
  /* Insert a node after current one. */
  inline void InsertAfterCurrent(CListNode &lnNew) {
    li_CurrentNode->IterationInsertAfter(lnNew);
  };
  /* Insert a node before current one. */
  inline void InsertBeforeCurrent(CListNode &lnNew) {
    li_CurrentNode->IterationInsertBefore(lnNew);
  };

  /* Get current element. */
  Cbase &Current(void) { return *((Cbase*)(li_CurrentNode->p_data)); }
  Cbase &operator*(void) { return *((Cbase*)(li_CurrentNode->p_data)); }
  operator Cbase* (void) { return ((Cbase*)(li_CurrentNode->p_data)); }
  Cbase *operator->(void) { return ((Cbase*)(li_CurrentNode->p_data)); }
};

// declare a list iterator for a class with a CListNode
#define LISTITER(baseclass) CListIter<baseclass>

// make 'for' construct for walking a list
#define FOREACHINLIST(baseclass, head, iter) \
  for ( LISTITER(baseclass) iter(head); !iter.IsPastEnd(); iter.MoveToNext() )

// make 'for' construct for walking a list, keeping the iterator for later use
#define FOREACHINLISTKEEP(baseclass, head, iter) \
  LISTITER(baseclass) iter(head); \
  for (; !iter.IsPastEnd(); iter.MoveToNext() )

// make 'for' construct for deleting a list
#define FORDELETELIST(baseclass, head, iter)		  \
   for ( LISTITER(baseclass) iter(head), iter##next;	  \
   iter##next=iter, iter##next.IsPastEnd() || (iter##next.MoveToNext(),1), !iter.IsPastEnd(); \
     iter = iter##next)

// get the pointer to the first element in the list
#define LIST_HEAD(listhead, baseclass) \
  ( (baseclass *) (listhead.Head().p_data) )
// get the pointer to the last element in the list
#define LIST_TAIL(listhead, baseclass) \
  ( (baseclass *) (listhead.Tail().p_data ) )

// get the pointer to the predecessor of the element
#define LIST_PRED(element, baseclass, member) \
  ( (baseclass *) ((element).member.Pred().p_data) )
// get the pointer to the successor of the element
#define LIST_SUCC(element, baseclass, member) \
  ( (baseclass *) ((element).member.Succ().p_data) )



#endif  /* include-once check. */

