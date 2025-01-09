#ifndef OMITBUTTON
#define OMITBUTTON

#include <DPushButton>

namespace GrandSearch {

// 透明背景 icon可左可右
class OmitButton : public DTK_WIDGET_NAMESPACE::DPushButton
{
    Q_OBJECT
public:
    explicit OmitButton(QWidget * parent = nullptr);
    QSize sizeHint() const override;

public slots:
    void onUpdateSystemTheme();

protected:
    void paintEvent(QPaintEvent* e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void updateRectSize();
    QIcon setIconColor(QIcon icon, QColor color);

private:
    bool m_isHover            = false;
    bool m_isPress            = false;
    QIcon m_iconOrig;
};
}

#endif // OMITBUTTON
