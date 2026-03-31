#include "examen.h"
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>
#include <QRegularExpression>
#include <QPdfWriter>
#include <QPainter>
#include <QDateTime>
#include <QDir>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>

Examen::Examen(int id_examen, int id_client, QString type, QString resultat, QDate date_examen)
{
    this->id_examen=id_examen;
    this->id_client=id_client;
    this->type=type;
    this->resultat=resultat;
    this->date_examen=date_examen;
}
//--------------------------------------------------------CRUD----------------------------------------------------

// ---------------- AJOUTER ----------------
bool Examen::ajouter()
{
    //controle de saisie
    if (date_examen > QDate::currentDate()) {
        QMessageBox::warning(nullptr, "Erreur de date", "La date d'examen ne peut pas être dans le futur !");
        return false;
    }
    // Vérifier que le client existe
    {
        QSqlQuery q;
        q.prepare("SELECT COUNT(*) FROM CLIENT WHERE ID_CLIENT = :id");
        q.bindValue(":id", id_client);

        if (!q.exec() || !q.next() || q.value(0).toInt() == 0) {
            QMessageBox::warning(nullptr, "Erreur", "Client introuvable !");
            qDebug() << "Client introuvable:" << id_client << q.lastError().text();
            return false;
        }
    }


    // Ajouter l'examen
    QSqlQuery query;
    query.prepare("INSERT INTO EXAMEN (ID_EXAMEN, TYPE, DATE_EXAMEN, RESULTAT, ID_CLIENT) " "VALUES (:id_examen, :type, :date_examen, :resultat, :id_client)");
    query.bindValue(":id_examen", id_examen);
    query.bindValue(":type", type);
    query.bindValue(":date_examen", date_examen);
    query.bindValue(":resultat", resultat);
    query.bindValue(":id_client", id_client);

    return query.exec();
}

// ---------------- SUPPRIMER ----------------
bool Examen::supprimer(int id_examen)
{
    QSqlQuery query;
    query.prepare("DELETE FROM examen WHERE id_examen = :id");
    query.bindValue(":id", id_examen);
    return query.exec();
}

// ---------------- MODIFIER ----------------
bool Examen::modifier(int id_examen)
{
    //controle de saisie
    if (date_examen > QDate::currentDate()) {
        QMessageBox::warning(nullptr, "Erreur de date", "La date d'examen ne peut pas être dans le futur !");
        return false;
    }
    QSqlQuery query;
    query.prepare(
        "UPDATE examen SET "
        "  id_client = COALESCE(NULLIF(:id_client, 0), id_client),"
        "  type = COALESCE(NULLIF(:type, ''), type),"
        "  resultat = COALESCE(NULLIF(:resultat, ''), resultat),"
        "  date_examen = COALESCE(:date, date_examen)"
        " WHERE id_examen = :id_examen"
        );

    query.bindValue(":id_examen", id_examen);
    query.bindValue(":id_client", id_client);
    query.bindValue(":type", type.trimmed());
    query.bindValue(":resultat", resultat.trimmed());
    query.bindValue(":date", date_examen.isValid() ? date_examen : QVariant());

    return query.exec();
}

// ---------------- AFFICHER ----------------
QSqlQueryModel* Examen::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT ID_EXAMEN, TYPE, DATE_EXAMEN, ID_CLIENT, RESULTAT FROM EXAMEN");

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Type"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Date"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("ID Client"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Résultat"));

    return model;
}
// ---------------- RECHERCHER ----------------
QSqlQueryModel* Examen::rechercher(const QString &critere)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlQuery query;

    if (critere.trimmed().isEmpty()) {
        // Afficher tous les examens si aucun critère
        query.prepare("SELECT ID_EXAMEN, TYPE, DATE_EXAMEN, ID_CLIENT, RESULTAT FROM EXAMEN");
    } else {
        // Recherche par ID, Type, Résultat ou ID Client
        query.prepare("SELECT ID_EXAMEN, TYPE, DATE_EXAMEN, ID_CLIENT, RESULTAT "
                      "FROM EXAMEN "
                      "WHERE LOWER(TYPE) LIKE LOWER(:critere) "
                      "   OR LOWER(RESULTAT) LIKE LOWER(:critere) "
                      "   OR TO_CHAR(DATE_EXAMEN, 'DD/MM/YYYY') LIKE :critere "
                      "   OR CAST(ID_EXAMEN AS VARCHAR2(20)) LIKE :critere "
                      "   OR CAST(ID_CLIENT AS VARCHAR2(20)) LIKE :critere");
        query.bindValue(":critere", "%" + critere + "%");
    }

    query.exec();
    model->setQuery(std::move(query));

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID Examen"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Type"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Date"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("ID Client"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Résultat"));

    return model;
}
// ---------------- TRI ----------------
QSqlQueryModel* Examen::trier(const QString &critere)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QString queryStr;
    QString c = critere.trimmed().toLower();

    if (c.contains("id croissant (0") || c.contains("0–9"))
        queryStr = "SELECT ID_EXAMEN, TYPE, DATE_EXAMEN, ID_CLIENT, RESULTAT FROM EXAMEN ORDER BY ID_EXAMEN ASC";
    else if (c.contains("id croissant (9") || c.contains("9–0"))
        queryStr = "SELECT ID_EXAMEN, TYPE, DATE_EXAMEN, ID_CLIENT, RESULTAT FROM EXAMEN ORDER BY ID_EXAMEN DESC";
    else if (c.contains("récemment") || c.contains("recent"))
        queryStr = "SELECT ID_EXAMEN, TYPE, DATE_EXAMEN, ID_CLIENT, RESULTAT FROM EXAMEN ORDER BY DATE_EXAMEN DESC";
    else
        queryStr = "SELECT ID_EXAMEN, TYPE, DATE_EXAMEN, ID_CLIENT, RESULTAT FROM EXAMEN";

    model->setQuery(queryStr);
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID Examen"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Type"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Date"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("ID Client"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Résultat"));
    return model;
}
// ---------------- EXPORT-PDF ----------------
bool Examen::exporterPDF(const QString &filePath)
{
    QPdfWriter pdf(filePath);
    pdf.setPageSize(QPageSize(QPageSize::A4));
    pdf.setResolution(300);

    QPainter painter(&pdf);
    if (!painter.isActive())
        return false;

    // Fonts
    QFont titleFont("Times New Roman", 16, QFont::Bold);
    QFont headerFont("Times New Roman", 10, QFont::Bold);
    QFont textFont("Times New Roman", 9);

    // Title
    painter.setFont(titleFont);
    painter.drawText(200, 150, "Liste des examens");
    painter.setFont(QFont("Times New Roman", 9, QFont::StyleItalic));
    painter.drawText(4300, 150, "Généré le : " + QDate::currentDate().toString("dd/MM/yyyy"));
    painter.setFont(headerFont);

    // Column X positions (balanced for A4)
    int col1 = 100;   // ID Examen
    int col2 = 500;   // Type
    int col3 = 1000;  // Date
    int col4 = 1500;  // ID Client
    int col5 = 2000;  // Résultat

    int y = 400;
    int rowHeight = 220;
    int bottomMargin = 10500;

    // Headers
    painter.drawText(col1, y, "ID Examen");
    painter.drawText(col2, y, "Type");
    painter.drawText(col3, y, "Date");
    painter.drawText(col4, y, "ID Client");
    painter.drawText(col5, y, "Résultat");

    y += rowHeight;
    painter.setFont(textFont);

    // Data
    QSqlQuery query("SELECT ID_EXAMEN, TYPE, DATE_EXAMEN, ID_CLIENT, RESULTAT FROM EXAMEN ORDER BY ID_EXAMEN ASC");
    while (query.next()) {
        if (y > bottomMargin) {
            pdf.newPage();
            y = 400;
            painter.setFont(headerFont);
            painter.drawText(col1, y, "ID Examen");
            painter.drawText(col2, y, "Type");
            painter.drawText(col3, y, "Date");
            painter.drawText(col4, y, "ID Client");
            painter.drawText(col5, y, "Résultat");
            y += rowHeight;
            painter.setFont(textFont);
        }

        painter.drawText(col1, y, query.value(0).toString());
        painter.drawText(col2, y, query.value(1).toString());
        painter.drawText(col3, y, query.value(2).toDate().toString("dd/MM/yyyy"));
        painter.drawText(col4, y, query.value(3).toString());
        painter.drawText(col5, y, query.value(4).toString());

        y += rowHeight;
    }

    painter.end();
    return true;
}
//-------------------------------------------------stat-------------------------------------------
// --------------------------------------------------------
//        STAT : Pourcentage d'examens Réussis / Échoués
// --------------------------------------------------------
QChartView* Examen::genererDonutReussite()
{
    int nbReussi = 0;
    int nbEchoue = 0;

    // ---- SQL basé sur TON tableau ----
    QSqlQuery q(
        "SELECT "
        "SUM(CASE WHEN RESULTAT = 'Réussi' THEN 1 ELSE 0 END) AS REUSSI, "
        "SUM(CASE WHEN RESULTAT = 'Échoué' THEN 1 ELSE 0 END) AS ECHOUES "
        "FROM EXAMEN"
        );

    if (q.next()) {
        nbReussi = q.value("REUSSI").toInt();
        nbEchoue = q.value("ECHOUES").toInt();
    }

    int total = nbReussi + nbEchoue;

    if (total == 0)
        return nullptr;

    double pctReussi = (nbReussi * 100.0) / total;
    double pctEchoue = (nbEchoue * 100.0) / total;

    // ---- Couleurs pastel stylées ----
    QColor colReussi("#8BC34A");   // vert clair (succès)
    QColor colEchoue("#E91E63");   // rose foncé (échec)

    QPieSeries *series = new QPieSeries();
    series->setHoleSize(0.60);
    series->setPieSize(0.90);

    QPieSlice *sliceReussi = series->append(
        QString("Réussi : %1%").arg(QString::number(pctReussi, 'f', 1)),
        nbReussi
        );
    QPieSlice *sliceEchoue = series->append(
        QString("Échoué : %1%").arg(QString::number(pctEchoue, 'f', 1)),
        nbEchoue
        );

    sliceReussi->setBrush(colReussi);
    sliceEchoue->setBrush(colEchoue);

    sliceReussi->setLabelVisible(true);
    sliceEchoue->setLabelVisible(true);
    sliceReussi->setLabelColor(Qt::white);
    sliceEchoue->setLabelColor(Qt::white);

    sliceReussi->setLabelFont(QFont("Poppins", 15, QFont::Bold));
    sliceEchoue->setLabelFont(QFont("Poppins", 15, QFont::Bold));

    QObject::connect(sliceReussi, &QPieSlice::hovered,
                     [sliceReussi](bool hovered){
                         sliceReussi->setExploded(hovered);
                         sliceReussi->setExplodeDistanceFactor(0.10);
                     });

    QObject::connect(sliceEchoue, &QPieSlice::hovered,
                     [sliceEchoue](bool hovered){
                         sliceEchoue->setExploded(hovered);
                         sliceEchoue->setExplodeDistanceFactor(0.10);
                     });

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->legend()->setAlignment(Qt::AlignRight);
    chart->legend()->setLabelColor(Qt::white);
    chart->legend()->setFont(QFont("Poppins", 15));
    chart->setTitleBrush(QBrush(Qt::white));
    chart->legend()->setMarkerShape(QLegend::MarkerShapeRectangle);
    chart->legend()->setAlignment(Qt::AlignRight);  // optionnel
    chart->legend()->setBackgroundVisible(false);

    chart->setBackgroundVisible(false);

    QChartView *view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);
    view->setStyleSheet("background: transparent;");

    return view;
}

// ---------------- METIER ----------------
QSqlQueryModel* Examen::detecterNonRealisesALaDatePrevue()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlQuery query;

    query.prepare(
        "SELECT ID_EXAMEN, TYPE, DATE_EXAMEN, ID_CLIENT, RESULTAT "
        "FROM EXAMEN "
        "WHERE DATE_EXAMEN < :today "
        "  AND (RESULTAT IS NULL OR TRIM(RESULTAT) = '' OR RESULTAT = '— Sélectionner —')"
        );
    query.bindValue(":today", QDate::currentDate());

    query.exec();
    model->setQuery(std::move(query));

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID Examen"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Type"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Date prévue"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("ID Client"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Statut"));

    return model;
}
