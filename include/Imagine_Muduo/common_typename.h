#ifndef IMAGINE_MUDUO_COMMON_TYPENAME_H
#define IMAGINE_MUDUO_COMMON_TYPENAME_H

#include "Imagine_Time/Imagine_Time.h"
#include "Imagine_Log/Imagine_Log.h"

#include <functional>

namespace Imagine_Muduo
{

class Connection;

using EventHandler = std::function<void()>;                                     // 不同的Channel有不同的处理逻辑,暂时写死,不允许用户更改
using ConnectionCallback = std::function<void(Connection* conn)>;

using TimerCallback = ::Imagine_Tool::Imagine_Time::TimerCallback;
using Timer = ::Imagine_Tool::Imagine_Time::Timer;
using TimeStamp = ::Imagine_Tool::Imagine_Time::TimeStamp;
using TimeUtil = ::Imagine_Tool::Imagine_Time::TimeUtil;
using TimerPtrCmp = ::Imagine_Tool::Imagine_Time::TimeUtil::TimerPtrCmp;

using Logger = ::Imagine_Tool::Imagine_Log::Logger;
using SingletonLogger = ::Imagine_Tool::Imagine_Log::SingletonLogger;
using NonSingletonLogger = ::Imagine_Tool::Imagine_Log::NonSingletonLogger;

} // namespace Imagine_Muduo


#endif