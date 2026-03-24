#include "quickcapturedialog.h"
#include "../database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QDateTime>
#include <QUuid>
#include <QDate>
#include <QFont>

static QString newId() {
    return QString::number(QDateTime::currentMSecsSinceEpoch(), 36)
           + QUuid::createUuid().toString(QUuid::WithoutBraces).left(6);
}

QuickCaptureDialog::QuickCaptureDialog(Database *db, const QStringList &allLanguages,
                                       QWidget *parent)
    : QDialog(parent), m_db(db)
{
    setWindowTitle("Quick Capture");
    setMinimumWidth(520);
    setModal(true);

    auto *root = new QVBoxLayout(this);
    root->setSpacing(10);
    root->setContentsMargins(16, 16, 16, 16);

    auto *header = new QLabel("Quick Capture");
    QFont hf = header->font();
    hf.setPointSize(11);
    hf.setBold(true);
    header->setFont(hf);
    root->addWidget(header);

    auto *form = new QFormLayout;
    form->setSpacing(8);

    m_title = new QLineEdit;
    m_title->setPlaceholderText("e.g., rsync exclude pattern");
    form->addRow("Title *", m_title);

    m_language = new QComboBox;
    m_language->addItem("(select language)");
    m_language->addItems(allLanguages);
    form->addRow("Language", m_language);

    m_tag = new QLineEdit;
    m_tag->setPlaceholderText("e.g., networking, one-liner");
    form->addRow("Tag(s)", m_tag);

    root->addLayout(form);

    auto *codeLabel = new QLabel("Code *");
    root->addWidget(codeLabel);

    m_code = new QPlainTextEdit;
    m_code->setPlaceholderText("Paste your code here...");
    m_code->setMinimumHeight(160);
    QFont mono("Monospace");
    mono.setStyleHint(QFont::Monospace);
    mono.setPointSize(10);
    m_code->setFont(mono);
    m_code->setStyleSheet("background:#0d0d0d; color:#7ec8a0;");
    root->addWidget(m_code);

    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();
    auto *cancel = new QPushButton("Cancel");
    auto *save   = new QPushButton("Save Shard");
    save->setDefault(true);
    save->setObjectName("primaryBtn");
    btnRow->addWidget(cancel);
    btnRow->addWidget(save);
    root->addLayout(btnRow);

    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(save,   &QPushButton::clicked, this, &QuickCaptureDialog::save);
}

void QuickCaptureDialog::save() {
    if (m_title->text().trimmed().isEmpty()) {
        m_title->setFocus();
        return;
    }
    if (m_code->toPlainText().trimmed().isEmpty()) {
        m_code->setFocus();
        return;
    }

    Shard s;
    s.id         = newId();
    s.title      = m_title->text().trimmed();
    s.language   = (m_language->currentIndex() > 0) ? m_language->currentText() : "";
    s.code       = m_code->toPlainText();
    s.category   = "snippet";
    s.familiarity= "fresh";
    s.reviewEnabled   = false;
    s.reviewInterval  = 0;
    s.reviewRepetitions = 0;
    s.reviewEase = 2.5;
    s.reviewNext = QDate::currentDate().toString("yyyy-MM-dd");

    QString now = QDateTime::currentDateTime().toString(Qt::ISODate);
    s.createdAt  = now;
    s.modifiedAt = now;

    // Parse tags
    QString tagStr = m_tag->text().trimmed();
    if (!tagStr.isEmpty()) {
        for (const QString &t : tagStr.split(',')) {
            QString trimmed = t.trimmed().toLower();
            if (!trimmed.isEmpty()) s.tags << trimmed;
        }
        s.tags.removeDuplicates();
    }

    if (m_db->saveShard(s)) {
        emit shardSaved(s);
        accept();
    }
}
