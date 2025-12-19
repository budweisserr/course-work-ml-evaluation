#include "main_window.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    bridge = std::make_unique<PythonBridge>(this);
    
    connect(bridge.get(), &PythonBridge::errorOccurred, 
            this, &MainWindow::onPythonError);
    
    if (!bridge->initialize("predict_service.py")) {
        QMessageBox::critical(this, "error", "failed to initialize python bridge");
        close();
        return;
    }
    
    modelInfo = bridge->getModelInfo();
    
    if (modelInfo.features.empty()) {
        QMessageBox::critical(this, "error", "failed to load model info");
        close();
        return;
    }
    
    setupUi();
}

void MainWindow::setupUi() {
    setWindowTitle("heart disease risk predictor");
    resize(600, 750);
    
    central = new QWidget(this);
    setCentralWidget(central);
    
    mainLayout = new QVBoxLayout(central);
    
    title = new QLabel("heart disease risk assessment", this);
    QFont titleFont = title->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setAlignment(Qt::AlignCenter);
    //titleFont.addWidget(title);
    
    modelInfoLabel = new QLabel(this);
    modelInfoLabel->setText(
        QString("model: %1 | accuracy: %2%")
        .arg(QString::fromStdString(modelInfo.model_name))
        .arg(modelInfo.accuracy * 100, 0, 'f', 1)
    );
    modelInfoLabel->setAlignment(Qt::AlignCenter);
    modelInfoLabel->setStyleSheet("QLabel { color: #666; margin: 5px; }");
    mainLayout->addWidget(modelInfoLabel);
    
    scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scrollWidget = new QWidget();
    grid = new QGridLayout(scrollWidget);
    
    std::map<std::string, std::string> featureDesc = {
        {"age", "age (years)"},
        {"sex", "sex (0=female, 1=male)"},
        {"cp", "chest pain type (0-3)"},
        {"trestbps", "resting blood pressure (mm hg)"},
        {"chol", "cholesterol (mg/dl)"},
        {"fbs", "fasting blood sugar >120 (0/1)"},
        {"restecg", "resting ecg (0-2)"},
        {"thalch", "max heart rate"},
        {"exang", "exercise angina (0/1)"},
        {"oldpeak", "st depression"},
        {"slope", "st slope (0-2)"},
        {"ca", "vessels colored (0-3)"},
        {"thal", "thalassemia (1-3)"}
    };
    
    int row = 0;
    for (const auto& feature : modelInfo.features) {
        QLabel* label = new QLabel(QString::fromStdString(
            featureDesc.count(feature) ? featureDesc[feature] : feature
        ), scrollWidget);
        
        QLineEdit* input = new QLineEdit(scrollWidget);
        input->setPlaceholderText("enter value");
        
        grid->addWidget(label, row, 0);
        grid->addWidget(input, row, 1);
        
        inputFields[feature] = input;
        row++;
    }
    
    scroll->setWidget(scrollWidget);
    mainLayout->addWidget(scroll);
    
    resultLabel = new QLabel("enter patient data and click predict", this);
    resultLabel->setAlignment(Qt::AlignCenter);
    resultLabel->setStyleSheet("QLabel { padding: 15px; font-size: 14pt; }");
    resultLabel->setWordWrap(true);
    mainLayout->addWidget(resultLabel);
    
    buttonLayout = new QHBoxLayout();
    
    predictButton = new QPushButton("predict", this);
    predictButton->setStyleSheet("QPushButton { padding: 10px; font-size: 12pt; }");
    connect(predictButton, &QPushButton::clicked, this, &MainWindow::onPredictClicked);
    
    clearButton = new QPushButton("clear", this);
    clearButton->setStyleSheet("QPushButton { padding: 10px; font-size: 12pt; }");
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::onClearClicked);
    
    buttonLayout->addWidget(predictButton);
    buttonLayout->addWidget(clearButton);
    mainLayout->addLayout(buttonLayout);
}

std::vector<float> MainWindow::collectFeatures() {
    std::vector<float> features;
    
    for (const auto& feature_name : modelInfo.features) {
        auto it = inputFields.find(feature_name);
        if (it == inputFields.end()) {
            throw std::runtime_error("missing input field: " + feature_name);
        }
        
        QString text = it->second->text().trimmed();
        if (text.isEmpty()) {
            throw std::runtime_error("empty field: " + feature_name);
        }
        
        bool ok;
        float value = text.toFloat(&ok);
        if (!ok) {
            throw std::runtime_error("invalid value for: " + feature_name);
        }
        
        features.push_back(value);
    }
    
    return features;
}

void MainWindow::onPredictClicked() {
    try {
        predictButton->setEnabled(false);
        
        auto features = collectFeatures();
        PredictionResult result = bridge->predict(features);
        
        if (!result.success) {
            QMessageBox::warning(this, "prediction error", 
                QString::fromStdString(result.error_message));
            predictButton->setEnabled(true);
            return;
        }
        
        QString risk_text;
        QString style;
        
        if (result.prediction == 0) {
            risk_text = QString("risk: LOW\nno heart disease detected\nprobability: %1%")
                .arg((1 - result.probability) * 100, 0, 'f', 1);
            style = "QLabel { background-color: #4CAF50; color: white; "
                   "padding: 15px; font-size: 14pt; font-weight: bold; }";
        } else {
            risk_text = QString("risk: HIGH\nheart disease detected\nprobability: %1%")
                .arg(result.probability * 100, 0, 'f', 1);
            style = "QLabel { background-color: #F44336; color: white; "
                   "padding: 15px; font-size: 14pt; font-weight: bold; }";
        }
        
        resultLabel->setText(risk_text);
        resultLabel->setStyleSheet(style);
        
        predictButton->setEnabled(true);
        
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "input error", e.what());
        predictButton->setEnabled(true);
    }
}

void MainWindow::onClearClicked() {
    for (auto& pair : inputFields) {
        pair.second->clear();
    }
    resultLabel->setText("enter patient data and click predict");
    resultLabel->setStyleSheet("QLabel { padding: 15px; font-size: 14pt; }");
}

void MainWindow::onPythonError(const QString& error) {
    qDebug() << "python error:" << error;
}