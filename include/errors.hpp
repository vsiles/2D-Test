#ifndef ERRORS_H_INCLUDED
#define ERRORS_H_INCLUDED

#include <iostream>
#include <SDL.h>

/* Log a SDL error */
static inline void logSDLError(const std::string &msg,
                               std::ostream &os = std::cerr)
{
    os << "[SDL ] " << msg << " error: " << SDL_GetError() << std::endl;
}

/* Log a normal message */
static inline void log(const std::string &msg, std::ostream &os = std::cout)
{
    os << "[    ] " << msg << std::endl;
}

/* Log a raw error */
static inline void logError(const std::string &msg,
                            std::ostream &os = std::cerr)
{
    os << "[XXXX] " << msg << std::endl;
}

#endif /* ERRORS_H_INCLUDED */
