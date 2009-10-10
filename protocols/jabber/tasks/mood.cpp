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

#include "mood.h"

#include <KLocale>

#define MOOD_NS "http://jabber.org/protocol/mood"

Mood::Mood(Type aType, const QString &aText)
{
	mType = aType;
	mText = aText;
}

Mood::Mood(const QDomElement &/* mood */)
{

}

QDomElement Mood::toXml(QDomDocument &doc)
{
	QDomElement mood = doc.createElement("mood");
	mood.setAttribute("xmlns", MOOD_NS);
    QString moodId = MoodManager::self()->getMoodId(mType);
    if(!moodId.isEmpty())
    {
        QDomElement e = doc.createElement(MoodManager::self()->getMoodId(mType));
        mood.appendChild(e);
        if(!mText.isEmpty())
        {
            QDomElement text = doc.createElement("text");
            QDomText t = doc.createTextNode(mText);
            text.appendChild(t);
            mood.appendChild(text);
        }
    }
	return mood;
}

MoodManager *MoodManager::s_self = 0L;

MoodManager *MoodManager::self()
{
	if(!s_self)
		s_self = new MoodManager;
	return s_self;
}

const QString &MoodManager::getMoodId(Mood::Type t) const
{
	return ids[t];
}

const QString &MoodManager::getMoodName(Mood::Type t) const
{
	return names[t];
}

MoodManager::MoodManager()
{
	ids.resize(Mood::Worried + 1);
	names.resize(Mood::Worried + 1);
	ids[Mood::None] = "";
	names[Mood::None] = i18n("None");
	ids[Mood::Afraid] = "afraid";
	names[Mood::Afraid] = i18n("Afraid");
	ids[Mood::Amazed] = "amazed";
	names[Mood::Amazed] = i18n("Amazed");
	ids[Mood::Angry] = "angry";
	names[Mood::Angry] = i18n("Angry");
	ids[Mood::Annoyed] = "annoyed";
	names[Mood::Annoyed] = i18n("Annoyed");
	ids[Mood::Anxious] = "anxious";
	names[Mood::Anxious] = i18n("Anxious");
	ids[Mood::Aroused] = "aroused";
	names[Mood::Aroused] = i18n("Aroused");
	ids[Mood::Ashamed] = "ashamed";
	names[Mood::Ashamed] = i18n("Ashamed");
	ids[Mood::Bored] = "bored";
	names[Mood::Bored] = i18n("Bored");
	ids[Mood::Brave] = "brave";
	names[Mood::Brave] = i18n("Brave");
	ids[Mood::Calm] = "calm";
	names[Mood::Calm] = i18n("Calm");
	ids[Mood::Cold] = "cold";
	names[Mood::Cold] = i18n("Cold");
	ids[Mood::Confused] = "confused";
	names[Mood::Confused] = i18n("Confused");
	ids[Mood::Contented] = "contented";
	names[Mood::Contented] = i18n("Contented");
	ids[Mood::Cranky] = "cranky";
	names[Mood::Cranky] = i18n("Cranky");
	ids[Mood::Curious] = "curious";
	names[Mood::Curious] = i18n("Curious");
	ids[Mood::Depressed] = "depressed";
	names[Mood::Depressed] = i18n("Depressed");
	ids[Mood::Disappointed] = "disappointed";
	names[Mood::Disappointed] = i18n("Disappointed");
	ids[Mood::Disgusted] = "disgusted";
	names[Mood::Disgusted] = i18n("Disgusted");
	ids[Mood::Distracted] = "distracted";
	names[Mood::Distracted] = i18n("Distracted");
	ids[Mood::Embarrassed] = "embarrassed";
	names[Mood::Embarrassed] = i18n("Embarrassed");
	ids[Mood::Excited] = "excited";
	names[Mood::Excited] = i18n("Excited");
	ids[Mood::Flirtatious] = "flirtatious";
	names[Mood::Flirtatious] = i18n("Flirtatious");
	ids[Mood::Frustrated] = "frustrated";
	names[Mood::Frustrated] = i18n("Frustrated");
	ids[Mood::Grumpy] = "grumpy";
	names[Mood::Grumpy] = i18n("Grumpy");
	ids[Mood::Guilty] = "guilty";
	names[Mood::Guilty] = i18n("Guilty");
	ids[Mood::Happy] = "happy";
	names[Mood::Happy] = i18n("Happy");
	ids[Mood::Hot] = "hot";
	names[Mood::Hot] = i18n("Hot");
	ids[Mood::Humbled] = "humbled";
	names[Mood::Humbled] = i18n("Humbled");
	ids[Mood::Humiliated] = "humiliated";
	names[Mood::Humiliated] = i18n("Humiliated");
	ids[Mood::Hungry] = "hungry";
	names[Mood::Hungry] = i18n("Hungry");
	ids[Mood::Hurt] = "hurt";
	names[Mood::Hurt] = i18n("Hurt");
	ids[Mood::Impressed] = "impressed";
	names[Mood::Impressed] = i18n("Impressed");
	ids[Mood::In_awe] = "in_awe";
	names[Mood::In_awe] = i18n("In awe");
	ids[Mood::In_love] = "in_love";
	names[Mood::In_love] = i18n("In love");
	ids[Mood::Indignant] = "indignant";
	names[Mood::Indignant] = i18n("Indignant");
	ids[Mood::Interested] = "interested";
	names[Mood::Interested] = i18n("Interested");
	ids[Mood::Intoxicated] = "intoxicated";
	names[Mood::Intoxicated] = i18n("Intoxicated");
	ids[Mood::Invincible] = "invincible";
	names[Mood::Invincible] = i18n("Invincible");
	ids[Mood::Jealous] = "jealous";
	names[Mood::Jealous] = i18n("Jealous");
	ids[Mood::Lonely] = "lonely";
	names[Mood::Lonely] = i18n("Lonely");
	ids[Mood::Mean] = "mean";
	names[Mood::Mean] = i18n("Mean");
	ids[Mood::Moody] = "moody";
	names[Mood::Moody] = i18n("Moody");
	ids[Mood::Nervous] = "nervous";
	names[Mood::Nervous] = i18n("Nervous");
	ids[Mood::Neutral] = "neutral";
	names[Mood::Neutral] = i18n("Neutral");
	ids[Mood::Offended] = "offended";
	names[Mood::Offended] = i18n("Offended");
	ids[Mood::Playful] = "playful";
	names[Mood::Playful] = i18n("Playful");
	ids[Mood::Proud] = "proud";
	names[Mood::Proud] = i18n("Proud");
	ids[Mood::Relieved] = "relieved";
	names[Mood::Relieved] = i18n("Relieved");
	ids[Mood::Remorseful] = "remorseful";
	names[Mood::Remorseful] = i18n("Remorseful");
	ids[Mood::Restless] = "restless";
	names[Mood::Restless] = i18n("Restless");
	ids[Mood::Sad] = "sad";
	names[Mood::Sad] = i18n("Sad");
	ids[Mood::Sarcastic] = "sarcastic";
	names[Mood::Sarcastic] = i18n("Sarcastic");
	ids[Mood::Serious] = "serious";
	names[Mood::Serious] = i18n("Serious");
	ids[Mood::Shocked] = "shocked";
	names[Mood::Shocked] = i18n("Shocked");
	ids[Mood::Shy] = "shy";
	names[Mood::Shy] = i18n("Shy");
	ids[Mood::Sick] = "sick";
	names[Mood::Sick] = i18n("Sick");
	ids[Mood::Sleepy] = "sleepy";
	names[Mood::Sleepy] = i18n("Sleepy");
	ids[Mood::Stressed] = "stressed";
	names[Mood::Stressed] = i18n("Stressed");
	ids[Mood::Surprised] = "surprised";
	names[Mood::Surprised] = i18n("Surprised");
	ids[Mood::Thirsty] = "thirsty";
	names[Mood::Thirsty] = i18n("Thirsty");
	ids[Mood::Worried] = "worried";
	names[Mood::Worried] = i18n("Worried");
}
