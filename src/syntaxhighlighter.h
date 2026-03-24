#pragma once
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QList>

class SyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    explicit SyntaxHighlighter(QTextDocument *parent = nullptr);
    void setLanguage(const QString &language);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct Rule {
        QRegularExpression pattern;
        QTextCharFormat    format;
    };

    void buildRules(const QString &language);
    void applyMultilineComment(const QString &text,
                               const QString &startStr,
                               const QString &endStr,
                               int stateId);

    QList<Rule>       m_rules;
    QTextCharFormat   m_mlCommentFmt;

    // For languages with /* */ block comments
    QRegularExpression m_mlCommentStart;
    QRegularExpression m_mlCommentEnd;
    bool               m_hasBlockComment = false;
};
