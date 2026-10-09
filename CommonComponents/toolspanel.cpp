#include "toolspanel.h"
#include "CommonComponents/drawContext.h"
#include "CommonComponents/EventHandling.h"
#include "CommonComponents/ScreenSpacePicking.h"

#include <QOpenGLContext>

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
  unsigned int i;
  for( i=0; i<iconvector.size();i++){
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    
    glOrtho(-cntx->w()/2,cntx->w()/2,-cntx->h()/2,cntx->h()/2,-300,300);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef((-cntx->w()/2+100*(i+0.7)),(cntx->h()/2-60),200);

    iconvector[i]->Draw(cntx);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
  }
}

void ToolPanel::AskForData(sV2::Serializer *s){}

void ToolPanel::TreeScan(TSOCntx *cntx){
  if(cntx==&TSOCntx::TSO_Init){
    show=1;
  }
}
