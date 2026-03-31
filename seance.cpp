#include "seance.h"
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>
#include <QRegularExpression>
#include <QPdfWriter>
#include <QPainter>
#include <QDateTime>
#include <QDir>
#include <QSqlRecord>
#include <QtCharts>
#include <QLineSeries>

Seance::Seance(int id, QDate date, QTime heure, int idClient, int idEmploye, int matricule)
{
    this->id_seance = id;
    this->date_seance = date;
    this->heure = heure;
    this->id_client = idClient;
    this->id_employe = idEmploye;
    this->matricule = matricule;
}
// Add these getter implementations in seance.cpp
int Seance::getIdSeance() const {
    return id_seance;
}

QDate Seance::getDateSeance() const {
    return date_seance;
}

QTime Seance::getHeure() const {
    return heure;
}

int Seance::getIdClient() const {
    return id_client;
}

int Seance::getIdEmploye() const {
    return id_employe;
}

//-------------------------------------------------CRUD-------------------------------------------------------

// ---------------- AJOUTER ----------------

bool Seance::ajouter()
{
    //controle de saisie
    if (date_seance.isValid() && date_seance < QDate::currentDate()) {
        QMessageBox::warning(nullptr, "Erreur de date", "la date de la séance ne peut pas être dans le passé.");
        qDebug() << "Erreur : la date de la séance ne peut pas être dans le passé.";
        return false;
    }
    //Client existe
    {
        QSqlQuery q;
        q.prepare("SELECT COUNT(*) FROM CLIENT WHERE ID_CLIENT = :id");
        q.bindValue(":id", id_client);
        if (!q.exec() || !q.next() || q.value(0).toInt() == 0) {
            QMessageBox::warning(nullptr, "Erreur", "Client introuvable");
            qDebug() << "Client introuvable:" << id_client << q.lastError().text();
            return false;
        }
    }

    //Employé existe
    {
        QSqlQuery q;
        q.prepare("SELECT COUNT(*) FROM EMPLOYE WHERE ID_EMPLOYE = :id");
        q.bindValue(":id", id_employe);
        if (!q.exec() || !q.next() || q.value(0).toInt() == 0) {
            QMessageBox::warning(nullptr, "Erreur", "Employe introuvable");

            qDebug() << "Employe introuvable:" << id_employe << q.lastError().text();
            return false;
        }
    }

    //Véhicule existe
    {
        QSqlQuery q;
        q.prepare("SELECT COUNT(*) FROM VEHICULE WHERE MATRICULE = :m");
        q.bindValue(":m", matricule);
        if (!q.exec() || !q.next() || q.value(0).toInt() == 0) {
            QMessageBox::warning(nullptr, "Erreur", "Vehicule introuvable");

            qDebug() << "Vehicule introuvable:" << matricule << q.lastError().text();
            return false;
        }
    }

    //INSERT
    QSqlQuery query;
    query.prepare(
        "INSERT INTO SEANCE (ID_SEANCE, DATE_SEANCE, HEURE, ID_EMPLOYE, ID_CLIENT, MATRICULE) "
        "VALUES (:id, :date, :heure, :id_emp, :id_cli, :mat)"
        );

    // NB: ta colonne HEURE est de type NUMBER. On stocke HHMM (ex: 930 pour 09:30).
    int hhmm = heure.hour() * 100 + heure.minute();

    query.bindValue(":id", id_seance);
    query.bindValue(":date", date_seance);
    query.bindValue(":heure", hhmm);
    query.bindValue(":id_emp", id_employe);
    query.bindValue(":id_cli", id_client);
    query.bindValue(":mat", matricule);

    if (!query.exec()) {
        qDebug() << "Erreur insertion SEANCE:" << query.lastError().text();
        return false;
    }

    return true;
}


// ---------------- SUPPRIMER ----------------
bool Seance::supprimer(int id)
{   //TO DO
    QSqlQuery query;
    query.prepare("DELETE FROM seance WHERE id_seance = :id");
    query.bindValue(":id", id);
    return query.exec();
}

// ---------------- MODIFIER ----------------
bool Seance::modifier(int id)
{
    //Contrôle de saisie : la date ne doit pas être dans le passé
    if (date_seance.isValid() && date_seance < QDate::currentDate()) {
        QMessageBox::warning(nullptr, "Erreur", "la date de la séance ne peut pas être dans le passé.");

        qDebug() << "Erreur : la date de la séance ne peut pas être dans le passé.";
        return false;
    }
    QSqlQuery query;

    //Début de la requête dynamique
    QString req = "UPDATE seance SET ";
    QStringList updates;

    //On ajoute les champs seulement s'ils ont une valeur modifiée
    if (date_seance.isValid() && date_seance != QDate(2000, 1, 1))
        updates << "date_seance = :date";
    if (heure.isValid())
        updates << "heure = :heure";
    if (id_client != 0)
        updates << "id_client = :id_client";
    if (id_employe != 0)
        updates << "id_employe = :id_employe";
    if (matricule != 0)
        updates << "matricule = :matricule";

    //Si aucun champ modifié, on annule
    if (updates.isEmpty())
        return false;

    req += updates.join(", ");
    req += " WHERE id_seance = :id";

    query.prepare(req);
    query.bindValue(":id", id);

    //On bind uniquement ce qui existe
    if (req.contains(":date"))
        query.bindValue(":date", date_seance);
    if (req.contains(":heure")) {
        int hhmm = heure.hour() * 100 + heure.minute();
        query.bindValue(":heure", hhmm);
    }
    if (req.contains(":id_client"))
        query.bindValue(":id_client", id_client);
    if (req.contains(":id_employe"))
        query.bindValue(":id_employe", id_employe);
    if (req.contains(":matricule"))
        query.bindValue(":matricule", matricule);

    //Exécution
    if (!query.exec()) {
        qDebug() << "Erreur modification SEANCE:" << query.lastError().text();
        return false;
    }

    return true;
}


// ---------------- AFFICHER ----------------
QSqlQueryModel* Seance::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();

    model->setQuery(
        "SELECT ID_SEANCE, "
        "TO_CHAR(DATE_SEANCE, 'DD/MM/YYYY') AS DATE_SEANCE, "
        // Conversion de l'heure (nombre) en HH:MM
        "CASE "
        "WHEN LENGTH(HEURE) = 3 THEN SUBSTR(HEURE, 1, 1) || ':' || SUBSTR(HEURE, 2, 2) "
        "WHEN LENGTH(HEURE) = 4 THEN SUBSTR(HEURE, 1, 2) || ':' || SUBSTR(HEURE, 3, 2) "
        "ELSE TO_CHAR(HEURE) END AS HEURE, "
        "ID_CLIENT, ID_EMPLOYE, MATRICULE "
        "FROM SEANCE"
        );

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID Séance"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Date"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Heure"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("ID Client"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("ID Employé"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Matricule"));

    return model;
}
//----------------------------------------------------------chercher-------------------------------
QSqlQueryModel* Seance::rechercher(const QString &critere)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlQuery query;

    if (critere.trimmed().isEmpty()) {
        //Afficher toutes les séances si aucun critère
        query.prepare("SELECT ID_SEANCE, TO_CHAR(DATE_SEANCE, 'DD/MM/YYYY') AS DATE_SEANCE, "
                      "CASE "
                      "WHEN LENGTH(HEURE) = 3 THEN SUBSTR(HEURE, 1, 1) || ':' || SUBSTR(HEURE, 2, 2) "
                      "WHEN LENGTH(HEURE) = 4 THEN SUBSTR(HEURE, 1, 2) || ':' || SUBSTR(HEURE, 3, 2) "
                      "ELSE TO_CHAR(HEURE) END AS HEURE, "
                      "ID_CLIENT, ID_EMPLOYE, MATRICULE "
                      "FROM SEANCE");
    } else {
        //Recherche par ID, date, heure, client, employé ou matricule
        query.prepare("SELECT ID_SEANCE, TO_CHAR(DATE_SEANCE, 'DD/MM/YYYY') AS DATE_SEANCE, "
                      "CASE "
                      "WHEN LENGTH(HEURE) = 3 THEN SUBSTR(HEURE, 1, 1) || ':' || SUBSTR(HEURE, 2, 2) "
                      "WHEN LENGTH(HEURE) = 4 THEN SUBSTR(HEURE, 1, 2) || ':' || SUBSTR(HEURE, 3, 2) "
                      "ELSE TO_CHAR(HEURE) END AS HEURE, "
                      "ID_CLIENT, ID_EMPLOYE, MATRICULE "
                      "FROM SEANCE "
                      "WHERE TO_CHAR(DATE_SEANCE, 'DD/MM/YYYY') LIKE :critere "
                      "   OR CAST(ID_SEANCE AS VARCHAR2(20)) LIKE :critere "
                      "   OR CAST(ID_CLIENT AS VARCHAR2(20)) LIKE :critere "
                      "   OR CAST(ID_EMPLOYE AS VARCHAR2(20)) LIKE :critere "
                      "   OR CAST(MATRICULE AS VARCHAR2(20)) LIKE :critere");
        query.bindValue(":critere", "%" + critere + "%");
    }

    query.exec();
    model->setQuery(query);

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID Séance"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Date"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Heure"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("ID Client"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("ID Employé"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Matricule"));

    return model;
}


//----------------------------------------------------------tri-------------------------------

QSqlQueryModel* Seance::trier(const QString &critere)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    const QString c = critere.trimmed().toLower();

    const char* baseSelect =
        "SELECT ID_SEANCE, "
        "       TO_CHAR(DATE_SEANCE,'DD/MM/YYYY') AS DATE_AFF, "
        "       CASE WHEN LENGTH(HEURE)=3 THEN SUBSTR(HEURE,1,1)||':'||SUBSTR(HEURE,2,2) "
        "            WHEN LENGTH(HEURE)=4 THEN SUBSTR(HEURE,1,2)||':'||SUBSTR(HEURE,3,2) "
        "            ELSE TO_CHAR(HEURE) END AS HEURE_AFF, "
        "       ID_CLIENT, ID_EMPLOYE, MATRICULE "
        "FROM SEANCE ";

    QString q;
    if (c.startsWith("date proche")) {                                // ✅ exact intent
        q = QString(baseSelect) + "ORDER BY DATE_SEANCE ASC, HEURE ASC";
    } else if (c.startsWith("date éloignée") || c.startsWith("date eloignee")) {
        q = QString(baseSelect) + "ORDER BY DATE_SEANCE DESC, HEURE DESC";
    } else if (c.contains("récemment") || c.contains("recent")) {
        q = QString(baseSelect) + "ORDER BY ROWID DESC";
    } else if (c.contains("ancienne")) {
        q = QString(baseSelect) + "ORDER BY ROWID ASC";
    } else {
        q = QString(baseSelect) + "ORDER BY DATE_SEANCE ASC";
    }

    model->setQuery(q);
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID Séance"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Date"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Heure"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("ID Client"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("ID Employé"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Matricule"));
    return model;
}

// ---------------- EXPORT-PDF ----------------


bool Seance::exporterPDF(const QString &filePath)
{
    QPdfWriter pdf(filePath);
    pdf.setPageSize(QPageSize(QPageSize::A4));
    pdf.setResolution(300);

    QPainter painter(&pdf);
    if (!painter.isActive())
        return false;

    // 🖋 Fonts
    QFont titleFont("Times New Roman", 16, QFont::Bold);
    QFont headerFont("Times New Roman", 10, QFont::Bold);
    QFont textFont("Times New Roman", 9);

    //Title centered
    painter.setFont(titleFont);
    painter.drawText(QRect(0, 100, pdf.width(), 200),
                     Qt::AlignHCenter, "Liste des séances");

    //"Généré le" at top-right
    painter.setFont(QFont("Times New Roman", 9, QFont::StyleItalic));
    painter.drawText(QRect(0, 300, pdf.width() - 100, 100),
                     Qt::AlignRight,
                     "Généré le : " + QDate::currentDate().toString("dd/MM/yyyy"));

    painter.setFont(headerFont);

    //Column layout
    int col1 = 100;   // ID Séance
    int col2 = 500;   // Date
    int col3 = 900;  // Heure
    int col4 = 1300;  // ID Client
    int col5 = 1700;  // ID Employé
    int col6 = 2100;  // Matricule

    int y = 600;
    int rowHeight = 220;
    int bottomMargin = 10500;

    //Headers
    painter.drawText(col1, y, "ID Séance");
    painter.drawText(col2, y, "Date");
    painter.drawText(col3, y, "Heure");
    painter.drawText(col4, y, "ID Client");
    painter.drawText(col5, y, "ID Employé");
    painter.drawText(col6, y, "Matricule");

    y += rowHeight;
    painter.setFont(textFont);

    // 🗂 Data
    QSqlQuery query(
        "SELECT ID_SEANCE, "
        "TO_CHAR(DATE_SEANCE,'DD/MM/YYYY'), "
        "CASE "
        "  WHEN LENGTH(HEURE)=3 THEN SUBSTR(HEURE,1,1)||':'||SUBSTR(HEURE,2,2) "
        "  WHEN LENGTH(HEURE)=4 THEN SUBSTR(HEURE,1,2)||':'||SUBSTR(HEURE,3,2) "
        "  ELSE TO_CHAR(HEURE) END AS HEURE, "
        "ID_CLIENT, ID_EMPLOYE, MATRICULE "
        "FROM SEANCE ORDER BY DATE_SEANCE ASC"
        );

    while (query.next()) {
        if (y > bottomMargin) {
            pdf.newPage();
            y = 400;
            painter.setFont(headerFont);
            painter.drawText(col1, y, "ID Séance");
            painter.drawText(col2, y, "Date");
            painter.drawText(col3, y, "Heure");
            painter.drawText(col4, y, "ID Client");
            painter.drawText(col5, y, "ID Employé");
            painter.drawText(col6, y, "Matricule");
            y += rowHeight;
            painter.setFont(textFont);
        }

        painter.drawText(col1, y, query.value(0).toString());
        painter.drawText(col2, y, query.value(1).toString());
        painter.drawText(col3, y, query.value(2).toString());
        painter.drawText(col4, y, query.value(3).toString());
        painter.drawText(col5, y, query.value(4).toString());
        painter.drawText(col6, y, query.value(5).toString());
        y += rowHeight;
    }

    painter.end();
    return true;
}
//------------------------------------------------------statiqtique-------------------------------------------------
//------------------------------------------------------STATISTIQUES AMÉLIORÉES-------------------------------------------------

// ==================== STATISTIQUES PAR PÉRIODE ====================

QSqlQueryModel* Seance::statistiquesParPeriode(const QString &periode)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlQuery query;

    if (periode == "jour") {
        // Stats par jour de la semaine
        query.prepare(
            "SELECT "
            "  CASE TO_CHAR(date_seance, 'D', 'NLS_DATE_LANGUAGE=FRENCH') "
            "    WHEN '1' THEN 'Dimanche' "
            "    WHEN '2' THEN 'Lundi' "
            "    WHEN '3' THEN 'Mardi' "
            "    WHEN '4' THEN 'Mercredi' "
            "    WHEN '5' THEN 'Jeudi' "
            "    WHEN '6' THEN 'Vendredi' "
            "    WHEN '7' THEN 'Samedi' "
            "  END AS jour, "
            "  COUNT(*) AS total_seances, "
            "  TO_CHAR(date_seance, 'D', 'NLS_DATE_LANGUAGE=FRENCH') AS jour_num "
            "FROM SEANCE "
            "GROUP BY TO_CHAR(date_seance, 'D', 'NLS_DATE_LANGUAGE=FRENCH') "
            "ORDER BY jour_num"
            );
    } else if (periode == "mois") {
        // Stats par mois
        query.prepare(
            "SELECT "
            "  TO_CHAR(date_seance, 'Month YYYY', 'NLS_DATE_LANGUAGE=FRENCH') AS mois, "
            "  COUNT(*) AS total_seances, "
            "  TO_CHAR(date_seance, 'YYYY-MM') AS mois_num "
            "FROM SEANCE "
            "GROUP BY TO_CHAR(date_seance, 'Month YYYY', 'NLS_DATE_LANGUAGE=FRENCH'), "
            "         TO_CHAR(date_seance, 'YYYY-MM') "
            "ORDER BY mois_num DESC"
            );
    } else if (periode == "semaine") {
        // Stats par semaine
        query.prepare(
            "SELECT "
            "  'Semaine ' || TO_CHAR(date_seance, 'IW') || ' - ' || TO_CHAR(date_seance, 'YYYY') AS semaine, "
            "  COUNT(*) AS total_seances, "
            "  TO_CHAR(date_seance, 'IYYY-IW') AS semaine_num "
            "FROM SEANCE "
            "GROUP BY TO_CHAR(date_seance, 'IW'), "
            "         TO_CHAR(date_seance, 'YYYY'), "
            "         TO_CHAR(date_seance, 'IYYY-IW') "
            "ORDER BY semaine_num DESC"
            );
    }

    if (!query.exec()) {
        qDebug() << "Erreur stats période:" << query.lastError().text();
        return model;
    }

    model->setQuery(query);
    return model;
}

// ==================== STATS AVANCÉES ====================

QMap<QString, int> Seance::statsAvancees()
{
    QMap<QString, int> stats;

    // Total des séances
    QSqlQuery q1("SELECT COUNT(*) FROM SEANCE");
    if (q1.next()) {
        stats["total"] = q1.value(0).toInt();
    }

    // Séances aujourd'hui
    QSqlQuery q2;
    q2.prepare("SELECT COUNT(*) FROM SEANCE WHERE DATE_SEANCE = :today");
    q2.bindValue(":today", QDate::currentDate());
    if (q2.exec() && q2.next()) {
        stats["aujourdhui"] = q2.value(0).toInt();
    }

    // Séances cette semaine
    QSqlQuery q3;
    q3.prepare(
        "SELECT COUNT(*) FROM SEANCE "
        "WHERE TO_CHAR(DATE_SEANCE, 'IYYY-IW') = TO_CHAR(:today, 'IYYY-IW')"
        );
    q3.bindValue(":today", QDate::currentDate());
    if (q3.exec() && q3.next()) {
        stats["semaine"] = q3.value(0).toInt();
    }

    // Séances ce mois
    QSqlQuery q4;
    q4.prepare(
        "SELECT COUNT(*) FROM SEANCE "
        "WHERE TO_CHAR(DATE_SEANCE, 'YYYY-MM') = TO_CHAR(:today, 'YYYY-MM')"
        );
    q4.bindValue(":today", QDate::currentDate());
    if (q4.exec() && q4.next()) {
        stats["mois"] = q4.value(0).toInt();
    }

    // Séances à venir
    QSqlQuery q5;
    q5.prepare("SELECT COUNT(*) FROM SEANCE WHERE DATE_SEANCE > :today");
    q5.bindValue(":today", QDate::currentDate());
    if (q5.exec() && q5.next()) {
        stats["avenir"] = q5.value(0).toInt();
    }

    // Séances passées
    QSqlQuery q6;
    q6.prepare("SELECT COUNT(*) FROM SEANCE WHERE DATE_SEANCE < :today");
    q6.bindValue(":today", QDate::currentDate());
    if (q6.exec() && q6.next()) {
        stats["passees"] = q6.value(0).toInt();
    }

    // Client le plus actif
    QSqlQuery q7(
        "SELECT ID_CLIENT, COUNT(*) AS nb FROM SEANCE "
        "GROUP BY ID_CLIENT ORDER BY nb DESC FETCH FIRST 1 ROW ONLY"
        );
    if (q7.next()) {
        stats["client_actif"] = q7.value(0).toInt();
        stats["client_actif_count"] = q7.value(1).toInt();
    }

    // Employé le plus sollicité
    QSqlQuery q8(
        "SELECT ID_EMPLOYE, COUNT(*) AS nb FROM SEANCE "
        "GROUP BY ID_EMPLOYE ORDER BY nb DESC FETCH FIRST 1 ROW ONLY"
        );
    if (q8.next()) {
        stats["employe_actif"] = q8.value(0).toInt();
        stats["employe_actif_count"] = q8.value(1).toInt();
    }

    return stats;
}

// ==================== GÉNÉRATION GRAPHIQUE AMÉLIORÉE ====================

QChartView* Seance::genererGraphiqueBarres(const QString &periode)
{
    QSqlQueryModel *model = statistiquesParPeriode(periode);

    if (!model || model->rowCount() == 0) {
        delete model;
        return nullptr;
    }

    // Créer la série de barres
    QBarSeries *series = new QBarSeries();
    QBarSet *set = new QBarSet("Séances");

    // Couleur moderne
    set->setBrush(QColor("#FFC300"));
    set->setBorderColor(QColor("#FF8C00"));
    set->setLabelColor(Qt::white);

    QStringList categories;

    for (int i = 0; i < model->rowCount(); i++) {
        QString label = model->record(i).value(0).toString().trimmed();
        int count = model->record(i).value(1).toInt();

        *set << count;
        categories << label;
    }

    series->append(set);
    series->setLabelsVisible(true);
    series->setLabelsFormat("@value");

    // Créer le chart
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Statistiques des séances par " + periode);
    chart->setTitleBrush(Qt::white);
    chart->setBackgroundVisible(false);
    chart->legend()->setVisible(false);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // Axe X
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setLabelsColor(Qt::white);
    axisX->setGridLineVisible(false);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // Axe Y
    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Nombre de séances");
    axisY->setTitleBrush(Qt::white);
    axisY->setLabelsColor(Qt::white);
    axisY->setGridLineColor(QColor("#4A4A6A"));
    axisY->applyNiceNumbers();
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    // Hover effect
    QObject::connect(set, &QBarSet::hovered, [set](bool hovered, int) {
        if (hovered) {
            set->setBrush(QColor("#FFD700"));
        } else {
            set->setBrush(QColor("#FFC300"));
        }
    });

    // Vue finale
    QChartView *view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);
    view->setMinimumHeight(400);

    delete model;
    return view;
}

// ==================== GRAPHIQUE EN LIGNE AMÉLIORÉ ====================

QChartView* Seance::genererGraphiqueLigne(const QString &periode)
{
    QSqlQueryModel *model = statistiquesParPeriode(periode);

    if (!model || model->rowCount() == 0) {
        delete model;
        return nullptr;
    }

    QSplineSeries *series = new QSplineSeries();
    series->setColor(QColor("#FFC300"));
    series->setPen(QPen(QColor("#FFC300"), 3));
    series->setPointsVisible(true);

    QStringList labels;
    int maxValue = 0;

    for (int i = 0; i < model->rowCount(); i++) {
        QString label = model->record(i).value(0).toString().trimmed();
        int count = model->record(i).value(1).toInt();

        series->append(i, count);
        labels << label;

        if (count > maxValue) maxValue = count;
    }

    // Chart
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Évolution des séances par " + periode);
    chart->setTitleBrush(Qt::white);
    chart->setBackgroundVisible(false);
    chart->legend()->hide();
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // Axe X
    QCategoryAxis *axisX = new QCategoryAxis();
    for (int i = 0; i < labels.size(); i++) {
        axisX->append(labels[i], i);
    }
    axisX->setLabelsColor(Qt::white);
    axisX->setGridLineVisible(false);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // Axe Y
    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, maxValue + 2);
    axisY->setTitleText("Nombre de séances");
    axisY->setTitleBrush(Qt::white);
    axisY->setLabelsColor(Qt::white);
    axisY->setGridLineColor(QColor("#4A4A6A"));
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    // Vue finale
    QChartView *view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);
    view->setMinimumHeight(400);

    delete model;
    return view;
}

// ==================== MÉTHODES SIMPLIFIÉES (COMPATIBILITÉ) ====================

void Seance::statsParJour(QStringList &labels, QList<int> &counts)
{
    labels.clear();
    counts.clear();

    QSqlQueryModel *model = statistiquesParPeriode("jour");

    for (int i = 0; i < model->rowCount(); i++) {
        labels << model->record(i).value(0).toString().trimmed();
        counts << model->record(i).value(1).toInt();
    }

    delete model;
}

void Seance::statsParMois(QStringList &labels, QList<int> &counts)
{
    labels.clear();
    counts.clear();

    QSqlQueryModel *model = statistiquesParPeriode("mois");

    for (int i = 0; i < model->rowCount(); i++) {
        labels << model->record(i).value(0).toString().trimmed();
        counts << model->record(i).value(1).toInt();
    }

    delete model;
}

QChartView* Seance::genererCourbe(const QStringList &labels,
                                  const QList<int> &counts,
                                  const QString &titre)
{
    if (labels.isEmpty() || counts.isEmpty()) {
        return nullptr;
    }

    QSplineSeries *series = new QSplineSeries();
    series->setColor(QColor("#FFC300"));
    series->setPen(QPen(QColor("#FFC300"), 4));
    series->setPointsVisible(true);

    int maxVal = 0;
    for (int i = 0; i < counts.size(); i++) {
        series->append(i, counts[i]);
        if (counts[i] > maxVal) maxVal = counts[i];
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(titre);
    chart->setTitleBrush(Qt::white);
    chart->setBackgroundVisible(false);
    chart->legend()->hide();
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // Axe X
    QCategoryAxis *axisX = new QCategoryAxis();
    for (int i = 0; i < labels.size(); i++)
        axisX->append(labels[i], i);

    axisX->setLabelsColor(Qt::white);
    axisX->setGridLineVisible(false);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // Axe Y
    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, maxVal + 2);
    axisY->setTitleText("Nombre de séances");
    axisY->setTitleBrush(Qt::white);
    axisY->setLabelsColor(Qt::white);
    axisY->setGridLineColor(QColor("#4A4A6A"));
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QChartView *view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);
    view->setMinimumHeight(400);

    return view;
}
////////////////////weather////////////////////////////

// Get matricule (this should already exist in your seance.cpp, but here it is)
int Seance::getMatricule() const {
    return matricule;
}

// Get client name from database
QString Seance::getClientName(int idClient)
{
    QSqlQuery query;
    query.prepare("SELECT NOM, PRENOM FROM CLIENT WHERE ID_CLIENT = :id");
    query.bindValue(":id", idClient);

    if (query.exec() && query.next()) {
        QString nom = query.value("NOM").toString();
        QString prenom = query.value("PRENOM").toString();
        return prenom + " " + nom;
    }

    return "Client #" + QString::number(idClient);
}

// Get employee name from database
QString Seance::getEmployeName(int idEmploye)
{
    QSqlQuery query;
    query.prepare("SELECT NOM, PRENOM FROM EMPLOYE WHERE ID_EMPLOYE = :id");
    query.bindValue(":id", idEmploye);

    if (query.exec() && query.next()) {
        QString nom = query.value("NOM").toString();
        QString prenom = query.value("PRENOM").toString();
        return prenom + " " + nom;
    }

    return "Employé #" + QString::number(idEmploye);
}

// Group all seances by date
QMap<QDate, QList<Seance>> Seance::groupSeancesByDate()
{
    QMap<QDate, QList<Seance>> seancesByDate;

    // Query database directly instead of calling getAllSeances()
    QSqlQuery q("SELECT ID_SEANCE, DATE_SEANCE, HEURE, ID_CLIENT, ID_EMPLOYE, MATRICULE FROM SEANCE");

    while (q.next()) {
        int id = q.value(0).toInt();
        QDate d = q.value(1).toDate();
        int h = q.value(2).toInt();
        QTime heure(h / 100, h % 100);
        int idc = q.value(3).toInt();
        int ide = q.value(4).toInt();
        int mat = q.value(5).toInt();

        Seance s(id, d, heure, idc, ide, mat);
        seancesByDate[d].append(s);
    }

    return seancesByDate;
}

// Get count of seances per date
QMap<QDate, int> Seance::getSeancesCountByDate()
{
    QMap<QDate, int> countByDate;
    QSqlQuery query("SELECT DATE_SEANCE, COUNT(*) FROM SEANCE GROUP BY DATE_SEANCE");

    while (query.next()) {
        QDate date = query.value(0).toDate();
        int count = query.value(1).toInt();
        countByDate[date] = count;
    }

    return countByDate;
}
QList<Seance> Seance::getSeancesByDate(const QDate &date)
{
    QList<Seance> list;
    QSqlQuery q;
    q.prepare("SELECT ID_SEANCE, DATE_SEANCE, HEURE, ID_CLIENT, ID_EMPLOYE, MATRICULE "
              "FROM SEANCE WHERE DATE_SEANCE = :d");
    q.bindValue(":d", date);
    q.exec();

    while (q.next()) {
        int id = q.value(0).toInt();
        QDate d = q.value(1).toDate();
        int h = q.value(2).toInt();
        QTime heure(h / 100, h % 100);
        int idc = q.value(3).toInt();
        int ide = q.value(4).toInt();
        int mat = q.value(5).toInt();

        list.append(Seance(id, d, heure, idc, ide, mat));
    }
    return list;
}
