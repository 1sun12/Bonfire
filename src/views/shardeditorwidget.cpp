#include "shardeditorwidget.h"
#include "../database.h"
#include "../syntaxhighlighter.h"
#include "../sm2.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QStackedWidget>
#include <QMessageBox>
#include <QDateTime>
#include <QDate>
#include <QUuid>
#include <QFont>

static QString newId() {
    return QString::number(QDateTime::currentMSecsSinceEpoch(), 36)
           + QUuid::createUuid().toString(QUuid::WithoutBraces).left(6);
}

static QFont monoFont(int pt = 10) {
    QFont f("Monospace");
    f.setStyleHint(QFont::Monospace);
    f.setPointSize(pt);
    return f;
}

// Helper: colored badge label
static QLabel *badge(const QString &text, const QString &color) {
    auto *lbl = new QLabel(text);
    lbl->setStyleSheet(QString("background:%1; color:white; border-radius:3px; "
                               "padding:1px 6px; font-size:10px; font-family:monospace;").arg(color));
    lbl->setFixedHeight(18);
    return lbl;
}

ShardEditorWidget::ShardEditorWidget(Database *db, QWidget *parent)
    : QWidget(parent), m_db(db)
{
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    // ---- Toolbar ----
    auto *toolbar = new QFrame;
    toolbar->setObjectName("editorToolbar");
    toolbar->setFixedHeight(44);
    auto *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(12, 0, 12, 0);
    tbLayout->setSpacing(8);

    auto *backBtn = new QPushButton("← Back");
    backBtn->setObjectName("toolBtn");
    tbLayout->addWidget(backBtn);
    tbLayout->addStretch();

    m_markReviewedBtn = new QPushButton("Mark Reviewed");
    m_markReviewedBtn->setObjectName("toolBtn");
    m_editBtn   = new QPushButton("Edit");
    m_editBtn->setObjectName("toolBtn");
    m_saveBtn   = new QPushButton("Save");
    m_saveBtn->setObjectName("primaryBtn");
    m_cancelBtn = new QPushButton("Cancel");
    m_cancelBtn->setObjectName("toolBtn");
    m_deleteBtn = new QPushButton("Delete");
    m_deleteBtn->setObjectName("dangerBtn");

    tbLayout->addWidget(m_markReviewedBtn);
    tbLayout->addWidget(m_deleteBtn);
    tbLayout->addWidget(m_cancelBtn);
    tbLayout->addWidget(m_editBtn);
    tbLayout->addWidget(m_saveBtn);

    outerLayout->addWidget(toolbar);

    // ---- Stacked: view / edit ----
    m_stack = new QStackedWidget;
    outerLayout->addWidget(m_stack, 1);

    // ---- VIEW MODE ----
    {
        auto *scroll = new QScrollArea;
        scroll->setWidgetResizable(true);
        scroll->setFrameShape(QFrame::NoFrame);
        auto *w = new QWidget;
        auto *vl = new QVBoxLayout(w);
        vl->setContentsMargins(24, 20, 24, 24);
        vl->setSpacing(14);

        m_vTitle = new QLabel;
        QFont tf = m_vTitle->font();
        tf.setPointSize(16);
        tf.setBold(true);
        m_vTitle->setFont(tf);
        m_vTitle->setWordWrap(true);
        vl->addWidget(m_vTitle);

        m_vMeta = new QLabel;
        m_vMeta->setObjectName("mutedText");
        m_vMeta->setWordWrap(true);
        vl->addWidget(m_vMeta);

        // Code block
        auto *codeFrame = new QFrame;
        codeFrame->setObjectName("codeFrame");
        auto *cfl = new QVBoxLayout(codeFrame);
        cfl->setContentsMargins(0, 0, 0, 0);
        cfl->setSpacing(0);
        m_vCode = new QPlainTextEdit;
        m_vCode->setReadOnly(true);
        m_vCode->setFont(monoFont());
        m_vCode->setStyleSheet("background:#0d0d0d; color:#7ec8a0; border:none;");
        m_vCode->setMinimumHeight(120);
        m_vHighlighter = new SyntaxHighlighter(m_vCode->document());
        cfl->addWidget(m_vCode);
        vl->addWidget(codeFrame);

        auto *descLabel = new QLabel("Description");
        descLabel->setObjectName("sectionTitle");
        vl->addWidget(descLabel);
        m_vDescription = new QLabel;
        m_vDescription->setWordWrap(true);
        m_vDescription->setObjectName("descText");
        vl->addWidget(m_vDescription);

        m_vSource = new QLabel;
        m_vSource->setObjectName("mutedText");
        m_vSource->setWordWrap(true);
        vl->addWidget(m_vSource);

        m_vTags = new QLabel;
        m_vTags->setObjectName("mutedText");
        vl->addWidget(m_vTags);

        vl->addStretch();
        scroll->setWidget(w);
        m_viewWidget = scroll;
        m_stack->addWidget(m_viewWidget);
    }

    // ---- EDIT MODE ----
    {
        auto *scroll = new QScrollArea;
        scroll->setWidgetResizable(true);
        scroll->setFrameShape(QFrame::NoFrame);
        auto *w  = new QWidget;
        auto *vl = new QVBoxLayout(w);
        vl->setContentsMargins(24, 20, 24, 24);
        vl->setSpacing(12);

        auto *form = new QFormLayout;
        form->setSpacing(10);
        form->setLabelAlignment(Qt::AlignRight);

        m_eTitle = new QLineEdit;
        m_eTitle->setPlaceholderText("Short, descriptive title");
        form->addRow("Title *", m_eTitle);

        m_eLanguage = new QComboBox;
        m_eLanguage->setEditable(false);
        form->addRow("Language", m_eLanguage);

        m_eCategory = new QComboBox;
        for (const QString &c : categories()) m_eCategory->addItem(c);
        form->addRow("Category", m_eCategory);

        m_eFamiliarity = new QComboBox;
        for (const QString &f : familiarities()) m_eFamiliarity->addItem(f);
        form->addRow("Familiarity", m_eFamiliarity);

        m_eSource = new QLineEdit;
        m_eSource->setPlaceholderText("URL, book, man page…");
        form->addRow("Source", m_eSource);

        m_eTags = new QLineEdit;
        m_eTags->setPlaceholderText("Comma-separated: networking, one-liner");
        form->addRow("Tags", m_eTags);

        m_reviewCheck = new QPushButton("Enable Spaced Repetition");
        m_reviewCheck->setCheckable(true);
        m_reviewCheck->setObjectName("toggleBtn");
        form->addRow("Review", m_reviewCheck);

        vl->addLayout(form);

        auto *codeLabel = new QLabel("Code *");
        codeLabel->setObjectName("sectionTitle");
        vl->addWidget(codeLabel);

        m_eCode = new QPlainTextEdit;
        m_eCode->setFont(monoFont());
        m_eCode->setMinimumHeight(200);
        m_eCode->setStyleSheet("background:#0d0d0d; color:#7ec8a0; border:1px solid #333;");
        m_eHighlighter = new SyntaxHighlighter(m_eCode->document());
        vl->addWidget(m_eCode);

        auto *descLabel = new QLabel("Description");
        descLabel->setObjectName("sectionTitle");
        vl->addWidget(descLabel);

        m_eDescription = new QTextEdit;
        m_eDescription->setPlaceholderText("Why does this work? When would you use it?");
        m_eDescription->setMaximumHeight(100);
        vl->addWidget(m_eDescription);

        vl->addStretch();
        scroll->setWidget(w);
        m_editWidget = scroll;
        m_stack->addWidget(m_editWidget);
    }

    // Connections
    connect(backBtn,          &QPushButton::clicked, this, &ShardEditorWidget::backRequested);
    connect(m_editBtn,        &QPushButton::clicked, this, &ShardEditorWidget::enterEditMode);
    connect(m_cancelBtn,      &QPushButton::clicked, this, &ShardEditorWidget::cancelEdit);
    connect(m_saveBtn,        &QPushButton::clicked, this, &ShardEditorWidget::save);
    connect(m_deleteBtn,      &QPushButton::clicked, this, &ShardEditorWidget::deleteShard);
    connect(m_markReviewedBtn,&QPushButton::clicked, this, &ShardEditorWidget::markReviewed);
    connect(m_eLanguage, &QComboBox::currentTextChanged, this, &ShardEditorWidget::onLanguageChanged);
}

QStringList ShardEditorWidget::allLanguages() const {
    QStringList langs = defaultLanguages();
    for (const QString &cl : m_db->customLanguages())
        if (!langs.contains(cl)) langs << cl;
    langs.sort();
    return langs;
}

void ShardEditorWidget::refreshLanguages() {
    QStringList langs = allLanguages();
    QString cur = m_eLanguage->currentText();
    m_eLanguage->blockSignals(true);
    m_eLanguage->clear();
    m_eLanguage->addItem("(none)");
    m_eLanguage->addItems(langs);
    m_eLanguage->addItem("+ Add custom language…");
    int idx = m_eLanguage->findText(cur);
    m_eLanguage->setCurrentIndex(idx >= 0 ? idx : 0);
    m_eLanguage->blockSignals(false);
}

void ShardEditorWidget::loadShard(const QString &id) {
    if (id.isEmpty()) {
        m_isNew = true;
        m_shard = Shard();
        m_shard.id       = newId();
        m_shard.category = "snippet";
        m_shard.familiarity = "fresh";
        m_shard.reviewEase  = 2.5;
        m_shard.reviewNext  = QDate::currentDate().toString("yyyy-MM-dd");
        QString now = QDateTime::currentDateTime().toString(Qt::ISODate);
        m_shard.createdAt  = now;
        m_shard.modifiedAt = now;
        refreshLanguages();
        showEditMode();
    } else {
        m_isNew = false;
        QList<Shard> all = m_db->allShards();
        for (const Shard &s : all) {
            if (s.id == id) { m_shard = s; break; }
        }
        refreshLanguages();
        populateViewMode();
        showViewMode();
    }
}

void ShardEditorWidget::showViewMode() {
    m_stack->setCurrentWidget(m_viewWidget);
    m_editBtn->setVisible(true);
    m_saveBtn->setVisible(false);
    m_cancelBtn->setVisible(false);
    m_deleteBtn->setVisible(!m_isNew);
    m_markReviewedBtn->setVisible(m_shard.reviewEnabled);
}

void ShardEditorWidget::showEditMode() {
    populateEditMode();
    m_stack->setCurrentWidget(m_editWidget);
    m_editBtn->setVisible(false);
    m_saveBtn->setVisible(true);
    m_cancelBtn->setVisible(!m_isNew);
    m_deleteBtn->setVisible(false);
    m_markReviewedBtn->setVisible(false);
}

void ShardEditorWidget::populateViewMode() {
    m_vTitle->setText(m_shard.title);

    QStringList meta;
    if (!m_shard.language.isEmpty())
        meta << m_shard.language;
    meta << m_shard.category << m_shard.familiarity;
    if (m_shard.reviewEnabled)
        meta << QString("Review: %1").arg(m_shard.reviewNext);
    m_vMeta->setText(meta.join("  ·  "));

    m_vCode->setPlainText(m_shard.code);
    m_vHighlighter->setLanguage(m_shard.language);

    m_vDescription->setText(m_shard.description.isEmpty()
                             ? "(no description)" : m_shard.description);
    m_vSource->setText(m_shard.source.isEmpty()
                       ? "" : "Source: " + m_shard.source);

    QStringList tagDisplay;
    for (const QString &t : m_shard.tags) tagDisplay << "#" + t;
    m_vTags->setText(tagDisplay.join("  "));
}

void ShardEditorWidget::populateEditMode() {
    m_eTitle->setText(m_shard.title);

    int langIdx = m_eLanguage->findText(m_shard.language);
    m_eLanguage->setCurrentIndex(langIdx > 0 ? langIdx : 0);

    int catIdx = m_eCategory->findText(m_shard.category);
    m_eCategory->setCurrentIndex(catIdx >= 0 ? catIdx : 0);

    int famIdx = m_eFamiliarity->findText(m_shard.familiarity);
    m_eFamiliarity->setCurrentIndex(famIdx >= 0 ? famIdx : 0);

    m_eSource->setText(m_shard.source);
    m_eTags->setText(m_shard.tags.join(", "));
    m_eCode->setPlainText(m_shard.code);
    m_eDescription->setPlainText(m_shard.description);
    m_reviewCheck->setChecked(m_shard.reviewEnabled);
    m_eHighlighter->setLanguage(m_shard.language);
}

void ShardEditorWidget::syncFormToShard() {
    m_shard.title       = m_eTitle->text().trimmed();
    m_shard.language    = (m_eLanguage->currentIndex() > 0 &&
                           m_eLanguage->currentText() != "+ Add custom language…")
                          ? m_eLanguage->currentText() : "";
    m_shard.category    = m_eCategory->currentText();
    m_shard.familiarity = m_eFamiliarity->currentText();
    m_shard.source      = m_eSource->text().trimmed();
    m_shard.code        = m_eCode->toPlainText();
    m_shard.description = m_eDescription->toPlainText().trimmed();
    m_shard.reviewEnabled = m_reviewCheck->isChecked();

    QStringList tags;
    for (const QString &t : m_eTags->text().split(',')) {
        QString trimmed = t.trimmed().toLower();
        if (!trimmed.isEmpty()) tags << trimmed;
    }
    tags.removeDuplicates();
    m_shard.tags = tags;
}

void ShardEditorWidget::enterEditMode() {
    showEditMode();
}

void ShardEditorWidget::cancelEdit() {
    if (m_isNew) {
        emit backRequested();
    } else {
        populateViewMode();
        showViewMode();
    }
}

void ShardEditorWidget::save() {
    syncFormToShard();
    if (m_shard.title.isEmpty()) {
        m_eTitle->setFocus();
        return;
    }

    // Handle "+ Add custom language"
    if (m_eLanguage->currentText() == "+ Add custom language…") {
        m_shard.language = "";
    }

    if (m_db->saveShard(m_shard)) {
        m_isNew = false;
        populateViewMode();
        showViewMode();
        emit shardSaved(m_shard);
    }
}

void ShardEditorWidget::deleteShard() {
    auto btn = QMessageBox::question(this, "Delete Shard",
        QString("Delete \"%1\"? This cannot be undone.").arg(m_shard.title),
        QMessageBox::Yes | QMessageBox::Cancel);
    if (btn == QMessageBox::Yes) {
        m_db->deleteShard(m_shard.id);
        emit shardDeleted(m_shard.id);
        emit backRequested();
    }
}

void ShardEditorWidget::markReviewed() {
    SM2Result res = sm2(4, m_shard.reviewInterval,
                        m_shard.reviewRepetitions, m_shard.reviewEase);
    m_shard.reviewInterval    = res.interval;
    m_shard.reviewRepetitions = res.repetitions;
    m_shard.reviewEase        = res.easeFactor;
    m_shard.reviewNext        = res.nextReview;
    m_shard.lastReviewed      = QDateTime::currentDateTime().toString(Qt::ISODate);
    m_db->saveShard(m_shard);
    populateViewMode();
    showViewMode();
    emit shardSaved(m_shard);
}

void ShardEditorWidget::onLanguageChanged(const QString &lang) {
    if (lang == "+ Add custom language…") return;
    m_eHighlighter->setLanguage(lang);
}
