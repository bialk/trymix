#ifndef dispview_h
#define dispview_h

#include <list>
#include <QOpenGLFunctions>
#include "editviewobj.h"
#include "eventhnd.h"
//#include <sigc++/sigc++.h>
#include <memory>
#include <functional>
#include <memory>

class ToolPanel;
class EditViewObj;
class ViewCtrl;
class Lights;
class Serializer;
//class ViewSurf;
class ImagePlane;
//class Matting;

class GLScene: public std::list<EditViewObj*>{
 public:
  void Add(EditViewObj* globj);
  void Remove(EditViewObj* globj);
};

class DispView;
class DispViewEH: public EvtHandle{
 public:
  DispView *dv;
  DispViewEH(DispView* v);
  virtual void Handle(EventBall *eventball);
};


class DispView{
 public:
  // signaling interface
  std::function<void()> redraw;
  std::function<void()> quit;
  std::function<void(const char*)> dbgmsg;


  DispView();
  ~DispView();

  //main window event handler list

  // common functionality
  int noredisplay;
  int redisplay;

  int drawsimple;

  GLScene scene;

  void Draw(DrawCntx *cntx);
  void TreeScan(TSOCntx *cntx);

  // main application entities
  std::unique_ptr<ToolPanel> toolpanel;
  std::unique_ptr<ViewCtrl>  viewctrl;
  std::unique_ptr<Lights>    lights;
  //std::unique_ptr<ViewSurf>  viewsurf;
  std::unique_ptr<ImagePlane>  imageplane;
  //std::unique_ptr<Matting>  matting;

  // object selection mechanics
  GLuint glnamecount;
  GLuint GetNewName();
  std::function<void()> reselect;
  int selectid;
  void SelectRst();
  int SelectObj(int x, int y);

  void AskForData(Serializer *s);

  // snapshot of the screen  
  long nsnap;
  void snapshot();

  DispViewEH   dispvieweh;

  //EvthStack     evq_view;
  //EvthStack     evq_scene;
};

#endif


