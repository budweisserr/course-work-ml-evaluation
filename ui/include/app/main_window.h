#pragma once
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QGroupBox>
#include <QScrollArea>
#include <QFont>
#include <QDebug>
#include <QTimer>
#include <QStyle>
#include <QDoubleValidator>
#include <stdexcept>
#include <map>
#include <memory>
#include <optional>
#include "python_bridge.h"

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow() = default;

private slots:
  void onPredictClicked();
  void onClearClicked();
  void onPythonError(const QString& error);

private:
  void setupUi();
  std::optional<std::vector<float>> validateAndCollect();

  std::unique_ptr<PythonBridge> bridge;
  ModelInfo modelInfo;
  std::map<std::string, QLineEdit*> inputFields;
  QLabel* resultLabel;
  QLabel* modelInfoLabel;
  QPushButton* predictButton;
  QPushButton* clearButton;

  QWidget* central;
  QVBoxLayout* mainLayout;
  QLabel* title;
  QScrollArea* scroll;
  QWidget* scrollWidget;
  QGridLayout* grid;
  QHBoxLayout* buttonLayout;
};

#endif // MAINWINDOW_H