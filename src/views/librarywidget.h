#pragma once
#include <QWidget>
#include "../shard.h"

class QLineEdit;
class QComboBox;
class QListWidget;
class QLabel;
class Database;

class LibraryWidget : public QWidget {
    Q_OBJECT
public:
    explicit LibraryWidget(Database *db, QWidget *parent = nullptr);

    void refresh();

signals:
    void openShard(const QString &id);
    void requestNewShard();

private slots:
    void applyFilter();
    void onItemDoubleClicked();

private:
    void populateList(const QList<Shard> &shards);
    QList<Shard> filteredSorted() const;

    Database    *m_db;
    QList<Shard> m_allShards;

    QLineEdit   *m_search;
    QComboBox   *m_filterLang;
    QComboBox   *m_filterCat;
    QComboBox   *m_filterFam;
    QComboBox   *m_filterTag;
    QComboBox   *m_sort;
    QListWidget *m_list;
    QLabel      *m_countLbl;
};
