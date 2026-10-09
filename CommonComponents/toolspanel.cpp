#include "toolspanel.h"
#include "CommonComponents/drawContext.h"
#include "CommonComponents/EventHandling.h"
#include "CommonComponents/ScreenSpacePicking.h"

#include <QMatrix4x4>

void ToolPanel::Add(EditViewObj * ic){
  iconvector.push_back(ic);
}

EditViewObj* ToolPanel::pick(int mouseX, int mouseY) const
{
  if(!show)
    return nullptr;
  std::vector<QPointF> centers;
  centers.reserve(iconvector.size());
  for(std::size_t index = 0; index < iconvector.size(); ++index)
    centers.emplace_back(100.0 * (static_cast<qreal>(index) + 0.7), 60.0);
  const int index = pickClosestScreenPoint(
    QPointF(static_cast<qreal>(mouseX), static_cast<qreal>(mouseY)),
    centers, 35.0);
  return index >= 0 ? iconvector[static_cast<std::size_t>(index)] : nullptr;
}

void ToolPanel::Draw(DrawCntx *cntx){
  if (!show) return;
  QMatrix4x4 projection;
  projection.ortho(-cntx->w() * 0.5F, cntx->w() * 0.5F,
                   -cntx->h() * 0.5F, cntx->h() * 0.5F,
                   -300.0F, 300.0F);
  for(std::size_t index = 0; index < iconvector.size(); ++index) {
    QMatrix4x4 modelView;
    modelView.translate(-cntx->w() * 0.5F
                          + 100.0F * (static_cast<float>(index) + 0.7F),
                        cntx->h() * 0.5F - 60.0F, 200.0F);
    iconvector[index]->DrawWithMatrices(cntx, projection, modelView);
  }
}

void ToolPanel::AskForData(sV2::Serializer *s){}

void ToolPanel::TreeScan(TSOCntx *cntx){
  if(cntx==&TSOCntx::TSO_Init){
    show=1;
  }
}
