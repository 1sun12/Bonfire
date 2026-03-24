#include "mainwindow.h"
#include "database.h"
#include "views/dashboardwidget.h"
#include "views/librarywidget.h"
#include "views/shardeditorwidget.h"
#include "views/reviewwidget.h"
#include "dialogs/quickcapturedialog.h"
#include <QStackedWidget>
#include <QSplitter>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QShortcut>
#include <QKeySequence>
#include <QStandardPaths>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QDate>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Bonfire");
    setMinimumSize(900, 620);
    resize(1280, 800);

    // --- Database init ---
    m_db = new Database(this);
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    if (!m_db->init(dataDir + "/vault.db")) {
        QMessageBox::critical(this, "Database Error", m_db->lastError());
    }

    // --- Root layout: sidebar + content ---
    auto *central  = new QWidget;
    auto *rootLayout = new QHBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);
    setCentralWidget(central);

    // ---- Sidebar ----
    auto *sidebar = new QWidget;
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(148);
    auto *sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(8, 12, 8, 12);
    sideLayout->setSpacing(4);

    auto *appName = new QLabel("Bonfire");
    appName->setObjectName("appName");
    sideLayout->addWidget(appName);
    sideLayout->addSpacing(8);

    m_navDashboard = new QPushButton("Dashboard");
    m_navLibrary   = new QPushButton("Library");
    m_navReview    = new QPushButton("Review");

    for (QPushButton *btn : {m_navDashboard, m_navLibrary, m_navReview}) {
        btn->setObjectName("navBtn");
        btn->setCheckable(true);
        btn->setFlat(true);
        btn->setFixedHeight(34);
        sideLayout->addWidget(btn);
    }

    sideLayout->addStretch();

    m_navNew = new QPushButton("+ New Shard");
    m_navNew->setObjectName("primaryBtn");
    m_navNew->setFixedHeight(34);
    sideLayout->addWidget(m_navNew);

    rootLayout->addWidget(sidebar);

    // Separator line
    auto *sep = new QFrame;
    sep->setFrameShape(QFrame::VLine);
    sep->setStyleSheet("border:none; border-left:1px solid #2a2a2a;");
    rootLayout->addWidget(sep);

    // ---- Main content (stacked views) ----
    m_stack     = new QStackedWidget;
    m_dashboard = new DashboardWidget(m_db, m_stack);
    m_library   = new LibraryWidget(m_db, m_stack);
    m_editor    = new ShardEditorWidget(m_db, m_stack);
    m_review    = new ReviewWidget(m_db, m_stack);

    m_stack->addWidget(m_dashboard);
    m_stack->addWidget(m_library);
    m_stack->addWidget(m_editor);
    m_stack->addWidget(m_review);
    rootLayout->addWidget(m_stack, 1);

    // ---- Connections ----
    connect(m_navDashboard, &QPushButton::clicked, this, &MainWindow::navigateDashboard);
    connect(m_navLibrary,   &QPushButton::clicked, this, &MainWindow::navigateLibrary);
    connect(m_navReview,    &QPushButton::clicked, this, &MainWindow::navigateReview);
    connect(m_navNew,       &QPushButton::clicked, this, &MainWindow::openQuickCapture);

    // Dashboard signals
    connect(m_dashboard, &DashboardWidget::openShard,      this, &MainWindow::navigateEditor);
    connect(m_dashboard, &DashboardWidget::startReview,    this, &MainWindow::navigateReview);
    connect(m_dashboard, &DashboardWidget::requestNewShard,this, &MainWindow::openQuickCapture);
    connect(m_dashboard, &DashboardWidget::requestExport,  this, &MainWindow::onExport);
    connect(m_dashboard, &DashboardWidget::requestImport,  this, &MainWindow::onImport);

    // Library signals
    connect(m_library, &LibraryWidget::openShard,      this, &MainWindow::navigateEditor);
    connect(m_library, &LibraryWidget::requestNewShard,this, [this]{ navigateEditor(); });

    // Editor signals
    connect(m_editor, &ShardEditorWidget::backRequested, this, &MainWindow::navigateLibrary);
    connect(m_editor, &ShardEditorWidget::shardSaved,    this, [this]{ onShardSaved(); });
    connect(m_editor, &ShardEditorWidget::shardDeleted,  this, [this]{ onShardDeleted(); });

    // Review signals
    connect(m_review, &ReviewWidget::sessionComplete, this, &MainWindow::navigateDashboard);

    // ---- Keyboard shortcuts ----
    auto *scNew    = new QShortcut(QKeySequence("Ctrl+N"), this);
    auto *scSearch = new QShortcut(QKeySequence("Ctrl+K"), this);
    connect(scNew,    &QShortcut::activated, this, &MainWindow::openQuickCapture);
    connect(scSearch, &QShortcut::activated, this, [this]{
        navigateLibrary();
        // Library's search field will be focused
    });

    // ---- Initial state ----
    navigateDashboard();
}

MainWindow::~MainWindow() {}

void MainWindow::setupNav() {}

void MainWindow::setActiveNav(QPushButton *btn) {
    for (QPushButton *b : {m_navDashboard, m_navLibrary, m_navReview})
        b->setChecked(b == btn);
}

void MainWindow::navigateDashboard() {
    setActiveNav(m_navDashboard);
    m_dashboard->refresh();
    m_stack->setCurrentWidget(m_dashboard);
}

void MainWindow::navigateLibrary() {
    setActiveNav(m_navLibrary);
    m_library->refresh();
    m_stack->setCurrentWidget(m_library);
}

void MainWindow::navigateEditor(const QString &shardId) {
    setActiveNav(nullptr);
    m_editor->loadShard(shardId);
    m_stack->setCurrentWidget(m_editor);
}

void MainWindow::navigateReview() {
    setActiveNav(m_navReview);
    QString today = QDate::currentDate().toString("yyyy-MM-dd");
    QList<Shard> due;
    for (const Shard &s : m_db->allShards()) {
        if (s.reviewEnabled && !s.reviewNext.isEmpty() && s.reviewNext <= today)
            due << s;
    }
    if (due.isEmpty()) {
        QMessageBox::information(this, "Review", "No shards are due for review today.");
        navigateDashboard();
        return;
    }
    m_review->startSession(due);
    m_stack->setCurrentWidget(m_review);
}

void MainWindow::onShardSaved() {
    // Refresh views in the background — they'll pull fresh data when shown
}

void MainWindow::onShardDeleted() {
    navigateLibrary();
}

void MainWindow::onExport() {
    QString path = QFileDialog::getSaveFileName(this, "Export Bonfire",
        QDir::homePath() + "/bonfire-export-" +
        QDate::currentDate().toString("yyyy-MM-dd") + ".json",
        "JSON files (*.json)");
    if (path.isEmpty()) return;
    if (m_db->exportToJson(path)) {
        QMessageBox::information(this, "Export", "Export complete.");
    } else {
        QMessageBox::warning(this, "Export Failed", m_db->lastError());
    }
}

void MainWindow::onImport() {
    QString path = QFileDialog::getOpenFileName(this, "Import Bonfire",
        QDir::homePath(), "JSON files (*.json)");
    if (path.isEmpty()) return;
    int imported, skipped;
    if (m_db->importFromJson(path, imported, skipped)) {
        QMessageBox::information(this, "Import Complete",
            QString("Imported: %1\nSkipped (duplicates): %2").arg(imported).arg(skipped));
        m_library->refresh();
        m_dashboard->refresh();
    } else {
        QMessageBox::warning(this, "Import Failed", m_db->lastError());
    }
}

void MainWindow::openQuickCapture() {
    QStringList langs = defaultLanguages();
    for (const QString &cl : m_db->customLanguages())
        if (!langs.contains(cl)) langs << cl;
    langs.sort();

    auto *dlg = new QuickCaptureDialog(m_db, langs, this);
    connect(dlg, &QuickCaptureDialog::shardSaved, this, [this](const Shard &) {
        m_library->refresh();
        m_dashboard->refresh();
    });
    dlg->exec();
    dlg->deleteLater();
}
