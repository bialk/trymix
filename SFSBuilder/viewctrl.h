#ifndef viewctrl_h
#define viewctrl_h

#include "editviewobj.h"

#include <glm/gtx/quaternion.hpp>
#include <functional>

class ViewCtrl: public EditViewObj{
 public:

  // view camera point controlling methods
  enum Opercode{
    Rotate,
    Translate,
    Scale,
    FoV
  };

  ViewCtrl();
  ~ViewCtrl();
  void AskForData(Serializer *s) override;
  void TreeScan(TSOCntx *cntx) override;
  void Draw(DrawCntx *cntx) override;

  glm::mat4 const& getModelViewMtrx();
  glm::mat4 const& updateModelViewMtrx();

  glm::mat4 const& getProjectionMtrx();
  glm::mat4 const& updateProjectionMtrx(int w, int h);
  glm::mat4 const& updateProjectionMtrx();

  glm::mat4 const& updateSelectionMtrx(float gx=0, float gy=0);
  glm::mat4 const& getSelectionMtrx();

  std::function<void(float x, float y)>  startOperation(ViewCtrl::Opercode opercode, float x, float y);
  void reset();

  int background; // 0 - black, 1 - white

private:
  int prjtype;    // 1 - orthogonal, 2 - perspecive
  
  void fplay();
  int play_method;

  float m_w,m_h;
  float m_scale=1.f;
  float m_fovY=glm::radians(35.f);
  glm::quat m_rot{1.0,.0,.0,.0};
  glm::vec3 m_trans{.0,.0,-200};

  glm::mat4 m_P{1.0};
  glm::mat4 m_Ps{1.0};
  glm::mat4 m_MV{1.0};
};



#endif

