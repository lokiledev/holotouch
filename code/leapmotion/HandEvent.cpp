#include "leapmotion/HandEvent.h"

//types must be greater than User
HandEvent::HandEvent(QEvent::Type pType,
          Vector pPos,
          HandId_t pId, int pSelect)
    : QEvent(pType),
      pos_(pPos),
      hand_(pId),
      itemSelected_(pSelect)
{
}

Vector HandEvent::pos()
{
    return pos_;
}

HandEvent::HandId_t HandEvent::hand()
{
    return hand_;
}

int HandEvent::item()
{
    return itemSelected_;
}
