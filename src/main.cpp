#include <QApplication>
#include <QStyleFactory>
#include <QPalette>
#include <QFont>
#include "mainwindow.h"

static void applyDarkTheme(QApplication &app) {
    app.setStyle(QStyleFactory::create("Fusion"));

    QPalette p;
    p.setColor(QPalette::Window,          QColor(26,  26,  26));
    p.setColor(QPalette::WindowText,      QColor(204, 204, 204));
    p.setColor(QPalette::Base,            QColor(35,  35,  35));
    p.setColor(QPalette::AlternateBase,   QColor(42,  42,  42));
    p.setColor(QPalette::ToolTipBase,     QColor(45,  45,  45));
    p.setColor(QPalette::ToolTipText,     QColor(204, 204, 204));
    p.setColor(QPalette::Text,            QColor(204, 204, 204));
    p.setColor(QPalette::Button,          QColor(37,  37,  38));
    p.setColor(QPalette::ButtonText,      QColor(204, 204, 204));
    p.setColor(QPalette::BrightText,      Qt::red);
    p.setColor(QPalette::Link,            QColor(77,  148, 255));
    p.setColor(QPalette::Highlight,       QColor(9,   71,  113));
    p.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    p.setColor(QPalette::Disabled, QPalette::WindowText, QColor(85, 85, 85));
    p.setColor(QPalette::Disabled, QPalette::Text,       QColor(85, 85, 85));
    p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(85, 85, 85));
    app.setPalette(p);

    app.setStyleSheet(R"(
        /* ---- Base ---- */
        QWidget {
            font-family: 'Inter', 'Segoe UI', 'Helvetica Neue', sans-serif;
            font-size: 13px;
            color: #cccccc;
        }

        QScrollArea, QScrollArea > QWidget > QWidget {
            background: #1a1a1a;
        }

        /* ---- Sidebar ---- */
        QWidget#sidebar {
            background: #111111;
        }
        QLabel#appName {
            color: #cccccc;
            font-size: 13px;
            font-weight: bold;
            padding: 4px 4px 8px 4px;
        }

        /* ---- Nav buttons ---- */
        QPushButton#navBtn {
            background: transparent;
            color: #888888;
            border: none;
            border-radius: 5px;
            text-align: left;
            padding: 0 10px;
            font-size: 13px;
        }
        QPushButton#navBtn:hover {
            background: #1e1e1e;
            color: #cccccc;
        }
        QPushButton#navBtn:checked {
            background: #1e3a5f;
            color: #ffffff;
        }

        /* ---- Primary button ---- */
        QPushButton#primaryBtn {
            background: #0078d4;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 4px 14px;
            font-weight: bold;
        }
        QPushButton#primaryBtn:hover  { background: #1084d8; }
        QPushButton#primaryBtn:pressed { background: #006bbf; }

        /* ---- Tool/secondary button ---- */
        QPushButton#toolBtn {
            background: #2d2d2d;
            color: #aaaaaa;
            border: 1px solid #3a3a3a;
            border-radius: 4px;
            padding: 4px 10px;
        }
        QPushButton#toolBtn:hover  { background: #383838; color: #cccccc; }
        QPushButton#toolBtn:pressed { background: #252525; }

        /* ---- Danger button ---- */
        QPushButton#dangerBtn {
            background: #3d1515;
            color: #f87171;
            border: 1px solid #5a1f1f;
            border-radius: 4px;
            padding: 4px 10px;
        }
        QPushButton#dangerBtn:hover { background: #521c1c; }

        /* ---- Toggle button (review enable) ---- */
        QPushButton#toggleBtn {
            background: #2d2d2d;
            color: #888888;
            border: 1px solid #3a3a3a;
            border-radius: 4px;
            padding: 4px 12px;
        }
        QPushButton#toggleBtn:checked {
            background: #1a3a1a;
            color: #10b981;
            border-color: #10b981;
        }

        /* ---- Inputs ---- */
        QLineEdit, QTextEdit, QPlainTextEdit {
            background: #252525;
            border: 1px solid #3a3a3a;
            border-radius: 4px;
            padding: 4px 8px;
            color: #cccccc;
            selection-background-color: #094771;
        }
        QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus {
            border-color: #0078d4;
        }

        QComboBox {
            background: #252525;
            border: 1px solid #3a3a3a;
            border-radius: 4px;
            padding: 4px 8px;
            color: #cccccc;
        }
        QComboBox:focus { border-color: #0078d4; }
        QComboBox::drop-down { border: none; width: 20px; }
        QComboBox QAbstractItemView {
            background: #2d2d2d;
            border: 1px solid #3a3a3a;
            selection-background-color: #094771;
        }

        /* ---- Lists ---- */
        QListWidget {
            background: #1a1a1a;
            border: none;
            outline: none;
        }
        QListWidget::item:selected { background: #094771; }
        QListWidget::item:hover    { background: #2a2a2a; }

        /* ---- Panels / cards ---- */
        QFrame#panel {
            background: #222222;
            border: 1px solid #2e2e2e;
            border-radius: 6px;
        }
        QFrame#statCard {
            background: #222222;
            border: 1px solid #2e2e2e;
            border-radius: 6px;
        }
        QFrame#reviewCard {
            background: #222222;
            border: 1px solid #333333;
            border-radius: 8px;
        }
        QFrame#codeFrame {
            background: #0d0d0d;
            border: 1px solid #2a2a2a;
            border-radius: 4px;
        }

        /* ---- Editor toolbar ---- */
        QFrame#editorToolbar {
            background: #181818;
            border-bottom: 1px solid #2a2a2a;
        }

        /* ---- Typography helpers ---- */
        QLabel#sectionTitle {
            color: #666666;
            font-size: 11px;
            font-weight: bold;
            letter-spacing: 0.5px;
        }
        QLabel#mutedText  { color: #666666; font-size: 12px; }
        QLabel#descText   { color: #aaaaaa; line-height: 1.5; }
        QLabel#rowTitle   { color: #cccccc; font-size: 13px; }
        QLabel#statLabel  { color: #666666; font-size: 11px; }

        /* ---- List rows ---- */
        QPushButton#listRow {
            background: transparent;
            border: none;
            border-radius: 4px;
            text-align: left;
        }
        QPushButton#listRow:hover { background: #2a2a2a; }

        /* ---- Scrollbars ---- */
        QScrollBar:vertical {
            background: #1a1a1a;
            width: 8px;
            margin: 0;
        }
        QScrollBar::handle:vertical {
            background: #3a3a3a;
            border-radius: 4px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover { background: #4a4a4a; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }

        QScrollBar:horizontal {
            background: #1a1a1a;
            height: 8px;
        }
        QScrollBar::handle:horizontal {
            background: #3a3a3a;
            border-radius: 4px;
        }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }

        /* ---- Form labels ---- */
        QFormLayout QLabel {
            color: #888888;
            font-size: 12px;
        }

        /* ---- Message boxes ---- */
        QMessageBox { background: #222222; }
        QMessageBox QLabel { color: #cccccc; }

        /* ---- Dialog ---- */
        QDialog { background: #1e1e1e; }
    )");
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setOrganizationName("Bonfire");
    app.setApplicationName("Bonfire");
    app.setApplicationVersion("1.0.0");

    applyDarkTheme(app);

    MainWindow w;
    w.show();

    return app.exec();
}
