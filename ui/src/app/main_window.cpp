#include "main_window.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    bridge = std::make_unique<PythonBridge>(this);
    connect(bridge.get(), &PythonBridge::errorOccurred, this, &MainWindow::onPythonError);

    auto env = EnvLoader::load();
    QString pythonService = QString::fromStdString(env["PYTHON_SERVICE_PATH"]);

    if (!bridge->initialize(pythonService)) {
        QMessageBox::critical(this, "System Error", "Could not initialize Python Bridge.\nCheck logs.");
        QTimer::singleShot(0, this, &MainWindow::close);
        return;
    }

    modelInfo = bridge->getModelInfo();
    if (modelInfo.features.empty()) {
        QMessageBox::critical(this, "Model Error", "Failed to load model metadata.");
        QTimer::singleShot(0, this, &MainWindow::close);
        return;
    }

    setupUi();
}

void MainWindow::setupUi() {
    setWindowTitle("Heart Disease Risk Predictor");
    resize(500, 700);

    QString style = R"(
        QMainWindow { background-color: #f5f7fa; }
        QLabel { font-family: 'Segoe UI', sans-serif; color: #2c3e50; }
        QLabel#Title { font-size: 22px; font-weight: bold; color: #34495e; margin-bottom: 10px; }
        QLabel#Subtitle { font-size: 12px; color: #7f8c8d; }
        QLineEdit {
            padding: 8px; border: 1px solid #bdc3c7; border-radius: 4px; background: white; font-size: 14px;
        }
        QLineEdit:focus { border: 2px solid #3498db; }
        QLineEdit[error="true"] { border: 2px solid #e74c3c; background: #fdf0ef; }
        QPushButton {
            background-color: #3498db; color: white; padding: 12px;
            border-radius: 6px; font-size: 14px; font-weight: bold;
        }
        QPushButton:hover { background-color: #2980b9; }
        QPushButton:disabled { background-color: #bdc3c7; }
        QPushButton#ClearBtn { background-color: #ecf0f1; color: #7f8c8d; border: 1px solid #bdc3c7; }
        QPushButton#ClearBtn:hover { background-color: #bdc3c7; color: white; }
        QScrollArea { border: none; background: transparent; }
    )";
    this->setStyleSheet(style);

    central = new QWidget(this);
    setCentralWidget(central);
    mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    title = new QLabel("Heart Disease Assessment", this);
    title->setObjectName("Title");
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    modelInfoLabel = new QLabel(QString("Model: %1 | Accuracy: %2%")
        .arg(QString::fromStdString(modelInfo.model_name))
        .arg(modelInfo.accuracy * 100, 0, 'f', 1), this);
    modelInfoLabel->setObjectName("Subtitle");
    modelInfoLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(modelInfoLabel);

    scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scrollWidget = new QWidget();
    grid = new QGridLayout(scrollWidget);
    grid->setSpacing(15);

    std::map<std::string, std::string> featureDesc = {
        {"age", "Age (years)"}, {"sex", "Sex (0=F, 1=M)"}, {"cp", "Chest Pain (0-3)"},
        {"trestbps", "Resting BP (mm Hg)"}, {"chol", "Cholesterol (mg/dl)"},
        {"fbs", "Fasting Sugar >120 (0/1)"}, {"restecg", "Resting ECG (0-2)"},
        {"thalch", "Max Heart Rate"}, {"exang", "Exercise Angina (0/1)"},
        {"oldpeak", "ST Depression"}, {"slope", "ST Slope (0-2)"},
        {"ca", "Vessels Colored (0-3)"}, {"thal", "Thalassemia (1-3)"}
    };

    int row = 0;
    for (const auto& feature : modelInfo.features) {
        QLabel* label = new QLabel(QString::fromStdString(
            featureDesc.count(feature) ? featureDesc[feature] : feature
        ), scrollWidget);
        label->setFont(QFont("Segoe UI", 10, QFont::Bold));

        QLineEdit* input = new QLineEdit(scrollWidget);
        input->setPlaceholderText("0.0");

        input->setValidator(new QDoubleValidator(0, 1000, 2, input));

        grid->addWidget(label, row, 0);
        grid->addWidget(input, row, 1);
        inputFields[feature] = input;
        row++;
    }

    scroll->setWidget(scrollWidget);
    mainLayout->addWidget(scroll);

    resultLabel = new QLabel("Enter patient data to begin", this);
    resultLabel->setAlignment(Qt::AlignCenter);
    resultLabel->setStyleSheet("background: #ecf0f1; border-radius: 6px; padding: 15px; color: #7f8c8d;");
    mainLayout->addWidget(resultLabel);

    buttonLayout = new QHBoxLayout();

    clearButton = new QPushButton("Clear", this);
    clearButton->setObjectName("ClearBtn");

    predictButton = new QPushButton("Analyze Risk", this);
    predictButton->setCursor(Qt::PointingHandCursor);

    connect(predictButton, &QPushButton::clicked, this, &MainWindow::onPredictClicked);
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::onClearClicked);

    buttonLayout->addWidget(clearButton);
    buttonLayout->addWidget(predictButton);
    mainLayout->addLayout(buttonLayout);
}

// Helper to validate and collect data gracefully
std::optional<std::vector<float>> MainWindow::validateAndCollect() {
    std::vector<float> data;
    bool hasError = false;

    for (const auto& feature : modelInfo.features) {
        QLineEdit* field = inputFields[feature];
        QString text = field->text().trimmed();

        if (text.isEmpty()) {
            field->setProperty("error", true);
            field->style()->unpolish(field);
            field->style()->polish(field);
            hasError = true;
        } else {
            field->setProperty("error", false);
            field->style()->unpolish(field);
            field->style()->polish(field);
            data.push_back(text.toFloat());
        }
    }

    if (hasError) return std::nullopt;
    return data;
}

void MainWindow::onPredictClicked() {
    auto featuresOpt = validateAndCollect();

    if (!featuresOpt.has_value()) {
        resultLabel->setText("Missing Information\nPlease fill in all highlighted fields.");
        resultLabel->setStyleSheet("background: #e74c3c; color: white; padding: 15px; border-radius: 6px; font-weight: bold;");
        return;
    }

    predictButton->setEnabled(false);
    resultLabel->setText("Analyzing...");

    PredictionResult result = bridge->predict(featuresOpt.value());

    if (!result.success) {
        resultLabel->setText("Error: " + QString::fromStdString(result.error_message));
        resultLabel->setStyleSheet("background: #34495e; color: white; padding: 15px;");
    } else {
        bool highRisk = (result.prediction == 1);
        float prob = (highRisk ? result.probability : (1.0 - result.probability)) * 100.0;

        QString text = QString("RISK: %1\nProbability: %2%")
                .arg(highRisk ? "HIGH" : "LOW")
                .arg(prob, 0, 'f', 1);

        QString color = highRisk ? "#e74c3c" : "#2ecc71"; // Red or Green
        resultLabel->setText(text);
        resultLabel->setStyleSheet(QString("background: %1; color: white; padding: 15px; border-radius: 6px; font-size: 16px; font-weight: bold;").arg(color));
    }

    predictButton->setEnabled(true);
}

void MainWindow::onClearClicked() {
    for (auto& pair : inputFields) {
        pair.second->clear();
        pair.second->setProperty("error", false);
        pair.second->style()->unpolish(pair.second);
        pair.second->style()->polish(pair.second);
    }
    resultLabel->setText("Enter patient data to begin");
    resultLabel->setStyleSheet("background: #ecf0f1; border-radius: 6px; padding: 15px; color: #7f8c8d;");
}

void MainWindow::onPythonError(const QString& error) {
    qWarning() << "Bridge Error:" << error;
}