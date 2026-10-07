#include "ProjectTree.h"
#include "Projects_TreeItem.h"
#include "PolygonTests/PolygonTests_TreeItem.h"
#include "SFSBuilder/SFSBuilder_TreeItem.h"
#include "CameraControl/CameraControl_TreeItem.h"
#include "BlurTests/BlurTests_TreeItem.h"
#include "MRFSegm/MRFSegm_TreeItem.h"
#include "ShaderMix/ShaderMix_TreeItem.h"
#include "CentralWidget.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QHeaderView>
#include <QMenu>
#include <QShowEvent>
#include <QStyle>
#include <QToolButton>

std::vector<ProjectTree::AbstractFactoryItem *> const& ProjectTree::TreeItemFactoryList()
{

  static std::vector<AbstractFactoryItem*> treeItems{
    new FactoryItem<PolygonTests_TreeItem>,
        new FactoryItem<SFSBuilder_TreeItem>,
        new FactoryItem<CameraControl_TreeItem>,
        new FactoryItem<BlurTests_TreeItem>,
        new FactoryItem<ShaderMix_TreeItem>,
        new FactoryItem<MRFSegm_TreeItem>
  };

  return treeItems;
}


ProjectTree::ProjectTree(QWidget *parent)
  :QTreeWidget(parent)
  ,m_gl(new CentralWidget)
{

   m_gl->setProjectTree(this);

  // constructing actions for context menu for the whole tree (pops it when mouse off any item)
  for(auto& i: ProjectTree::TreeItemFactoryList()){
    auto newAct = new QAction(QIcon(i->iconPath()),
                              QString(QObject::tr("Create \"%1\"")).arg(i->name()));
    newAct->setStatusTip(QObject::tr("Create new project item"));
    newAct->connect(newAct, &QAction::triggered, newAct,
      [this, i]()
      {
        auto newItem = i->create();
        newItem->setIcon(0,QIcon(i->iconPath()));
        invisibleRootItem()->addChild(newItem);
        addRemoveButton(newItem);
        //setExpanded(true);
      }
    );
    m_actions.append(newAct);
  }

  auto newAct = new QAction(QIcon(":/Resource/warning32.ico"), QObject::tr("Clear Tree"));
  newAct->setStatusTip(QObject::tr("Clear project tree"));
  newAct->connect(newAct, &QAction::triggered, newAct,
    [this]()
    {
      while(invisibleRootItem()->childCount())
          delete invisibleRootItem()->takeChild(0);
      m_activeTreeItem=nullptr;
    }
  );
  m_actions.append(newAct);


  // context menu handler
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, &QTreeWidget::customContextMenuRequested,
          this, [this](const QPoint& pos)
  {
    ProjectTreeItem *selectedItem = dynamic_cast<ProjectTreeItem*>(this->itemAt( pos ));
    if(selectedItem){
      QMenu menu;
      menu.addActions(selectedItem->contextMenuActions());
      menu.exec( mapToGlobal(pos) );
    }
    else{
        QMenu menu;
        menu.addActions(m_actions);
        menu.exec( mapToGlobal(pos) );
    }
  });

  // item selector
  connect(this, &QTreeWidget::itemClicked,
          this, [this](QTreeWidgetItem* item, int col){

    auto mainwnd = findParentOfType<MainWindow>(this->parent());

    if(m_activeTreeItem)
      m_activeTreeItem->activateProjectTreeItem(mainwnd->UI()->dockWidget_params, false);
    m_activeTreeItem=dynamic_cast<ProjectTreeItem*>(item);
    m_activeTreeItem->activateProjectTreeItem(mainwnd->UI()->dockWidget_params);
    m_gl->update();
  });
}

void
ProjectTree::showModel(DrawCntx* cx)
{
  if(m_activeTreeItem)
    m_activeTreeItem->showModel(cx);
}

CentralWidget*
ProjectTree::gl(){
  return m_gl;
}

void ProjectTree::addRemoveButton(ProjectTreeItem* item)
{
  constexpr int removeColumn = 2;
  const QSize buttonSize = removeButtonSize();
  configureRemoveColumn(buttonSize.width());

  auto* button = new QToolButton(this);
  button->setAutoRaise(true);
  button->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
  const int iconExtent = style()->pixelMetric(
    QStyle::PM_SmallIconSize, nullptr, button);
  button->setIconSize(QSize(iconExtent, iconExtent));
  button->setToolTip(QObject::tr("Remove %1").arg(item->text(0)));
  button->setFixedSize(buttonSize);
  setItemWidget(item, removeColumn, button);

  connect(button, &QToolButton::clicked, this, [this, item]() {
    if (item->treeWidget() == this)
      removeItem(item);
  });
}

void ProjectTree::showEvent(QShowEvent* event)
{
  QTreeWidget::showEvent(event);
  configureRemoveColumn(removeButtonSize().width());
}

QSize ProjectTree::removeButtonSize() const
{
  QToolButton button;
  button.setAutoRaise(true);
  button.setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
  const int iconExtent = style()->pixelMetric(
    QStyle::PM_SmallIconSize, nullptr, &button);
  button.setIconSize(QSize(iconExtent, iconExtent));
  return button.sizeHint();
}

void ProjectTree::configureRemoveColumn(int buttonWidth)
{
  constexpr int removeColumn = 2;
  headerItem()->setText(removeColumn, {});
  header()->setStretchLastSection(false);
  header()->setSectionResizeMode(0, QHeaderView::Stretch);
  header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
  header()->setSectionResizeMode(removeColumn, QHeaderView::Fixed);
  header()->resizeSection(removeColumn, buttonWidth + 2);
}

void ProjectTree::removeItem(ProjectTreeItem* item)
{
  if(m_activeTreeItem == item)
    m_activeTreeItem = nullptr;
  delete item;
}
