# ML Evaluation Coursework

![Language](https://img.shields.io/github/languages/top/budweisserr/course-work-ml-evaluation)
![Repo size](https://img.shields.io/github/repo-size/budweisserr/course-work-ml-evaluation)
![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)

> End-to-end example of training and evaluating a machine-learning model in Python and using it from a C++ desktop UI.

---

## Table of Contents

- [Overview](#overview)
- [Getting Started](#getting-started)
    - [Python model environment](#python-model-environment)
    - [C++ UI build](#c-ui-build)
- [How to Use](#how-to-use)
    - [1. Train and save the model](#1-train-and-save-the-model)
    - [2. Create .env file in the project root folder](#2-create-env-file-in-the-project-root-folder)
    - [3. Start the C++ UI](#3-start-the-c-ui)
- [Implementation Details](#implementation-details)
    - [Model side (`model/`)](#model-side-model)
    - [UI side (`ui/`)](#ui-side-ui)
- [License](#license)

---

## Overview

This repository contains a coursework project for a Big Data / Machine Learning subject.  
The idea is to show a **full pipeline**:

1. Data exploration, training and evaluation of ML models in Python.
2. Packaging the trained model into a simple **service**.
3. Consuming that service from a **C++ GUI application**, so the user can interactively run predictions and see results without touching Python directly.

The `model/` folder is responsible for **training and serving the ML model**, while the `ui/` folder contains a **C++ desktop application** that talks to that service.

---

## Getting Started

### Python model environment

1. **Navigate to the model directory:**
   ```bash
   cd model
   ```

2. **Create a virtual environment** (recommended):
   ```bash
   python -m venv venv
   source venv/bin/activate  # On Windows: venv\Scripts\activate
   ```

3. **Install dependencies:**
   ```bash
   pip install -r requirements.txt
   ```

4. **Edit or run ipynb file.**

### C++ UI build

1. **Prerequisites:**
    - CMake (version 4.0 or higher)
    - A C++17 compatible compiler (GCC, Clang, MSVC)
    - Qt5 or Qt6 (depending on configuration)

2. **Build the application:**
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```

---

## How to Use

### 1. Train and save the model

From the `model/` directory with your virtual environment activated run ipynb file to generate models (pkl's and json)

This script will:
- Load and preprocess the dataset
- Train the machine learning model
- Evaluate performance metrics
- Save the trained model to disk (same folder)

### 2. Create .env file in the project root folder

.env file should look like this

```bash
DATASET_PATH=YOUR_PATH/heart_disease_uci.csv
MODEL_PATH=YOUR_PATH/best_model.pkl
SCALER_PATH=YOUR_PATH/scaler.pkl
METADATA_PATH=YOUR_PATH/model_metadata.json
PYTHON_SERVICE_PATH=YOUR_PATH/predict_service.py
PYTHON_INTERPRETER_PATH=YOUR_PATH/bin/python3
```

### 3. Start the C++ UI

From the build directory:

```bash
./course-work-ml-evaluation  # On Windows: course-work-ml-evaluation.exe
```

The GUI application will launch and connect to the model service. You can now:
- Input data through the interface
- Request predictions

---

## Implementation Details

### Model side (`model/`)

**Key files:**
- `model_training.ipynb` - Main training script that handles data loading, preprocessing, model training, and evaluation
- `predict_service.py` - Loads the trained model and serves predictions
- `kaggledownload.py` - Downloads heart disease uci dataset in .csv format

**Typical workflow:**
- Data is loaded and cleaned
- Features are engineered and normalized
- Multiple models may be trained and compared
- Best model is selected based on evaluation metrics
- Model is serialized using pickle or onnx

### UI side (`ui/`)

**Key components:**
- `src/mainwindow.cpp/h` - Main window implementation with Qt widgets
- `src/python_bridge.cpp/h` - Bridge between Python ant C++ for communicating with the model service
- `CMakeLists.txt` - Build configuration

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
