#include "vehicule.h"
#include <QSqlError>
#include <QDebug>
#include <QRegularExpression>
#include <QMessageBox>
#include <QPdfWriter>
#include <QPainter>
#include <QDateTime>
#include <QDir>
#include <QObject>
#include <QtCharts/QPieSlice>
#include <QtCharts/QPieSeries>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>

Vehicule::Vehicule(int matricule, QString marque, QString modele, QString type, QString kilometrage, QString localisation)
{
    this->matricule = matricule;
    this->marque = marque;
    this->modele = modele;
    this->type = type;
    this->kilometrage = kilometrage;
    this->localisation = localisation;
}


// ------------------- AJOUTER -------------------
bool Vehicule::ajouter()
{
    // --- Contrôle de saisie ---
    QRegularExpression regexLettres("^[A-Za-zÀ-Ö Ø-öø-ÿ\\s'-]+$");     // Lettres et espaces
    QRegularExpression regexModele("^[A-Za-z0-9À-Ö Ø-öø-ÿ\\s'-]+$");   // Lettres, chiffres, espaces
    QRegularExpression regexChiffres("^\\d+$");                       // Chiffres uniquement

    if (!regexLettres.match(marque.trimmed()).hasMatch()) {
        QMessageBox::warning(nullptr, "Erreur", "Marque invalide (lettres uniquement).");
        return false;
    }

    if (!regexModele.match(modele.trimmed()).hasMatch()) {
        QMessageBox::warning(nullptr, "Erreur", "Modèle invalide (lettres, chiffres et espaces uniquement).");
        return false;
    }

    if (!regexChiffres.match(kilometrage.trimmed()).hasMatch()) {
        QMessageBox::warning(nullptr, "Erreur", "Kilométrage invalide (chiffres uniquement).");
        return false;
    }

    if (localisation.trimmed().isEmpty()) {
        QMessageBox::warning(nullptr, "Erreur", "Localisation est obligatoire.");
        return false;
    }

    // Ajout
    QSqlQuery query;
    query.prepare("INSERT INTO vehicule (matricule, marque, modele, type, kilometrage, localisation) "
                  "VALUES (:matricule, :marque, :modele, :type, :kilometrage, :localisation)");
    query.bindValue(":matricule", matricule);
    query.bindValue(":marque", marque);
    query.bindValue(":modele", modele);
    query.bindValue(":type", type);
    query.bindValue(":kilometrage", kilometrage);
    query.bindValue(":localisation", localisation);

    return query.exec();
}

// ------------------- SUPPRIMER -------------------
bool Vehicule::supprimer(int matricule)
{
    QSqlQuery query;
    query.prepare("DELETE FROM vehicule WHERE matricule = :matricule");
    query.bindValue(":matricule", matricule);
    return query.exec();
}

// ------------------- MODIFIER -------------------
bool Vehicule::modifier(int matricule)
{
    // --- Contrôle de saisie ---
    QRegularExpression regexLettres("^[A-Za-zÀ-Ö Ø-öø-ÿ\\s'-]+$");
    QRegularExpression regexChiffres("^\\d+$");
    QRegularExpression regexModele("^[A-Za-z0-9À-Ö Ø-öø-ÿ\\s'-]+$");

    if (!marque.isEmpty() && !regexLettres.match(marque.trimmed()).hasMatch()) {
        QMessageBox::warning(nullptr, "Erreur", "Marque invalide (lettres uniquement).");
        return false;
    }

    if (!modele.isEmpty() && !regexModele.match(modele.trimmed()).hasMatch()) {
        QMessageBox::warning(nullptr, "Erreur", "Modèle invalide (lettres, chiffres et espaces uniquement).");
        return false;
    }

    if (!kilometrage.isEmpty() && !regexChiffres.match(kilometrage.trimmed()).hasMatch()) {
        QMessageBox::warning(nullptr, "Erreur", "Kilométrage invalide (chiffres uniquement).");
        return false;
    }

    // Update
    QSqlQuery query;
    query.prepare(
        "UPDATE vehicule SET "
        "  marque = COALESCE(NULLIF(:marque, ''), marque),"
        "  modele = COALESCE(NULLIF(:modele, ''), modele),"
        "  type = COALESCE(NULLIF(:type, ''), type),"
        "  kilometrage = COALESCE(NULLIF(:kilometrage, ''), kilometrage),"
        "  localisation = COALESCE(NULLIF(:localisation, ''), localisation)"
        " WHERE matricule = :matricule"
        );

    query.bindValue(":matricule", matricule);
    query.bindValue(":marque", marque.trimmed());
    query.bindValue(":modele", modele.trimmed());
    query.bindValue(":type", type.trimmed());
    query.bindValue(":kilometrage", kilometrage.trimmed());
    query.bindValue(":localisation", localisation.trimmed());

    return query.exec();
}

// ------------------- AFFICHER -------------------
QSqlQueryModel* Vehicule::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT matricule, marque, modele, type, kilometrage, localisation FROM vehicule");
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Matricule"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Marque"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Modèle"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Type"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Kilométrage"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Localisation"));
    return model;
}

// ---------------- RECHERCHER ----------------
QSqlQueryModel* Vehicule::rechercher(const QString &critere)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlQuery query;

    if (critere.trimmed().isEmpty()) {
        // Afficher tous les véhicules si aucun critère
        query.prepare("SELECT matricule, marque, modele, type, kilometrage, localisation FROM vehicule");
    } else {
        // Recherche par matricule, marque, modèle, type, kilométrage ou localisation
        query.prepare("SELECT matricule, marque, modele, type, kilometrage, localisation "
                      "FROM vehicule "
                      "WHERE LOWER(marque) LIKE LOWER(:critere) "
                      "   OR LOWER(modele) LIKE LOWER(:critere) "
                      "   OR LOWER(type) LIKE LOWER(:critere) "
                      "   OR LOWER(localisation) LIKE LOWER(:critere) "
                      "   OR CAST(matricule AS VARCHAR2(20)) LIKE :critere "
                      "   OR CAST(kilometrage AS VARCHAR2(20)) LIKE :critere");
        query.bindValue(":critere", "%" + critere + "%");
    }

    query.exec();
    model->setQuery(query);

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Matricule"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Marque"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Modèle"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Type"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Kilométrage"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Localisation"));

    return model;
}


// ---------------- TRI ----------------
QSqlQueryModel* Vehicule::trier(const QString &critere)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QString queryStr;

    QString c = critere.trimmed().toLower(); // normaliser le texte du ComboBox

    if (c.contains("matricule croissant"))
        queryStr = "SELECT * FROM vehicule ORDER BY matricule ASC";
    else if (c.contains("matricule décroissant") || c.contains("matricule decroissant"))
        queryStr = "SELECT * FROM vehicule ORDER BY matricule DESC";
    else if (c.contains("marque a-z"))
        queryStr = "SELECT * FROM vehicule ORDER BY marque ASC";
    else if (c.contains("marque z-a"))
        queryStr = "SELECT * FROM vehicule ORDER BY marque DESC";
    else if (c.contains("localisation a-z"))
        queryStr = "SELECT * FROM vehicule ORDER BY localisation ASC";
    else if (c.contains("localisation z-a"))
        queryStr = "SELECT * FROM vehicule ORDER BY localisation DESC";
    else
        queryStr = "SELECT * FROM vehicule"; // affichage normal

    model->setQuery(queryStr);
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Matricule"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Marque"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Modèle"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Type"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Kilométrage"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Localisation"));
    return model;
}


// ---------------- EXPORT-PDF ----------------
bool Vehicule::exporterPDF(const QString &filePath)
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
    painter.drawText(200, 150, "Liste des véhicules");
    painter.setFont(QFont("Times New Roman", 9, QFont::StyleItalic));
    painter.drawText(4300, 150, "Généré le : " + QDate::currentDate().toString("dd/MM/yyyy"));
    painter.setFont(headerFont);

    // Ajuster les positions des colonnes pour mieux tenir sur la page A4
    // Largeur approximative d'une page A4 en pixels à 300 DPI : ~3500
    int col1 = 100;   // Matricule
    int col2 = 400;   // Marque
    int col3 = 800;   // Modèle
    int col4 = 1200;  // Type
    int col5 = 1600;  // Kilométrage
    int col6 = 2000;  // Localisation (ajusté pour rester dans la page)

    int y = 400;
    int rowHeight = 220;
    int bottomMargin = 10500;

    // Headers
    painter.drawText(col1, y, "Matricule");
    painter.drawText(col2, y, "Marque");
    painter.drawText(col3, y, "Modèle");
    painter.drawText(col4, y, "Type");
    painter.drawText(col5, y, "Kilométrage");
    painter.drawText(col6, y, "Localisation");

    y += rowHeight;
    painter.setFont(textFont);

    // Data - DEBUG: Vérifions ce que la requête retourne
    QSqlQuery query("SELECT matricule, marque, modele, type, kilometrage, localisation FROM vehicule ORDER BY matricule ASC");

    qDebug() << "Export PDF - Nombre de véhicules:" << query.size();

    while (query.next()) {
        qDebug() << "Véhicule:" << query.value(0).toString()
                 << query.value(1).toString()
                 << query.value(2).toString()
                 << query.value(3).toString()
                 << query.value(4).toString()
                 << query.value(5).toString(); // Localisation

        if (y > bottomMargin) {
            pdf.newPage();
            y = 400;
            painter.setFont(headerFont);
            painter.drawText(col1, y, "Matricule");
            painter.drawText(col2, y, "Marque");
            painter.drawText(col3, y, "Modèle");
            painter.drawText(col4, y, "Type");
            painter.drawText(col5, y, "Kilométrage");
            painter.drawText(col6, y, "Localisation");
            y += rowHeight;
            painter.setFont(textFont);
        }

        painter.drawText(col1, y, query.value(0).toString());
        painter.drawText(col2, y, query.value(1).toString());
        painter.drawText(col3, y, query.value(2).toString());
        painter.drawText(col4, y, query.value(3).toString());
        painter.drawText(col5, y, query.value(4).toString());
        painter.drawText(col6, y, query.value(5).toString()); // Localisation

        y += rowHeight;
    }

    painter.end();

    // Vérifier si le fichier a été créé
    QFile file(filePath);
    if (file.exists()) {
        qDebug() << "Fichier PDF créé avec succès:" << filePath;
        return true;
    } else {
        qDebug() << "Erreur: Fichier PDF non créé";
        return false;
    }
}

// -------------------------------- STAT VEHICULE --------------------------------
QSqlQueryModel* Vehicule::statistiquesVehicule()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT TYPE, COUNT(*) FROM VEHICULE GROUP BY TYPE ORDER BY TYPE");
    return model;
}

// ----------------------------
//  DONUT + ANIMATIONS
// ----------------------------
QChartView* Vehicule::genererDonut(QSqlQueryModel *model)
{
    // ------- CREATE DONUT -------
    QPieSeries *series = new QPieSeries();
    series->setHoleSize(0.40);      // donut épais
    series->setPieSize(0.65);       // donut pas trop grand

    QStringList couleurs = { "#5FB6ED","#0C2A6E", "#3A3F47" };

    for (int i = 0; i < model->rowCount(); i++)
    {
        int total = model->record(i).value(1).toInt();

        QPieSlice *slice = series->append("", total); // <-- IMPORTANT : pas de label !
        slice->setBrush(QColor(couleurs.at(i)));
        slice->setBorderWidth(0);
        slice->setLabelVisible(false);

        // Animation hover simple
        QObject::connect(slice, &QPieSlice::hovered, [slice](bool state){
            slice->setExploded(state);
            slice->setExplodeDistanceFactor(0.08);
        });
    }

    // ------- CHART -------
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->legend()->hide();
    chart->setBackgroundVisible(false);
    chart->setMargins(QMargins(0,0,0,0));
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // Taille raisonnable
    chart->setMinimumSize(330, 330);

    QChartView *view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);
    view->setStyleSheet("background: transparent;");
    view->setMinimumSize(330, 330);

    return view;
}

// ==================== JSON POUR LA CARTE ====================
QString Vehicule::getVehiclesAsJSON()
{
    QSqlQuery query;
    query.prepare("SELECT matricule, marque, modele, type, kilometrage, localisation FROM vehicule");

    QJsonArray array;

    if(query.exec()) {
        while(query.next()) {
            QJsonObject obj;
            obj["matricule"] = query.value(0).toInt();
            obj["marque"] = query.value(1).toString();
            obj["modele"] = query.value(2).toString();
            obj["type"] = query.value(3).toString();
            obj["kilometrage"] = query.value(4).toInt();
            obj["localisation"] = query.value(5).toString();

            // Ajouter les coordonnées basées sur la localisation
            QString localisation = query.value(5).toString();
            if (localisation.toLower() == "tunis") {
                obj["latitude"] = 36.8065;
                obj["longitude"] = 10.1815;
            } else if (localisation.toLower() == "sousse") {
                obj["latitude"] = 35.8256;
                obj["longitude"] = 10.6369;
            } else if (localisation.toLower() == "bizerte") {
                obj["latitude"] = 37.2741;
                obj["longitude"] = 9.8733;
            } else {
                // Coordonnées par défaut (Tunis)
                obj["latitude"] = 36.8065;
                obj["longitude"] = 10.1815;
            }

            array.append(obj);
        }
    }

    QJsonDocument doc(array);
    return QString::fromUtf8(doc.toJson());
}
