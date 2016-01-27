#ifndef SHELL_LOCALFILESYSTEMREPLY_H
#define SHELL_LOCALFILESYSTEMREPLY_H

#include <QNetworkReply>

class LocalFileSystemReply : public QNetworkReply {
    Q_OBJECT
public:
    explicit LocalFileSystemReply(QNetworkAccessManager::Operation op,
                                  const QNetworkRequest &req,
                                  QObject* parent = nullptr,
                                  QIODevice *outgoingData = nullptr);
    ~LocalFileSystemReply();


    void close() override;
    bool isSequential() const override;
    qint64 bytesAvailable() const override;

public Q_SLOTS:
    void abort() override;
    void ignoreSslErrors() override;

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;
    void sslConfigurationImplementation(QSslConfiguration &) const override;
    void setSslConfigurationImplementation(const QSslConfiguration &) override;
    void ignoreSslErrorsImplementation(const QList<QSslError> &) override;

private:
    QByteArray content;
    qint64 offset = 0;

    QTimer* timer = nullptr;
};


#endif //SHELL_LOCALFILESYSTEMREPLY_H
