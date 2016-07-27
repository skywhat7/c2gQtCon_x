#include "datautil.hpp"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QImage>

const QString YYYY_MM_DD = "yyyy-MM-dd";

DataUtil::DataUtil(QObject *parent) : QObject(parent)
{

}

void DataUtil::init(DataManager* dataManager)
{
    mDataManager = dataManager;
}

// P R E   C O N F E R E N C E   S T U F F
/**
 * Prepare will be done before submitting the App to AppStore
 * Use public Cache
 * Best practice:
 * 1. remove app from dev device
 * 2. start app, switch to public cache
 * 3. restart app
 * If all done send files from Phone to PC/Mac
 * Copy them into data-assets /prod or /test
 * Test again after deleting app from dev device
 * Now Users will get this directly by installing the app
 * and updates are fast
 */
void DataUtil::prepareConference() {
    // must use public cache
    if(!mDataManager->settingsData()->hasPublicCache()) {
        qWarning() << "Preparation must be done from public cache";
        return;
    }
    qDebug() << "PREPARE CONFERENCE ";
    // check dirs for pre-conference stuff
    // these directories never used for normal app-runtime
    // data/conference
    QString directory = mDataManager->mDataPath + "conference/";
    QDir myDir;
    bool exists;
    exists = myDir.exists(directory);
    if (!exists) {
        bool ok = myDir.mkpath(directory);
        if(!ok) {
            qWarning() << "Couldn't create conference dir " << directory;
            return;
        }
        qDebug() << "created directory conference " << directory;
    }
    // data/conference/speakerImages
    directory.append("speakerImages/");
    exists = myDir.exists(directory);
    if (!exists) {
        bool ok = myDir.mkpath(directory);
        if(!ok) {
            qWarning() << "Couldn't create speakerImages dir " << directory;
            return;
        }
        qDebug() << "created directory speakerImages " << directory;
    }
    // create some data for this specific conference
    prepareEventData();
    // prepare speaker
    prepareSpeaker();
    // prepare sessions
    // speaker must exist because sessions will be added to
    prepareSessions();
    // download speaker images
    prepareSpeakerImages();
    // now cache all the data
    // if ok - copied to qrc: data-assets
    qDebug() << "cache DATA";
    mDataManager->saveConferenceToCache();
    mDataManager->saveDayToCache();
    mDataManager->saveBuildingToCache();
    mDataManager->saveFloorToCache();
    mDataManager->saveRoomToCache();
    mDataManager->saveSessionToCache();
    mDataManager->saveSessionTrackToCache();
    mDataManager->saveScheduleItemToCache();
    mDataManager->saveSessionLinkToCache();

    mDataManager->saveSpeakerToCache();
    qDebug() << "PREPARE   D O N E - WAIT FOR IMAGES";
}

// conference, days, building-floor-rooms
void DataUtil::prepareEventData() {
    qDebug() << "PREPARE EVENT ";
    // CONFERENCE
    mDataManager->deleteConference();
    mDataManager->deleteDay();
    mDataManager->deleteSession();
    mDataManager->deleteScheduleItem();
    mDataManager->deleteSessionLink();
    mDataManager->deleteSpeaker();
    mDataManager->deleteSpeakerImage();
    //
    Conference* conference = mDataManager->createConference();
    int lastNr = 0;
    for (int i = 0; i < mDataManager->allBuilding().size(); ++i) {
        Building* building = (Building*) mDataManager->allBuilding().at(i);
        if(building->buildingId() > lastNr) {
            lastNr = building->buildingId();
        }
    }
    conference->setLastBuildingId(lastNr);
    lastNr = 0;
    for (int i = 0; i < mDataManager->allFloor().size(); ++i) {
        Floor* floor = (Floor*) mDataManager->allFloor().at(i);
        if(floor->floorId() > lastNr) {
            lastNr = floor->floorId();
        }
    }
    conference->setLastFloorId(lastNr);
    lastNr = 0;
    for (int i = 0; i < mDataManager->allRoom().size(); ++i) {
        Room* room = (Room*) mDataManager->allRoom().at(i);
        if(room->roomId() > lastNr) {
            lastNr = room->roomId();
        }
        room->clearSessions();
    }
    conference->setLastRoomId(lastNr);
    lastNr = 0;
    for (int i = 0; i < mDataManager->allSessionTrack().size(); ++i) {
        SessionTrack* track = (SessionTrack*) mDataManager->allSessionTrack().at(i);
        if(track->trackId() > lastNr) {
            lastNr = track->trackId();
        }
        track->clearSessions();
    }
    conference->setLastSessionTrackId(lastNr);

    conference->setConferenceName("QtCON");
    conference->setConferenceCity("Berlin");
    QString venueAddress;
    venueAddress = "BCC Berlin Congress Center";
    venueAddress.append("\n");
    venueAddress.append("Tagungszentrum Mitte");
    venueAddress.append("\n");
    venueAddress.append("Alexanderstr. 11");
    venueAddress.append("\n");
    venueAddress.append("10178 Berlin");
    venueAddress.append("\n");
    venueAddress.append("Deutschland");
    conference->setAddress(venueAddress);
    conference->setTimeZoneName("Europe/Amsterdam");
    conference->setTimeZoneOffsetSeconds(1 * 60 * 60); // +01:00 GMT
    conference->setConferenceFrom(QDate::fromString("2016-09-01", YYYY_MM_DD));
    conference->setConferenceTo(QDate::fromString("2015-09-04", YYYY_MM_DD));
    conference->setHashTag("#QtCon16");
    conference->setHomePage("http://qtcon.org/");
    QString coordinate;
    coordinate = QString::number(52.520778)+";"+QString::number(13.416515);
    // conference->coordinate()->setLatitude(52.520778);
    // conference->coordinate()->setLongitude(13.416515);
    conference->setCoordinate(coordinate);
    mDataManager->insertConference(conference);
    // DAYS
    // Days dayOfWeek 1=monday, 7 = sunday
    // thursday
    Day* day = mDataManager->createDay();
    day->setId(1);
    day->setWeekDay(4);
    day->setConferenceDay(QDate::fromString("2016-09-01", YYYY_MM_DD));
    conference->addToDays(day);
    mDataManager->insertDay(day);
    // friday
    day = mDataManager->createDay();
    day->setId(2);
    day->setWeekDay(5);
    day->setConferenceDay(QDate::fromString("2016-09-02", YYYY_MM_DD));
    conference->addToDays(day);
    mDataManager->insertDay(day);
    // saturday
    day = mDataManager->createDay();
    day->setId(3);
    day->setWeekDay(6);
    day->setConferenceDay(QDate::fromString("2016-09-03", YYYY_MM_DD));
    conference->addToDays(day);
    mDataManager->insertDay(day);
    // sunday
    day = mDataManager->createDay();
    day->setId(4);
    day->setWeekDay(1);
    day->setConferenceDay(QDate::fromString("2016-09-04", YYYY_MM_DD));
    conference->addToDays(day);
    mDataManager->insertDay(day);
    // BUILDING
    // Building and FLOORS already created
    // ROOMS created, but delete current sessions yet
    qDebug() << "ROOMS: #" << mDataManager->allRoom().size();
    for (int r = 0; r < mDataManager->allRoom().size(); ++r) {
        Room* room = (Room*) mDataManager->allRoom().at(r);
        room->clearSessions();
    }
    //
}

void DataUtil::prepareSessions()
{
    const QString schedulePath = mDataManager->mDataAssetsPath + "conference/schedule.json";
    qDebug() << "PREPARE SESSIONS ";
    QMultiMap<QString, Session*> sessionSortMap;
    Conference* conference;
    conference = (Conference*) mDataManager->allConference().first();

    QFile readFile(schedulePath);
    if(!readFile.exists()) {
        qWarning() << "Schedule Path not found " << schedulePath;
        return;
    }
    if (!readFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Couldn't open file: " << schedulePath;
        return;
    }
    QJsonDocument jda = QJsonDocument::fromJson(readFile.readAll());
    readFile.close();
    if(!jda.isObject()) {
        qWarning() << "Couldn't create JSON from file: " << schedulePath;
        return;
    }
    qDebug() << "QJsonDocument for schedule with Object :)";
    QVariantMap map;
    map = jda.toVariant().toMap();
    if(map.isEmpty()) {
        qWarning() << "Schedule is no Map";
        return;
    }
    map = map.value("schedule").toMap();
    if(map.isEmpty()) {
        qWarning() << "No 'schedule' found";
        return;
    }
    qDebug() << "VERSION: " + map.value("version").toString();
    map = map.value("conference").toMap();
    if(map.isEmpty()) {
        qWarning() << "No 'conference' found";
        return;
    }
    qDebug() << "TITLE: " << map.value("title").toString();
    QVariantList dayList;
    dayList = map.value("days").toList();
    if(dayList.isEmpty()) {
        qWarning() << "No 'days' found";
        return;
    }
    qDebug() << "DAYS: " << dayList.size();
    for (int i = 0; i < dayList.size(); ++i) {
        QVariantMap dayMap;
        dayMap = dayList.at(i).toMap();
        if(dayMap.isEmpty()) {
            qWarning() << "No 'DAY' found #" << i;
            continue;
        }
        QString dayDate;
        dayDate = dayMap.value("date").toString();
        qDebug() << "processing DATE: " << dayDate;
        Day* day;
        bool found = false;
        for (int dl = 0; dl < mDataManager->mAllDay.size(); ++dl) {
            day = (Day*) mDataManager->mAllDay.at(dl);
            if(day->conferenceDay().toString(YYYY_MM_DD) == dayDate) {
                found = true;
                break;
            }
        }
        if(!found) {
            qWarning() << "No Day* found for " << dayDate;
            continue;
        }
        QVariantMap roomMap;
        roomMap = dayMap.value("rooms").toMap();
        QStringList roomKeys = roomMap.keys();
        if(roomKeys.isEmpty()) {
            qWarning() << "No 'ROOMS' found for DAY # i";
            continue;
        }
        for (int r = 0; r < roomKeys.size(); ++r) {
            QVariantList sessionList;
            sessionList = roomMap.value(roomKeys.at(r)).toList();
            if(sessionList.isEmpty()) {
                qWarning() << "DAY: " << dayDate << " ROOM: " << roomKeys.at(r) << " ignored - No Sessions available";
                continue;
            }
            Room* room;
            found = false;
            for (int rl = 0; rl < mDataManager->mAllRoom.size(); ++rl) {
                room = (Room*) mDataManager->mAllRoom.at(rl);
                if(room->roomName() == roomKeys.at(r)) {
                    found = true;
                    break;
                }
            }
            if(!found) {
                qWarning() << "Room* not found for " << dayDate << " Room: " << roomKeys.at(r);
                // TODO add new Room
                continue;
            }
            for (int sl = 0; sl < sessionList.size(); ++sl) {
                QVariantMap sessionMap;
                sessionMap = sessionList.at(sl).toMap();
                if(sessionMap.isEmpty()) {
                    qWarning() << "No 'SESSION' Map DAY: " << dayDate << " ROOM: " << roomKeys.at(r);
                    continue;
                }
                // adjust persons
                // TODO if full_public_name set it to Speaker where it's not delivered from Server
                QStringList personKeys;
                QVariantList personsList;
                personsList = sessionMap.value("persons").toList();
                if (personsList.size() > 0) {
                    for (int pvl = 0; pvl < personsList.size(); ++pvl) {
                        QVariantMap map = personsList.at(pvl).toMap();
                        if(map.contains("id")) {
                            personKeys.append(map.value("id").toString());
                        }
                    }
                    sessionMap.insert("persons", personKeys);
                }
                // create and adjust links
                QStringList linkKeys;
                QVariantList linksList;
                linksList = sessionMap.value("links").toList();
                if (linksList.size() > 0) {
                    for (int lvl = 0; lvl < linksList.size(); ++lvl) {
                        QVariantMap map = linksList.at(lvl).toMap();
                        if(map.contains("url")) {
                            SessionLink* sessionLink = mDataManager->createSessionLink();
                            sessionLink->setUrl(map.value("url").toString());
                            sessionLink->setTitle(map.value("title").toString());
                            mDataManager->insertSessionLink(sessionLink);
                            linkKeys.append(sessionLink->uuid());
                        }
                    }
                    sessionMap.insert("links", linkKeys);
                }
                SessionAPI* sessionAPI = mDataManager->createSessionAPI();
                sessionAPI->fillFromForeignMap(sessionMap);
                // ignore unwanted Sessions
                if(sessionAPI->title() == "Registration and Coffee" && sessionAPI->room() != "B02") {
                    qDebug() << "unwanted session: " << sessionAPI->sessionId() << " " << sessionAPI->title() << " " << sessionAPI->room();
                    continue;
                }
                if(sessionAPI->title() == "Lunch" && sessionAPI->room() != "B02") {
                    qDebug() << "unwanted session: " << sessionAPI->sessionId() << " " << sessionAPI->title() << " " << sessionAPI->room();
                    continue;
                }
                if(sessionAPI->title() == "Coffee break" && sessionAPI->room() != "B02") {
                    qDebug() << "unwanted session: " << sessionAPI->sessionId() << " " << sessionAPI->title() << " " << sessionAPI->room();
                    continue;
                }
                if(sessionAPI->title() == "Evening event" && sessionAPI->room() != "B02") {
                    qDebug() << "unwanted session: " << sessionAPI->sessionId() << " " << sessionAPI->title() << " " << sessionAPI->room();
                    continue;
                }
                if(sessionAPI->title() == "Welcome" && sessionAPI->room() != "C01") {
                    qDebug() << "unwanted session: " << sessionAPI->sessionId() << " " << sessionAPI->title() << " " << sessionAPI->room();
                    continue;
                }
                Session* session = mDataManager->createSession();
                session->fillFromMap(sessionAPI->toMap());
                QStringList duration;
                duration = sessionAPI->duration().split(":");
                int minutes;
                if(duration.length() == 2) {
                    minutes = duration.last().toInt();
                    minutes += duration.first().toInt()*60;
                } else {
                    qWarning() << "Duration wrong: " << sessionAPI->duration() << " DAY: " << dayDate << " ROOM: " << roomKeys.at(r);
                }
                session->setMinutes(minutes);
                session->setEndTime(session->startTime().addSecs(minutes * 60));
                // refs
                // DAY
                session->setSessionDay(day->id());
                // ROOM
                session->setRoom(room->roomId());
                // TRACK
                SessionTrack* sessionTrack;
                found = false;
                QString trackName;
                trackName = sessionAPI->track();
                if(trackName.isEmpty()) {
                    trackName = "*****";
                }
                for (int tr = 0; tr < mDataManager->mAllSessionTrack.size(); ++tr) {
                    sessionTrack = (SessionTrack*) mDataManager->mAllSessionTrack.at(tr);
                    if(sessionTrack->name() == trackName) {
                        found = true;
                        break;
                    }
                }
                if(!found) {
                    sessionTrack = mDataManager->createSessionTrack();
                    conference->setLastSessionTrackId(conference->lastSessionTrackId()+1);
                    sessionTrack->setTrackId(conference->lastSessionTrackId());
                    sessionTrack->setName(trackName);
                    sessionTrack->setInAssets(true);
                    mDataManager->insertSessionTrack(sessionTrack);
                }
                session->setSessionTrack(sessionTrack->trackId());
                // SCHEDULE or what else
                // setting some boolean here makes it easier to distinguish in UI
                if (trackName == "Break" || (trackName == "Misc" && session->title().contains("Registration"))) {
                    ScheduleItem* scheduleItem = mDataManager->createScheduleItem();
                    if(session->title().contains("Evening event")) {
                        scheduleItem->setIsEvent(true);
                    } else {
                        if(trackName == "Break") {
                            scheduleItem->setIsBreak(true);
                        } else {
                            if(session->title().contains("Registration")) {
                                scheduleItem->setIsRegistration(true);
                            } else {
                                if(session->title().contains("Lunch")) {
                                    scheduleItem->setIsLunch(true);
                                } else {
                                    scheduleItem->setIsEvent(true);
                                }
                            }
                        }
                    }
                    scheduleItem->setSessionId(session->sessionId());
                    session->setScheduleItem(scheduleItem->sessionId());
                    scheduleItem->setSession(session->sessionId());
                    mDataManager->insertScheduleItem(scheduleItem);
                } else {
                    if(session->title().contains("Lightning")) {
                        session->setIsLightning(true);
                    } else {
                        if(session->title().contains("Keynote")) {
                            session->setIsKeynote(true);
                        } else {
                            if(trackName == "Community") {
                                session->setIsCommunity(true);
                            } else {
                                if(dayDate == "2016-09-01" && session->title().contains("Training")) {
                                    session->setIsTraining(true);
                                } else {
                                    session->setIsSession(true);
                                }
                            }
                        }
                    }
                }
                // SORT
                session->setSortKey(day->conferenceDay().toString(YYYY_MM_DD)+session->startTime().toString("HH:mm"));
                sessionSortMap.insert(session->sortKey(), session);
            } // end for session of a room
        } // room keys
    } // for days list
    // insert sorted Sessions
    QMapIterator<QString, Session*> sessionIterator(sessionSortMap);
    while (sessionIterator.hasNext()) {
        sessionIterator.next();
        Session* session = sessionIterator.value();
        mDataManager->insertSession(session);
        Room* room = mDataManager->findRoomByRoomId(session->room());
        if(room != NULL) {
            room->addToSessions(session);
        } else {
            qWarning() << "ROOM is NULL for Session " << session->sessionId() << " #:" << session->room();
        }
        Day* day = mDataManager->findDayById(session->sessionDay());
        if(day != NULL) {
            day->addToSessions(session);
        } else {
            qWarning() << "DAY is NULL for Session " << session->sessionId() << " #:" << session->sessionDay();
        }
        SessionTrack* sessionTrack = mDataManager->findSessionTrackByTrackId(session->sessionTrack());
        if(sessionTrack != NULL) {
            sessionTrack->addToSessions(session);
        } else {
            qWarning() << "TRACK is NULL for Session " << session->sessionId() << " #:" << session->sessionTrack();
        }
        for (int i = 0; i < session->presenterKeys().size(); ++i) {
            int pKey = session->presenterKeys().at(i).toInt();
            Speaker* speaker = (Speaker*) mDataManager->findSpeakerBySpeakerId(pKey);
            if(speaker != NULL) {
                speaker->addToSessions(session);
            } else {
                qWarning() << "SPEAKER is NULL for Session " << session->sessionId() << " #:" << pKey;
            }
        } // for presenter
    } // while all sessions
}

void DataUtil::prepareSpeaker()
{
    const QString speakersPath = mDataManager->mDataAssetsPath + "conference/speakers.json";
    qDebug() << "PREPARE SPEAKER ";
    QVariantList dataList;

    QFile readFile(speakersPath);
    if(!readFile.exists()) {
        qWarning() << "Speakers Path not found " << speakersPath;
        return;
    }
    if (!readFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Couldn't open file: " << speakersPath;
        return;
    }
    QJsonDocument jda = QJsonDocument::fromJson(readFile.readAll());
    readFile.close();
    if(!jda.isArray()) {
        qWarning() << "Couldn't create JSON from file: " << speakersPath;
        return;
    }
    qDebug() << "QJsonDocument for speakers with Array :)";
    QMultiMap<QString, Speaker*> mm;
    dataList = jda.toVariant().toList();
    const QString DEFAULT_IMAGE = "http://conf.qtcon.org/person_original.png";
    for (int i = 0; i < dataList.size(); ++i) {
        SpeakerAPI* speakerAPI = mDataManager->createSpeakerAPI();
        speakerAPI->fillFromForeignMap(dataList.at(i).toMap());

        Speaker* speaker = mDataManager->createSpeaker();
        speaker->setSpeakerId(speakerAPI->id());
        speaker->setBio(speakerAPI->bio());
        speaker->setName(speakerAPI->firstName());
        if(speaker->name().length() > 0) {
            speaker->setName(speaker->name()+" ");
        }
        speaker->setName(speaker->name()+speakerAPI->lastName());
        if(speaker->name().length() > 0) {
            speaker->setSortKey(speakerAPI->lastName().left(5).toUpper());
            speaker->setSortGroup(speaker->sortKey().left(1));
        } else {
            speaker->setSortKey("*");
            speaker->setSortGroup("*");
        }
        if(speakerAPI->avatar().length() > 0 && speakerAPI->avatar() != DEFAULT_IMAGE) {
            QString avatar = speakerAPI->avatar();
            QStringList sl = avatar.split("?");
            if(sl.size() > 1) {
                sl.removeLast();
                avatar = sl.join("?");
            }
            sl = avatar.split(".");
            if(sl.size() < 2) {
                qWarning() << "AVATAR wrong "+speakerAPI->avatar();
            } else {
                SpeakerImage* speakerImage = mDataManager->createSpeakerImage();
                speakerImage->setSpeakerId(speaker->speakerId());
                speakerImage->setOriginImageUrl(avatar.replace("http://","https://"));
                speakerImage->setSuffix(sl.last());
                mDataManager->insertSpeakerImage(speakerImage);
                speaker->resolveSpeakerImageAsDataObject(speakerImage);
            }
        }
        // using MultiMap to get Speakers sorted
        mm.insert(speaker->sortKey(), speaker);
    } // end for all SpeakersAPI
    // insert sorted Speakers
    mDataManager->mAllSpeaker.clear();
    QMapIterator<QString, Speaker*> speakerIterator(mm);
    while (speakerIterator.hasNext()) {
        speakerIterator.next();
        mDataManager->insertSpeaker(speakerIterator.value());
    }
}

void DataUtil::prepareSpeakerImages()
{
    const QString speakerImagesPath = mDataManager->mDataPath + "conference/speakerImages/";
    qDebug() << "storing Speaker Images here: " << speakerImagesPath;
    if (mDataManager->allSpeakerImage().size() > 0) {
        SpeakerImage* speakerImage = (SpeakerImage*) mDataManager->mAllSpeakerImage.at(0);
        QString fileName;
        fileName = "speaker_";
        fileName.append(QString::number(speakerImage->speakerId()));
        fileName.append('.');
        fileName.append(speakerImage->suffix());
        mImageLoader = new ImageLoader(speakerImage->originImageUrl(), speakerImagesPath+fileName, this);
        bool res = connect(mImageLoader, SIGNAL(loaded(QObject*, int, int)), this,
                           SLOT(onSpeakerImageLoaded(QObject*, int, int)));
        if (!res) {
            Q_ASSERT(res);
        }
        mImageLoader->loadSpeaker(speakerImage);
    }
}

void DataUtil::onSpeakerImageLoaded(QObject *dataObject, int width, int height)
{
    mImageLoader->deleteLater();
    qDebug() << "onSpeakerImage  L O A D E D ";
    SpeakerImage* speakerImage = (SpeakerImage*) dataObject;
    speakerImage->setDownloadSuccess(true);
    speakerImage->setDownloadFailed(false);
    speakerImage->setInAssets(true);
    speakerImage->setInData(false);
    prepareHighDpiImages(speakerImage, width, height);
    // more to load ?
    const QString speakerImagesPath = mDataManager->mDataPath + "conference/speakerImages/";
    for (int i = 0; i < mDataManager->allSpeakerImage().size(); ++i) {
        SpeakerImage* speakerImage = (SpeakerImage*) mDataManager->allSpeakerImage().at(i);
        if (!speakerImage->downloadSuccess() && !speakerImage->downloadFailed()) {
            qDebug() << "loading..." << speakerImage->speakerId() << " " << width << "x" << height;
            QString fileName;
            fileName = "speaker_";
            fileName.append(QString::number(speakerImage->speakerId()));
            fileName.append('.');
            fileName.append(speakerImage->suffix());
            mImageLoader = new ImageLoader(speakerImage->originImageUrl(), speakerImagesPath+fileName, this);
            bool res = connect(mImageLoader, SIGNAL(loaded(QObject*, int, int)), this,
                               SLOT(onSpeakerImageLoaded(QObject*, int, int)));
            if (!res) {
                Q_ASSERT(res);
            }
            mImageLoader->loadSpeaker(speakerImage);
            return;
        }
    } // for all speaker images
    // N OW cache speaker images
    mDataManager->saveSpeakerImageToCache();
    qDebug() << "SPEAKER IMAGES   D O W N L O A D E D";
}

void DataUtil::prepareHighDpiImages(SpeakerImage* speakerImage, int width, int height) {
    const QString speakerImagesPath = mDataManager->mDataPath + "conference/speakerImages/";
    QString fileName;
    fileName = speakerImagesPath + "speaker_";
    fileName.append(QString::number(speakerImage->speakerId()));
    QString originFileName;
    originFileName = fileName + "." + speakerImage->suffix();
    const int size1 = 64;
    const int size2 = 128;
    const int size3 = 192;
    const int size4 = 256;
    if(width >= height) {
        if(width < size1) {
            speakerImage->setMaxScaleFactor(0);
            return;
        }
        QFile originFile(originFileName);
        if(!originFile.exists()) {
            qWarning() << "SpeakerImage Path not found " << originFileName;
            return;
        }
        if (!originFile.open(QIODevice::ReadOnly)) {
            qWarning() << "Couldn't open file: " << originFileName;
            return;
        }
        QImage originImage = QImage::fromData(originFile.readAll());
        if(originImage.isNull()) {
            qWarning() << "Cannot construct Image from file: " << originFileName;
            return;
        }
        QImage scaledImage;
        if(width >= size1) {
            scaledImage = originImage.scaledToWidth(size1);
            scaledImage.save(originFileName);
            speakerImage->setMaxScaleFactor(1);
        }
        if(width >= size2) {
            scaledImage = originImage.scaledToWidth(size2);
            scaledImage.save(fileName+"@2x."+speakerImage->suffix());
            speakerImage->setMaxScaleFactor(2);
        }
        if(width >= size3) {
            scaledImage = originImage.scaledToWidth(size3);
            scaledImage.save(fileName+"@3x."+speakerImage->suffix());
            speakerImage->setMaxScaleFactor(3);
        }
        if(width >= size4) {
            scaledImage = originImage.scaledToWidth(size4);
            scaledImage.save(fileName+"@4x."+speakerImage->suffix());
            speakerImage->setMaxScaleFactor(4);
        }
    } else {
        if(height < size1) {
            speakerImage->setMaxScaleFactor(0);
            return;
        }
        QFile originFile(originFileName);
        if(!originFile.exists()) {
            qWarning() << "SpeakerImage Path not found " << originFileName;
            return;
        }
        if (!originFile.open(QIODevice::ReadOnly)) {
            qWarning() << "Couldn't open file: " << originFileName;
            return;
        }
        QImage originImage = QImage::fromData(originFile.readAll());
        if(originImage.isNull()) {
            qWarning() << "Cannot construct Image from file: " << originFileName;
            return;
        }
        QImage scaledImage;
        if(height >= size1) {
            scaledImage = originImage.scaledToHeight(size1);
            scaledImage.save(originFileName);
            speakerImage->setMaxScaleFactor(1);
        }
        if(height >= size2) {
            scaledImage = originImage.scaledToHeight(size2);
            scaledImage.save(fileName+"@2x."+speakerImage->suffix());
            speakerImage->setMaxScaleFactor(2);
        }
        if(height >= size3) {
            scaledImage = originImage.scaledToHeight(size3);
            scaledImage.save(fileName+"@3x."+speakerImage->suffix());
            speakerImage->setMaxScaleFactor(3);
        }
        if(height >= size4) {
            scaledImage = originImage.scaledToHeight(size4);
            scaledImage.save(fileName+"@4x."+speakerImage->suffix());
            speakerImage->setMaxScaleFactor(4);
        }
    }
}
