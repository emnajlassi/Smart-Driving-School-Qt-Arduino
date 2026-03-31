#include "employe.h"
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>
#include <QRegularExpression>   //for validation patterns
#include <QPdfWriter>
#include <QPainter>
#include <QDateTime>
#include <QDir>



employe::employe(int id, QString nom, QString prenom, QString telephone, QString password, QString specialite, QString question, QString reponse, QString poste) {
    this->id_employe = id;
    this->nom = nom;
    this->prenom = prenom;
    this->telephone = telephone;
    this->password = password;
    this->specialite = specialite;
    this->question = question;
    this->reponse = reponse;
    this->poste = poste;

}

//-------------------------------------------------CRUD-------------------------------------------------------

//------------------------------------ajouter---------------------------------

bool employe::ajouter()
{
    //Vérifier les champs non vides
    if (id_employe == 0 || nom.trimmed().isEmpty() || prenom.trimmed().isEmpty() ||
        telephone.trimmed().isEmpty() ||question.trimmed().isEmpty() ||reponse.trimmed().isEmpty() ||poste.trimmed().isEmpty()|| password.trimmed().isEmpty() || specialite.trimmed().isEmpty()) {
        QMessageBox::warning(nullptr,"Erreur","Champs obligatoires manquants !");
        return false;
    }
    //Contrôle de saisie
    static const QRegularExpression regexNom("^[A-Za-zÀ-ÿ\\s]+$"); // Lettres + espaces
    static const QRegularExpression regexTel("^[0-9]+$");          // Chiffres uniquement
    if (!regexNom.match(nom).hasMatch()) {
        QMessageBox::warning(nullptr, "Erreur", "Nom invalide — uniquement lettres ou espaces.");
        return false;
    }
    if (!regexNom.match(prenom).hasMatch()) {
        QMessageBox::warning(nullptr, "Erreur", "Nom invalide — uniquement lettres ou espaces.");
        return false;
    }
    if (!regexTel.match(telephone).hasMatch()) {
        QMessageBox::warning(nullptr, "Erreur", "Téléphone invalide — doit contenir uniquement des chiffres .");
        return false;
    }
    QSqlQuery query;
    query.prepare("INSERT INTO employe (id_employe, nom, prenom, telephone, password, specialite,question,reponse,poste) " "VALUES (:id, :nom, :prenom, :telephone, :password, :specialite, :question, :reponse, :poste)");
    query.bindValue(":id", id_employe);
    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":telephone", telephone);
    query.bindValue(":password", password);
    query.bindValue(":question", question);
    query.bindValue(":reponse", reponse);
    query.bindValue(":poste", poste);
    query.bindValue(":specialite", specialite);

    return query.exec();
}
//------------------------------------supprimer---------------------------------

bool employe::supprimer(int id)
{
    // Vérifier si l'ID existe avant de supprimer
    QSqlQuery check;
    check.prepare("SELECT COUNT(*) FROM employe WHERE id_employe = :id");
    check.bindValue(":id", id);
    check.exec();
    check.next();

    if (check.value(0).toInt() == 0) {
        QMessageBox::warning(nullptr, "Erreur", "Suppression impossible : l'ID employé n'existe pas.");
        return false;
    }
    QSqlQuery query;
    QString res = QString::number(id);
    query.prepare("DELETE FROM employe WHERE id_employe = :id");
    query.bindValue(":id", res);
    return query.exec();
}

//------------------------------------modifier---------------------------------
bool employe::modifier(int id)
{
    // Vérifier si l'ID existe avant de modifier
    QSqlQuery check;
    check.prepare("SELECT COUNT(*) FROM employe WHERE id_employe = :id");
    check.bindValue(":id", id);
    check.exec();
    check.next();

    if (check.value(0).toInt() == 0) {
        QMessageBox::warning(nullptr, "Erreur", "Modification impossible : l'ID employé n'existe pas.");
        return false;
    }

    // Contrôle de saisie
    static const QRegularExpression regexNom("^[A-Za-zÀ-ÿ\\s]+$");
    static const QRegularExpression regexTel("^[0-9]+$");

    if (!nom.isEmpty() && !regexNom.match(nom).hasMatch()) {
        QMessageBox::warning(nullptr, "Erreur", "Nom invalide — uniquement lettres ou espaces.");
        return false;
    }
    if (!prenom.isEmpty() && !regexNom.match(prenom).hasMatch()) {
        QMessageBox::warning(nullptr, "Erreur", "Nom invalide — uniquement lettres ou espaces.");
        return false;
    }
    if (!telephone.isEmpty() && !regexTel.match(telephone).hasMatch()) {
        QMessageBox::warning(nullptr, "Erreur", "Téléphone invalide — doit contenir uniquement des chiffres.");
        return false;
    }

    // Prevent updating password, question, and answer
    if (!password.isEmpty() || !question.isEmpty() || !reponse.isEmpty()) {
        QMessageBox::warning(nullptr, "Erreur", "Vous ne pouvez pas modifier le mot de passe, la question et la réponse.");
        return false;  // Return false to prevent the update
    }

    // Continue with the update for other fields
    QSqlQuery query;
    query.prepare(
        "UPDATE employe SET "
        "  nom = COALESCE(NULLIF(:nom, ''), nom),"
        "  prenom = COALESCE(NULLIF(:prenom, ''), prenom),"
        "  telephone = COALESCE(NULLIF(:telephone, ''), telephone),"
        "  specialite = COALESCE(NULLIF(:specialite, ''), specialite),"
        "  poste = COALESCE(NULLIF(:poste, ''), poste) "  // Correctly set the poste field
        "WHERE id_employe = :id"
        );
    query.bindValue(":id", id);
    query.bindValue(":nom", nom.trimmed());
    query.bindValue(":prenom", prenom.trimmed());
    query.bindValue(":telephone", telephone.trimmed());
    query.bindValue(":specialite", specialite.trimmed());
    query.bindValue(":poste", poste.trimmed());  // Add the binding for poste

    bool success = query.exec();

    if (!success) {
        qDebug() << "Error updating employee:" << query.lastError().text();
    }

    return success;
}


//------------------------------------afficher---------------------------------

QSqlQueryModel* employe::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    // Modify the query to exclude the motdepasse column
    QString queryStr = "SELECT id_employe, nom, prenom, specialite, telephone, poste FROM employe";
    model->setQuery(queryStr);

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Prénom"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Spécialité"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Poste"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Téléphone"));

    return model;
}
//***************************************************
QString employe::getPosteById(int id)
{
    QSqlQuery query;
    query.prepare("SELECT poste FROM employe WHERE id_employe = :id");
    query.bindValue(":id", id);
    if(query.exec() && query.next())
        return query.value(0).toString();
    return "";
}

//-------------------------------------------------------------METIER------------------------------------------------------

QString employe::verifierLoginNom(int id, QString password)
{
    QSqlQuery query;
    query.prepare("SELECT nom, prenom FROM employe WHERE id_employe = :id AND password = :pwd");
    query.bindValue(":id", id);
    query.bindValue(":pwd", password);

    if (query.exec() && query.next()) {
        QString nomComplet = query.value("prenom").toString() + " " + query.value("nom").toString();
        return nomComplet;
    }
    return ""; // Retourne vide si identifiants incorrects
}
// Check if employee exists
bool employe::doesEmployeeExist(int id)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM employe WHERE id_employe = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;  // Return true if the employee exists
    }
    return false;
}

// Get the security question for the employee
QString employe::getSecurityQuestion(int id)
{
    QSqlQuery query;
    query.prepare("SELECT question FROM employe WHERE id_employe = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        return query.value(0).toString();  // Return the security question
    }
    return "";
}

// Verify if the provided question matches the stored question
bool employe::verifyQuestion(int id, QString question)
{
    QSqlQuery query;
    query.prepare("SELECT question FROM employe WHERE id_employe = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        QString storedQuestion = query.value(0).toString();
        return storedQuestion == question;  // Check if the provided question matches
    }
    return false;
}

// Verify if the provided answer matches the stored answer
bool employe::verifyAnswer(int id, QString question, QString answer)
{
    QSqlQuery query;
    query.prepare("SELECT reponse FROM employe WHERE id_employe = :id AND question = :question");
    query.bindValue(":id", id);
    query.bindValue(":question", question);

    if (query.exec() && query.next()) {
        QString correctAnswer = query.value(0).toString();
        return correctAnswer == answer;  // Check if the answer matches
    }
    return false;
}

// Update the employee's password
bool employe::updatePassword(int id, QString newPassword)
{
    QSqlQuery query;
    query.prepare("UPDATE employe SET password = :newPassword WHERE id_employe = :id");
    query.bindValue(":id", id);
    query.bindValue(":newPassword", newPassword);
    return query.exec();  // Return true if update is successful
}
//----------------------------------------------------------chercher-------------------------------
QSqlQueryModel* employe::rechercher(const QString &critere)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlQuery query;

    if (critere.trimmed().isEmpty()) {
        // Show all employees when search bar is empty
        query.prepare("SELECT id_employe, nom, prenom, telephone, specialite, poste FROM employe");
    } else {
        query.prepare("SELECT id_employe, nom, prenom, telephone, specialite, poste "
                      "FROM employe "
                      "WHERE LOWER(nom) LIKE LOWER(:critere) "
                      "   OR LOWER(prenom) LIKE LOWER(:critere) "
                      "   OR LOWER(specialite) LIKE LOWER(:critere) "
                      "   OR LOWER(poste) LIKE LOWER(:critere) "
                      "   OR telephone LIKE :critere "
                      "   OR CAST(id_employe AS VARCHAR2(20)) LIKE :critere");

        query.bindValue(":critere", "%" + critere + "%");
    }

    query.exec();
    model->setQuery(query);

    // Set headers
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Prénom"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Téléphone"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Spécialité"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Poste"));

    return model;
}


//----------------------------------------------------------tri-------------------------------
QSqlQueryModel* employe::trier(const QString &critere)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QString queryStr;

    QString c = critere.trimmed().toLower();

    if (c.contains("id croissant"))
        queryStr = "SELECT id_employe, nom, prenom, telephone, specialite, poste FROM employe ORDER BY id_employe ASC";
    else if (c.contains("id décroissant") || c.contains("id decroissant"))
        queryStr = "SELECT id_employe, nom, prenom, telephone, specialite, poste FROM employe ORDER BY id_employe DESC";
    else if (c.contains("nom a"))
        queryStr = "SELECT id_employe, nom, prenom, telephone, specialite, poste FROM employe ORDER BY nom ASC";
    else if (c.contains("nom z"))
        queryStr = "SELECT id_employe, nom, prenom, telephone, specialite, poste FROM employe ORDER BY nom DESC";
    else if (c.contains("poste a"))
        queryStr = "SELECT id_employe, nom, prenom, telephone, specialite, poste FROM employe ORDER BY poste ASC";
    else if (c.contains("poste z"))
        queryStr = "SELECT id_employe, nom, prenom, telephone, specialite, poste FROM employe ORDER BY poste DESC";
    else
        queryStr = "SELECT id_employe, nom, prenom, telephone, specialite, poste FROM employe";

    model->setQuery(queryStr);

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Prénom"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Téléphone"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Spécialité"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Poste"));

    return model;
}

//------------------------------------------------------stat------------------------------------------

QSqlQueryModel* employe::statistiquesParSpecialite() const
{
    QSqlQueryModel *model = new QSqlQueryModel();

    model->setQuery(
        "SELECT SPECIALITE AS specialite, COUNT(*) AS total "
        "FROM EMPLOYE "
        "GROUP BY SPECIALITE"
        );

    return model;
}
//----------------------------------------------------------export-pdf-------------------------------
bool employe::exporterPDF(const QString &filePath)
{
    QPdfWriter pdf(filePath);
    pdf.setPageSize(QPageSize(QPageSize::A4));
    pdf.setResolution(300);

    QPainter painter(&pdf);
    if (!painter.isActive())
        return false;

    // Fonts avec tailles ajustées pour les grandes lignes
    QFont titleFont("Chaparral Pro", 35, QFont::Bold);
    QFont headerFont("Arial", 14, QFont::Bold);
    QFont textFont("Arial", 12);
    QFont dateFont("Arial", 10);
    QFont pageNumberFont("Arial", 10);

    // Couleurs
    QColor titleColor(244, 143, 134);
    QColor headerBgColor(240, 240, 240);
    QColor borderColor(200, 200, 200);

    int y = 150;
    int pageNumber = 1;
    bool isFirstPage = true;

    // Fonction pour dessiner l'en-tête de page
    auto drawPageHeader = [&](int currentPage, bool firstPage) {
        if (firstPage) {
            // Logo et titre uniquement sur la première page
            QImage logo(":/images/image.png");
            if (!logo.isNull()) {
                QSize logoSize(200, 200);
                int logoX = 2173;
                painter.drawImage(logoX, 10, logo.scaled(logoSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            }

            // Titre principal centré
            painter.setFont(titleFont);
            painter.setPen(titleColor);
            QString title = "Liste des employés";
            painter.drawText(570, 150, title);

            // Date d'exportation alignée à droite
            QDateTime currentTime = QDateTime::currentDateTime();
            QString dateTime = currentTime.toString("dd/MM/yyyy HH:mm:ss");
            painter.setFont(dateFont);
            painter.setPen(Qt::black);
            painter.drawText(1728, 350, "Généré le : " + dateTime);
        }

        // Numéro de page en bas sur toutes les pages
        painter.setFont(pageNumberFont);
        painter.setPen(Qt::gray);
        painter.drawText(1240, 3430, QString::number(currentPage));
    };

    // Dessiner l'en-tête de la première page
    drawPageHeader(pageNumber, isFirstPage);

    y = 450; // Position Y après l'en-tête pour le tableau

    // Définition des colonnes
    int col1 = 70;    // ID
    int col2 = col1 + 450;   // Nom
    int col3 = col2 + 450;   // Prénom
    int col4 = col3 + 450;   // Téléphone
    int col5 = col4 + 450;   // Spécialité

    int columnWidth = 450;
    int rowHeight = 100;
    int bottomMargin = 3400;

    // Fonction pour dessiner l'en-tête du tableau
    auto drawTableHeader = [&](int startY) {
        painter.setPen(QPen(borderColor, 2));
        painter.setFont(headerFont);
        painter.setBrush(headerBgColor);

        // Dessiner les cellules d'en-tête
        painter.drawRect(col1, startY, columnWidth, rowHeight);
        painter.drawRect(col2, startY, columnWidth, rowHeight);
        painter.drawRect(col3, startY, columnWidth, rowHeight);
        painter.drawRect(col4, startY, columnWidth, rowHeight);
        painter.drawRect(col5, startY, columnWidth, rowHeight);

        // Texte d'en-tête centré
        painter.setPen(Qt::black);

        QRect idHeaderRect(col1, startY, columnWidth, rowHeight);
        painter.drawText(idHeaderRect, Qt::AlignCenter, "ID");

        QRect nomHeaderRect(col2, startY, columnWidth, rowHeight);
        painter.drawText(nomHeaderRect, Qt::AlignCenter, "Nom");

        QRect prenomHeaderRect(col3, startY, columnWidth, rowHeight);
        painter.drawText(prenomHeaderRect, Qt::AlignCenter, "Prénom");

        QRect telHeaderRect(col4, startY, columnWidth, rowHeight);
        painter.drawText(telHeaderRect, Qt::AlignCenter, "Téléphone");

        QRect specHeaderRect(col5, startY, columnWidth, rowHeight);
        painter.drawText(specHeaderRect, Qt::AlignCenter, "Spécialité");
    };

    // Dessiner l'en-tête du tableau sur la première page
    drawTableHeader(y);
    y += rowHeight;

    // Données du tableau
    QSqlQuery query("SELECT id_employe, nom, prenom, telephone, specialite FROM employe ORDER BY id_employe ASC");

    while (query.next()) {
        // Vérifier si on a besoin d'une nouvelle page
        if (y + rowHeight > bottomMargin) {
            pdf.newPage();
            pageNumber++;
            isFirstPage = false; // Ce n'est plus la première page

            // Dessiner seulement le numéro de page sur les pages suivantes
            drawPageHeader(pageNumber, isFirstPage);

            y = 100; // Commencer plus haut sur les pages suivantes

            // Dessiner l'en-tête du tableau sur la nouvelle page
            drawTableHeader(y);
            y += rowHeight;
        }

        // Dessiner les bordures des cellules de données
        painter.setPen(QPen(borderColor, 1));
        painter.setBrush(Qt::NoBrush);

        painter.drawRect(col1, y, columnWidth, rowHeight);
        painter.drawRect(col2, y, columnWidth, rowHeight);
        painter.drawRect(col3, y, columnWidth, rowHeight);
        painter.drawRect(col4, y, columnWidth, rowHeight);
        painter.drawRect(col5, y, columnWidth, rowHeight);

        // Dessiner le contenu des cellules
        painter.setFont(textFont);
        painter.setPen(Qt::black);

        QRect idRect(col1, y, columnWidth, rowHeight);
        painter.drawText(idRect, Qt::AlignCenter, query.value(0).toString());

        QRect nomRect(col2, y, columnWidth, rowHeight);
        painter.drawText(nomRect, Qt::AlignCenter, query.value(1).toString());

        QRect prenomRect(col3, y, columnWidth, rowHeight);
        painter.drawText(prenomRect, Qt::AlignCenter, query.value(2).toString());

        QRect telRect(col4, y, columnWidth, rowHeight);
        painter.drawText(telRect, Qt::AlignCenter, query.value(3).toString());

        QRect specRect(col5, y, columnWidth, rowHeight);
        painter.drawText(specRect, Qt::AlignCenter, query.value(4).toString());

        y += rowHeight;
    }

    painter.end();
    return true;
}
