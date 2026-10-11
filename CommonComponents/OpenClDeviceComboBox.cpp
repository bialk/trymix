#include "OpenClDeviceComboBox.h"

#include "apputil/OpenClDevices.h"

#include <QDebug>
#include <QSizePolicy>

#include <exception>

OpenClDeviceComboBox::OpenClDeviceComboBox(QWidget* parent)
  : QComboBox(parent)
{
  setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
  setMinimumContentsLength(12);
  setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
  connect(this, &QComboBox::currentIndexChanged, this, [this](int index) {
    setToolTip(index >= 0 ? itemData(index, Qt::ToolTipRole).toString()
                          : QString{});
  });
  refreshDevices();
}

void OpenClDeviceComboBox::refreshDevices()
{
  const int previousDevice = selectedDeviceIndex();
  clear();

  try {
    const auto devices = apputil::enumerateOpenClDevices({true, false});
    for(std::size_t index = 0; index < devices.size(); ++index) {
      const QString description =
        QString::fromStdString(devices[index].description());
      addItem(QString::fromStdString(devices[index].deviceName),
              static_cast<int>(index));
      setItemData(count() - 1, description, Qt::ToolTipRole);
    }
  }
  catch(const std::exception& error) {
    qWarning() << "Could not enumerate OpenCL GPUs:" << error.what();
  }

  if(count() == 0) {
    addItem(tr("No OpenCL GPU found"), -1);
    setToolTip(tr("No available OpenCL GPU was detected"));
    setEnabled(false);
    return;
  }

  setEnabled(true);
  const int previousIndex = findData(previousDevice);
  if(previousIndex >= 0)
    setCurrentIndex(previousIndex);
  setToolTip(itemData(currentIndex(), Qt::ToolTipRole).toString());
}

bool OpenClDeviceComboBox::hasDevices() const
{
  return isEnabled() && selectedDeviceIndex() >= 0;
}

int OpenClDeviceComboBox::selectedDeviceIndex() const
{
  return currentData().toInt();
}
