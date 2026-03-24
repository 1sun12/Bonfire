#pragma once
#include <QWidget>
#include "../shard.h"

class QLabel;
class Database;

class DashboardWidget : public QWidget {
    Q_OBJECT
public:
    explicit DashboardWidget(Database *db, QWidget *parent = nullptr);

    void refresh();

signals:
    void openShard(const QString &id);
    void startReview();
    void requestNewShard();
    void requestExport();
    void requestImport();

private:
    Database *m_db;
    QLabel   *m_totalLbl;
    QLabel   *m_dueLbl;
    QLabel   *m_langsLbl;
    QLabel   *m_shakyLbl;
    QWidget  *m_langBreakdown;
    QWidget  *m_dueList;
    QWidget  *m_recentList;

    void buildDueList(const QList<Shard> &due);
    void buildRecentList(const QList<Shard> &shards);
    void buildLangBreakdown(const QList<Shard> &shards);
};
