// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Unit test for IconButton::paintEvent override (BUG-197177).
// Tests the real IconButton class from generaltoolbar.h, verifying
// that the custom paintEvent produces rendering distinct from the
// default QToolButton and does not crash in normal / text-only states.

#include <gtest/gtest.h>

#include <QApplication>
#include <QToolButton>
#include <QPixmap>
#include <QImage>
#include <QColor>
#include <QPainter>
#include <QIcon>

#include <DWidget>
#include <DStyle>
#include <DGuiApplicationHelper>
#include <DPlatformTheme>
#include <DFontSizeManager>

#include "generalwidget/generaltoolbar.h"
#include "utils/utils.h"

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

// Stub for Utils::iconThemeSuffix() — GeneralToolBar::initUi() calls this,
// but we only test IconButton; provide a no-op to satisfy the linker.
namespace GrandSearch {
QString Utils::iconThemeSuffix() { return QString(); }
}

// 1. IconButton renders differently from default QToolButton.
TEST(IconButtonPaintEvent, ProducesDifferentRendering)
{
    GrandSearch::IconButton customBtn;
    customBtn.setText("Open");
    customBtn.resize(80, 36);

    QToolButton defaultBtn;
    defaultBtn.setText("Open");
    defaultBtn.resize(80, 36);

    QPixmap customPix(customBtn.size());
    customBtn.render(&customPix);
    QPixmap defaultPix(defaultBtn.size());
    defaultBtn.render(&defaultPix);

    QImage customImg = customPix.toImage();
    QImage defaultImg = defaultPix.toImage();

    int diffPixels = 0;
    for (int y = 0; y < customImg.height(); ++y) {
        for (int x = 0; x < customImg.width(); ++x) {
            if (customImg.pixelColor(x, y) != defaultImg.pixelColor(x, y))
                ++diffPixels;
        }
    }
    EXPECT_GT(diffPixels, 0)
        << "IconButton paintEvent should produce different rendering from default QToolButton";
}

// 2. IconButton does not crash in normal state.
TEST(IconButtonPaintEvent, NormalStateNoCrash)
{
    GrandSearch::IconButton btn;
    btn.setText("Open Path");
    btn.resize(108, 36);
    QPixmap pix(btn.size());
    EXPECT_NO_FATAL_FAILURE(btn.render(&pix));
    EXPECT_FALSE(pix.toImage().isNull());
}

// 3. IconButton handles text-only rendering (no icon).
TEST(IconButtonPaintEvent, TextOnlyRenders)
{
    GrandSearch::IconButton btn;
    btn.setText("Copy Path");
    btn.setToolButtonStyle(Qt::ToolButtonTextOnly);
    btn.resize(108, 36);
    QPixmap pix(btn.size());
    EXPECT_NO_FATAL_FAILURE(btn.render(&pix));
    EXPECT_FALSE(pix.toImage().isNull());
}

namespace {
// Render an IconButton in a given interaction state onto an opaque white
// background, so the semi-transparent hover/press overlays can be compared.
// `iconColor` is used for the solid icon; it lets callers pick a color that
// contrasts with the system active color to make the hover/press tint visible.
QImage renderButtonState(bool hover, bool press, const QColor &iconColor = Qt::black)
{
    GrandSearch::IconButton btn;
    btn.setText("Open");
    btn.setIconSize(QSize(16, 16));
    btn.resize(80, 36);

    QPixmap iconPix(16, 16);
    iconPix.fill(Qt::transparent);
    {
        QPainter ip(&iconPix);
        ip.setPen(Qt::NoPen);
        ip.setBrush(iconColor);
        ip.drawRect(2, 2, 12, 12);
    }
    btn.setIcon(QIcon(iconPix));

    // underMouse() reads WA_UnderMouse; DStyle::getState() additionally
    // requires WA_Hover to report hover/press instead of normal.
    if (hover || press) {
        btn.setAttribute(Qt::WA_Hover, true);
        btn.setAttribute(Qt::WA_UnderMouse, true);
    }
    if (press)
        btn.setDown(true);

    btn.ensurePolished();

    QImage canvas(btn.size(), QImage::Format_ARGB32_Premultiplied);
    canvas.fill(Qt::white);
    QPainter bp(&canvas);
    btn.render(&bp);
    bp.end();
    return canvas;
}

int countNearColor(const QImage &img, const QColor &target, int tol = 16)
{
    int count = 0;
    for (int y = 0; y < img.height(); ++y) {
        for (int x = 0; x < img.width(); ++x) {
            const QColor c = img.pixelColor(x, y);
            if (qAbs(c.red() - target.red()) <= tol
                && qAbs(c.green() - target.green()) <= tol
                && qAbs(c.blue() - target.blue()) <= tol)
                ++count;
        }
    }
    return count;
}

bool colorsClose(const QColor &a, const QColor &b)
{
    return qAbs(a.red() - b.red()) <= 40 && qAbs(a.green() - b.green()) <= 40
            && qAbs(a.blue() - b.blue()) <= 40;
}
}   // namespace

// 4. Hover state paints a darker (5% black) overlay than the normal state.
TEST(IconButtonPaintEvent, HoverStatePaintsOverlay)
{
    const QImage normal = renderButtonState(false, false);
    const QImage hover = renderButtonState(true, false);
    // Top-centre pixel is background (icon and text are vertically centred).
    const QPoint probe(40, 3);
    EXPECT_LT(qGray(hover.pixel(probe)), qGray(normal.pixel(probe)))
        << "hover should paint a darker background overlay";
}

// 5. Press state paints a darker (10% black) overlay than hover.
TEST(IconButtonPaintEvent, PressStatePaintsDeeperOverlay)
{
    const QImage hover = renderButtonState(true, false);
    const QImage press = renderButtonState(true, true);
    const QPoint probe(40, 3);
    EXPECT_LT(qGray(press.pixel(probe)), qGray(hover.pixel(probe)))
        << "press should paint a deeper background overlay than hover";
}

// 6. Both hover and press tint the icon with the system active color
//    (BUG-197177: hover previously kept the default icon/text colors).
TEST(IconButtonPaintEvent, HoverAndPressUseActiveColor)
{
    const QColor active =
        DGuiApplicationHelper::instance()->systemTheme()->activeColor();

    // Use an icon color that contrasts with the active color so the tint is
    // detectable regardless of what the platform reports.
    const QColor iconColor = colorsClose(active, QColor(255, 0, 255))
            ? QColor(0, 255, 0)   // green when active is magenta
            : QColor(255, 0, 255);   // magenta otherwise

    const QImage normal = renderButtonState(false, false, iconColor);
    const QImage hover = renderButtonState(true, false, iconColor);
    const QImage press = renderButtonState(true, true, iconColor);

    if (colorsClose(active, iconColor)) {
        // Should not happen given the choice above, but keep the test sane.
        EXPECT_FALSE(hover.isNull());
        return;
    }

    const int normalActive = countNearColor(normal, active);
    const int hoverActive = countNearColor(hover, active);
    const int pressActive = countNearColor(press, active);

    EXPECT_EQ(normalActive, 0)
        << "normal state must not use the active color for the icon";
    EXPECT_GT(hoverActive, 0)
        << "hover state should tint the icon with the active color (" << active.name().toStdString() << ")";
    EXPECT_GT(pressActive, 0)
        << "press state should tint the icon with the active color (" << active.name().toStdString() << ")";
}
