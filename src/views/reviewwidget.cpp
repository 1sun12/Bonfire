#include "reviewwidget.h"
#include "../database.h"
#include "../syntaxhighlighter.h"
#include "../sm2.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QFrame>
#include <QStackedWidget>
#include <QFont>
#include <QDateTime>

ReviewWidget::ReviewWidget(Database *db, QWidget *parent)
    : QWidget(parent), m_db(db)
{
    m_stack = new QStackedWidget(this);
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(m_stack);

    // ============ CARD PAGE ============
    m_cardPage = new QWidget;
    auto *cardLayout = new QVBoxLayout(m_cardPage);
    cardLayout->setContentsMargins(40, 32, 40, 32);
    cardLayout->setSpacing(16);

    // Progress
    m_progressLbl = new QLabel;
    m_progressLbl->setObjectName("mutedText");
    m_progressLbl->setAlignment(Qt::AlignRight);
    cardLayout->addWidget(m_progressLbl);

    // Card frame
    auto *card = new QFrame;
    card->setObjectName("reviewCard");
    auto *cl = new QVBoxLayout(card);
    cl->setContentsMargins(28, 24, 28, 24);
    cl->setSpacing(12);

    // Language badge + title
    auto *headerRow = new QHBoxLayout;
    m_langBadge = new QLabel;
    m_langBadge->setFixedHeight(20);
    headerRow->addWidget(m_langBadge);
    headerRow->addStretch();
    cl->addLayout(headerRow);

    m_titleLbl = new QLabel;
    QFont tf = m_titleLbl->font();
    tf.setPointSize(18);
    tf.setBold(true);
    m_titleLbl->setFont(tf);
    m_titleLbl->setWordWrap(true);
    cl->addWidget(m_titleLbl);

    m_metaLbl = new QLabel;
    m_metaLbl->setObjectName("mutedText");
    cl->addWidget(m_metaLbl);

    auto *sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("border:none; border-top:1px solid #333;");
    cl->addWidget(sep);

    m_descLbl = new QLabel;
    m_descLbl->setWordWrap(true);
    m_descLbl->setObjectName("descText");
    cl->addWidget(m_descLbl);

    // Hidden code placeholder
    m_hiddenMsg = new QLabel("Code hidden — try to recall, then reveal");
    m_hiddenMsg->setAlignment(Qt::AlignCenter);
    m_hiddenMsg->setObjectName("mutedText");
    m_hiddenMsg->setStyleSheet("font-style:italic; padding:24px;");
    cl->addWidget(m_hiddenMsg);

    // Code view (shown after reveal)
    m_codeView = new QPlainTextEdit;
    m_codeView->setReadOnly(true);
    QFont mono("Monospace");
    mono.setStyleHint(QFont::Monospace);
    mono.setPointSize(10);
    m_codeView->setFont(mono);
    m_codeView->setStyleSheet("background:#0d0d0d; color:#7ec8a0; border:1px solid #333;");
    m_codeView->setMinimumHeight(140);
    m_codeView->hide();
    m_highlighter = new SyntaxHighlighter(m_codeView->document());
    cl->addWidget(m_codeView);

    cardLayout->addWidget(card, 1);

    // Reveal button
    m_revealBtn = new QPushButton("Reveal Code");
    m_revealBtn->setObjectName("primaryBtn");
    m_revealBtn->setFixedHeight(38);
    cardLayout->addWidget(m_revealBtn);

    // Rating row (hidden until revealed)
    m_ratingRow = new QWidget;
    auto *rl = new QHBoxLayout(m_ratingRow);
    rl->setSpacing(10);
    auto *prompt = new QLabel("How did you do?");
    prompt->setObjectName("mutedText");
    rl->addWidget(prompt);
    rl->addStretch();

    const QList<QPair<QString,QString>> ratings = {
        {"Forgot",  "#ef4444"},
        {"Hard",    "#f59e0b"},
        {"Good",    "#3b82f6"},
        {"Easy",    "#10b981"},
    };
    for (const auto &[label, color] : ratings) {
        auto *btn = new QPushButton(label);
        btn->setFixedHeight(34);
        btn->setStyleSheet(QString("QPushButton { background:%1; color:white; border:none; "
                                   "border-radius:4px; padding:0 16px; font-weight:bold; }"
                                   "QPushButton:hover { opacity:0.85; }").arg(color));
        rl->addWidget(btn);
        connect(btn, &QPushButton::clicked, this, [this, label]{ rate(label); });
    }
    m_ratingRow->hide();
    cardLayout->addWidget(m_ratingRow);

    connect(m_revealBtn, &QPushButton::clicked, this, &ReviewWidget::reveal);

    m_stack->addWidget(m_cardPage);

    // ============ SUMMARY PAGE ============
    m_summaryPage = new QWidget;
    auto *sl = new QVBoxLayout(m_summaryPage);
    sl->setContentsMargins(40, 60, 40, 40);
    sl->setAlignment(Qt::AlignTop);

    auto *summaryTitle = new QLabel("Session Complete");
    QFont stf = summaryTitle->font();
    stf.setPointSize(16);
    stf.setBold(true);
    summaryTitle->setFont(stf);
    sl->addWidget(summaryTitle);

    sl->addSpacing(12);
    m_summaryLbl = new QLabel;
    m_summaryLbl->setObjectName("descText");
    sl->addWidget(m_summaryLbl);

    sl->addSpacing(24);
    auto *doneBtn = new QPushButton("Back to Dashboard");
    doneBtn->setObjectName("primaryBtn");
    doneBtn->setFixedWidth(200);
    sl->addWidget(doneBtn);

    connect(doneBtn, &QPushButton::clicked, this, &ReviewWidget::sessionComplete);

    m_stack->addWidget(m_summaryPage);
}

void ReviewWidget::startSession(const QList<Shard> &dueShards) {
    m_queue   = dueShards;
    m_current = 0;
    m_forgot  = 0;
    m_results.clear();
    showCard();
}

void ReviewWidget::showCard() {
    if (m_current >= m_queue.size()) {
        showSummary();
        return;
    }

    const Shard &s = m_queue[m_current];

    m_progressLbl->setText(QString("%1 / %2").arg(m_current + 1).arg(m_queue.size()));

    // Language badge
    QString lang = s.language.isEmpty() ? "?" : s.language;
    m_langBadge->setText(lang);
    m_langBadge->setStyleSheet(QString(
        "background:%1; color:white; border-radius:3px; "
        "padding:1px 8px; font-family:monospace; font-size:10px;"
    ).arg(langColor(s.language)));

    m_titleLbl->setText(s.title);

    QStringList meta;
    meta << s.category;
    if (!s.familiarity.isEmpty()) meta << s.familiarity;
    m_metaLbl->setText(meta.join("  ·  "));

    m_descLbl->setText(s.description.isEmpty() ? "(no description)" : s.description);
    m_descLbl->setVisible(!s.description.isEmpty());

    // Reset reveal state
    m_codeView->setPlainText(s.code);
    m_highlighter->setLanguage(s.language);
    m_codeView->hide();
    m_hiddenMsg->show();
    m_revealBtn->show();
    m_ratingRow->hide();

    m_stack->setCurrentWidget(m_cardPage);
}

void ReviewWidget::reveal() {
    m_hiddenMsg->hide();
    m_codeView->show();
    m_revealBtn->hide();
    m_ratingRow->show();
}

void ReviewWidget::rate(const QString &rating) {
    const Shard &s = m_queue[m_current];
    int q = qualityFromRating(rating);

    SM2Result res = sm2(q, s.reviewInterval, s.reviewRepetitions, s.reviewEase);

    Shard updated = s;
    updated.reviewInterval    = res.interval;
    updated.reviewRepetitions = res.repetitions;
    updated.reviewEase        = res.easeFactor;
    updated.reviewNext        = res.nextReview;
    updated.lastReviewed      = QDateTime::currentDateTime().toString(Qt::ISODate);
    m_db->saveShard(updated);

    m_results << rating;
    if (rating == "Forgot") m_forgot++;
    m_current++;
    showCard();
}

void ReviewWidget::showSummary() {
    int total  = m_results.size();
    int forgot = m_forgot;
    int good   = total - forgot;

    m_summaryLbl->setText(
        QString("Reviewed: %1 shard%2\n"
                "Remembered: %3\n"
                "Forgot: %4")
        .arg(total).arg(total != 1 ? "s" : "")
        .arg(good).arg(forgot)
    );

    m_stack->setCurrentWidget(m_summaryPage);
}
