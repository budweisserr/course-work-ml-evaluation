#pragma once
#ifndef MODEL_INFO_H
#define MODEL_INFO_H

#include <string>
#include <vector>

struct ModelInfo {
  std::string model_name;
  std::vector<std::string> features;
  int num_features;
  double accuracy;
  double precision;
  double recall;
  double f1_score;
};

#endif // MODEL_INFO_H
