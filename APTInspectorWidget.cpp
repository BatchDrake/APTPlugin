//
//    APTInspectorWidget.cpp: APT inspector tab widget
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
#include "APTInspectorWidget.h"
#include "ui_APTInspectorWidget.h"
#include <QFileDialog>
#include <analyzer/inspector/params.h>
#include <SuWidgetsHelpers.h>
#include <QMessageBox>
#include <SigDiggerHelpers.h>

#define STRINGFY(x) #x
#define STORE(field) obj.set(STRINGFY(field), this->field)
#define LOAD(field) this->field = conf.get(STRINGFY(field), this->field)


void
APTInspectorWidgetConfig::deserialize(Suscan::Object const &conf)
{
  LOAD(falseColor);
  LOAD(channel);
  LOAD(palette);
  LOAD(saveDir);
}

Suscan::Object &&
APTInspectorWidgetConfig::serialize(void)
{
  Suscan::Object obj(SUSCAN_OBJECT_TYPE_OBJECT);

  obj.setClass("APTInspectorWidgetConfig");

  STORE(falseColor);
  STORE(channel);
  STORE(palette);
  STORE(saveDir);

  return this->persist(obj);
}


SUBOOL
DecoderUI_onSync(struct sigutils_apt_decoder *, void *userdata, SUSCOUNT)
{
  APTInspectorWidget *ui = reinterpret_cast<APTInspectorWidget *>(userdata);

  ui->flashLed(ui->ui->syncLed, DECODER_UI_FLASH_TIMEOUT_MS);

  return SU_TRUE;
}

SUBOOL
DecoderUI_onCarrier(struct sigutils_apt_decoder *, void *userdata, SUFLOAT snr)
{
  APTInspectorWidget *ui = reinterpret_cast<APTInspectorWidget *>(userdata);

  ui->ui->carrierLed->setState(
        snr > SU_APT_MIN_CARRIER_DB
        ? KLed::On
        : KLed::Off);

  return SU_TRUE;
}

SUBOOL
DecoderUI_onLine(struct sigutils_apt_decoder *, void *, SUFLOAT)
{
  return SU_TRUE;
}

SUBOOL
DecoderUI_onLineData(
    struct sigutils_apt_decoder *,
    void *userdata,
    SUSCOUNT,
    enum sigutils_apt_decoder_channel ch,
    SUBOOL highsnr,
    const uint8_t *data,
    size_t size)
{
  APTInspectorWidget *ui = reinterpret_cast<APTInspectorWidget *>(userdata);
  MapWidget::Channel channel = MapWidget::MAP_WIDGET_CHANNEL_A;

  switch (ch) {
    case SU_APT_DECODER_CHANNEL_A:
      channel = MapWidget::MAP_WIDGET_CHANNEL_A;
      ui->flashLed(ui->ui->channelALed, DECODER_UI_FLASH_TIMEOUT_MS);
      break;

    case SU_APT_DECODER_CHANNEL_B:
      channel = MapWidget::MAP_WIDGET_CHANNEL_B;
      ui->flashLed(ui->ui->channelBLed, DECODER_UI_FLASH_TIMEOUT_MS);
      break;
  }

  ui->ensureMapArea();
  ui->m_mapArea->pushLine(channel, data, size, highsnr);
  ui->adjustScrollbar();

  ui->ui->statusLabel->setText(
        "Decoded " + QString::number(ui->m_mapArea->lines()) + " lines");

  return SU_TRUE;
}

////////////////////////////// Private members ////////////////////////////////
void
APTInspectorWidget::adjustScrollbar()
{
  if (m_mapArea != nullptr) {
    if (m_mapArea->viewPortLines() < m_mapArea->lines()) {
      this->ui->offsetScrollBar->setMaximum(
            m_mapArea->lines() - m_mapArea->viewPortLines());
      this->ui->offsetScrollBar->setPageStep(m_mapArea->viewPortLines());
    } else {
      this->ui->offsetScrollBar->setMaximum(0);
      this->ui->offsetScrollBar->setPageStep(0);
    }
  } else {
    this->ui->offsetScrollBar->setMaximum(0);
    this->ui->offsetScrollBar->setPageStep(0);
  }

  this->ui->offsetScrollBar->setEnabled(
        this->ui->offsetScrollBar->maximum() > 0);
}

void
APTInspectorWidget::ensureMapArea(void)
{
  if (m_mapArea == nullptr) {
    int index;
    m_mapArea = new MapWidget(this);
    index = this->ui->mapStack->addWidget(m_mapArea);
    this->ui->mapStack->setCurrentIndex(index);

    this->onPaletteChanged();

    if (this->ui->channelAButton->isChecked())
      m_mapArea->setCurrentChannel(MapWidget::MAP_WIDGET_CHANNEL_A);
    else
      m_mapArea->setCurrentChannel(MapWidget::MAP_WIDGET_CHANNEL_B);

    m_mapArea->setFalseColorEnabled(
          this->ui->falseColorCheck->isChecked());

    connect(
          m_mapArea,
          SIGNAL(geometryChanged()),
          this,
          SLOT(onGeometryChanged()));

    connect(
          m_mapArea,
          SIGNAL(offsetChanged(int)),
          this,
          SLOT(onOffsetChanged(int)));
  }
}

void
APTInspectorWidget::flashLed(KLed *led, unsigned int ms)
{
  QTimer *timer = nullptr;

  if (led == this->ui->syncLed)
    timer = &this->syncTimer;
  else if (led == this->ui->channelALed)
    timer = &this->channelATimer;
  else if (led == this->ui->channelBLed)
    timer = &this->channelBTimer;

  if (timer != nullptr) {
    timer->setSingleShot(true);
    timer->setInterval(ms);
    timer->setProperty("led", QVariant::fromValue<KLed *>(led));
    led->setState(KLed::On);
    timer->start();
  }
}

void
APTInspectorWidget::connectUi(void)
{
  connect(
        &this->syncTimer,
        SIGNAL(timeout()),
        this,
        SLOT(onLedTimeout()));

  connect(
        &this->channelATimer,
        SIGNAL(timeout()),
        this,
        SLOT(onLedTimeout()));

  connect(
        &this->channelBTimer,
        SIGNAL(timeout()),
        this,
        SLOT(onLedTimeout()));

  connect(
        this->ui->channelAButton,
        SIGNAL(toggled(bool)),
        this,
        SLOT(onChannelChanged()));

  connect(
        this->ui->channelBButton,
        SIGNAL(toggled(bool)),
        this,
        SLOT(onChannelChanged()));

  connect(
        this->ui->saveButton,
        SIGNAL(clicked(bool)),
        this,
        SLOT(onSave()));

  connect(
        this->ui->falseColorCheck,
        SIGNAL(clicked(bool)),
        this,
        SLOT(onFalseColorToggled()));


  connect(
        this->ui->snrSpin,
        SIGNAL(valueChanged(double)),
        this,
        SLOT(onSNRChanged()));

  connect(
        this->ui->offsetScrollBar,
        SIGNAL(valueChanged(int)),
        this,
        SLOT(onScroll()));

  connect(
        this->ui->paletteCombo,
        SIGNAL(activated(int)),
        this,
        SLOT(onPaletteChanged()));
}

//////////////////////////// External API /////////////////////////////////////
void
APTInspectorWidget::feed(const SUCOMPLEX *data, size_t len)
{
  su_apt_decoder_feed_ex(m_decoder, m_phase, data, len);
}

APTInspectorWidget::APTInspectorWidget(
    SigDigger::InspectionWidgetFactory *factory,
    Suscan::AnalyzerRequest const &request,
    SigDigger::UIMediator *mediator,
    QWidget *parent) :
  InspectionWidget(factory, request, mediator, parent),
  ui(new Ui::DecoderUI)
{
  ui->setupUi(this);

  SigDigger::SigDiggerHelpers::instance()->populatePaletteCombo(
        this->ui->paletteCombo);

  this->connectUi();
}

// Other overriden methods
std::string
APTInspectorWidget::getLabel() const
{
  return "APT inspector";
}

Suscan::Serializable *
APTInspectorWidget::allocConfig()
{
  m_widgetConfig = new APTInspectorWidgetConfig();

  return m_widgetConfig;
}

void
APTInspectorWidget::applyConfig()
{
  int index;

  this->ui->channelAButton->setChecked(m_widgetConfig->channel == 0);
  this->ui->channelBButton->setChecked(m_widgetConfig->channel == 1);

  this->ui->falseColorCheck->setChecked(m_widgetConfig->falseColor);

  index = this->ui->paletteCombo->findText(
        QString::fromStdString(m_widgetConfig->palette));
  if (index != -1)
    this->ui->paletteCombo->setCurrentIndex(index);

  this->onFalseColorToggled();
  this->onPaletteChanged();
}

// Overridable methods
void
APTInspectorWidget::attachAnalyzer(Suscan::Analyzer *analyzer)
{
  Suscan::Config cfg(this->config().getInstance());

  if (cfg.hasPrefix("audio")) {
    cfg.set("audio.cutoff", SCAST(SUFLOAT, DECODER_AUDIO_CUTOFF));
    cfg.set("audio.volume", 1.f); // We handle this at UI level
    cfg.set("audio.sample-rate", SCAST(uint64_t, DECODER_PREFERRED_SAMPLE_RATE));
    cfg.set("audio.demodulator", 2ul); // 2 is FM
    cfg.set("audio.squelch", false);
    cfg.set("audio.squelch-level", 0.f);
  } else {
    // Manual clock recovery
    cfg.set(
          "clock.type",
          SCAST(uint64_t, SUSCAN_INSPECTOR_BAUDRATE_CONTROL_MANUAL));

    // Agree on sample rate
    cfg.set(
          "clock.baud",
          SCAST(SUFLOAT, DECODER_PREFERRED_SAMPLE_RATE));

    cfg.set("clock.running", true);

    // Enable matched filter for better noise rejection
    cfg.set(
          "mf.type",
          SCAST(uint64_t, SUSCAN_INSPECTOR_MATCHED_FILTER_MANUAL));

    cfg.set(
          "mf.roll-off",
          SCAST(float, 0));

    if (cfg.hasPrefix("agc."))
      cfg.set("agc.enabled", false);

    // If enabled from ASK, disable PLL (APT has one on its own)
    if (cfg.hasPrefix("ask.")) {
      m_phase = false;
      cfg.set("ask.use-pll", false);
    } else {
      m_phase = true;
    }

    // If enabled from FSK, enable quadrature demodulator
    if (cfg.hasPrefix("fsk."))
      cfg.set("fsk.quad-demod", true);
  }

  // Request change and wait for acknowledgement
  analyzer->setInspectorConfig(this->request().handle, cfg);
}

bool
APTInspectorWidget::createDecoder()
{
  sigutils_apt_decoder_callbacks cb =
      sigutils_apt_decoder_callbacks_INITIALIZER;

  cb.userdata     = this;
  cb.on_carrier   = DecoderUI_onCarrier;
  cb.on_sync      = DecoderUI_onSync;
  cb.on_line      = DecoderUI_onLine;
  cb.on_line_data = DecoderUI_onLineData;

  m_decoder = su_apt_decoder_new(m_sampleRate, &cb);

  if (m_decoder == nullptr)
    return false;

  su_apt_decoder_set_snr(
        m_decoder,
        SU_POWER_MAG_RAW(this->ui->snrSpin->value()));

  this->ui->sampleRateSpin->setText(
        QString::number(
          SCAST(unsigned, m_sampleRate)) + " sp/s");

  return true;
}

void
APTInspectorWidget::destroyDecoder()
{
  if (m_decoder !=  nullptr) {
    this->ui->sampleRateSpin->setText("N / A");
    su_apt_decoder_destroy(m_decoder);
    m_decoder = nullptr;
    m_sampleRate = 0;
  }
}

void
APTInspectorWidget::detachAnalyzer()
{
  this->destroyDecoder();
}

void
APTInspectorWidget::inspectorMessage(Suscan::InspectorMessage const &msg)
{
  if (msg.getKind() == SUSCAN_ANALYZER_INSPECTOR_MSGKIND_SET_CONFIG) {
    Suscan::Config config(msg.getCConfig());
    float reportedRate = 0;

    if (config.get("clock.baud") != nullptr)
      reportedRate = config.get("clock.baud")->getFloat();
    else if (config.get("audio.sample-rate"))
      reportedRate = SCAST(float, config.get("audio.sample-rate")->getUint64());

    if (!sufreleq(m_sampleRate, reportedRate, 1e-3f)) {
      this->destroyDecoder();
      m_sampleRate = reportedRate;
    }

    if (!sufreleq(m_sampleRate, 0, 1e-3f)) {
      if (!this->createDecoder()) {
        QMessageBox::critical(
              this,
              "Failed to create decoder",
              "APT decoder failed to initialize, please verify that the "
              "channel bandwidth is wide enough and try again");
      }
    }
  }
}

void
APTInspectorWidget::samplesMessage(Suscan::SamplesMessage const &msg)
{
  if (m_decoder != nullptr)
    this->feed(msg.getSamples(), msg.getCount());
}


APTInspectorWidget::~APTInspectorWidget()
{
  this->destroyDecoder();

  delete ui;
}

//////////////////////////////// Slots /////////////////////////////////////////
void
APTInspectorWidget::onLedTimeout(void)
{
  QTimer *sender = static_cast<QTimer *>(QObject::sender());
  KLed *led = sender->property("led").value<KLed *>();

  led->setState(KLed::Off);
}

void
APTInspectorWidget::onChannelChanged(void)
{
  int channel =
      this->ui->channelAButton->isChecked() ? 0 : 1;

  m_widgetConfig->channel = channel;

  if (m_mapArea != nullptr) {
    if (channel == 0)
      m_mapArea->setCurrentChannel(MapWidget::MAP_WIDGET_CHANNEL_A);
    else
      m_mapArea->setCurrentChannel(MapWidget::MAP_WIDGET_CHANNEL_B);
  }

  this->update();
}

void
APTInspectorWidget::onSNRChanged(void)
{
  su_apt_decoder_set_snr(
        m_decoder,
        SU_POWER_MAG_RAW(this->ui->snrSpin->value()));
}

void
APTInspectorWidget::onScroll(void)
{
  if (m_mapArea != nullptr)
    m_mapArea->setOffset(this->ui->offsetScrollBar->value());
}

void
APTInspectorWidget::onSave(void)
{
  QString fileName;

  fileName = QFileDialog::getSaveFileName(
        this,
        "Save file",
        QString::fromStdString(m_widgetConfig->saveDir),
        "Image files (*.png)");

  if (fileName.size() > 0 && m_mapArea != nullptr) {
    QFileInfo fi(fileName);
    m_widgetConfig->saveDir = fi.absolutePath().toStdString();
    m_mapArea->saveTo(fileName);
  }
}

void
APTInspectorWidget::onReset(void)
{

}

void
APTInspectorWidget::onGeometryChanged()
{
  this->adjustScrollbar();
}

void
APTInspectorWidget::onOffsetChanged(int val)
{
  this->ui->offsetScrollBar->setValue(val);
}

void
APTInspectorWidget::onPaletteChanged()
{
  int index = this->ui->paletteCombo->currentIndex();

  if (index != -1) {
    const QColor *gradient =
        SigDigger::SigDiggerHelpers::instance()->getPalette(index)->getGradient();

    if (gradient != nullptr && m_mapArea != nullptr) {
      m_mapArea->setGradient(gradient);
      m_widgetConfig->palette =
          this->ui->paletteCombo->currentText().toStdString();
    }
  }
}

void
APTInspectorWidget::onFalseColorToggled()
{
  bool falseColor = this->ui->falseColorCheck->isChecked();

  if (m_mapArea != nullptr)
    m_mapArea->setFalseColorEnabled(falseColor);

  this->ui->channelAButton->setEnabled(!falseColor);
  this->ui->channelBButton->setEnabled(!falseColor);

  m_widgetConfig->falseColor = falseColor;

  this->ui->paletteCombo->setEnabled(falseColor);
}
