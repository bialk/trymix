#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "serializer.h"
//#include "system.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

static char const zero_ch = '\0';

// Moved here to avoid a warning on strncpy()
void CSync_CStr::Load(Serializer *s)
{
	while(1){
		int type=s->ss->NextItem();
		if(type==1) {
			return;
		} else if(type==2) {
			char const* str;
			s->ss->GetItem(&str);
			strncpy(obj,str,sz);
		} else if(type==0) {
			Serializer(s).Load(); 
		}
	}
}

//Simple Binary storage
//////////////////////

StorageStreamSimpleBinary::StorageStreamSimpleBinary()
:  f(),
   type(),
   ddata(),
   fdata(),
   idata(),
   shortlen(),
   biglen()
{
}

StorageStreamSimpleBinary::~StorageStreamSimpleBinary(){
   Close();
}

//stream initialization operation
int StorageStreamSimpleBinary::OpenForWrite(const char * fname){
   Close();
   f=fopen(fname,"w+b");
   return !f;
}
int StorageStreamSimpleBinary::OpenForRead(const char * fname){
   Close();
   f=fopen(fname,"rb");
   return !f;
}
void StorageStreamSimpleBinary::Close(){
   if(f!=NULL){
      fclose(f); 
      f=NULL;
   }
}

//atomic storage operations
const char* StorageStreamSimpleBinary::GetNodeName(){
   return &strdata[0];
}


int StorageStreamSimpleBinary::NextItem(){   
   //data set parsing   
   fread(&type,1,1,f);
   if(!f || feof(f)) return 1;
   switch(type){
      case 0: // start node
      case 2: // string
      case 6: // binary
         if (type == 6) {
            fread(&biglen, 1, sizeof(biglen), f);
            shortlen = biglen;
         } else {
            fread(&shortlen, 1, sizeof(shortlen), f);
            biglen = shortlen;
         }
         strdata.resize(biglen + 1);
         fread(&strdata[0], 1, biglen, f);
         strdata[biglen] = '\0';
         return type == 0 ? 0 : 2;
      case 1: // end node
         return 1;
      case 3: // int
         fread(&idata,sizeof(idata),1,f);
         return 2;
      case 4: // float
         fread(&fdata,sizeof(fdata),1,f);
         return 2;
      case 5: // double
         fread(&ddata,sizeof(ddata),1,f);
         return 2;
   }
   return 2;
}

void StorageStreamSimpleBinary::PutStartNode(const char *s){
   char t = 0;
   unsigned short len = strlen(s);
   fwrite(&t,1,1,f);
   fwrite(&len,sizeof(len),1,f);
   fwrite(s,1,len,f);
}
void StorageStreamSimpleBinary::PutEndNode(const char *s){
   char t = 1;
   fwrite(&t,1,1,f);
}
void StorageStreamSimpleBinary::GetItem(int* v){
   switch(type){
      case 2: //from string
         *v = atoi(&strdata[0]);
         break;
      case 3: //from int
         *v = idata;
         break;
      case 4: //from float
         *v = (int)fdata;
         break;
      case 5: //from double
         *v = (int)ddata;
         break;
      default:
         *v = 0;
         break;
   }
}
void StorageStreamSimpleBinary::GetItem(float* v){
   switch(type){
      case 2: //from string
         *v = (float)atof(&strdata[0]);
         break;
      case 3: //from int
         *v = (float)idata;
         break;
      case 4: //from float
         *v = fdata;
         break;
      case 5: //from double
         *v = (float)ddata;
         break;
      default:
         *v = 0.0f;
         break;
   }
}

void StorageStreamSimpleBinary::GetItem(double* v){
   switch(type){
      case 2: //from string
         *v = atof(&strdata[0]);
         break;
      case 3: //from int
         *v = (double)idata;
         break;
      case 4: //from float
         *v = fdata;
         break;
      case 5: //from double
         *v = ddata;
         break;
      default:
         *v = 0.0;
         break;
   }
}

void StorageStreamSimpleBinary::GetItem(char const** v){
   switch(type){
      case 2: //from string
      case 6: //from binary
         *v = &strdata[0];
         break;
      case 3: //from int
         strdata.resize(32);
         sprintf(&strdata[0], "%d", idata);
         *v = &strdata[0];
         break;
      case 4: // from float
         strdata.resize(32);
         sprintf(&strdata[0], "%e", fdata);
         *v = &strdata[0];
         break;
      case 5: // from double
         strdata.resize(32);
         sprintf(&strdata[0], "%e", ddata);
         *v = &strdata[0];
         break;
   }
}

void StorageStreamSimpleBinary::GetItem(void const** v, int* n){
   switch(type){
      case 2: //from string
      case 6: //from binary
         *v = &strdata[0];
         *n = biglen;
      default:
         *v = zero_ch;
         *n = 0;
   }
}

void StorageStreamSimpleBinary::PutItem(int* v){
   char t = 3;
   fwrite(&t,1,1,f);
   fwrite(v,sizeof(int),1,f);
}
void StorageStreamSimpleBinary::PutItem(float* v){
   char t = 4;
   fwrite(&t,1,1,f);
   fwrite(v,sizeof(float),1,f);
}
void StorageStreamSimpleBinary::PutItem(double* v){
   char t = 5;
   fwrite(&t,1,1,f);
   fwrite(v,sizeof(double),1,f);
}
void StorageStreamSimpleBinary::PutItem(const char* v){
   char t = 2;
   unsigned short len = strlen(v);
   fwrite(&t,1,1,f);
   fwrite(&len,sizeof(len),1,f);
   fwrite(v,1,len,f);
}
void StorageStreamSimpleBinary::PutItem(void const* v, int n)
{
   char t = 6;
   unsigned int len = n;
   fwrite(&t,1,1,f);
   fwrite(&len,sizeof(len),1,f);
   fwrite(v,1,len,f);
}



// StorageStreamSimpleIostream
///////////////////////////////////

StorageStreamSimpleIostream::StorageStreamSimpleIostream(std::iostream* st)
:  strm(st),
   type(),
   ddata(),
   fdata(),
   idata(),
   shortlen(),
   biglen()
{
}

StorageStreamSimpleIostream::~StorageStreamSimpleIostream(){
}

//atomic storage operations
const char* StorageStreamSimpleIostream::GetNodeName(){
   return &strdata[0];
}


int StorageStreamSimpleIostream::NextItem(){   
   //data set parsing
   strm->read((char*)&type, 1);
   if(!*strm) return 1;
   switch(type){
      case 0: // start node
      case 2: // string
      case 6: // blob
         if (type == 6) {
            strm->read((char*)&biglen, sizeof(biglen));
            shortlen = biglen;
         } else {
            strm->read((char*)&shortlen, sizeof(shortlen));
            biglen = shortlen;
         }
         strdata.resize(biglen + 1);
         strm->read(&strdata[0], biglen);
         strdata[biglen] = '\0';
         return type == 0 ? 0 : 2;
      case 1: // end node
         return 1;
      case 3: // data node string start node
         strm->read((char*)&idata,sizeof(idata));
         return 2;
      case 4: // data node string start node
         strm->read((char*)&fdata,sizeof(fdata));
         return 2;
      case 5: // data node string start node
         strm->read((char*)&ddata,sizeof(ddata));
         return 2;
   }
   return 2;
}

void StorageStreamSimpleIostream::PutStartNode(const char *s){
   char t = 0;
   unsigned short len = strlen(s);
   strm->write((char const*)&t, 1);
   strm->write((char const*)&len, sizeof(len));
   strm->write((char const*)s, len);
}
void StorageStreamSimpleIostream::PutEndNode(const char *s){
   char t = 1;
   strm->write((char const*)&t, 1);
}
void StorageStreamSimpleIostream::GetItem(int* v){
   switch(type){
      case 2: //from string
         *v = atoi(&strdata[0]);
         break;
      case 3: //from int
         *v = idata;
         break;
      case 4: //from float
         *v = (int)fdata;
         break;
      case 5: //from double
         *v = (int)ddata;
         break;
      default:
         *v = 0;
         break;
   }
}
void StorageStreamSimpleIostream::GetItem(float* v){
   switch(type){
      case 2: //from string
         *v = (float)atof(&strdata[0]);
         break;
      case 3: //from int
         *v = (float)idata;
         break;
      case 4: //from float
         *v = fdata;
         break;
      case 5: //from double
         *v = (float)ddata;
         break;
      default:
         *v = 0.0f;
         break;
   }
}

void StorageStreamSimpleIostream::GetItem(double* v){
   switch(type){
      case 2: //from string
         *v = atof(&strdata[0]);
         break;
      case 3: //from int
         *v = (double)idata;
         break;
      case 4: //from float
         *v = fdata;
         break;
      case 5: //from double
         *v = ddata;
         break;
      default:
         *v = 0.0;
         break;
   }
}

void StorageStreamSimpleIostream::GetItem(char const** v){
   switch(type){
      case 2: //from string
      case 6: //from binary
         *v = &strdata[0];
         break;
      case 3: //from int
         strdata.resize(32);
         sprintf(&strdata[0], "%d", idata);
         *v = &strdata[0];
         break;
      case 4: // from float
         strdata.resize(32);
         sprintf(&strdata[0], "%e", fdata);
         *v = &strdata[0];
         break;
      case 5: // from double
         strdata.resize(32);
         sprintf(&strdata[0], "%e", ddata);
         *v = &strdata[0];
         break;
   }
}
void StorageStreamSimpleIostream::GetItem(void const** v, int* n){
   switch(type){
      case 2: //from string
      case 6: //from binary
         *v = &strdata[0];
         *n = biglen;
         break;
   }
}


void StorageStreamSimpleIostream::PutItem(int* v){
   char t = 3;
   strm->write((char const*)&t, 1);
   strm->write((char const*)v, sizeof(*v));
}
void StorageStreamSimpleIostream::PutItem(float* v){
   char t = 4;
   strm->write((char const*)&t, 1);
   strm->write((char const*)v, sizeof(*v));
}
void StorageStreamSimpleIostream::PutItem(double* v){
   char t = 5;
   strm->write((char const*)&t, 1);
   strm->write((char const*)v, sizeof(*v));
}
void StorageStreamSimpleIostream::PutItem(const char* v){
   char t = 2;
   unsigned short len = strlen(v);
   strm->write((char const*)&t, 1);
   strm->write((char const*)&len, sizeof(len));
   strm->write(v, len);
}
void StorageStreamSimpleIostream::PutItem(const void* v, int n){
   char t = 6;
   unsigned int len = n;
   strm->write((char const*)&t, 1);
   strm->write((char const*)&len, sizeof(len));
   if (len != 0) { // This check is mandatory.
      strm->write((char const*)v, len);
   }
}


//Indexed Binary storage
//////////////////////

StorageStreamIndexedBinary::StorageStreamIndexedBinary()
:  saveindex(0),
   idxcntr(0),
   f(),
   type(),
   ddata(),
   fdata(),
   idata(),
   shortlen(),
   biglen()
{
}

StorageStreamIndexedBinary::~StorageStreamIndexedBinary(){
   Close();
}

//stream initialization operation
int StorageStreamIndexedBinary::OpenForWrite(const char * fname){
   Close();
   f=fopen(fname,"w+b");
   if(!f) return 1;
   saveindex=1;
   indexfilename=fname;
   indexfilename+=".idx";
   return 0;
}

int StorageStreamIndexedBinary::OpenForRead(const char * fname){
   Close();
   if(ReadIndex((std::string(fname)+".idx").c_str())) return 1;
   f=fopen(fname,"rb");
   if(!f) return 1;
   return 0;
}

int StorageStreamIndexedBinary::ReadIndex(const char * fname){
   //reading index
   FILE *fidx=fopen(fname,"r+b");
   if(!fidx) return 1;
   int maxidx=0;
   while(fidx && !feof(fidx)){
      char key[200],value[50];
      fgets(key,200,fidx);
      fgets(value,200,fidx);
      key[strlen(key)-1]=0;
      value[strlen(value)-1]=0;
      int val=atoi(value);
      str2int[key]=val;
      if(val>maxidx) maxidx=val;
   }
   int2str.resize(maxidx+1);
   std::map<std::string, int>::iterator it;
   for(it=str2int.begin();it!=str2int.end();it++)
      int2str[it->second]=it->first.c_str();
   fclose(fidx);
   return 0;
}

int StorageStreamIndexedBinary::WriteIndex(const char * fname){
   //writting index
   FILE *fidx=fopen(fname,"w+b");
   if(!fidx) return 1;
   std::map<std::string, int>::iterator it;
   for(it=str2int.begin();it!=str2int.end();it++){
      fprintf(fidx,"%s\n",it->first.c_str());
      fprintf(fidx,"%i\n",it->second);
   }
   fclose(fidx);
   return 0;
}


void StorageStreamIndexedBinary::Close(){
   if(f!=NULL){
      fclose(f);
      f=NULL;
   }
   if(saveindex){
      WriteIndex(indexfilename.c_str());
      saveindex=0;      
   }
   idxcntr=0;
   str2int.clear();
}

//atomic storage operations
const char* StorageStreamIndexedBinary::GetNodeName(){
   return &strdata[0];
}


int StorageStreamIndexedBinary::NextItem(){   
   //data set parsing   
   fread(&type,1,1,f);
   int node_id;
   if(!f || feof(f)) return 1;
   switch(type){
     case 0: { // start node
         fread(&node_id,sizeof(int),1,f);
         char const* str = int2str[node_id];
         shortlen = biglen = strlen(str);
         strdata.resize(biglen + 1);
         memcpy(&strdata[0], str, biglen + 1);
         return 0;
      }
      case 1: // end node
         return 1;
      case 2: // string
      case 6: // binary
         if (type == 6) {
            fread(&biglen, 1, sizeof(biglen), f);
            shortlen = biglen;
         } else {
            fread(&shortlen, 1, sizeof(shortlen), f);
            biglen = shortlen;
         }
         strdata.resize(biglen + 1);
         fread(&strdata[0],1,biglen,f);
         strdata[biglen]='\0';
         return 2;
      case 3: // data node string start node
         fread(&idata,sizeof(idata),1,f);
         return 2;
      case 4: // data node string start node
         fread(&fdata,sizeof(fdata),1,f);
         return 2;
      case 5: // data node string start node
         fread(&ddata,sizeof(ddata),1,f);
         return 2;
   }
   return 2;
}

void StorageStreamIndexedBinary::PutStartNode(const char *s){
   std::pair< std::map<std::string,int>::iterator, bool> it = 
   str2int.insert(std::pair<std::string,int>(s,0));
   if(it.second)
      it.first->second=++idxcntr;

   int node_id = it.first->second;

   char t = 0;
   fwrite(&t,1,1,f);
   fwrite(&node_id,sizeof(int),1,f);
}
void StorageStreamIndexedBinary::PutEndNode(const char *s){
   char t = 1;
   fwrite(&t,1,1,f);
}
void StorageStreamIndexedBinary::GetItem(int* v){
   switch(type){
      case 2: //from string
         *v = atoi(&strdata[0]);
         break;
      case 3: //from int
         *v = idata;
         break;
      case 4: //from float
         *v = (int)fdata;
         break;
      case 5: //from double
         *v = (int)ddata;
         break;
      default:
         *v = 0;
         break;
   }
}
void StorageStreamIndexedBinary::GetItem(float* v){
   switch(type){
      case 2: //from string
         *v = (float)atof(&strdata[0]);
         break;
      case 3: //from int
         *v = (float)idata;
         break;
      case 4: //from float
         *v = fdata;
         break;
      case 5: //from float
         *v = (float)ddata;
         break;
      default:
         *v = 0.0f;
         break;
   }
}
void StorageStreamIndexedBinary::GetItem(double* v){
   switch(type){
      case 2: //from string
         *v = atof(&strdata[0]);
         break;
      case 3: //from int
         *v = (double)idata;
         break;
      case 4: //from float
         *v = fdata;
         break;
      case 5: //from double
         *v = ddata;
         break;
      default:
         *v = 0.0;
         break;
   }
}
void StorageStreamIndexedBinary::GetItem(char const** v){
   switch(type){
      case 2: //from string
      case 6: //from binary
         *v = &strdata[0];
         break;
      case 3: //from int
         sprintf(&strdata[0], "%d", idata);
         *v = &strdata[0];
         break;
	  case 4: // from float
         sprintf(&strdata[0], "%e", fdata);
         *v = &strdata[0];
         break;
      case 5: // from double 
         sprintf(&strdata[0], "%e", ddata);
         *v = &strdata[0];
         break;
   }
}
void StorageStreamIndexedBinary::GetItem(void const** v, int* n){
   switch(type){
      case 2: //from string
      case 6: //from binary
         *v = &strdata[0];
         *n = biglen;
         break;
      default:
         *v = &zero_ch;
         *n = 0;
         break;
   }
}


void StorageStreamIndexedBinary::PutItem(int* v){
   char t = 3;
   fwrite(&t,1,1,f);
   fwrite(v,sizeof(int),1,f);
}
void StorageStreamIndexedBinary::PutItem(float* v){
   char t = 4;
   fwrite(&t,1,1,f);
   fwrite(v,sizeof(float),1,f);
}
void StorageStreamIndexedBinary::PutItem(double* v){
   char t = 5;
   fwrite(&t,1,1,f);
   fwrite(v,sizeof(double),1,f);
}
void StorageStreamIndexedBinary::PutItem(const char* v){
   char t = 2;
   unsigned short len = strlen(v);
   fwrite(&t,1,1,f);
   fwrite(&len,sizeof(len),1,f);
   fwrite(v,1,len,f);
}
void StorageStreamIndexedBinary::PutItem(void const* v, int n){
   char t = 6;
   unsigned int len = n;
   fwrite(&t,1,1,f);
   fwrite(&len,sizeof(len),1,f);
   fwrite(v,1,len,f);
}


//SimpleXML storage
////////////////////

StorageStreamSimpleXML::StorageStreamSimpleXML()
   :f(NULL)
   ,tabsz(1){
   buf.resize(4096, '\0');
   strbegin = strend = &buf[0];
   bufBeginOff = 0;
   bufEndOff = 0;
}
StorageStreamSimpleXML::~StorageStreamSimpleXML(){
   Close();
}

//stream initialization operation
int StorageStreamSimpleXML::OpenForWrite(const char * fname){
   Close();
   f=fopen(fname,"w+t");
   return !f;
}
int StorageStreamSimpleXML::OpenForRead(const char * fname){
   Close();
   f=fopen(fname,"rt");
   return !f;
}
void StorageStreamSimpleXML::Close(){
   if(f!=NULL){
      fclose(f); 
      f=NULL;
   }
}

//atomic storage operations
const char* StorageStreamSimpleXML::GetNodeName(){
   return strbegin;
}

bool StorageStreamSimpleXML::nextLine(char** begin, char** end)
{
   do {
      char* nl = (char*)memchr(&buf[0] + bufBeginOff, '\n', bufEndOff - bufBeginOff);
      if (nl) {
         *begin = &buf[0] + bufBeginOff;
         *end = nl;
		 if (nl != &buf[0] && nl[-1] == '\r') {
			--*end;
		 }
         **end = '\0';
         
         bufBeginOff = (nl - &buf[0]) + 1;
         if (bufBeginOff == bufEndOff) {
            bufBeginOff = bufEndOff = 0;
         }

         return true;
      }
   } while (readMore());
   
   // So, we haven't found a newline symbol.
   // We are still going to set *begin and *end, putting
   // a null character to **end.
   if (bufEndOff != buf.size()) {
      buf[bufEndOff] = '\0';
   } else {
      buf.push_back('\0');
   }
   *begin = &buf[0] + bufBeginOff;
   *end = &buf[0] + bufEndOff;

   if (bufBeginOff != bufEndOff) {
      // We have something in the buffer, but it's not a newline.
      // Let's pretend it does end with a newline.
      bufBeginOff = bufEndOff = 0;
      return true;
   }
   
   return false;
}

bool StorageStreamSimpleXML::readMore()
{
   if (buf.size() == bufEndOff) {
      if (bufEndOff - bufBeginOff < buf.size() / 2) {
         // Move the data to the beginning of the buffer.
         memmove(&buf[0], &buf[0] + bufBeginOff, bufEndOff - bufBeginOff);
         bufEndOff -= bufBeginOff;
         bufBeginOff = 0;
      } else {
         buf.resize(buf.size() * 2);
      }
   }

   int r = fread(&buf[0] + bufEndOff, 1, buf.size() - bufEndOff, f);
   if (r <= 0) {
      return false;
   }
   bufEndOff += r;
   return true;
}

int StorageStreamSimpleXML::NextItem(){
   if (!nextLine(&strbegin, &strend)) {
      return 1; // end node
   }
   
   for (; *strbegin == ' '; ++strbegin) {
      // skip initial whitespace
   }
   
   if (*strbegin != '<' || strend[-1] != '>') {
      return 2; // data node
   }

   ++strbegin; // skip the '<' character
   --strend; // trim the '>' character
   *strend = '\0';
   
   if (*strbegin == '/') {
      ++strbegin;
      return 1; // end node
   }
   
   if (strncmp(strbegin, "binary>", 7) != 0) {
      return 0; // start node
   }
   
   if ((strend - strbegin) < 8 || strcmp(strend - 8, "</binary") != 0) {
      return 0; // start node
   }
   strbegin += 7; // strlen("binary>");
   strend -= 8; // strlen("</binary");
   
   strend = decodeBase64InPlace(strbegin, strend);
   return 2; // data node
}

void StorageStreamSimpleXML::PutStartNode(const char *s){
   fprintf(f, "%s<%s>\n", indent.c_str(), s);
   indent.resize(indent.size() + tabsz, ' ');
}
void StorageStreamSimpleXML::PutEndNode(const char *s){
   indent.resize(indent.size() - tabsz);
   fprintf(f, "%s</%s>\n", indent.c_str(), s);
}
void StorageStreamSimpleXML::GetItem(int* v){
   *v = atoi(strbegin);
}
void StorageStreamSimpleXML::GetItem(float* v){
   *v = (float)atof(strbegin);
}
void StorageStreamSimpleXML::GetItem(double* v){
   *v = atof(strbegin);
}
void StorageStreamSimpleXML::GetItem(char const** v){
   *v = strbegin;
}
void StorageStreamSimpleXML::GetItem(void const** v, int* n){
   // Binary data.
   *v = strbegin;
   *n = strend - strbegin;
}

void StorageStreamSimpleXML::PutItem(int* v){
   fprintf(f, "%s%d\n", indent.c_str(), *v);   
}
void StorageStreamSimpleXML::PutItem(float* v){
   fprintf(f, "%s%e\n", indent.c_str(), *v);   
}
void StorageStreamSimpleXML::PutItem(double* v){
   fprintf(f, "%s%e\n", indent.c_str(), *v);   
}
void StorageStreamSimpleXML::PutItem(const char* v){
   fprintf(f, "%s%s\n", indent.c_str(), v);   
}
void StorageStreamSimpleXML::PutItem(void const* v, int n)
{
   fprintf(f, "%s<binary>%s</binary>\n", indent.c_str(),
      toBase64((char const*)v, (char const*)v + n).c_str());
}

char* StorageStreamSimpleXML::decodeBase64InPlace(char* begin, char* end)
{
   int c1, c2, c3, c4;
   char const* g = begin; // get ptr
   char* p = begin; // put ptr
   while(g != end) {
      do {
         c1 = m_base64DecodeChars[*g++ & 0xff];
      } while(g != end && c1 == -1);
      if (c1 == -1) {
         break;
      }

      do {
         c2 = m_base64DecodeChars[*g++ & 0xff];
      } while(g != end && c2 == -1);
      if (c2 == -1) {
         break;
      }

      *p++ = char((c1 << 2) | (c2 & 0x30) >> 4);
      
      do {
         if (*g == '=') {
            goto done;
         }
         c3 = m_base64DecodeChars[*g++ & 0xff];
      } while(g != end && c3 == -1);
      if (c3 == -1) {
         break;
      }

      *p++ = char(((c2 & 0x0F) << 4) | ((c3 & 0x3C) >> 2));

      do {
         if (*g == '=') {
            goto done;
         }
         c4 = m_base64DecodeChars[*g++ & 0xff];
      } while(g != end && c4 == -1);
      if (c4 == -1) {
         break;
      }

      *p++ = char(((c3 & 0x03) << 6) | c4);
   }

done:
   assert(p <= end);
   *p = '\0';
   return p;
}

std::string StorageStreamSimpleXML::toBase64(char const* begin, char const* end)
{
   std::string out;

   char const* g = begin;
   while (g != end) {
      int c1 = *g++ & 0xff;
      if (g == end) {
         out += m_base64EncodeChars[c1 >> 2];
         out += m_base64EncodeChars[(c1 & 0x03) << 4];
         out += '=';
         out += '=';
         break;
      }

      int c2 = *g++ & 0xff;
      if (g == end) {
         out += m_base64EncodeChars[c1 >> 2];
         out += m_base64EncodeChars[((c1 & 0x03) << 4) | ((c2 & 0xF0) >> 4)];
         out += m_base64EncodeChars[(c2 & 0x0F) << 2];
         out += '=';
         break;
      }

      int c3 = *g++ & 0xff;
      out += m_base64EncodeChars[c1 >> 2];
      out += m_base64EncodeChars[((c1 & 0x03) << 4) | ((c2 & 0xF0) >> 4)];
      out += m_base64EncodeChars[((c2 & 0x0F) << 2) | ((c3 & 0xC0) >> 6)];
      out += m_base64EncodeChars[c3 & 0x3F];
   }
   
   return out;
}

char const StorageStreamSimpleXML::m_base64EncodeChars[] = 
   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

signed char const StorageStreamSimpleXML::m_base64DecodeChars[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
    -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1
};









//THIS IS TEST CODE SECTION 
// (uncomment last line to set debug on)
////////////////////////////////////////



class DynClassBase{
public:
   static int count;
   static  DynClassBase *AskForObject(const char *cname);
   virtual const char *AskForClassName()=0;
   virtual void AskForData(Serializer *s)=0;
};

int DynClassBase::count = 0;

class DynClassABC: public DynClassBase{
public:
   int a,b,c;
   DynClassABC(){
      a=count++;
      b=count++;
      c=count++;
   }
   const char *AskForClassName(){
      return "DynClassABC";
   }
   void AskForData(Serializer *s){
      s->Item("a",Sync(&a));
      s->Item("b",Sync(&b));
      s->Item("c",Sync(&c));
   }
};

class DynClassABCD: public DynClassBase{
public:
   int a,b,c,d;
   DynClassABCD(){
      a=count++;
      b=count++;
      c=count++;
      d=count++;
   }
   const char *AskForClassName(){
      return "DynClassABCD";
   }
   void AskForData(Serializer *s){
      s->Item("a",Sync(&a));
      s->Item("b",Sync(&b));
      s->Item("c",Sync(&c));
      s->Item("d",Sync(&d));
   }
};

DynClassBase *DynClassBase::AskForObject(const char *cname){
   if(strcmp(cname,"DynClassABC")==0)
      return new DynClassABC;
   else if(strcmp(cname,"DynClassABCD")==0)
      return new DynClassABCD;
   // return 0 if object can not be constructed
   return 0;
}



class EmbedObject2{
public:
   int otherint;
   float otherfloat;

   

   void AskForData(Serializer *s){
      s->Item("otherint",Sync(&otherint));
      s->Item("otherfloat",Sync(&otherfloat));
   }
};

class EmbedObject{
public:
   int otherint;
   float otherfloat;
   int n;

   EmbedObject2 eobjA[1000];
   EmbedObject2 eobjB;
   EmbedObject2 eobjC;

   EmbedObject(){
      int i;
      for(i=0;i<1000;i++){
         eobjA[i].otherint=i;
         eobjA[i].otherfloat=i+0.10f;
      }    
   }

   void AskForData(Serializer *s){
      n=100;
      s->Item("otherint",Sync(&otherint));
      s->Item("otherfloat",Sync(&otherfloat));
      s->Item("Vector of EmbedObjectsA",Sync(eobjA,n));
      s->Item("EmbedObjectB",Sync(&eobjB));
      s->Item("EmbedObjectC",Sync(&eobjC));
   }

};








class MainObject{
public:
   // this is a vector of pointer to the base class
   // object class restored dynamically from stream
   std::vector< CSrlzPtr<DynClassBase> > dynobjlist;

   int testint;
   float testfloat;
   std::vector<EmbedObject> stdvect;
   char name[200];
   char chr;
   std::list<int> intlist;
   std::set<int> intset;
   std::set<double> doubleset;

   std::map<int,std::string> map_int_float;
  

   EmbedObject eobj;

   MainObject(){
      stdvect.resize(1000);
      strcpy(name,"this is a test string");
      intlist.push_back(1);
      intlist.push_back(3);
      intlist.push_back(7);
      intlist.push_back(12);

      intset.insert(10);
      intset.insert(7);
      intset.insert(2);
      intset.insert(8);
      intset.insert(4);
      intset.insert(3);

      doubleset.insert(10e20);
      doubleset.insert(7e-20);
      doubleset.insert(2e2);
      doubleset.insert(8e0);
      doubleset.insert(4);
      doubleset.insert(3);

      dynobjlist.push_back(CSrlzPtr<DynClassBase>(new DynClassABC));
      dynobjlist.push_back(CSrlzPtr<DynClassBase>(new DynClassABC));
      dynobjlist.push_back(CSrlzPtr<DynClassBase>(new DynClassABC));
      dynobjlist.push_back(CSrlzPtr<DynClassBase>(new DynClassABCD));
      dynobjlist.push_back(CSrlzPtr<DynClassBase>(new DynClassABCD));

      
      map_int_float[1]="one";
      map_int_float[2]="two";
      map_int_float[3]="tree";
      map_int_float[4]="four";

   }
   void AskForData(Serializer *s){
      s->Item("dynobjlist", Sync(&dynobjlist));
      s->Item("map_int_float",Sync(&map_int_float));
      s->Item("intlist",Sync(&intlist));
      s->Item("intset",Sync(&intset));
      s->Item("doubleset",Sync(&doubleset));
      s->Item("TestInt",Sync(&testint));
      s->Item("TestFloat",Sync(&testfloat));
      s->Item("EmbedObject",Sync(&eobj));
      s->Item("stdvector",Sync(&stdvect));
      s->Item("TestChar",Sync(&chr));
      s->Item("TestName",Sync(name,200));
   }

};




class TestAll{
public:
   TestAll(){

      MainObject mobj;
      mobj.testfloat=123.456F;
      mobj.testint=321;
      mobj.eobj.otherfloat=1.005F;
      mobj.eobj.otherint=22; 

      
      //StorageStreamSimpleXML2 ss;
      StorageStreamSimpleBinary ss;
      //StorageStreamIndexedBinary ss;

      Serializer srlz(&ss);
      srlz.Item("MainObject", Sync(&mobj));

      if(!ss.OpenForWrite("test_data.txt")){
         srlz.Store();
         ss.Close();
         printf("writting passed\n");
      }

      if(!ss.OpenForRead("test_data.txt")){
         srlz.Load();
         ss.Close();
         printf("reading passed\n");
      }

      if(!ss.OpenForWrite("test_data1.txt")){
         srlz.Store();
         ss.Close();
         printf("re-writting passed\n");
      }

      ss.Close();
      printf("test finished\n");

      //wait for key
      char buf[200];
      fgets(buf,200,stdin);
   }
};
//static TestAll testall;
