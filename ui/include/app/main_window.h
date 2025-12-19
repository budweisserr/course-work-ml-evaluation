#pragma once
#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QTimer>
#include <QMessageBox>
#include <QDoubleValidator>
#include <QStyle>
#include <memory>
#include <map>
#include <vector>
#include <optional>
#include <random>

#include "python_bridge.h"
#include "model_info.h"

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);

private slots:
  void onPredictClicked();
  void onClearClicked();
  void onPythonError(const QString& error);


  void onThemeToggle();
  void onLangToggle();
  void onRandomData();

private:
  void setupUi();
  void setupToolbar();
  void updateTheme();
  void updateTexts();
  std::optional<std::vector<float>> validateAndCollect();

  // Components
  QWidget *central;
  QVBoxLayout *mainLayout;
  QHBoxLayout *headerLayout;
  QHBoxLayout *buttonLayout;
  QScrollArea *scroll;
  QWidget *scrollWidget;
  QGridLayout *grid;

  QLabel *title;
  QLabel *modelInfoLabel;
  QLabel *resultLabel;

  QPushButton *predictButton;
  QPushButton *clearButton;

  QPushButton *themeBtn;
  QPushButton *langBtn;
  QPushButton *randomBtn;

  std::unique_ptr<PythonBridge> bridge;
  ModelInfo modelInfo;
  std::map<std::string, QLineEdit*> inputFields;
  std::map<std::string, QLabel*> featureLabels;

  // State
  bool isDarkTheme = false;
  QString currentLang = "en";
};

#endif // MAIN_WINDOW_H