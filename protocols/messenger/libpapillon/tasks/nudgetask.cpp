namespace Papillon
{

class NudgeTask::Private
{
public:
	Private()
	{}
};

NudgeTask::NudgeTask(Papillon::Task *parent)
 : Papillon::Task(parent), d(new Private)
{
}

NudgeTask::~NudgeTask()
{
	delete d;
}

bool NudgeTask::take(Transfer *transfer)
{
	return false;
}

void NudgeTask::sendNudgeCommand()
{
	qDebug() << PAPILLON_FUNCINFO << "Sending Nudge";
}

void NudgeTask::onGo()
{
	sendNudgeCommand();
}

}

#include "nudgetask.moc"
