#include "database.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDateTime>
#include <QUuid>
#include <QDebug>

static QString generateId() {
    return QString::number(QDateTime::currentMSecsSinceEpoch(), 36)
           + QUuid::createUuid().toString(QUuid::WithoutBraces).left(6);
}

Database::Database(QObject *parent)
    : QObject(parent)
    , m_connectionName(generateId())
{}

Database::~Database() {
    if (QSqlDatabase::contains(m_connectionName)) {
        QSqlDatabase::removeDatabase(m_connectionName);
    }
}

bool Database::init(const QString &path) {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    db.setDatabaseName(path);
    if (!db.open()) {
        m_lastError = db.lastError().text();
        return false;
    }
    return createTables();
}

bool Database::createTables() {
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery q(db);

    const QString shardTable = R"(
        CREATE TABLE IF NOT EXISTS shards (
            id               TEXT PRIMARY KEY,
            title            TEXT NOT NULL,
            language         TEXT DEFAULT '',
            code             TEXT DEFAULT '',
            description      TEXT DEFAULT '',
            tags             TEXT DEFAULT '[]',
            category         TEXT DEFAULT 'snippet',
            familiarity      TEXT DEFAULT 'fresh',
            source           TEXT DEFAULT '',
            related_ids      TEXT DEFAULT '[]',
            created_at       TEXT NOT NULL,
            modified_at      TEXT NOT NULL,
            last_reviewed    TEXT DEFAULT '',
            review_enabled   INTEGER DEFAULT 0,
            review_interval  INTEGER DEFAULT 0,
            review_reps      INTEGER DEFAULT 0,
            review_ease      REAL    DEFAULT 2.5,
            review_next      TEXT    DEFAULT ''
        )
    )";

    const QString langTable = R"(
        CREATE TABLE IF NOT EXISTS custom_languages (
            name TEXT PRIMARY KEY
        )
    )";

    if (!q.exec(shardTable)) {
        m_lastError = q.lastError().text();
        return false;
    }
    if (!q.exec(langTable)) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

Shard Database::shardFromQuery(QSqlQuery &q) const {
    Shard s;
    s.id               = q.value("id").toString();
    s.title            = q.value("title").toString();
    s.language         = q.value("language").toString();
    s.code             = q.value("code").toString();
    s.description      = q.value("description").toString();
    s.category         = q.value("category").toString();
    s.familiarity      = q.value("familiarity").toString();
    s.source           = q.value("source").toString();
    s.createdAt        = q.value("created_at").toString();
    s.modifiedAt       = q.value("modified_at").toString();
    s.lastReviewed     = q.value("last_reviewed").toString();
    s.reviewEnabled    = q.value("review_enabled").toInt() != 0;
    s.reviewInterval   = q.value("review_interval").toInt();
    s.reviewRepetitions= q.value("review_reps").toInt();
    s.reviewEase       = q.value("review_ease").toDouble();
    s.reviewNext       = q.value("review_next").toString();

    auto parseList = [](const QString &json) -> QStringList {
        QStringList result;
        QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
        if (doc.isArray()) {
            for (const QJsonValue &v : doc.array())
                if (v.isString()) result << v.toString();
        }
        return result;
    };
    s.tags       = parseList(q.value("tags").toString());
    s.relatedIds = parseList(q.value("related_ids").toString());
    return s;
}

bool Database::shardExists(const QString &id) const {
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery q(db);
    q.prepare("SELECT 1 FROM shards WHERE id = ?");
    q.addBindValue(id);
    q.exec();
    return q.next();
}

QList<Shard> Database::allShards() const {
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery q(db);
    q.exec("SELECT * FROM shards ORDER BY modified_at DESC");
    QList<Shard> result;
    while (q.next())
        result << shardFromQuery(q);
    return result;
}

bool Database::saveShard(const Shard &shard) {
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery q(db);

    auto toJson = [](const QStringList &list) -> QString {
        QJsonArray arr;
        for (const QString &s : list) arr.append(s);
        return QJsonDocument(arr).toJson(QJsonDocument::Compact);
    };

    QString now = QDateTime::currentDateTime().toString(Qt::ISODate);

    if (shardExists(shard.id)) {
        q.prepare(R"(
            UPDATE shards SET
                title=?, language=?, code=?, description=?, tags=?,
                category=?, familiarity=?, source=?, related_ids=?,
                modified_at=?, last_reviewed=?, review_enabled=?,
                review_interval=?, review_reps=?, review_ease=?, review_next=?
            WHERE id=?
        )");
        q.addBindValue(shard.title);
        q.addBindValue(shard.language);
        q.addBindValue(shard.code);
        q.addBindValue(shard.description);
        q.addBindValue(toJson(shard.tags));
        q.addBindValue(shard.category);
        q.addBindValue(shard.familiarity);
        q.addBindValue(shard.source);
        q.addBindValue(toJson(shard.relatedIds));
        q.addBindValue(now);
        q.addBindValue(shard.lastReviewed);
        q.addBindValue(shard.reviewEnabled ? 1 : 0);
        q.addBindValue(shard.reviewInterval);
        q.addBindValue(shard.reviewRepetitions);
        q.addBindValue(shard.reviewEase);
        q.addBindValue(shard.reviewNext);
        q.addBindValue(shard.id);
    } else {
        q.prepare(R"(
            INSERT INTO shards
                (id, title, language, code, description, tags, category, familiarity,
                 source, related_ids, created_at, modified_at, last_reviewed,
                 review_enabled, review_interval, review_reps, review_ease, review_next)
            VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)
        )");
        q.addBindValue(shard.id);
        q.addBindValue(shard.title);
        q.addBindValue(shard.language);
        q.addBindValue(shard.code);
        q.addBindValue(shard.description);
        q.addBindValue(toJson(shard.tags));
        q.addBindValue(shard.category);
        q.addBindValue(shard.familiarity);
        q.addBindValue(shard.source);
        q.addBindValue(toJson(shard.relatedIds));
        q.addBindValue(shard.createdAt.isEmpty() ? now : shard.createdAt);
        q.addBindValue(now);
        q.addBindValue(shard.lastReviewed);
        q.addBindValue(shard.reviewEnabled ? 1 : 0);
        q.addBindValue(shard.reviewInterval);
        q.addBindValue(shard.reviewRepetitions);
        q.addBindValue(shard.reviewEase);
        q.addBindValue(shard.reviewNext);
    }

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

bool Database::deleteShard(const QString &id) {
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery q(db);
    q.prepare("DELETE FROM shards WHERE id = ?");
    q.addBindValue(id);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

QStringList Database::customLanguages() const {
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery q(db);
    q.exec("SELECT name FROM custom_languages ORDER BY name");
    QStringList result;
    while (q.next())
        result << q.value(0).toString();
    return result;
}

bool Database::addCustomLanguage(const QString &lang) {
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery q(db);
    q.prepare("INSERT OR IGNORE INTO custom_languages (name) VALUES (?)");
    q.addBindValue(lang);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

bool Database::removeCustomLanguage(const QString &lang) {
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery q(db);
    q.prepare("DELETE FROM custom_languages WHERE name = ?");
    q.addBindValue(lang);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

bool Database::exportToJson(const QString &filePath) const {
    QList<Shard> shards = allShards();
    QJsonArray arr;
    for (const Shard &s : shards) {
        QJsonObject obj;
        obj["id"]               = s.id;
        obj["title"]            = s.title;
        obj["language"]         = s.language;
        obj["code"]             = s.code;
        obj["description"]      = s.description;
        obj["category"]         = s.category;
        obj["familiarity"]      = s.familiarity;
        obj["source"]           = s.source;
        obj["createdAt"]        = s.createdAt;
        obj["modifiedAt"]       = s.modifiedAt;
        obj["lastReviewed"]     = s.lastReviewed;
        obj["reviewEnabled"]    = s.reviewEnabled;
        obj["reviewInterval"]   = s.reviewInterval;
        obj["reviewRepetitions"]= s.reviewRepetitions;
        obj["reviewEase"]       = s.reviewEase;
        obj["reviewNext"]       = s.reviewNext;

        QJsonArray tags;
        for (const QString &t : s.tags) tags.append(t);
        obj["tags"] = tags;

        QJsonArray related;
        for (const QString &r : s.relatedIds) related.append(r);
        obj["relatedIds"] = related;

        arr.append(obj);
    }

    QJsonObject root;
    root["shards"]       = arr;
    root["customLangs"]  = QJsonArray();
    root["exportedAt"]   = QDateTime::currentDateTime().toString(Qt::ISODate);

    // Include custom languages
    QJsonArray cl;
    for (const QString &l : customLanguages()) cl.append(l);
    root["customLangs"] = cl;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        m_lastError = file.errorString();
        return false;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

bool Database::importFromJson(const QString &filePath, int &imported, int &skipped) {
    imported = skipped = 0;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        m_lastError = file.errorString();
        return false;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        m_lastError = "Invalid JSON format";
        return false;
    }
    QJsonObject root = doc.object();
    QJsonArray shards = root["shards"].toArray();

    for (const QJsonValue &v : shards) {
        QJsonObject obj = v.toObject();
        QString id = obj["id"].toString();
        if (id.isEmpty()) { skipped++; continue; }

        if (shardExists(id)) { skipped++; continue; }

        Shard s;
        s.id               = id;
        s.title            = obj["title"].toString();
        s.language         = obj["language"].toString();
        s.code             = obj["code"].toString();
        s.description      = obj["description"].toString();
        s.category         = obj["category"].toString("snippet");
        s.familiarity      = obj["familiarity"].toString("fresh");
        s.source           = obj["source"].toString();
        s.createdAt        = obj["createdAt"].toString();
        s.modifiedAt       = obj["modifiedAt"].toString();
        s.lastReviewed     = obj["lastReviewed"].toString();
        s.reviewEnabled    = obj["reviewEnabled"].toBool();
        s.reviewInterval   = obj["reviewInterval"].toInt();
        s.reviewRepetitions= obj["reviewRepetitions"].toInt();
        s.reviewEase       = obj["reviewEase"].toDouble(2.5);
        s.reviewNext       = obj["reviewNext"].toString();

        for (const QJsonValue &t : obj["tags"].toArray())
            s.tags << t.toString();
        for (const QJsonValue &r : obj["relatedIds"].toArray())
            s.relatedIds << r.toString();

        if (saveShard(s)) imported++;
        else skipped++;
    }

    // Import custom languages
    for (const QJsonValue &l : root["customLangs"].toArray())
        addCustomLanguage(l.toString());

    return true;
}
