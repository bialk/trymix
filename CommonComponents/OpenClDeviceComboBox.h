#pragma once

#include <QComboBox>

class OpenClDeviceComboBox : public QComboBox
{
public:
  explicit OpenClDeviceComboBox(QWidget* parent = nullptr);

  void refreshDevices();
  bool hasDevices() const;
  int selectedDeviceIndex() const;
};
