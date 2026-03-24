#include "dashboardwidget.h"
#include "../database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QDate>
#include <QMap>
#include <algorithm>

// Helper: small colored badge label
static QLabel *badge(const QString &text, const QString &color) {
    auto *lbl = new QLabel(text);
    lbl->setStyleSheet(QString("background:%1; color:white; border-radius:3px; "
                               "padding:1px 5px; font-size:10px; font-family:monospace;").arg(color));
    lbl->setFixedHeight(18);
    return lbl;
}

// Helper: stat card
static QFrame *statCard(const QString &title, QLabel **valueLbl) {
    auto *card = new QFrame;
    card->setObjectName("statCard");
    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(12, 10, 12, 10);
    layout->setSpacing(2);
    auto *val = new QLabel("0");
    QFont f = val->font();
    f.setPointSize(22);
    f.setBold(true);
    val->setFont(f);
    layout->addWidget(val);
    auto *lbl = new QLabel(title);
    lbl->setObjectName("statLabel");
    layout->addWidget(lbl);
    if (valueLbl) *valueLbl = val;
    return card;
}

DashboardWidget::DashboardWidget(Database *db, QWidget *parent)
    : QWidget(parent), m_db(db)
{
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto *content = new QWidget;
    auto *root    = new QVBoxLayout(content);
    root->setContentsMargins(20, 20, 20, 20);
    root->setSpacing(16);

    // --- Header ---
    auto *header = new QHBoxLayout;
    auto *titleLbl = new QLabel("Bonfire");
    QFont tf = titleLbl->font();
    tf.setPointSize(14);
    tf.setBold(true);
    titleLbl->setFont(tf);
    header->addWidget(titleLbl);
    header->addStretch();

    auto *exportBtn = new QPushButton("Export");
    auto *importBtn = new QPushButton("Import");
    exportBtn->setObjectName("toolBtn");
    importBtn->setObjectName("toolBtn");
    header->addWidget(exportBtn);
    header->addWidget(importBtn);
    root->addLayout(header);

    connect(exportBtn, &QPushButton::clicked, this, &DashboardWidget::requestExport);
    connect(importBtn, &QPushButton::clicked, this, &DashboardWidget::requestImport);

    // --- Stats row ---
    auto *statsRow = new QHBoxLayout;
    statsRow->setSpacing(10);

    QLabel *dueLblStat;
    statsRow->addWidget(statCard("Total Shards",    &m_totalLbl));
    statsRow->addWidget(statCard("Due for Review",  &dueLblStat));
    statsRow->addWidget(statCard("Languages",       &m_langsLbl));
    statsRow->addWidget(statCard("Shaky",           &m_shakyLbl));
    m_dueLbl = dueLblStat;
    m_dueLbl->setStyleSheet("color:#f59e0b;");
    m_shakyLbl->setStyleSheet("color:#ef4444;");

    root->addLayout(statsRow);

    // --- Language breakdown ---
    auto *langSection = new QFrame;
    langSection->setObjectName("panel");
    auto *langLayout = new QVBoxLayout(langSection);
    langLayout->setContentsMargins(12, 10, 12, 10);
    langLayout->setSpacing(6);
    auto *langTitle = new QLabel("LANGUAGES");
    langTitle->setObjectName("sectionTitle");
    langLayout->addWidget(langTitle);
    m_langBreakdown = new QWidget;
    m_langBreakdown->setLayout(new QHBoxLayout);
    static_cast<QHBoxLayout*>(m_langBreakdown->layout())->setSpacing(8);
    static_cast<QHBoxLayout*>(m_langBreakdown->layout())->setContentsMargins(0,0,0,0);
    langLayout->addWidget(m_langBreakdown);
    root->addWidget(langSection);

    // --- Due for Review ---
    auto *dueSection = new QFrame;
    dueSection->setObjectName("panel");
    auto *dueLayout = new QVBoxLayout(dueSection);
    dueLayout->setContentsMargins(12, 10, 12, 10);
    dueLayout->setSpacing(6);

    auto *dueHeader = new QHBoxLayout;
    auto *dueTitleLbl = new QLabel("DUE FOR REVIEW");
    dueTitleLbl->setObjectName("sectionTitle");
    dueHeader->addWidget(dueTitleLbl);
    dueHeader->addStretch();
    auto *startBtn = new QPushButton("Start Review");
    startBtn->setObjectName("primaryBtn");
    dueHeader->addWidget(startBtn);
    dueLayout->addLayout(dueHeader);
    connect(startBtn, &QPushButton::clicked, this, &DashboardWidget::startReview);

    m_dueList = new QWidget;
    m_dueList->setLayout(new QVBoxLayout);
    m_dueList->layout()->setContentsMargins(0,0,0,0);
    m_dueList->layout()->setSpacing(2);
    dueLayout->addWidget(m_dueList);
    root->addWidget(dueSection);

    // --- Recently Added ---
    auto *recentSection = new QFrame;
    recentSection->setObjectName("panel");
    auto *recentLayout = new QVBoxLayout(recentSection);
    recentLayout->setContentsMargins(12, 10, 12, 10);
    recentLayout->setSpacing(6);
    auto *recentTitle = new QLabel("RECENTLY ADDED");
    recentTitle->setObjectName("sectionTitle");
    recentLayout->addWidget(recentTitle);
    m_recentList = new QWidget;
    m_recentList->setLayout(new QVBoxLayout);
    m_recentList->layout()->setContentsMargins(0,0,0,0);
    m_recentList->layout()->setSpacing(2);
    recentLayout->addWidget(m_recentList);
    root->addWidget(recentSection);

    root->addStretch();
    scroll->setWidget(content);

    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0,0,0,0);
    outerLayout->addWidget(scroll);
}

void DashboardWidget::refresh() {
    QList<Shard> shards = m_db->allShards();
    QString today = QDate::currentDate().toString("yyyy-MM-dd");

    QList<Shard> due;
    int shaky = 0;
    QMap<QString,int> langCount;

    for (const Shard &s : shards) {
        if (s.reviewEnabled && !s.reviewNext.isEmpty() && s.reviewNext <= today)
            due << s;
        if (s.familiarity == "shaky") shaky++;
        if (!s.language.isEmpty()) langCount[s.language]++;
    }

    m_totalLbl->setText(QString::number(shards.size()));
    m_dueLbl->setText(QString::number(due.size()));
    m_langsLbl->setText(QString::number(langCount.size()));
    m_shakyLbl->setText(QString::number(shaky));

    buildLangBreakdown(shards);
    buildDueList(due);
    buildRecentList(shards);
}

void DashboardWidget::buildLangBreakdown(const QList<Shard> &shards) {
    // Clear
    auto *layout = static_cast<QHBoxLayout*>(m_langBreakdown->layout());
    while (QLayoutItem *item = layout->takeAt(0)) {
        delete item->widget();
        delete item;
    }

    QMap<QString,int> counts;
    for (const Shard &s : shards)
        if (!s.language.isEmpty()) counts[s.language]++;

    if (counts.isEmpty()) {
        layout->addWidget(new QLabel("No shards yet."));
        layout->addStretch();
        return;
    }

    // Sort by count desc
    QList<QPair<QString,int>> sorted;
    for (auto it = counts.begin(); it != counts.end(); ++it)
        sorted.append({it.key(), it.value()});
    std::sort(sorted.begin(), sorted.end(), [](const auto &a, const auto &b){
        return a.second > b.second;
    });

    for (const auto &[lang, count] : sorted) {
        auto *row = new QWidget;
        auto *rl  = new QHBoxLayout(row);
        rl->setContentsMargins(0,0,0,0);
        rl->setSpacing(4);
        rl->addWidget(badge(lang, langColor(lang)));
        auto *cnt = new QLabel(QString::number(count));
        cnt->setObjectName("mutedText");
        rl->addWidget(cnt);
        layout->addWidget(row);
    }
    layout->addStretch();
}

void DashboardWidget::buildDueList(const QList<Shard> &due) {
    auto *layout = static_cast<QVBoxLayout*>(m_dueList->layout());
    while (QLayoutItem *item = layout->takeAt(0)) {
        delete item->widget();
        delete item;
    }

    if (due.isEmpty()) {
        auto *lbl = new QLabel("No reviews due today.");
        lbl->setObjectName("mutedText");
        layout->addWidget(lbl);
        return;
    }

    int shown = 0;
    for (const Shard &s : due) {
        if (shown++ >= 5) break;
        auto *row = new QPushButton;
        row->setObjectName("listRow");
        row->setFlat(true);
        auto *rl = new QHBoxLayout(row);
        rl->setContentsMargins(4, 4, 4, 4);
        rl->setSpacing(8);
        rl->addWidget(badge(s.language.isEmpty() ? "?" : s.language, langColor(s.language)));
        auto *title = new QLabel(s.title);
        title->setObjectName("rowTitle");
        rl->addWidget(title, 1);
        rl->addWidget(badge(s.familiarity, famColor(s.familiarity)));
        layout->addWidget(row);
        QString id = s.id;
        connect(row, &QPushButton::clicked, this, [this, id]{ emit openShard(id); });
    }

    if (due.size() > 5) {
        auto *more = new QLabel(QString("+%1 more").arg(due.size() - 5));
        more->setObjectName("mutedText");
        layout->addWidget(more);
    }
}

void DashboardWidget::buildRecentList(const QList<Shard> &shards) {
    auto *layout = static_cast<QVBoxLayout*>(m_recentList->layout());
    while (QLayoutItem *item = layout->takeAt(0)) {
        delete item->widget();
        delete item;
    }

    if (shards.isEmpty()) {
        auto *lbl = new QLabel("No shards yet. Press Ctrl+N to create one.");
        lbl->setObjectName("mutedText");
        layout->addWidget(lbl);
        return;
    }

    // shards already ordered by modified_at desc from DB
    int shown = 0;
    for (const Shard &s : shards) {
        if (shown++ >= 10) break;
        auto *row = new QPushButton;
        row->setObjectName("listRow");
        row->setFlat(true);
        auto *rl = new QHBoxLayout(row);
        rl->setContentsMargins(4, 4, 4, 4);
        rl->setSpacing(8);
        rl->addWidget(badge(s.language.isEmpty() ? "?" : s.language, langColor(s.language)));
        auto *title = new QLabel(s.title);
        title->setObjectName("rowTitle");
        rl->addWidget(title, 1);
        auto *cat = new QLabel(s.category);
        cat->setObjectName("mutedText");
        rl->addWidget(cat);
        layout->addWidget(row);
        QString id = s.id;
        connect(row, &QPushButton::clicked, this, [this, id]{ emit openShard(id); });
    }
}
