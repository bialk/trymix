#ifndef toolspanel_h
#define toolspanel_h
#include <vector>
#include "editviewobj.h"

class DispView;
class ViewCtrl;


class ToolPanel: public EditViewObj{
  std::vector<EditViewObj*> iconvector;
 public:
  ViewCtrl *m_viewctrl;
  int show;
  void Draw(DrawCntx *cntx);
  void TreeScan(TSOCntx *cntx);
  void AskForData(Serializer *s);

  void Add(EditViewObj* ic);
};

#endif 

