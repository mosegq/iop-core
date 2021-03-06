// Copyright (c) 2015-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "platformstyle.h"

#include "guiconstants.h"
#include "guiutil.h"

#include <QApplication>
#include <QColor>
#include <QIcon>
#include <QImage>
#include <QPalette>
#include <QPixmap>
#include <QSettings>

static const struct {
    const char *platformId;
    /** Show images on push buttons */
    const bool imagesOnButtons;
    /** Colorize single-color icons */
    const bool colorizeIcons;
    /** Extra padding/spacing in transactionview */
    const bool useExtraSpacing;
} platform_styles[] = {
    {"macosx", false, true, true},
    {"windows", true, false, false},
    /* Other: linux, unix, ... */
    {"other", true, true, false}
};
static const unsigned platform_styles_count = sizeof(platform_styles)/sizeof(*platform_styles);

namespace {
/* Local functions for colorizing single-color images */

void MakeSingleColorImage(QImage& img, const QColor& colorbase)
{   
    if(GUIUtil::customThemeIsSet()) 
    { 
        QColor colorLeft = QColor(108,200,239);
        QColor colorMid = QColor(102,204,204);
        QColor colorRight = QColor(12,175,165);
        
        img = img.convertToFormat(QImage::Format_ARGB32);
        for (int x = img.width(); x--; )
        {
            for (int y = img.height(); y--; )
            {
                const QRgb rgb = img.pixel(x, y);
                QColor col;
                float r;
                if(y < x) {
                    r = (x*1.0/img.width()-y*1.0/img.height())*1.25;
                    col = QColor(
                        colorMid.red()* (1-r) + colorRight.red()*r,
                        colorMid.green()* (1-r) + colorRight.green()*r,
                        colorMid.blue()* (1-r) + colorRight.blue()*r,
                        255);
                } else {
                    r = (y*1.0/img.height()-x*1.0/img.width())*1.25;
                    col = QColor(
                        colorMid.red()* (1-r) + colorLeft.red()*r,
                        colorMid.green()* (1-r) + colorLeft.green()*r,
                        colorMid.blue()* (1-r) + colorLeft.blue()*r,
                        255);
                }
                img.setPixel(x, y, qRgba(col.red(), col.green(), col.blue(), qAlpha(rgb)));
            }
        }
    } else {
        img = img.convertToFormat(QImage::Format_ARGB32);
        for (int x = img.width(); x--; )
        {
            for (int y = img.height(); y--; )
            {
                const QRgb rgb = img.pixel(x, y);
                img.setPixel(x, y, qRgba(colorbase.red(), colorbase.green(), colorbase.blue(), qAlpha(rgb)));
            }
        }
    }
}

QIcon ColorizeIcon(const QIcon& ico, const QColor& colorbase)
{
    QIcon new_ico;
    for (const QSize sz : ico.availableSizes())
    {
        QImage img(ico.pixmap(sz).toImage());
        MakeSingleColorImage(img, colorbase);
        new_ico.addPixmap(QPixmap::fromImage(img));
    }
    return new_ico;
}

QImage ColorizeImage(const QString& filename, const QColor& colorbase)
{
    QImage img(filename);
    MakeSingleColorImage(img, colorbase);
    return img;
}

QIcon ColorizeIcon(const QString& filename, const QColor& colorbase)
{
    return QIcon(QPixmap::fromImage(ColorizeImage(filename, colorbase)));
}

}


PlatformStyle::PlatformStyle(const QString &_name, bool _imagesOnButtons, bool _colorizeIcons, bool _useExtraSpacing):
    name(_name),
    imagesOnButtons(_imagesOnButtons),
    colorizeIcons(_colorizeIcons),
    useExtraSpacing(_useExtraSpacing),
    singleColor(0,0,0),
    textColor(0,0,0)
{
    if(GUIUtil::customThemeIsSet()) 
    {   
        //TODO: may be obsolete
        //dark theme
        imagesOnButtons = true;
        colorizeIcons = true;
        singleColor = QColor(12,175,165); 
        textColor = QColor(12,175,165);         
    } else {
        //default light theme
        // Determine icon highlighting color
        if (colorizeIcons) {
            const QColor colorHighlightBg(QApplication::palette().color(QPalette::Highlight));
            const QColor colorHighlightFg(QApplication::palette().color(QPalette::HighlightedText));
            const QColor colorText(QApplication::palette().color(QPalette::WindowText));
            const int colorTextLightness = colorText.lightness();
            QColor colorbase;
            if (abs(colorHighlightBg.lightness() - colorTextLightness) < abs(colorHighlightFg.lightness() - colorTextLightness))
                colorbase = colorHighlightBg;
            else
                colorbase = colorHighlightFg;
            singleColor = colorbase;
        }
        // Determine text color
        textColor = QColor(QApplication::palette().color(QPalette::WindowText));
    }
}

QImage PlatformStyle::SingleColorImage(const QString& filename) const
{
    if (!colorizeIcons)
        return QImage(filename);
    return ColorizeImage(filename, SingleColor());
}

QIcon PlatformStyle::SingleColorIcon(const QString& filename) const
{
    if (!colorizeIcons)
        return QIcon(filename);
    return ColorizeIcon(filename, SingleColor());
}

QIcon PlatformStyle::SingleColorIcon(const QIcon& icon) const
{
    if (!colorizeIcons)
        return icon;
    return ColorizeIcon(icon, SingleColor());
}

QIcon PlatformStyle::TextColorIcon(const QString& filename) const
{
    return ColorizeIcon(filename, TextColor());
}

QIcon PlatformStyle::TextColorIcon(const QIcon& icon) const
{
    return ColorizeIcon(icon, TextColor());
}

const PlatformStyle *PlatformStyle::instantiate(const QString &platformId)
{
    for (unsigned x=0; x<platform_styles_count; ++x)
    {
        if (platformId == platform_styles[x].platformId)
        {
            return new PlatformStyle(
                    platform_styles[x].platformId,
                    platform_styles[x].imagesOnButtons,
                    platform_styles[x].colorizeIcons,
                    platform_styles[x].useExtraSpacing);
        }
    }
    return 0;
}

