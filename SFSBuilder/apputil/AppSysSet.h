#ifndef AppSysSet_h
#define AppSysSet_h

#ifdef WIN32

#include <windows.h>
#include <direct.h>
#define mkdir(a,b) _mkdir(a)

#endif  

#include <string>
#include <sys/stat.h>

class AppSysSet{
public:
   const char *name_app;
   const char *name_vendor;

   std::string path_app;
   std::string path_home;
   std::string path_vendor;
   std::string path_localset;
   std::string path_sharedir;

   AppSysSet(const char *vendor,const char *pkgname);
   void Init(int argc, char **argv);
};

extern AppSysSet appsysset;

#endif
