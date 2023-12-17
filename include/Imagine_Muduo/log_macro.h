#ifndef IMAGINE_MUDUO_LOG_MACRO_H
#define IMAGINE_MUDUO_LOG_MACRO_H

#include "Imagine_Log/Imagine_Log.h"

#define OPEN_IMAGINE_MUDUO_LOG

#ifdef OPEN_IMAGINE_MUDUO_LOG
#define IMAGINE_MUDUO_LOG(LOG_MESSAGE...) \
    do { \
        LOG_INFO(LOG_MESSAGE); \
    } while(0)
#else
#define IMAGINE_MUDUO_LOG(LOG_MESSAGE...) do { } while(0)
#endif

#endif