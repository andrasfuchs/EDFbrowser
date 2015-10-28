#ifndef SIGNAL_H
#define SIGNAL_H

#include <float.h>

#include <QString>
#include <QColor>
#include <QVector>

enum class SubSignalType { Unknown = 0, LogScale = 1, SquareRoot = 2, FFT = 4, ZScore = 8, BMP = 16, Average = 32 };

class Signal : public QObject
{
    Q_OBJECT            // we need this to avoid the "undefined reference" error by the compilation because of the Qt's signals/events

    public:
      Signal();

      Signal(QString id, QString name, QString alias, QVector<double> values, QString h_name, QString h_unit, double h_density, QString v_name, QString v_unit, double v_density);

      QString GetId();

      QString GetName();

      QString GetAlias();
      void SetAlias(QString alias);

      QVector<double> GetValues();
      void SetValues(QVector<double> values);

      QColor GetColor();
      void SetColor(QColor color);

      double GetPosition();
      void SetPosition(double position);

      void GetSubSignal(SubSignalType type, double parameter);

      QString GetHorizontalName();
      QString GetHorizontalUnit();
      double GetHorizontalDensity();
      void SetHorizontalDensity(double h_density);

      QString GetVerticalName();
      QString GetVerticalUnit();
      double GetVerticalDensity();


      double SIGNAL_NA_VALUE = DBL_MIN;     // special value which represents the missing value

    signals:
      void aliasChanged(QString newAlias);
      void colorChanged(QColor newColor);
      void valuesChanged(QVector<double> newValues);
      void positionChanged(double newPosition);
      void horizontalDensityChanged(double density);

    private:
      QVector<double> values;               // signal values

      double position = 0,                  // our position at the moment (index of values array)
             h_density = 1.0,               // the horizontal density of the signal (e.g. number of values per Hz by and FFT signal)
             v_density = 1.0;               // the vertical density of the signal (e.g. number of values per Volt by and FFT amplitude signal)

      QString   id = "",
                name = "",
                alias = "",
                h_name = "",
                h_unit = "",
                v_name = "",
                v_unit = "";

      QColor color = QColor(255, 255, 0);   // default it to yellow
};

#endif
