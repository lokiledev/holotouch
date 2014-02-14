#include "leapmotion/HandEvent.h"

//types must be greater than User
HandEvent::HandEvent(QEvent::Type pType,
          Vector pPos,
          int pSelect,
          Selection_t pSelectMode)
    : QEvent(pType),
      pos_(pPos),
      itemSelected_(pSelect),
      selection_(pSelectMode)
{
}

Vector HandEvent::pos()
{
    return pos_;
}

int HandEvent::item()
{
    return itemSelected_;
}

HandEvent::Selection_t HandEvent::selectMode()
{
    return selection_;
}
