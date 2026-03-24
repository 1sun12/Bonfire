#pragma once
#include <QWidget>
#include "../shard.h"

class QLineEdit;
class QComboBox;
class QPlainTextEdit;
class QTextEdit;
class QLabel;
class QStackedWidget;
class QPushButton;
class SyntaxHighlighter;
class Database;

class ShardEditorWidget : public QWidget {
    Q_OBJECT
public:
    explicit ShardEditorWidget(Database *db, QWidget *parent = nullptr);

    void loadShard(const QString &id);  // empty id = new shard
    void refreshLanguages();

signals:
    void shardSaved(const Shard &shard);
    void shardDeleted(const QString &id);
    void backRequested();

private slots:
    void save();
    void deleteShard();
    void markReviewed();
    void onLanguageChanged(const QString &lang);
    void enterEditMode();
    void cancelEdit();

private:
    void showViewMode();
    void showEditMode();
    void populateViewMode();
    void populateEditMode();
    void syncFormToShard();
    QStringList allLanguages() const;

    Database          *m_db;
    Shard              m_shard;
    bool               m_isNew = false;

    // View mode widgets
    QWidget           *m_viewWidget;
    QLabel            *m_vTitle;
    QLabel            *m_vMeta;
    QLabel            *m_vDescription;
    QLabel            *m_vSource;
    QLabel            *m_vTags;
    QPlainTextEdit    *m_vCode;
    SyntaxHighlighter *m_vHighlighter;

    // Edit mode widgets
    QWidget           *m_editWidget;
    QLineEdit         *m_eTitle;
    QComboBox         *m_eLanguage;
    QComboBox         *m_eCategory;
    QComboBox         *m_eFamiliarity;
    QPlainTextEdit    *m_eCode;
    QTextEdit         *m_eDescription;
    QLineEdit         *m_eSource;
    QLineEdit         *m_eTags;
    QPushButton       *m_reviewCheck;
    SyntaxHighlighter *m_eHighlighter;

    QStackedWidget    *m_stack;
    QPushButton       *m_editBtn;
    QPushButton       *m_saveBtn;
    QPushButton       *m_cancelBtn;
    QPushButton       *m_deleteBtn;
    QPushButton       *m_markReviewedBtn;
};
