#ifndef  __ENG_LOG_H__
#define  __ENG_LOG_H__

class EngLog{
    private:
        EngLog();
        ~EngLog();

    public:

        static int error(const char* fmt, ...);
        static int info(const char* fmt, ...);
        static int warn(const char* fmt, ...);
        static int debug(const char* fmt, ...);
};


#endif