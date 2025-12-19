#include "python_bridge.h"
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>

PythonBridge::PythonBridge(QObject* parent)
    : QObject(parent), process(nullptr), initialized(false) {}

PythonBridge::~PythonBridge() {
    shutdown();
}

bool PythonBridge::initialize(const QString& pythonScript) {
    process = new QProcess(this);

    process->setProcessChannelMode(QProcess::MergedChannels);
    auto env = EnvLoader::load();

    QString pythonPath = QString::fromStdString(env["PYTHON_INTERPRETER_PATH"]);

    if (!QFile::exists(pythonPath)) {
        qWarning() << "Venv not found at:" << pythonPath << "using system python3";
        pythonPath = "python3";
    }

    process->start(pythonPath, QStringList() << pythonScript);

    if (!process->waitForStarted(5000)) {
        emit errorOccurred("Failed to start Python process: " + process->errorString());
        return false;
    }

    initialized = true;
    qDebug() << "Python bridge initialized using:" << pythonPath;
    return true;
}

QString PythonBridge::sendCommand(const QString& command) {
    if (!initialized || !process) return {};

    process->write(command.toUtf8() + "\n");

    if (!process->waitForReadyRead(5000)) {
        QByteArray crashLog = process->readAll();
        if (!crashLog.isEmpty()) {
            qDebug() << "Python Crash Log:" << crashLog;
        }
        emit errorOccurred("Timeout waiting for response");
        return {};
    }

    return QString::fromUtf8(process->readAll()).trimmed();
}

nmjson PythonBridge::parseResponse(const QString& response) {
    if (response.isEmpty()) return nmjson{};

    try {
        return nmjson::parse(response.toStdString());
    } catch (const nmjson::parse_error& e) {
        qWarning() << "JSON Parse Error. Raw Output:" << response;
        return nmjson{};
    }
}

ModelInfo PythonBridge::getModelInfo() {
    ModelInfo info{};
    QString response = sendCommand("INFO");
    nmjson json = parseResponse(response);

    if (!json.is_object() || json.value("status", "") != "success") {
        emit errorOccurred("Failed to retrieve model info");
        return info;
    }

    auto data = json["data"];
    info.model_name = data.value("model_name", "Unknown");
    info.num_features = data.value("num_features", 0);
    info.accuracy = data.value("metrics", nmjson::object()).value("accuracy", 0.0);

    if (data.contains("features")) {
        for (const auto& f : data["features"])
            info.features.push_back(f.get<std::string>());
    }

    return info;
}

PredictionResult PythonBridge::predict(const std::vector<float>& features) {
    PredictionResult result{};

    nmjson request;
    request["features"] = features;

    QString response = sendCommand(QString::fromStdString(request.dump()));
    nmjson json = parseResponse(response);

    if (!json.is_object()) {
        result.success = false;
        result.error_message = "Invalid response from Python (check logs)";
        return result;
    }

    if (json.value("status", "") == "success") {
        result.success = true;
        result.prediction = json.value("prediction", 0);
        result.probability = json.value("probability", 0.0);
    } else {
        result.success = false;
        result.error_message = json.value("message", "Unknown error");
        emit errorOccurred(QString::fromStdString(result.error_message));
    }

    return result;
}

void PythonBridge::shutdown() {
    if (process) {
        process->write("EXIT\n");
        process->waitForFinished(1000);
        process->kill();
        delete process;
        process = nullptr;
    }
}