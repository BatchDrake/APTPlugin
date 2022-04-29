//
//    DecoderUI.h: APT inspector widget implementation
//    Copyright (C) 2022 Gonzalo José Carracedo Carballal
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as
//    published by the Free Software Foundation, either version 3 of the
//    License, or (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful, but
//    WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this program.  If not, see
//    <http://www.gnu.org/licenses/>
//
#ifndef APTINSPECTORWIDGET_H
#define APTINSPECTORWIDGET_H

#include <QWidget>
#include <QTimer>
#include <sigutils/specific/apt.h>
#include <vector>
#include <MapWidget.h>
#include <SigDigger/InspectionWidgetFactory.h>

#define DECODER_UI_FLASH_TIMEOUT_MS   250
#define DECODER_PREFERRED_SAMPLE_RATE 44100
#define DECODER_AUDIO_CUTOFF          9000

class KLed;

namespace Ui {
  class DecoderUI;
}

SUBOOL DecoderUI_onSync(struct sigutils_apt_decoder *, void *, SUSCOUNT);
SUBOOL DecoderUI_onLine(struct sigutils_apt_decoder *, void *, SUFLOAT);
SUBOOL DecoderUI_onLineData(
    struct sigutils_apt_decoder *,
    void *,
    SUSCOUNT,
    enum sigutils_apt_decoder_channel,
    SUBOOL,
    const uint8_t *,
    size_t);

struct APTInspectorWidgetConfig : public Suscan::Serializable {
  bool falseColor = true;
  int  channel = 0;
  std::string palette = "Inferno (Feely)";
  std::string saveDir = ".";
  void deserialize(Suscan::Object const &conf) override;
  Suscan::Object &&serialize() override;
};

class APTInspectorWidget : public SigDigger::InspectionWidget
{
    Q_OBJECT

    su_apt_decoder_t *m_decoder    = nullptr;
    MapWidget        *m_mapArea    = nullptr;
    bool              m_phase      = false;
    float             m_sampleRate = 0;

    APTInspectorWidgetConfig *m_widgetConfig = nullptr;

    QTimer syncTimer;
    QTimer channelATimer;
    QTimer channelBTimer;

    void adjustScrollbar();
    void flashLed(KLed *, unsigned int ms);
    void ensureMapArea();
    void connectUi();
    void feed(const SUCOMPLEX *data, size_t len);
    void destroyDecoder();
    bool createDecoder();

  public:
    APTInspectorWidget(
        SigDigger::InspectionWidgetFactory *,
        Suscan::AnalyzerRequest const &,
        SigDigger::UIMediator *,
        QWidget *parent = nullptr);

    friend SUBOOL DecoderUI_onCarrier(struct sigutils_apt_decoder *, void *, SUFLOAT);
    friend SUBOOL DecoderUI_onSync(struct sigutils_apt_decoder *, void *, SUSCOUNT);
    friend SUBOOL DecoderUI_onLine(struct sigutils_apt_decoder *, void *, SUFLOAT);
    friend SUBOOL DecoderUI_onLineData(
        struct sigutils_apt_decoder *,
        void *,
        SUSCOUNT,
        enum sigutils_apt_decoder_channel,
        SUBOOL,
        const uint8_t *,
        size_t);

    ~APTInspectorWidget() override;

    // Other overriden methods
    std::string getLabel() const override;

    // Overriden config methods
    Suscan::Serializable *allocConfig() override;
    void applyConfig() override;

    // Overridable methods
    void attachAnalyzer(Suscan::Analyzer *) override;
    void detachAnalyzer() override;
    void inspectorMessage(Suscan::InspectorMessage const &) override;
    void samplesMessage(Suscan::SamplesMessage const &) override;

  public slots:
    void onLedTimeout();
    void onChannelChanged();
    void onSNRChanged();
    void onScroll();
    void onSave();
    void onReset();
    void onGeometryChanged();
    void onOffsetChanged(int);
    void onFalseColorToggled();
    void onPaletteChanged();

  private:
    Ui::DecoderUI *ui;
};

#endif // APTINSPECTORWIDGET_H
