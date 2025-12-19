#pragma once
#ifndef COURSE_WORK_ML_EVALUATION_PREDICTION_RESULT_H
#define COURSE_WORK_ML_EVALUATION_PREDICTION_RESULT_H

#include <string>

struct PredictionResult {
  bool success;
  int prediction;
  double probability;
  std::string error_message;
};

#endif // COURSE_WORK_ML_EVALUATION_PREDICTION_RESULT_H
