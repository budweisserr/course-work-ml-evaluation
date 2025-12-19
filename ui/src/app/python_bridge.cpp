#include "../../include/app/python_bridge.h"

PythonBridge::PythonBridge(QObject* parent)
    : QObject(parent),
      process(nullptr),
      initialized(false)
{
}

PythonBridge::~PythonBridge()
{
    shutdown();
}

bool PythonBridge::initialize(const QString& python_script)
{
    process = new QProcess(this);

    process->start("python3", QStringList() << python_script);

    if (!process->waitForStarted(5000)) {
        emit errorOccurred("failed to start python process");
        return false;
    }

    initialized = true;
    qDebug() << "python bridge initialized";
    return true;
}

QString PythonBridge::sendCommand(const QString& command)
{
    if (!initialized || !process) {
        return {};
    }

    QByteArray data = command.toUtf8();
    data.append('\n');
    process->write(data);
    process->waitForBytesWritten(1000);

    if (!process->waitForReadyRead(5000)) {
        emit errorOccurred("timeout waiting for python response");
        return {};
    }

    QString response = QString::fromUtf8(process->readLine()).trimmed();
    return response;
}

nmjson PythonBridge::parseResponse(const QString& response)
{
    QString trimmed = response.trimmed();
    if (trimmed.isEmpty()) {
        return nmjson{};
    }

    try {
        return nmjson::parse(trimmed.toStdString());
    } catch (const nmjson::parse_error& e) {
        qWarning() << "failed to parse json response:" << e.what();
        return nmjson{};
    }
}

ModelInfo PythonBridge::getModelInfo()
{
    ModelInfo info{};

    QString response = sendCommand("INFO");
    nmjson json = parseResponse(response);

    if (!json.is_object()) {
        emit errorOccurred("invalid json response for model info");
        return info;
    }

    std::string status = json.value("status", std::string{});
    if (status != "success") {
        emit errorOccurred("failed to get model info");
        return info;
    }

    if (!json.contains("data") || !json["data"].is_object()) {
        emit errorOccurred("model info response missing data field");
        return info;
    }

    const nmjson& data = json["data"];

    info.model_name = data.value("model_name", std::string{});
    info.num_features = data.value("num_features", 0);

    if (data.contains("features") && data["features"].is_array()) {
        for (const auto& f : data["features"]) {
            info.features.push_back(f.get<std::string>());
        }
    }

    if (data.contains("metrics") && data["metrics"].is_object()) {
        const nmjson& metrics = data["metrics"];
        info.accuracy  = metrics.value("accuracy",  0.0);
        info.precision = metrics.value("precision", 0.0);
        info.recall    = metrics.value("recall",    0.0);
        info.f1_score  = metrics.value("f1_score",  0.0);
    }

    return info;
}

PredictionResult PythonBridge::predict(const std::vector<float>& features)
{
    PredictionResult result{};
    result.success = false;

    nmjson request;
    request["features"] = features;

    QString command = QString::fromStdString(request.dump());
    QString response = sendCommand(command);
    nmjson json = parseResponse(response);

    if (!json.is_object()) {
        result.error_message = "invalid json response from python";
        emit errorOccurred(QString::fromStdString(result.error_message));
        return result;
    }

    std::string status = json.value("status", std::string{});
    if (status != "success") {
        result.error_message = json.value("message", std::string{"unknown error"});
        emit errorOccurred(QString::fromStdString(result.error_message));
        return result;
    }

    result.success = true;
    result.prediction  = json.value("prediction", 0);
    result.probability = json.value("probability", 0.0);

    return result;
}

void PythonBridge::shutdown()
{
    if (process) {
        process->write("EXIT\n");
        process->waitForBytesWritten(1000);
        process->waitForFinished(3000);

        if (process->state() == QProcess::Running) {
            process->kill();
        }

        delete process;
        process = nullptr;
    }
    initialized = false;
}
