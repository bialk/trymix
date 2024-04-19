#ifndef PROJECTTREE_H
#define PROJECTTREE_H

#include "drawContext.h"

#include <QTreeWidget>

class ProjectTreeItem;
class CentralWidget;
class DrawCntx;
class ProjectTreeItem;

class ProjectTree : public QTreeWidget
{
public:
  ProjectTree(QWidget* parent);
  virtual void showModel(DrawCntx* cx);
  CentralWidget* gl();
  void removeItem(ProjectTreeItem* item);

  struct AbstractFactoryItem
    {
      virtual QString name() = 0;
      virtual QString iconPath() = 0;
      virtual ProjectTreeItem* create()= 0;
    };


  template<typename T>
  struct FactoryItem: public AbstractFactoryItem
    {
      QString name() override { return T::name();}
      QString iconPath() override { return T::iconPath();}
      ProjectTreeItem* create() override {
          return new T;
      }
    };

  static std::vector<AbstractFactoryItem*> const& TreeItemFactoryList();
private:
  ProjectTreeItem* m_activeTreeItem{nullptr};
  CentralWidget* m_gl{nullptr};
  QList<QAction*> m_actions;
};

#endif // PROJECTTREE_H
