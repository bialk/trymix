#ifndef MATRIXCONVERSIONS_H
#define MATRIXCONVERSIONS_H

#include <QMatrix4x4>

#include <glm/mat4x4.hpp>

// GLM indexes matrices as matrix[column][row], while QMatrix4x4 uses
// matrix(row, column). Copying explicitly preserves the transformation and
// supports GLM matrices with float or double components and any qualifier.
// QMatrix4x4 stores floats, so wider component types are narrowed here at the
// rendering boundary.
template<typename T, glm::qualifier Q>
QMatrix4x4 toQMatrix4x4(const glm::mat<4, 4, T, Q>& matrix)
{
  QMatrix4x4 result;
  for(int row = 0; row < 4; ++row) {
    for(int column = 0; column < 4; ++column)
      result(row, column) = static_cast<float>(matrix[column][row]);
  }
  return result;
}

#endif
