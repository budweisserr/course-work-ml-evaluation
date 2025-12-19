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
    updateTheme();
}

void MainWindow::setupUi() {
    resize(500, 750);

    central = new QWidget(this);
    setCentralWidget(central);
    mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    setupToolbar();

    title = new QLabel(this);
    title->setObjectName("Title");
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    modelInfoLabel = new QLabel(this);
    modelInfoLabel->setObjectName("Subtitle");
    modelInfoLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(modelInfoLabel);

    scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scrollWidget = new QWidget();
    grid = new QGridLayout(scrollWidget);
    grid->setSpacing(15);

    int row = 0;
    for (const auto& feature : modelInfo.features) {
        QLabel* label = new QLabel(scrollWidget);
        label->setFont(QFont("Segoe UI", 10, QFont::Bold));
        featureLabels[feature] = label;

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

    resultLabel = new QLabel(this);
    resultLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(resultLabel);

    buttonLayout = new QHBoxLayout();

    clearButton = new QPushButton(this);
    clearButton->setObjectName("ClearBtn");

    predictButton = new QPushButton(this);
    predictButton->setCursor(Qt::PointingHandCursor);

    connect(predictButton, &QPushButton::clicked, this, &MainWindow::onPredictClicked);
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::onClearClicked);

    buttonLayout->addWidget(clearButton);
    buttonLayout->addWidget(predictButton);
    mainLayout->addLayout(buttonLayout);

    updateTexts();
}

void MainWindow::setupToolbar() {
    QHBoxLayout* toolLayout = new QHBoxLayout();
    toolLayout->setAlignment(Qt::AlignRight);

    randomBtn = new QPushButton("🎲", this);
    randomBtn->setToolTip("Fill with random test data");
    randomBtn->setFixedSize(40, 40);
    connect(randomBtn, &QPushButton::clicked, this, &MainWindow::onRandomData);

    themeBtn = new QPushButton("🌙", this);
    themeBtn->setFixedSize(40, 40);
    connect(themeBtn, &QPushButton::clicked, this, &MainWindow::onThemeToggle);

    langBtn = new QPushButton("UA", this);
    langBtn->setFixedSize(40, 40);
    langBtn->setCheckable(true);
    connect(langBtn, &QPushButton::clicked, this, &MainWindow::onLangToggle);

    toolLayout->addWidget(randomBtn);
    toolLayout->addWidget(langBtn);
    toolLayout->addWidget(themeBtn);

    mainLayout->addLayout(toolLayout);
}

void MainWindow::onThemeToggle() {
    isDarkTheme = !isDarkTheme;
    themeBtn->setText(isDarkTheme ? "☀️" : "🌙");
    updateTheme();
}

void MainWindow::updateTheme() {
    QString bg = isDarkTheme ? "#2c3e50" : "#f5f7fa";
    QString text = isDarkTheme ? "#ecf0f1" : "#2c3e50";
    QString inputBg = isDarkTheme ? "#34495e" : "white";
    QString border = isDarkTheme ? "#7f8c8d" : "#bdc3c7";

    QString style = QString(R"(
        QMainWindow { background-color: %1; }
        QLabel { color: %2; font-family: 'Segoe UI'; }
        QLabel#Title { font-size: 22px; font-weight: bold; }
        QLabel#Subtitle { font-size: 12px; color: #7f8c8d; }
        QLineEdit {
            padding: 8px; border: 1px solid %4; border-radius: 4px;
            background: %3; color: %2; font-size: 14px;
        }
        QLineEdit:focus { border: 2px solid #3498db; }
        QPushButton {
            background-color: #3498db; color: white; padding: 10px;
            border-radius: 6px; font-weight: bold; font-size: 14px;
        }
        QPushButton#ClearBtn {
            background-color: transparent; color: %2; border: 1px solid %4;
        }
        QScrollArea { border: none; background: transparent; }
    )").arg(bg, text, inputBg, border);

    this->setStyleSheet(style);
}

void MainWindow::onLangToggle() {
    currentLang = (currentLang == "en") ? "ua" : "en";
    langBtn->setText(currentLang == "en" ? "UA" : "EN");
    updateTexts();
}

void MainWindow::updateTexts() {
    bool isEn = (currentLang == "en");

    title->setText(isEn ? "Heart Disease Prediction" : "Прогнозування Хвороб Серця");
    predictButton->setText(isEn ? "Analyze Risk" : "Аналізувати Ризик");
    clearButton->setText(isEn ? "Clear" : "Очистити");
    randomBtn->setToolTip(isEn ? "Fill Random" : "Заповнити Випадково");

    modelInfoLabel->setText(QString("%1: %2 | %3: %4%")
        .arg(isEn ? "Model" : "Модель", QString::fromStdString(modelInfo.model_name))
        .arg(isEn ? "Accuracy" : "Точність", QString::number(modelInfo.accuracy * 100, 'f', 1)));

    if (resultLabel->text().isEmpty() || resultLabel->text().startsWith("Enter") || resultLabel->text().startsWith("Введіть")) {
        resultLabel->setText(isEn ? "Enter patient data to begin" : "Введіть дані пацієнта для початку");
        resultLabel->setStyleSheet("background: #ecf0f1; border-radius: 6px; padding: 15px; color: #7f8c8d;");
    }

    static std::map<std::string, std::pair<QString, QString>> dict = {
        {"age", {"Age (years)", "Вік (років)"}},
        {"sex", {"Sex (0=F, 1=M)", "Стать (0=Ж, 1=Ч)"}},
        {"cp", {"Chest Pain (0-3)", "Біль у грудях (0-3)"}},
        {"trestbps", {"Resting BP (mm Hg)", "Артеріальний тиск"}},
        {"chol", {"Cholesterol (mg/dl)", "Холестерин"}},
        {"fbs", {"Fasting Sugar >120 (0/1)", "Цукор > 120 (0/1)"}},
        {"restecg", {"Resting ECG (0-2)", "ЕКГ спокою (0-2)"}},
        {"thalch", {"Max Heart Rate", "Макс. пульс"}},
        {"exang", {"Exercise Angina (0/1)", "Стенокардія (0/1)"}},
        {"oldpeak", {"ST Depression", "Депресія ST"}},
        {"slope", {"ST Slope (0-2)", "Нахил ST (0-2)"}},
        {"ca", {"Vessels Colored (0-3)", "Судини (0-3)"}},
        {"thal", {"Thalassemia (1-3)", "Таласемія (1-3)"}}
    };

    for (const auto& [key, label] : featureLabels) {
        if (dict.count(key)) {
            label->setText(isEn ? dict[key].first : dict[key].second);
        } else {
            label->setText(QString::fromStdString(key));
        }
    }
}

void MainWindow::onRandomData() {
    static std::mt19937 gen(std::random_device{}());

    auto randInt = [&](int min, int max) {
        return std::uniform_int_distribution<>(min, max)(gen);
    };

    for (auto& [key, input] : inputFields) {
        if (key == "age") input->setText(QString::number(randInt(29, 77)));
        else if (key == "sex") input->setText(QString::number(randInt(0, 1)));
        else if (key == "cp") input->setText(QString::number(randInt(0, 3)));
        else if (key == "trestbps") input->setText(QString::number(randInt(94, 200)));
        else if (key == "chol") input->setText(QString::number(randInt(126, 564)));
        else if (key == "fbs") input->setText(QString::number(randInt(0, 1)));
        else if (key == "restecg") input->setText(QString::number(randInt(0, 2)));
        else if (key == "thalch") input->setText(QString::number(randInt(71, 202)));
        else if (key == "exang") input->setText(QString::number(randInt(0, 1)));
        else if (key == "oldpeak") input->setText(QString::number(randInt(0, 60) / 10.0)); // 0.0 to 6.0
        else if (key == "slope") input->setText(QString::number(randInt(0, 2)));
        else if (key == "ca") input->setText(QString::number(randInt(0, 3)));
        else if (key == "thal") input->setText(QString::number(randInt(1, 3)));
    }
}

std::optional<std::vector<float>> MainWindow::validateAndCollect() {
    std::vector<float> data;
    bool hasError = false;

    for (const auto& feature : modelInfo.features) {
        QLineEdit* field = inputFields[feature];
        QString text = field->text().trimmed();

        if (text.isEmpty()) {
            field->setStyleSheet("border: 2px solid #e74c3c; background: #fdf0ef;");
            hasError = true;
        } else {
            field->setStyleSheet("");
            data.push_back(text.toFloat());
        }
    }
    return hasError ? std::nullopt : std::make_optional(data);
}
void MainWindow::onPredictClicked() {
    auto featuresOpt = validateAndCollect();

    if (!featuresOpt.has_value()) {
        resultLabel->setText(currentLang == "en" ? "Fill all fields!" : "Заповніть всі поля!");
        resultLabel->setStyleSheet("background: #e74c3c; color: white; padding: 15px; border-radius: 6px;");
        return;
    }

    predictButton->setEnabled(false);
    resultLabel->setText(currentLang == "en" ? "Thinking..." : "Аналіз...");

    PredictionResult result = bridge->predict(featuresOpt.value());

    if (!result.success) {
        resultLabel->setText("Error: " + QString::fromStdString(result.error_message));
        resultLabel->setStyleSheet("background: #34495e; color: white; padding: 15px;");
    } else {
        bool highRisk = (result.prediction == 1);
        float prob = (highRisk ? result.probability : (1.0 - result.probability)) * 100.0;

        QString riskStr = highRisk ? (currentLang == "en" ? "HIGH" : "ВИСОКИЙ")
                                   : (currentLang == "en" ? "LOW" : "НИЗЬКИЙ");

        QString probLabel = currentLang == "en" ? "Probability" : "Ймовірність";

        QString text = QString("%1: %2\n%3: %4%")
                .arg(currentLang == "en" ? "RISK" : "РИЗИК")
                .arg(riskStr)
                .arg(probLabel)
                .arg(prob, 0, 'f', 1);

        QString color = highRisk ? "#e74c3c" : "#2ecc71";
        resultLabel->setText(text);
        resultLabel->setStyleSheet(QString("background: %1; color: white; padding: 15px; border-radius: 6px; font-size: 16px; font-weight: bold;").arg(color));
    }

    predictButton->setEnabled(true);
}

void MainWindow::onClearClicked() {
    for (auto& pair : inputFields) {
        pair.second->clear();
        pair.second->setStyleSheet("");
    }
    updateTexts();
}

void MainWindow::onPythonError(const QString& error) {
    qWarning() << "Bridge Error:" << error;
}