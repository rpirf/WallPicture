#pragma once
#include <QString>

namespace WP
{
    static QString buttonStyle()
    {
        return
            "QPushButton {"
            "border: 0.077em solid rgb(180, 180, 180);"
            "border-radius: 2.462em;"
            "background-color: rgba(168, 255, 241, 100);"
            "}"
            "QPushButton:hover { background-color:rgba(168, 255, 241, 150) }"
            "QPushButton:pressed { background-color: rgba(250, 250, 250, 200 ) }"
            ;
    }

    static QString signatureStyle()
    {
        return
            "QLabel {"
            "border: 0.077em solid rgb(180, 180, 180);"
            "background-color: rgb(255, 255, 255);"
            "padding: 20px;"
            "border-radius: 15px;"
            "font-family: Times New Roman;"
            "font-size: 22px;"
            "}"
            ;
    }

    static QString dialogStyle()
    {
        return "font-size: 18pt;";
    }

    static QString splashStyle()
    {
        return "font-size: 26pt;"
            "font-family: Times New Roman;";
    }
}
