#pragma once
#include <QWidget>
#include <QPdfDocument>
#include <QPdfView>

class PdfPreviewWidget : public QWidget {
    Q_OBJECT
public:
    explicit PdfPreviewWidget(QWidget *parent = nullptr);

    QPdfDocument *document() const;
    QPdfView *pdfView() const;

    int zoomPreference() const;
    void setZoomPreference(int pref);

    bool hasDocument() const;

public slots:
    void zoomIn();
    void zoomOut();
    void fitPage();
    void fitWidth();
    void fitHeight();
    void applyZoomPreference();
    void clearDocument();
    void reloadDocument(const QString &path);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QPdfView *m_pdfView;
    QPdfDocument *m_pdfDoc;
    bool m_panning = false;
    QPoint m_panStart;
    int m_zoomPref = 0;
    qreal m_savedHFrac = 0;
    qreal m_savedVFrac = 0;
    bool m_resumeScroll = false;
};

