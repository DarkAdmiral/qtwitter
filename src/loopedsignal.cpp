#include "loopedsignal.h"

LoopedSignal::LoopedSignal( int _period, QObject *parent ) : QThread( parent ), period ( _period )
{
  start();
}

void LoopedSignal::run()
{
  forever {
    emit ping();
    sleep( period );
  }
}

void LoopedSignal::setPeriod( int _period )
{
  if ( period != _period ) {
    period = _period;
    terminate();
    start();
  }
}