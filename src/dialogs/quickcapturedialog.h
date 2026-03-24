#pragma once
#include <QDialog>
#include "../shard.h"

class QLineEdit;
class QComboBox;
class QPlainTextEdit;
class Database;

class QuickCaptureDialog : public QDialog {
    Q_OBJECT
public:
    explicit QuickCaptureDialog(Database *db, const QStringList &allLanguages,
                                QWidget *parent = nullptr);

signals:
    void shardSaved(const Shard &shard);

private slots:
    void save();

private:
    Database       *m_db;
    QLineEdit      *m_title;
    QComboBox      *m_language;
    QPlainTextEdit *m_code;
    QLineEdit      *m_tag;
};
