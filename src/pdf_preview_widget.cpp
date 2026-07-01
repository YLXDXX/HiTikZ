#include "pdf_preview_widget.h"
#include <QVBoxLayout>
#include <QScrollBar>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QCursor>
#include <QTimer>

PdfPreviewWidget::PdfPreviewWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_pdfDoc = new QPdfDocument(this);
    m_pdfView = new QPdfView;
    m_pdfView->setDocument(m_pdfDoc);
    m_pdfView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pdfView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pdfView->viewport()->installEventFilter(this);
    m_pdfView->viewport()->setCursor(Qt::OpenHandCursor);
    m_pdfView->setMinimumHeight(0);

    layout->addWidget(m_pdfView);

    connect(m_pdfDoc, &QPdfDocument::statusChanged, this, [this](QPdfDocument::Status s) {
        if (s == QPdfDocument::Status::Ready && m_resumeScroll) {
            m_resumeScroll = false;
            QTimer::singleShot(20, this, [this]() {
                QScrollBar *h = m_pdfView->horizontalScrollBar();
                QScrollBar *v = m_pdfView->verticalScrollBar();
                if (h && h->maximum() > 0)
                    h->setValue(qRound(m_savedHFrac * h->maximum()));
                if (v && v->maximum() > 0)
                    v->setValue(qRound(m_savedVFrac * v->maximum()));
            });
        }
    });
}

QPdfDocument *PdfPreviewWidget::document() const
{
    return m_pdfDoc;
}

QPdfView *PdfPreviewWidget::pdfView() const
{
    return m_pdfView;
}

int PdfPreviewWidget::zoomPreference() const
{
    return m_zoomPref;
}

void PdfPreviewWidget::setZoomPreference(int pref)
{
    m_zoomPref = pref;
}

bool PdfPreviewWidget::hasDocument() const
{
    return m_pdfDoc && m_pdfDoc->status() == QPdfDocument::Status::Ready;
}

void PdfPreviewWidget::zoomIn()
{
    QPoint vpPos = m_pdfView->viewport() ? m_pdfView->viewport()->mapFromGlobal(QCursor::pos()) : QPoint();
    qreal oldFactor = m_pdfView->zoomFactor();
    qreal newFactor = oldFactor * 1.25;

    m_zoomPref = -1;
    m_pdfView->setZoomMode(QPdfView::ZoomMode::Custom);
    m_pdfView->setZoomFactor(newFactor);

    if (m_pdfView->viewport()) {
        qreal scale = newFactor / oldFactor;
        QPoint delta(vpPos.x() * (scale - 1.0), vpPos.y() * (scale - 1.0));
        if (m_pdfView->horizontalScrollBar())
            m_pdfView->horizontalScrollBar()->setValue(
                m_pdfView->horizontalScrollBar()->value() + delta.x());
        if (m_pdfView->verticalScrollBar())
            m_pdfView->verticalScrollBar()->setValue(
                m_pdfView->verticalScrollBar()->value() + delta.y());
    }
}

void PdfPreviewWidget::zoomOut()
{
    QPoint vpPos = m_pdfView->viewport() ? m_pdfView->viewport()->mapFromGlobal(QCursor::pos()) : QPoint();
    qreal oldFactor = m_pdfView->zoomFactor();
    qreal newFactor = qMax(0.1, oldFactor / 1.25);

    m_zoomPref = -1;
    m_pdfView->setZoomMode(QPdfView::ZoomMode::Custom);
    m_pdfView->setZoomFactor(newFactor);

    if (m_pdfView->viewport()) {
        qreal scale = newFactor / oldFactor;
        QPoint delta(vpPos.x() * (scale - 1.0), vpPos.y() * (scale - 1.0));
        if (m_pdfView->horizontalScrollBar())
            m_pdfView->horizontalScrollBar()->setValue(
                m_pdfView->horizontalScrollBar()->value() + delta.x());
        if (m_pdfView->verticalScrollBar())
            m_pdfView->verticalScrollBar()->setValue(
                m_pdfView->verticalScrollBar()->value() + delta.y());
    }
}

void PdfPreviewWidget::fitPage()
{
    m_zoomPref = 0;
    m_pdfView->setZoomMode(QPdfView::ZoomMode::FitInView);
}

void PdfPreviewWidget::fitWidth()
{
    m_zoomPref = 1;
    m_pdfView->setZoomMode(QPdfView::ZoomMode::FitToWidth);
}

void PdfPreviewWidget::fitHeight()
{
    if (!m_pdfDoc || m_pdfDoc->status() != QPdfDocument::Status::Ready)
        return;

    QSizeF pageSize = m_pdfDoc->pagePointSize(0);
    if (pageSize.isEmpty()) return;

    QSize vpSize = m_pdfView->viewport() ? m_pdfView->viewport()->size() : QSize();
    if (vpSize.height() <= 0) return;

    qreal dpi = m_pdfView->logicalDpiY();
    qreal pagePixels = pageSize.height() * dpi / 72.0;
    qreal ratio = vpSize.height() / pagePixels;

    m_zoomPref = 2;
    m_pdfView->setZoomMode(QPdfView::ZoomMode::Custom);
    m_pdfView->setZoomFactor(ratio);
}

void PdfPreviewWidget::applyZoomPreference()
{
    if (!hasDocument()) return;

    switch (m_zoomPref) {
    case 0: m_pdfView->setZoomMode(QPdfView::ZoomMode::FitInView); break;
    case 1: m_pdfView->setZoomMode(QPdfView::ZoomMode::FitToWidth); break;
    case 2: fitHeight(); break;
    default: break;
    }
}

void PdfPreviewWidget::clearDocument()
{
    QScrollBar *h = m_pdfView->horizontalScrollBar();
    QScrollBar *v = m_pdfView->verticalScrollBar();
    if (hasDocument()) {
        m_savedHFrac = (h && h->maximum() > 0) ? (qreal)h->value() / h->maximum() : 0;
        m_savedVFrac = (v && v->maximum() > 0) ? (qreal)v->value() / v->maximum() : 0;
        m_resumeScroll = true;
    }
    m_pdfDoc->close();
}

void PdfPreviewWidget::reloadDocument(const QString &path)
{
    QScrollBar *h = m_pdfView->horizontalScrollBar();
    QScrollBar *v = m_pdfView->verticalScrollBar();
    if (hasDocument()) {
        m_savedHFrac = (h && h->maximum() > 0) ? (qreal)h->value() / h->maximum() : 0;
        m_savedVFrac = (v && v->maximum() > 0) ? (qreal)v->value() / v->maximum() : 0;
        m_resumeScroll = true;
    }
    m_pdfDoc->load(path);
}

bool PdfPreviewWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_pdfView->viewport()) {
        if (event->type() == QEvent::Wheel) {
            QWheelEvent *we = static_cast<QWheelEvent *>(event);
            if (we->angleDelta().y() > 0)
                zoomIn();
            else if (we->angleDelta().y() < 0)
                zoomOut();
            return true;
        }

        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            if (me->button() == Qt::LeftButton) {
                m_panning = true;
                m_panStart = me->pos();
                m_pdfView->viewport()->setCursor(Qt::ClosedHandCursor);
                return true;
            }
        }

        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            if (me->button() == Qt::LeftButton && m_panning) {
                m_panning = false;
                m_pdfView->viewport()->setCursor(Qt::OpenHandCursor);
                return true;
            }
        }

        if (event->type() == QEvent::MouseMove && m_panning) {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            QPoint delta = m_panStart - me->pos();
            m_panStart = me->pos();
            if (m_pdfView->horizontalScrollBar())
                m_pdfView->horizontalScrollBar()->setValue(
                    m_pdfView->horizontalScrollBar()->value() + delta.x());
            if (m_pdfView->verticalScrollBar())
                m_pdfView->verticalScrollBar()->setValue(
                    m_pdfView->verticalScrollBar()->value() + delta.y());
            return true;
        }
    }

    return QWidget::eventFilter(obj, event);
}
