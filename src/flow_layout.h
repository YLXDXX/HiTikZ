#pragma once
#include <QLayout>
#include <QList>
#include <QStyle>

class FlowLayout : public QLayout {
    Q_OBJECT
public:
    explicit FlowLayout(QWidget *parent, int margin = 0, int hSpacing = 4, int vSpacing = 4);
    ~FlowLayout();

    void addItem(QLayoutItem *item) override;
    int horizontalSpacing() const;
    int verticalSpacing() const;
    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int) const override;
    int count() const override;
    QLayoutItem *itemAt(int index) const override;
    QSize minimumSize() const override;
    void setGeometry(const QRect &rect) override;
    QSize sizeHint() const override;
    QLayoutItem *takeAt(int index) override;

    int rowCount() const { return m_rowCount; }

private:
    int doLayout(const QRect &rect, bool testOnly) const;
    int smartSpacing(QStyle::PixelMetric pm) const;

    QList<QLayoutItem *> m_items;
    int m_hSpace;
    int m_vSpace;
    mutable int m_rowCount = 1;
};
