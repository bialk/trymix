#include "AppSysSet.h"
#include <stdio.h>
#include <stdlib.h>

//////////////////
//class AppSysSet


AppSysSet::AppSysSet(const char *vendor,const char *pkgname):name_app(pkgname),name_vendor(vendor){}

void AppSysSet::Init(int argc, char **argv){

#ifdef WIN32
   char full_exepath[_MAX_PATH + 1];
   GetModuleFileNameA(NULL, full_exepath, _MAX_PATH + 1);
   //printf("path_app=%s\n",full_path);

   path_app = full_exepath;
   size_t pos = path_app.find_last_of("\\",path_app.size());
   if(pos==-1)
      path_app="."; 
   else
      path_app.replace(pos,path_app.size(),"");

   path_home = getenv("USERPROFILE");

   //char profilepath[250];
   //ExpandEnvironmentStringsA("%userprofile%",profilepath,250);
   //path_home=profilepath;

   path_vendor = path_home + "\\Local Settings\\Application Data\\" + name_vendor;
   path_localset = path_vendor + "\\" + name_app;
   path_sharedir = path_app;
#else
   // extract path to the application
   char str[1000];
   int length = readlink("/proc/self/exe", str, 1000);
   str[length] = '\0';
   path_app=str;
   size_t pos = path_app.find_last_of("/",path_app.size());
   if(pos==path_app.npos)
      path_app="."; 
   else
      path_app.replace(pos,path_app.size(),"");
   path_home=getenv("HOME");
   path_vendor = path_home + "/." + name_vendor;
   path_localset = path_vendor + "/" + name_app;
#ifdef GCC_LINUX
   path_sharedir = path_app;
#else
   path_sharedir = PACKAGE_SHAREDIR;
#endif
#endif

   printf("path application: %s\n",path_app.c_str());
   printf("path home:        %s\n",path_home.c_str());
   printf("path vendor:      %s\n",path_vendor.c_str());
   printf("path localset:    %s\n",path_localset.c_str());
   printf("path sharedir:    %s\n",path_sharedir.c_str());
   printf("\n");


   // create dir for local settings
   struct stat st;
   if(stat(path_vendor.c_str(),&st))
      mkdir(path_vendor.c_str(),0755); 
   if(stat(path_localset.c_str(),&st))
      mkdir(path_localset.c_str(),0755);

}
