#ifndef SCREENSPACEPICKING_H
#define SCREENSPACEPICKING_H

#include <QPointF>

#include <vector>

inline int pickClosestScreenPoint(
    const QPointF& cursor, const std::vector<QPointF>& points, qreal radius)
{
  const qreal maximumDistanceSquared = radius * radius;
  qreal closestDistanceSquared = maximumDistanceSquared;
  int closestIndex = -1;
  for(int index = 0; index < static_cast<int>(points.size()); ++index) {
    const QPointF delta = points[static_cast<std::size_t>(index)] - cursor;
    const qreal distanceSquared = delta.x() * delta.x() + delta.y() * delta.y();
    if(distanceSquared <= closestDistanceSquared) {
      closestDistanceSquared = distanceSquared;
      closestIndex = index;
    }
  }
  return closestIndex;
}

#endif // SCREENSPACEPICKING_H
