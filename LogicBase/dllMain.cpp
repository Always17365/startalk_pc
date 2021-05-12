#include "LogicBase.h"
#include "logicbase_global.h"

extern "C" LOGICBASE_EXPORT ILogicBase* Handle()
{
	return new LogicBase;
}