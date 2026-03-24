#pragma once
#include <QWidget>
#include "../shard.h"

class QLabel;
class QPlainTextEdit;
class QPushButton;
class QStackedWidget;
class SyntaxHighlighter;
class Database;

class ReviewWidget : public QWidget {
    Q_OBJECT
public:
    explicit ReviewWidget(Database *db, QWidget *parent = nullptr);

    void startSession(const QList<Shard> &dueShards);

signals:
    void sessionComplete();

private slots:
    void reveal();
    void rate(const QString &rating);

private:
    void showCard();
    void showSummary();

    Database          *m_db;
    QList<Shard>       m_queue;
    int                m_current = 0;
    int                m_forgot  = 0;
    QList<QString>     m_results;

    QStackedWidget    *m_stack;

    // Card page
    QWidget           *m_cardPage;
    QLabel            *m_progressLbl;
    QLabel            *m_langBadge;
    QLabel            *m_titleLbl;
    QLabel            *m_metaLbl;
    QLabel            *m_descLbl;
    QLabel            *m_hiddenMsg;
    QPlainTextEdit    *m_codeView;
    SyntaxHighlighter *m_highlighter;
    QPushButton       *m_revealBtn;
    QWidget           *m_ratingRow;

    // Summary page
    QWidget           *m_summaryPage;
    QLabel            *m_summaryLbl;
};
