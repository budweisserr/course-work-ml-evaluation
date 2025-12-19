#pragma once
#ifndef PREDICTION_RESULT_H
#define PREDICTION_RESULT_H

#include <string>

struct PredictionResult {
  bool success;
  int prediction;
  double probability;
  std::string error_message;
};

#endif // PREDICTION_RESULT_H
