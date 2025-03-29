#ifndef toolspanel_h
#define toolspanel_h
#include <vector>
#include "CommonComponents/drawContext.h"

class DispView;
class ViewCtrl;
class DrawCntx;


class ToolPanel: public EditViewObj{
  std::vector<EditViewObj*> iconvector;
 public:
  ViewCtrl *m_viewctrl = nullptr;
  int show = 1;
  void Draw(DrawCntx *cntx);
  void TreeScan(TSOCntx *cntx);
  void AskForData(sV2::Serializer *s);

  void Add(EditViewObj* ic);
};

#endif 

