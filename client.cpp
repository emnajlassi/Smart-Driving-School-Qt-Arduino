#include "client.h"
#include <QSqlError>
#include <QDebug>
#include <QRegularExpression>
#include <QMessageBox>
#include <QPdfWriter>
#include <QPainter>
#include <QDateTime>
#include <QDir>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QSqlRecord>
#include "qrcodegen.hpp"
#include <QDesktopServices>
#include <QUrl>
// Pour l'email
#include <QtNetwork/QTcpSocket>
#include <QTextStream>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QEventLoop>
Client::Client(int id, QString nom, QString prenom, QString niveau, QString telephone, QString email)
{
    this->id_client=id;
    this->nom=nom;
    this->prenom=prenom;
    this->niveau=niveau;
    this->telephone=telephone;
    this->email=email;
}

//---------------------------------------------------------CRUD-----------------------------------------------------------------

// ---------------- AJOUTER ----------------
bool Client::ajouter()
{
    //Contrôle de saisie
    QRegularExpression regexNom("^[A-Za-zÀ-ÖØ-öø-ÿ\\s'-]+$");  // only letters/spaces
    QRegularExpression regexTel("^[0-9]{8,15}$");              // only numbers (8–15)
    QRegularExpression regexEmail("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");

    if (!regexNom.match(nom).hasMatch()) {
        QMessageBox::warning(nullptr, "Erreur", "Nom invalide — uniquement lettres et espaces !");
        return false;
    }

    if (!regexNom.match(prenom).hasMatch()) {
        QMessageBox::warning(nullptr, "Erreur", "Prénom invalide — uniquement lettres et espaces !");
        return false;
    }

    if (!regexTel.match(telephone).hasMatch()) {
        QMessageBox::warning(nullptr, "Erreur", "Numéro de téléphone invalide — uniquement chiffres !");
        return false;
    }

    if (!regexEmail.match(email).hasMatch()) {
        QMessageBox::warning(nullptr, "Erreur", "Adresse e-mail invalide !");
        return false;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO CLIENT (ID_CLIENT, NOM, PRENOM, NIVEAU, TELEPHONE, EMAIL) " "VALUES (:id, :nom, :prenom, :niveau, :telephone, :email)");
    query.bindValue(":id", id_client);
    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":niveau", niveau);
    query.bindValue(":telephone", telephone);
    query.bindValue(":email", email);
    return query.exec();
}

// ---------------- SUPPRIMER ----------------
bool Client::supprimer(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM CLIENT WHERE ID_CLIENT = :id");
    query.bindValue(":id", id);
    return query.exec();
}

// ---------------- MODIFIER ----------------
bool Client::modifier(int id)
{
    //Contrôle de saisie
    QRegularExpression regexNom("^[A-Za-zÀ-ÖØ-öø-ÿ\\s'-]+$");
    QRegularExpression regexTel("^[0-9]{8,15}$");
    QRegularExpression regexEmail("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");

    if (!nom.trimmed().isEmpty() && !regexNom.match(nom).hasMatch()) {
        QMessageBox::warning(nullptr, "Erreur", "Nom invalide — uniquement lettres et espaces !");
        return false;
    }

    if (!prenom.trimmed().isEmpty() && !regexNom.match(prenom).hasMatch()) {
        QMessageBox::warning(nullptr, "Erreur", "Prénom invalide — uniquement lettres et espaces !");
        return false;
    }

    if (!telephone.trimmed().isEmpty() && !regexTel.match(telephone).hasMatch()) {
        QMessageBox::warning(nullptr, "Erreur", "Numéro de téléphone invalide !");
        return false;
    }

    if (!email.trimmed().isEmpty() && !regexEmail.match(email).hasMatch()) {
        QMessageBox::warning(nullptr, "Erreur", "Adresse e-mail invalide !");
        return false;
    }
    QSqlQuery query;
    query.prepare(
        "UPDATE CLIENT SET "
        "  NOM = COALESCE(NULLIF(:nom, ''), NOM), "
        "  PRENOM = COALESCE(NULLIF(:prenom, ''), PRENOM), "
        "  NIVEAU = COALESCE(NULLIF(:niveau, ''), NIVEAU), "
        "  TELEPHONE = COALESCE(NULLIF(:telephone, ''), TELEPHONE), "
        "  EMAIL = COALESCE(NULLIF(:email, ''), EMAIL) "
        "WHERE ID_CLIENT = :id"
        );

    query.bindValue(":id", id);
    query.bindValue(":nom", nom.trimmed());
    query.bindValue(":prenom", prenom.trimmed());
    query.bindValue(":niveau", niveau.trimmed());
    query.bindValue(":telephone", telephone.trimmed());
    query.bindValue(":email", email.trimmed());
    return query.exec();
}

// ---------------- AFFICHER ----------------
QSqlQueryModel* Client::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT ID_CLIENT, NOM, PRENOM, NIVEAU, TELEPHONE, EMAIL FROM CLIENT");

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Prénom"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Niveau"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Téléphone"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Email"));
    return model;
}
// ---------------- RECHERCHER ----------------
QSqlQueryModel* Client::rechercher(const QString &critere)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlQuery query;

    if (critere.trimmed().isEmpty()) {
        // show all clients if search is empty
        query.prepare("SELECT ID_CLIENT, NOM, PRENOM, NIVEAU, TELEPHONE, EMAIL FROM CLIENT");
    } else {
        // search by ID, Nom, Prénom, Téléphone, or Email
        query.prepare("SELECT ID_CLIENT, NOM, PRENOM, NIVEAU, TELEPHONE, EMAIL "
                      "FROM CLIENT "
                      "WHERE LOWER(NOM) LIKE LOWER(:critere) "
                      "   OR LOWER(PRENOM) LIKE LOWER(:critere) "
                      "   OR LOWER(EMAIL) LIKE LOWER(:critere) "
                      "   OR TELEPHONE LIKE :critere "
                      "   OR CAST(ID_CLIENT AS VARCHAR2(20)) LIKE :critere");
        query.bindValue(":critere", "%" + critere + "%");
    }

    query.exec();
    model->setQuery(query);

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Prénom"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Niveau"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Téléphone"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Email"));

    return model;
}
// ---------------- TRI ----------------
QSqlQueryModel* Client::trier(const QString &critere)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QString queryStr;

    QString c = critere.trimmed().toLower(); // normalize input text

    if (c.contains("id croissant"))
        queryStr = "SELECT * FROM client ORDER BY id_client ASC";
    else if (c.contains("id décroissant") || c.contains("id decroissant"))
        queryStr = "SELECT * FROM client ORDER BY id_client DESC";
    else if (c.contains("nom a"))
        queryStr = "SELECT * FROM client ORDER BY nom ASC";
    else if (c.contains("nom z"))
        queryStr = "SELECT * FROM client ORDER BY nom DESC";
    else
        queryStr = "SELECT * FROM client";

    model->setQuery(queryStr);
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Prénom"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Niveau"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Téléphone"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Email"));
    return model;
}
// ---------------- EXPORT-PDF ----------------
bool Client::exporterPDF(const QString &filePath)
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
    painter.drawText(200, 150, "Liste des clients");
    painter.setFont(QFont("Times New Roman", 9, QFont::StyleItalic));
    painter.drawText(4300, 150, "Généré le : " + QDate::currentDate().toString("dd/MM/yyyy"));
    painter.setFont(headerFont);

    // Column X positions (balanced for A4)
    int col1 = 100;   // ID
    int col2 = 300;   // Nom
    int col3 = 600;  // Prénom
    int col4 = 900;  // Niveau
    int col5 = 1300;  // Téléphone
    int col6 = 1700;  // Email

    int y = 400;
    int rowHeight = 220;
    int bottomMargin = 10500;

    // Headers
    painter.drawText(col1, y, "ID");
    painter.drawText(col2, y, "Nom");
    painter.drawText(col3, y, "Prénom");
    painter.drawText(col4, y, "Niveau");
    painter.drawText(col5, y, "Téléphone");
    painter.drawText(col6, y, "Email");

    y += rowHeight;
    painter.setFont(textFont);

    // Data
    QSqlQuery query("SELECT id_client, nom, prenom, niveau, telephone, email FROM client ORDER BY id_client ASC");
    while (query.next()) {
        if (y > bottomMargin) {
            pdf.newPage();
            y = 400;
            painter.setFont(headerFont);
            painter.drawText(col1, y, "ID");
            painter.drawText(col2, y, "Nom");
            painter.drawText(col3, y, "Prénom");
            painter.drawText(col4, y, "Niveau");
            painter.drawText(col5, y, "Téléphone");
            painter.drawText(col6, y, "Email");
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
// N'oublie pas d'ajouter : using namespace qrcodegen; tout en haut ou dans la fonction

QPixmap Client::genererVraiQRCode(int size) const
{
    using namespace qrcodegen; // Utilisation du namespace de la nouvelle lib

    QString vCard = QStringLiteral(
                        "BEGIN:VCARD\r\n"
                        "VERSION:3.0\r\n"
                        "FN:%1 %2\r\n"
                        "N:%2;%1;;;\r\n"
                        "TEL;TYPE=CELL:%3\r\n"
                        "EMAIL:%4\r\n"
                        "NOTE:Client Auto-école | ID: %5 | Niveau: %6\r\n"
                        "END:VCARD"
                        ).arg(nom, prenom, telephone, email, QString::number(id_client), niveau);

    // Création du QR Code avec la nouvelle librairie
    QrCode qr = QrCode::encodeText(vCard.toUtf8().constData(), QrCode::Ecc::MEDIUM);

    // Dessin du QR Code
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    painter.setBrush(Qt::black);
    painter.setPen(Qt::NoPen);

    // Calcul de l'échelle
    // qr.getSize() donne la taille de la grille (ex: 21x21 modules)
    double scale = static_cast<double>(size) / qr.getSize();

    for (int y = 0; y < qr.getSize(); y++) {
        for (int x = 0; x < qr.getSize(); x++) {
            if (qr.getModule(x, y)) { // Si le module est noir
                painter.drawRect(QRectF(x * scale, y * scale, scale, scale));
            }
        }
    }

    return pixmap;
}
bool Client::genererBitakitTaarifPDF(const QString &filePath) const
{
    QPdfWriter pdf(filePath);
    pdf.setPageSize(QPageSize(QPageSize::A6));      // Format carte
    pdf.setResolution(300);
    pdf.setPageMargins(QMarginsF(20, 20, 20, 20));   // Marges aérées

    QPainter painter(&pdf);
    if (!painter.isActive()) return false;

    // Fond blanc
    painter.fillRect(QRect(0, 0, pdf.width(), pdf.height()), Qt::white);

    // Titre "SCAN HERE" en gros et centré
    painter.setPen(QColor("#2E86AB"));
    painter.setFont(QFont("Segoe UI", 28, QFont::Bold));
    painter.drawText(0, 80, pdf.width(), 80, Qt::AlignCenter, "SCAN HERE");

    // QR Code bien gros et centré
    QPixmap qr = genererVraiQRCode(380);  // QR bien visible
    int qrX = (pdf.width() - 380) / 2;
    int qrY = 180;
    painter.drawPixmap(qrX, qrY, qr);

    // Petit texte discret en bas
    painter.setPen(QColor("#666666"));
    painter.setFont(QFont("Segoe UI", 10, QFont::StyleItalic));
    painter.drawText(0, pdf.height() - 50, pdf.width(), 40, Qt::AlignCenter,
                     "Scannez pour ajouter le contact");

    painter.end();
    return true;
}
// ==================== FONCTION EMAIL ====================


bool Client::envoyerEmailConfirmation() const
{
    qDebug() << "=== DEBUG START ===";
    qDebug() << "🔄 Envoi email à:" << this->email;
    qDebug() << "Client data - Nom:" << nom << "Prénom:" << prenom;
    qDebug() << "ID:" << id_client << "Niveau:" << niveau;

    // ✅ PREMIÈRE FENÊTRE - Confirmation d'envoi
    QMessageBox::information(nullptr, "📨 Envoi en cours",
                             "Envoi de l'email de confirmation en cours...\n\n"
                             "Veuillez patienter pendant le traitement.");

    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl("https://api.emailjs.com/api/v1.0/email/send"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // ✅ ADD BROWSER HEADERS TO BYPASS BLOCK
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
    request.setRawHeader("Origin", "https://yourdomain.com");
    request.setRawHeader("Referer", "https://yourdomain.com/");

    QJsonObject emailData;
    emailData["service_id"] = "service_dhgzugb";  // ← NOUVEAU SERVICE ID!
    emailData["template_id"] = "template_t3steii";
    emailData["user_id"] = "GFFDECIyU3Fmdx8cT";

    QJsonObject templateParams;
    templateParams["to_email"] = this->email;
    templateParams["title"] = "Confirmation d'Inscription";
    templateParams["client_name"] = prenom + " " + nom;
    templateParams["client_id"] = QString::number(id_client);
    templateParams["client_level"] = niveau;
    templateParams["client_phone"] = telephone;
    templateParams["signup_date"] = QDateTime::currentDateTime().toString("dd/MM/yyyy");

    emailData["template_params"] = templateParams;

    QJsonDocument doc(emailData);
    QByteArray data = doc.toJson();

    qDebug() << "📨 Data being sent to EmailJS:" << data;

    QNetworkReply *reply = manager.post(request, data);
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    bool success = (reply->error() == QNetworkReply::NoError);
    QString response = QString(reply->readAll());
    int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    qDebug() << "=== DEBUG RESPONSE ===";
    qDebug() << "📨 HTTP Status:" << httpStatus;
    qDebug() << "📨 Full Response:" << response;
    qDebug() << "📨 Error:" << reply->errorString();
    qDebug() << "=== DEBUG END ===";

    reply->deleteLater();

    if (success && httpStatus == 200) {
        qDebug() << "✅ EMAIL ENVOYÉ AVEC SUCCÈS!";
        QMessageBox::information(nullptr, "✅ Envoi réussi",
                                 "L'email de confirmation a été envoyé avec succès.\n\n"
                                 "Le client recevra sous peu les détails de son inscription.");
        return true;
    } else {
        qDebug() << "❌ Échec - Status:" << httpStatus;
        QMessageBox::warning(nullptr, "⚠️ Échec d'envoi",
                             "L'envoi de l'email de confirmation a échoué.\n\n"
                             "Veuillez vérifier la connexion et réessayer.");
        return false;
    }
}
 //--------------------------stat----------------------------
QSqlQueryModel* Client::statistiquesParNiveau()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT niveau, COUNT(*) AS total FROM CLIENT GROUP BY niveau");

    // Debug optionnel
    qDebug() << "[Client::statistiquesParNiveau] rows =" << model->rowCount();
    for (int i = 0; i < model->rowCount(); ++i) {
        qDebug() << "ROW" << i << "=>"
                 << model->record(i).value("niveau").toString()
                 << model->record(i).value("total").toInt();
    }

    return model;
}
//------------------métier-avancé-examen----------------------------
// ---------------- METIER ----------------
QSqlQueryModel* Client::identifierClientsEnDifficulte()
{
    QSqlQueryModel *model = new QSqlQueryModel();

    model->setQuery(
        "SELECT c.ID_CLIENT, c.NOM, c.PRENOM, c.NIVEAU, c.TELEPHONE, c.EMAIL, "
        "       COALESCE(SUM(CASE WHEN e.RESULTAT = 'Échoué' THEN 1 ELSE 0 END), 0) AS NB_ECHECS, "
        "       COALESCE(SUM(CASE WHEN e.RESULTAT IS NULL OR TRIM(e.RESULTAT) = '' OR e.RESULTAT = '— Sélectionner —' THEN 1 ELSE 0 END), 0) AS NB_NON_REALISES "
        "FROM CLIENT c "
        "LEFT JOIN EXAMEN e ON e.ID_CLIENT = c.ID_CLIENT "
        "GROUP BY c.ID_CLIENT, c.NOM, c.PRENOM, c.NIVEAU, c.TELEPHONE, c.EMAIL "
        "HAVING SUM(CASE WHEN e.RESULTAT = 'Échoué' THEN 1 ELSE 0 END) >= 2 "
        "    OR SUM(CASE WHEN e.RESULTAT IS NULL OR TRIM(e.RESULTAT) = '' OR e.RESULTAT = '— Sélectionner —' THEN 1 ELSE 0 END) >= 1"
        );

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Prénom"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Niveau"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Téléphone"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Email"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Nb échecs"));
    model->setHeaderData(7, Qt::Horizontal, QObject::tr("Nb non réalisés"));

    return model;
}
