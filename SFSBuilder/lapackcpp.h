#ifndef lapackcpp_h
#define lapackcpp_h

#include <stdio.h>

class Lapackcpp{
public:
  int EigenValue(int size, float *matrix, float *value, char jobz='V', char uplo='U');
  int LeastSquare(int m, int n, double *a, double *b, int nrhs = 1);
  int SVD( int m, float *a, float *s, float *u, float *vt);
  int Invert( int m, float *a);
  int Invert( int m, double *a);
};




#define mml_matrix(m,n,t) (t*)alloca(m*n*sizeof(t))

#define define_MtxCalc(name,type,stack)   type name##_stkm[stack]; mml_Stack<type> name##_stk(name##_stkm); mml_MatCalc<type> name(&name##_stk)

class mml_midx{  
public:
  int m;
  mml_midx(int ld = 0):m(ld){} 
  int operator()(int i, int j){
    return i+j*m;
  }  
};


template <class T> class mml_Stack{  
 public:
  T *h;
  T *d;  
  mml_Stack(T *p):h(p),d(p){}
  void reset(){d=h;}
};

template <class T> class mml_MatCalc{
public:

  T *a;
  int m,n;
  mml_Stack<T> *st;

  mml_MatCalc(mml_Stack<T> *stk):n(0),m(0),st(stk),a(stk->h){};
  
  inline mml_MatCalc& start(){
    st->reset();
    return *this;
  }
  
  inline mml_MatCalc finish(float *b){
    std::copy(a,a+m*n,b);
    return *this;
  }
  inline mml_MatCalc finish(double *b){
    std::copy(a,a+m*n,b);
    return *this;
  }


  inline mml_MatCalc operator()(T *b, int im, int in){
    
    mml_MatCalc mc(*this);
    mc.a=b;
    mc.m=im; mc.n=in;
    return mc;
  }

  // print matrix for debugging
  inline mml_MatCalc& print(){
    mml_midx ida(m);
    int i,j;
    for(i=0;i<m;i++){
      for(j=0;j<n;j++){
	printf("% le ",(double)(a[ida(i,j)]));
      }
      printf("\n");
    }
    printf("\n");
    return *this;
  }


  // diagonal matrices  
  inline mml_MatCalc diag(int im, int in, T v=1){
    mml_MatCalc mc(*this);
    mc.a=st->d; st->d+=im*in;
    mml_midx ida(im); mc.m=im; mc.n=in;
    int i,j;
    for(i=0;i<mc.m;i++){
      for(j=0;j<mc.n;j++){
	if(i==j) 
	  mc.a[ida(i,j)] = v;
	else
	  mc.a[ida(i,j)] = 0;
      }
    } 
    return mc;
  }
  
  inline mml_MatCalc diag(int im, int in, T *v){    
    mml_MatCalc mc(*this);
    mc.a=st->d; st->d+=im*in;
    mml_midx ida(im); mc.m=im; mc.n=in;    
    int i,j;
    for(i=0;i<mc.m;i++){
      for(j=0;j<mc.n;j++){
	if(i==j) 
	  mc.a[ida(i,j)] = v[i];
	else
	  mc.a[ida(i,j)] = 0;
      }
    } 
    
    return mc;
  }

  // multiplication matrix*matrix
  inline mml_MatCalc operator*(mml_MatCalc mc){
    mml_MatCalc out(*this);
    out.a=st->d; st->d+=m*mc.n;
    out.m=m; out.n=mc.n;
    mml_midx ida(m), idb(mc.m);
    int i,j,k;
    for(i=0;i<m;i++){
      for(j=0;j<mc.n;j++){
	T &v = out.a[ida(i,j)] = 0;
	for(k=0;k<n;k++) v += a[ida(i,k)] * mc.a[idb(k,j)];
      }
    } 
    return out;
  }


  inline mml_MatCalc operator*(T v){
    mml_MatCalc out(*this);
    out.a=st->d; st->d+=m*n;
    out.m=m; out.n=n;
    mml_midx ida(m);
    int i,j,k;
    for(i=0;i<m;i++){
      for(j=0;j<n;j++){
	out.a[ida(i,j)] =
	  a[ida(i,j)]*v;
      }
    } 
    return out;
  }

  inline mml_MatCalc operator-(mml_MatCalc mc){
    mml_MatCalc out(*this);
    out.a=st->d; st->d+=m*n;
    out.m=m; out.n=n;
    mml_midx ida(m);
    int i,j,k;
    for(i=0;i<m;i++){
      for(j=0;j<n;j++){
	out.a[ida(i,j)] = 
	  a[ida(i,j)] - mc.a[ida(i,j)];
      }
    } 
    return out;
  }

  inline mml_MatCalc operator+(mml_MatCalc mc){
    mml_MatCalc out(*this);
    out.a=st->d; st->d+=m*n;
    out.m=m; out.n=n;
    mml_midx ida(m);
    int i,j,k;
    for(i=0;i<m;i++){
      for(j=0;j<n;j++){
	out.a[ida(i,j)] = 
	  a[ida(i,j)] + mc.a[ida(i,j)];
      }
    } 
    return out;
  }

  inline mml_MatCalc tran(){
    mml_MatCalc mc(*this);
    mc.a=st->d; st->d+=m*n;
    mml_midx ida(n), idb(m);
    int i,j;
    for(i=0;i<m;i++)
      for(j=0;j<n;j++)
	mc.a[ida(j,i)] = a[idb(i,j)];
    mc.m=n; mc.n=m; 
    return mc;
  }

  inline mml_MatCalc inv(){
    mml_MatCalc mc(*this);
    mc.a=st->d; st->d+=m*n;
    mml_midx ida(n), idb(m);

    std::copy(a,a+m*n,mc.a);
    Lapackcpp().Invert(m, mc.a);

    return mc;
  }
};





/*
template <class T> class mml_MatCalc{
public:

  T *a;
  int m,n;
  T *depth;
  T **pdepth;

  mml_MatCalc(T *stack):n(0),m(0),a(stack),depth(stack),pdepth(&depth){};
  
  inline mml_MatCalc& start(){
    depth = a;
    return *this;
  }
  
  inline mml_MatCalc finish(float *b){
    std::copy(a,a+m*n,b);
    return *this;
  }
  inline mml_MatCalc finish(double *b){
    std::copy(a,a+m*n,b);
    return *this;
  }


  inline mml_MatCalc operator()(T *b, int im, int in){
    
    mml_MatCalc mc(*this);
    mc.a=b;
    mc.m=im; mc.n=in;
    return mc;
  }

  // print matrix for debugging
  inline mml_MatCalc& print(){
    mml_midx ida(m);
    int i,j;
    for(i=0;i<m;i++){
      for(j=0;j<n;j++){
	printf("% le ",(double)(a[ida(i,j)]));
      }
      printf("\n");
    }
    printf("\n");
    return *this;
  }


  // diagonal matrices  
  inline mml_MatCalc diag(int im, int in, T v=1){
    mml_MatCalc mc(*this);
    mc.a=*pdepth; (*pdepth)+=im*in;
    mml_midx ida(im); mc.m=im; mc.n=in;
    int i,j;
    for(i=0;i<mc.m;i++){
      for(j=0;j<mc.n;j++){
	if(i==j) 
	  mc.a[ida(i,j)] = v;
	else
	  mc.a[ida(i,j)] = 0;
      }
    } 
    return mc;
  }
  
  inline mml_MatCalc diag(int im, int in, T *v){    
    mml_MatCalc mc(*this);
    mc.a=*pdepth; *pdepth+=im*in;
    mml_midx ida(im); mc.m=im; mc.n=in;    
    int i,j;
    for(i=0;i<mc.m;i++){
      for(j=0;j<mc.n;j++){
	if(i==j) 
	  mc.a[ida(i,j)] = v[i];
	else
	  mc.a[ida(i,j)] = 0;
      }
    } 
    
    return mc;
  }

  // multiplication matrix*matrix
  inline mml_MatCalc operator*(mml_MatCalc mc){
    mml_MatCalc out(*this);
    out.a=*pdepth; (*pdepth)+=m*mc.n;
    out.m=m; out.n=mc.n;
    mml_midx ida(m), idb(mc.m);
    int i,j,k;
    for(i=0;i<m;i++){
      for(j=0;j<mc.n;j++){
	T &v = out.a[ida(i,j)] = 0;
	for(k=0;k<n;k++) v += a[ida(i,k)] * mc.a[idb(k,j)];
      }
    } 
    return out;
  }


  inline mml_MatCalc operator*(T v){
    mml_MatCalc out(*this);
    out.a=*pdepth; (*pdepth)+=m*n;
    out.m=m; out.n=n;
    mml_midx ida(m);
    int i,j,k;
    for(i=0;i<m;i++){
      for(j=0;j<n;j++){
	out.a[ida(i,j)] =
	  a[ida(i,j)]*v;
      }
    } 
    return out;
  }

  inline mml_MatCalc operator-(mml_MatCalc mc){
    mml_MatCalc out(*this);
    out.a=*pdepth; (*pdepth)+=m*n;
    out.m=m; out.n=n;
    mml_midx ida(m);
    int i,j,k;
    for(i=0;i<m;i++){
      for(j=0;j<n;j++){
	out.a[ida(i,j)] = 
	  a[ida(i,j)] - mc.a[ida(i,j)];
      }
    } 
    return out;
  }

  inline mml_MatCalc operator+(mml_MatCalc mc){
    mml_MatCalc out(*this);
    out.a=*pdepth; (*pdepth)+=m*n;
    out.m=m; out.n=n;
    mml_midx ida(m);
    int i,j,k;
    for(i=0;i<m;i++){
      for(j=0;j<n;j++){
	out.a[ida(i,j)] = 
	  a[ida(i,j)] + mc.a[ida(i,j)];
      }
    } 
    return out;
  }

  inline mml_MatCalc tran(){
    mml_MatCalc mc(*this);
    mc.a=*pdepth; (*pdepth)+=m*n;
    mml_midx ida(n), idb(m);
    int i,j;
    for(i=0;i<m;i++)
      for(j=0;j<n;j++)
	mc.a[ida(j,i)] = a[idb(i,j)];
    mc.m=n; mc.n=m; 
    return mc;
  }

  inline mml_MatCalc inv(){
    mml_MatCalc mc(*this);
    mc.a=*pdepth; (*pdepth)+=m*n;
    mml_midx ida(n), idb(m);

    std::copy(a,a+m*n,mc.a);
    Lapackcpp().Invert(m, mc.a);

    return mc;
  }
};
*/

#endif
