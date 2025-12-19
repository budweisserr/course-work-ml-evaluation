#pragma once
#ifndef PYTHON_BRIDGE_H
#define PYTHON_BRIDGE_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QDebug>
#include <vector>
#include <string>

#include "model_info.h"
#include "prediction_result.h"
#include "json.hpp"

using nmjson = nlohmann::json;

class PythonBridge : public QObject {
  Q_OBJECT

public:
  explicit PythonBridge(QObject *parent = nullptr);
  ~PythonBridge();

  bool initialize(const QString& python_script);
  ModelInfo getModelInfo();
  PredictionResult predict(const std::vector<float>& features);
  void shutdown();

  signals:
      void errorOccurred(const QString& error);

private:
  QString sendCommand(const QString& command);
  nmjson parseResponse(const QString& response);

  QProcess* process;
  bool initialized;
};


#endif // PYTHON_BRIDGE_H
