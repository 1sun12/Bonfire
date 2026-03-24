#pragma once
#include <QObject>
#include <QList>
#include <QString>
#include "shard.h"

class QSqlQuery;

class Database : public QObject {
    Q_OBJECT
public:
    explicit Database(QObject *parent = nullptr);
    ~Database() override;

    bool init(const QString &path);

    QList<Shard> allShards() const;
    bool saveShard(const Shard &shard);
    bool deleteShard(const QString &id);

    QStringList customLanguages() const;
    bool addCustomLanguage(const QString &lang);
    bool removeCustomLanguage(const QString &lang);

    bool exportToJson(const QString &filePath) const;
    bool importFromJson(const QString &filePath, int &imported, int &skipped);

    QString lastError() const { return m_lastError; }

private:
    bool createTables();
    Shard shardFromQuery(QSqlQuery &q) const;
    bool shardExists(const QString &id) const;

    QString m_connectionName;
    mutable QString m_lastError;
};
