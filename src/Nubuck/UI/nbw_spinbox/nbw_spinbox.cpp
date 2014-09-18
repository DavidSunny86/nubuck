#include <assert.h>

#include <Nubuck\common\common.h>

#include <QVBoxLayout>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QSvgRenderer>
#include <QPushButton>
#include <QStyle>

#include "nbw_spinbox_controls.h"
#include "nbw_spinbox.h"

const float NBW_SpinBoxControls::s_padding = 5.0f;
const QSizeF NBW_SpinBoxControls::s_arrowSize(20.0f, 20.0f);

// accepted formats:
// "123", int, no leading 0
// "-0.123", double
// 12 / 3, rational
static bool ParseRational(const char* str, leda::rational& rval, int& typeFlag) {
    int num = 0;
    int denom = 0;
    double pow = 0.1;
    double frac = 0.0;

    const char* ptr = str;

    // ignore leading whitespace
    while('\0' != *ptr && isspace(*ptr)) ptr++;

    enum { 
        TYPE_INTEGER    = NBW_SpinBox::TypeFlags::INTEGER,
        TYPE_RATIONAL   = NBW_SpinBox::TypeFlags::RATIONAL,
        TYPE_DOUBLE     = NBW_SpinBox::TypeFlags::DOUBLE
    };
    int type;

    bool isNumNeg = false;
    bool isDenomNeg = false;

    int nextState, state = 1;

    char c;
    while('\0' != (c = *ptr)) {
        nextState = state;

        if(1 == state) {
            if('-' == c) {
                if(isNumNeg) {
                    common.printf("ERROR - ParseRational, state = %d\n", state);
                    common.printf("... expected DIGIT, got '%c'\n", c);
                    nextState = 0;
                } else {
                    isNumNeg = true;
                }
            } else if('0' == c) {
                type = TYPE_INTEGER;
                nextState = 2;
            } else if(isdigit(c)) {
                type = TYPE_INTEGER;
                num = c - '0';
                nextState = 4;
            } else {
                common.printf("ERROR - ParseRational, state = %d\n", state);
                common.printf("... expected '-' or DIGIT, got '%c'\n", c);
                nextState = 0;
            }
        }

        if(2 == state) {
            if('.' == c) {
                type = TYPE_DOUBLE;
                nextState = 3;
            } else if(isspace(c)) {
                nextState = 5;
            } else {
                common.printf("ERROR - ParseRational, state = %d\n", state);
                common.printf("... expected '.' or SPACE, got '%c'\n", c);
                nextState = 0;
            }
        }

        if(3 == state) {
            if(isdigit(c)) {
                frac += pow * (c - '0');
                pow /= 10.0;
            } else if(isspace(c)) {
                nextState = 9;
            } else {
                common.printf("ERROR - ParseRational, state = %d\n", state);
                common.printf("... expected DIGIT or SPACE, got '%c'\n", c);
                nextState = 0;
            }
        }

        if(4 == state) {
            if(isdigit(c)) {
                num *= 10;
                num += c - '0';
            } else if('.' == c) {
                type = TYPE_DOUBLE;
                nextState = 3;
            } else if(isspace(c)) {
                nextState = 5;
            } else {
                common.printf("ERROR - ParseRational, state = %d\n", state);
                common.printf("... expected DIGIT, '.' or SPACE, got '%c'\n", c);
                nextState = 0;
            }
        }

        if(5 == state) {
            if('/' == c) {
                type = TYPE_RATIONAL;
                nextState = 6;
            }
            else if(!isspace(c)) {
                common.printf("ERROR - ParseRational, state = %d\n", state);
                common.printf("... expected '/' or SPACE, got '%c'\n", c);
                nextState = 0;
            }
        }

        if(6 == state) {
            if('-' == c) {
                isDenomNeg = true;
                nextState = 7;
            } else if(isdigit(c) && '0' != c) {
                denom = c - '0';
                nextState = 8;
            } else if(!isspace(c)) {
                common.printf("ERROR - ParseRational, state = %d\n", state);
                common.printf("... expected '-', DIGIT > 0 or SPACE got '%c'\n", c);
                nextState = 0;
            }
        }

        if(7 == state) {
            if(isdigit(c) && '0' != c) {
                denom = c - '0';
                nextState = 8;
            } else {
                common.printf("ERROR - ParseRational, state = %d\n", state);
                common.printf("... expected DIGIT > 0, got '%c'\n", c);
                nextState = 0;
            }
        }

        if(8 == state) {
            if(isdigit(c)) {
                denom *= 10;
                denom += c - '0';
            } else if(isspace(c)) {
                nextState = 9;
            }else {
                common.printf("ERROR - ParseRational, state = %d\n", state);
                common.printf("... expected DIGIT, got '%c'\n", c);
                nextState = 0;
            }
        }

        if(9 == state) {
            if(!isspace(c)) {
                common.printf("ERROR - ParseRational, state = %d\n", state);
                common.printf("... expected SPACE, got '%c'\n", c);
                nextState = 0;
            }
        }

        state = nextState;
        ptr++;
    }

    if(2 == state || 3 == state || 4 == state || 5 == state || 8 == state || 9 == state) {
        typeFlag = type;

        if(TYPE_INTEGER == type) {
            if(isNumNeg) num *= -1;

            rval = leda::rational(leda::integer(num));
        } else if(TYPE_RATIONAL == type) {
            if(isNumNeg) num *= -1;
            if(isDenomNeg) denom *= -1;

            rval = leda::rational(num, denom);
        } else if(TYPE_DOUBLE == type) {
            double dval = num + frac;
            if(isNumNeg) dval *= -1.0;

            rval = leda::rational(dval);
        }
        return true;
    }

    return false;
}

void NBW_SpinBoxControls::SetValue(const leda::rational rval) {
    leda::rational clamped = leda::max(_min, leda::min(_max, rval));
    if(clamped != _rval) {
        _rval = clamped; 
        emit SigValueChanged();
    }
}

void NBW_SpinBoxControls::BeginDragging() {
    setProperty("dragging", true);
    style()->polish(this);
    emit SigBeginDragging();
}

void NBW_SpinBoxControls::EndDragging() {
    setProperty("dragging", false);
    style()->polish(this);
    emit SigEndDragging();
}

void NBW_SpinBoxControls::OnEditingFinished() {
    leda::rational rval;
    int typeFlag;
    bool success = ParseRational(_lineEdit->text().toAscii(), rval, typeFlag);
    if(success && typeFlag & _typeMask) {
        SetValue(rval);
    }

    _state = STATE_NORMAL;

    _lineEdit->hide();

    EndDragging();
}

void NBW_SpinBoxControls::resizeEvent(QResizeEvent* event) {
    const float wndWidth = event->size().width();
    const float wndHeight = event->size().height();

    const float centerY = 0.5f * (wndHeight - s_arrowSize.height());

    const QPointF lorig(s_padding, centerY);
    _arrowRegions[LEFT_ARROW] = QRectF(lorig, s_arrowSize);

    const QPointF rorig(wndWidth - (s_padding + s_arrowSize.width()), centerY);
    _arrowRegions[RIGHT_ARROW] = QRectF(rorig, s_arrowSize);

    _textRegion = QRectF(
        s_padding + s_arrowSize.width(),
        0.0f,
        wndWidth - 2.0f * (s_padding + s_arrowSize.width()),
        wndHeight);

    _lineEdit->setGeometry(_textRegion.toRect());
}

static float Length(const QPointF& p) {
    return sqrtf(p.x() * p.x() + p.y() * p.y());
}

static int Sign(int val) {
    if(0 < val) return 1;
    else if(0 > val) return -1;
    else return 0;
}

void NBW_SpinBoxControls::mouseMoveEvent(QMouseEvent* event) {
    if(STATE_DRAGGING == _state) {
        _distAccu += event->pos() - _dragOrigin;

        leda::rational rval1 = _rval;

        int dist = 50;

        int accu = M::Abs(_distAccu.x());
        int sign = Sign(_distAccu.x());
        while(dist < accu) {
            leda::rational rval = rval1 + sign * _singleStep;
            rval1 = leda::max(_min, leda::min(_max, rval));
            accu -= dist;
        }
        _distAccu.setX(sign * accu);

        accu = M::Abs(_distAccu.y());
        sign = Sign(_distAccu.y());
        while(dist < accu) {
            leda::rational rval = rval1 - sign * _singleStep; // invert y-axis
            rval1 = leda::max(_min, leda::min(_max, rval));
            accu -= dist;
        }
        _distAccu.setY(sign * accu);

        SetValue(rval1);

        QCursor::setPos(mapToGlobal(_dragOrigin));
        setCursor(QCursor(Qt::BlankCursor));
    }
    repaint();
}

void NBW_SpinBoxControls::mousePressEvent(QMouseEvent* event) {
    if(STATE_DRAGGING != _state && Qt::LeftButton == event->button()) {
        const QPoint mousePos = event->pos();
        if(Qt::ControlModifier & event->modifiers()) {
            _state = STATE_EDIT;
            _lineEdit->clear();
            _lineEdit->show();
            _lineEdit->setFocus();

            BeginDragging();
        } else if(_textRegion.contains(mousePos)) {
                _state      = STATE_DRAGGING;
                _dragOrigin = mousePos;
                _mousePos   = _dragOrigin;
                _rval0      = _rval;
                _distAccu   = QPoint(0, 0);

                setCursor(QCursor(Qt::BlankCursor));
                BeginDragging();
        } else if(_arrowRegions[LEFT_ARROW].contains(mousePos)) {
            SetValue(_rval - _singleStep);
        } else if(_arrowRegions[RIGHT_ARROW].contains(mousePos)) {
            SetValue(_rval + _singleStep);
        }
        repaint();
    }
}

void NBW_SpinBoxControls::mouseReleaseEvent(QMouseEvent*) {
    if(STATE_DRAGGING == _state) {
        _state = STATE_NORMAL;
        unsetCursor();
        EndDragging();
    }
}

void NBW_SpinBoxControls::paintEvent(QPaintEvent*) {
    const QPointF mousePos = mapFromGlobal(QCursor::pos());

    enum { NORMAL = 0, HOVER };

    QPainter painter(this);
    QSvgRenderer svgRenderers[2];
    svgRenderers[NORMAL].load(QString(":/ui/Images/arrow-left.svg"));
    svgRenderers[HOVER].load(QString(":/ui/Images/arrow-up-hover.svg"));

    const QRectF& drawRegion = _arrowRegions[LEFT_ARROW];

    // draw left arrow
    if(_arrowRegions[LEFT_ARROW].contains(mousePos)) {
        svgRenderers[HOVER].render(&painter, drawRegion);
    } else {
        svgRenderers[NORMAL].render(&painter, drawRegion);
    }

    // draw right arrow
    painter.save();
    painter.scale(-1.0, 1.0);
    painter.translate(-painter.window().width(), 0.0f);

    // draw left arrow
    if(_arrowRegions[RIGHT_ARROW].contains(mousePos)) {
        svgRenderers[HOVER].render(&painter, drawRegion);
    } else {
        svgRenderers[NORMAL].render(&painter, drawRegion);
    }

    painter.restore();

    if(STATE_NORMAL == _state || STATE_DRAGGING == _state) {
        const QString& _text = "Radius: ";

        const QString& text = _text + QString("%1").arg(_rval.to_double());

        painter.drawText(_textRegion, Qt::AlignCenter, text);
    }
}

NBW_SpinBoxControls::NBW_SpinBoxControls(QWidget* parent)
    : QFrame(parent)
    , _rval(0)
    , _singleStep(1)
    , _min(0)
    , _max(100)
    , _typeMask(NBW_SpinBox::TypeFlags::ALL)
    , _state(STATE_NORMAL)
{
    setObjectName("nbw_spinBox_controls");
    setProperty("dragging", false);
    setMouseTracking(true);

    _lineEdit = new QLineEdit(this);
    _lineEdit->setObjectName("nbw_spinBox_lineEdit");
    _lineEdit->setAlignment(Qt::AlignCenter);
    _lineEdit->hide();
    connect(_lineEdit, SIGNAL(editingFinished()), this, SLOT(OnEditingFinished()));
}

leda::rational NBW_SpinBoxControls::GetValue() const {
    return _rval;
}

leda::rational NBW_SpinBoxControls::minimum() const {
    return _min;
}

leda::rational NBW_SpinBoxControls::maximum() const {
    return _max;
}

void NBW_SpinBoxControls::setTypeMask(int mask) {
    _typeMask = mask;
}

void NBW_SpinBoxControls::setMinimum(const leda::rational val) {
    if(val < _max) _min = val;
}

void NBW_SpinBoxControls::setMaximum(const leda::rational val) {
    if(_min < val) _max = val;
}

void NBW_SpinBoxControls::setSingleStep(const leda::rational val) {
    if(0 < val) _singleStep = val;
}

void NBW_SpinBoxControls::setValue(const leda::rational val) {
    SetValue(val);
}

QSize NBW_SpinBox::s_preferredSize;

void NBW_SpinBox::ResizeProgressBar() {
    const leda::rational min = _controls->minimum();
    const leda::rational max = _controls->maximum();

    const leda::rational frac = (_controls->GetValue() - min) / (max - min);

    // draw the progress bar large enough so that the rounded corners
    // don't get screwed up
    const int width = leda::min(size().width(), leda::max(20, static_cast<int>(frac.to_double() * size().width())));
    QRect progressRegion(0, 0, width, size().height());
    _progress->setGeometry(progressRegion);
}

void NBW_SpinBox::OnBeginDragging() {
    setProperty("dragging", true);
    style()->polish(this);

    _progress->setProperty("dragging", true);
    _progress->style()->polish(_progress);
}

void NBW_SpinBox::OnEndDragging() {
    setProperty("dragging", false);
    style()->polish(this);

    _progress->setProperty("dragging", false);
    _progress->style()->polish(_progress);
}

void NBW_SpinBox::OnValueChanged() {
    ResizeProgressBar();

    emit SigValueChanged(_controls->GetValue());
}

void NBW_SpinBox::resizeEvent(QResizeEvent* event) {
    ResizeProgressBar();
    _controls->setGeometry(rect());
}

NBW_SpinBox::NBW_SpinBox(QWidget* parent)
    : QFrame(parent)
    , _progress(0)
    , _controls(0)
{
    setObjectName("nbw_spinBox");
    setProperty("dragging", false);

    _progress = new QFrame(this);
    _progress->setObjectName("nbw_spinBox_progress");
    _progress->setProperty("dragging", false);
    _progress->hide();

    _controls = new NBW_SpinBoxControls(this);
    _controls->show();
    connect(_controls, SIGNAL(SigBeginDragging()), this, SLOT(OnBeginDragging()));
    connect(_controls, SIGNAL(SigEndDragging()), this, SLOT(OnEndDragging()));
    connect(_controls, SIGNAL(SigValueChanged()), this, SLOT(OnValueChanged()));

    // find preferred size. depends on OS guidelines.
    // height is around 22px.
    if(!s_preferredSize.isValid()) {
        QPushButton* dummyButton = new QPushButton;
        s_preferredSize = dummyButton->sizeHint();
    }
}

void NBW_SpinBox::ShowProgressBar(bool show) {
    _showProgress = show;
    _progress->setVisible(show);
}

QSize NBW_SpinBox::sizeHint() const {
    assert(s_preferredSize.isValid());
    return s_preferredSize;
}

// just forward these to controls
leda::rational NBW_SpinBox::minimum() const { return _controls->minimum(); }
leda::rational NBW_SpinBox::maximum() const { return _controls->maximum(); }

void NBW_SpinBox::setTypeMask(int mask) { _controls->setTypeMask(mask); }

void NBW_SpinBox::setMinimum(const leda::rational val) { _controls->setMinimum(val); }
void NBW_SpinBox::setMaximum(const leda::rational val) { _controls->setMaximum(val); }
void NBW_SpinBox::setSingleStep(const leda::rational val) { _controls->setSingleStep(val); }
void NBW_SpinBox::setValue(const leda::rational val) { _controls->setValue(val); }