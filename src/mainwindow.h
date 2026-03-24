#pragma once
#include <QMainWindow>

class QStackedWidget;
class QPushButton;
class Database;
class DashboardWidget;
class LibraryWidget;
class ShardEditorWidget;
class ReviewWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void navigateDashboard();
    void navigateLibrary();
    void navigateEditor(const QString &shardId = {});
    void navigateReview();

    void onShardSaved();
    void onShardDeleted();
    void onExport();
    void onImport();
    void openQuickCapture();

private:
    void setupNav();
    void setActiveNav(QPushButton *btn);

    Database           *m_db;
    QStackedWidget     *m_stack;

    DashboardWidget    *m_dashboard;
    LibraryWidget      *m_library;
    ShardEditorWidget  *m_editor;
    ReviewWidget       *m_review;

    QPushButton        *m_navDashboard;
    QPushButton        *m_navLibrary;
    QPushButton        *m_navReview;
    QPushButton        *m_navNew;
};
