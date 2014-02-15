#include "leapmotion/HandEvent.h"

//types must be greater than User
HandEvent::HandEvent(QEvent::Type pType,
          Vector pPos,
          int pSelect,
          Selection_t pSelectMode,
          float pZoom)
    : QEvent(pType),
      pos_(pPos),
      itemSelected_(pSelect),
      selection_(pSelectMode),
      zoomOffset_(pZoom)
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

float HandEvent::zoom()
{
    return zoomOffset_;
}
