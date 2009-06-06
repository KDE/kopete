 /*
    Copyright (c) 2008 by Igor Janssen  <alaves17@gmail.com>

    Kopete    (c) 2008 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
 */

#ifndef MOOD_H
#define MOOD_H

#include <QVector>
#include <QDomElement>

// XEP-0107: User Mood

class Mood
{
public:
	enum Type {None = 0, Afraid, Amazed, Angry, Annoyed, Anxious, Aroused, Ashamed, Bored, Brave, Calm, Cold, Confused,
	Contented, Cranky, Curious, Depressed, Disappointed, Disgusted, Distracted, Embarrassed, Excited, Flirtatious,
	Frustrated, Grumpy, Guilty, Happy, Hot, Humbled, Humiliated, Hungry, Hurt, Impressed, In_awe, In_love,
	Indignant, Interested, Intoxicated, Invincible, Jealous, Lonely, Mean, Moody, Nervous, Neutral, Offended,
	Playful, Proud, Relieved, Remorseful, Restless, Sad, Sarcastic, Serious, Shocked, Shy, Sick, Sleepy, Stressed,
	Surprised, Thirsty, Worried};

	Mood(Type aType, const QString &aText = "");
	Mood(const QDomElement &mood);

	QDomElement toXml(QDomDocument &doc);

private:
	Type    mType;
	QString mText;
};

class MoodManager
{
public:
	static MoodManager *self();

	const QString &getMoodId(Mood::Type t) const;
	const QString &getMoodName(Mood::Type t) const;

private:
	MoodManager();
	static MoodManager *s_self;

	QVector<QString> ids;
	QVector<QString> names;
};

#endif
